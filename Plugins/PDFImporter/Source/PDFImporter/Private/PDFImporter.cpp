// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PDFImporter.h"
#include "PDFAsset/AssetTypeActions_PDF.h"
#include "PDFAsset/PDF.h"
#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/FileManager.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"

#include <string>

#define LOCTEXT_NAMESPACE "FPDFImporterModule"

void FPDFImporterModule::StartupModule()
{
	//PDFアセットのAssetTypeActionsをAssetToolsモジュールに登録
	PDF_AssetTypeActions = MakeShareable(new FAssetTypeActions_PDF);
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(PDF_AssetTypeActions.ToSharedRef());

	//dllファイルのパスを取得
	FString GhostscriptDllPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("ThirdParty"), TEXT("Ghostscript")));
#ifdef _WIN64
	GhostscriptDllPath = FPaths::Combine(GhostscriptDllPath, TEXT("Win64"));
#elif _WIN32
	GhostscriptDllPath = FPaths::Combine(GhostscriptDllPath, TEXT("Win32"));
#endif
	GhostscriptDllPath = FPaths::Combine(GhostscriptDllPath, TEXT("gsdll.dll"));

	//モジュールをロード
	GhostscriptModule = FPlatformProcess::GetDllHandle(*GhostscriptDllPath);
	if (GhostscriptModule == nullptr)
	{
		UE_LOG(PDFImporter, Fatal, TEXT("Failed to load Ghostscript module"));
	}

	//関数ポインタを取得
	CreateInstance = (CreateAPIInstance)FPlatformProcess::GetDllExport(GhostscriptModule, TEXT("gsapi_new_instance"));
	DeleteInstance = (DeleteAPIInstance)FPlatformProcess::GetDllExport(GhostscriptModule, TEXT("gsapi_delete_instance"));
	Init = (InitAPI)FPlatformProcess::GetDllExport(GhostscriptModule, TEXT("gsapi_init_with_args"));
	Exit = (ExitAPI)FPlatformProcess::GetDllExport(GhostscriptModule, TEXT("gsapi_exit"));
	if (CreateInstance == nullptr || DeleteInstance == nullptr || Init == nullptr || Exit == nullptr)
	{
		UE_LOG(PDFImporter, Fatal, TEXT("Failed to get Ghostscript function pointer"));
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	UE_LOG(PDFImporter, Log, TEXT("Ghostscrip dll loaded"));
}

void FPDFImporterModule::ShutdownModule()
{
	if (PDF_AssetTypeActions.IsValid())
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(PDF_AssetTypeActions.ToSharedRef());
		}
		PDF_AssetTypeActions.Reset();
	}

	FPlatformProcess::FreeDllHandle(GhostscriptModule);
	UE_LOG(PDFImporter, Log, TEXT("Ghostscript dll unloaded"));
}

UPDF* FPDFImporterModule::ConvertPdfToPdfAsset(const FString& InputPath, int Dpi, int FirstPage, int LastPage, const FString& Locale)
{
	IFileManager& FileManager = IFileManager::Get();

	//PDFがあるか確認
	if (!FileManager.FileExists(*InputPath))
	{
		UE_LOG(PDFImporter, Error, TEXT("File not found : %s"), *InputPath);
		return nullptr;
	}

	//作業用のディレクトリを作成
	FString TempDirPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ConvertTemp"));
	TempDirPath = FPaths::ConvertRelativePathToFull(TempDirPath);
	if (FileManager.DirectoryExists(*TempDirPath))
	{
		FileManager.DeleteDirectory(*TempDirPath);
	}
	FileManager.MakeDirectory(*TempDirPath);
	UE_LOG(PDFImporter, Log, TEXT("A working directory has been created (%s)"), *TempDirPath);

	//Ghostscriptを用いてPDFからjpg画像を作成
	FString OutputPath = FPaths::Combine(TempDirPath, TEXT("%10d.jpg"));
	bool bResult = ConvertPdfToJpeg(InputPath, OutputPath, Dpi, FirstPage, LastPage, Locale);

	//作成したjpg画像を読み込む
	TArray<UTexture2D*> Buffer;
	if (bResult)
	{
		//画像のファイルパスを取得
		TArray<FString> PageNames;
		FileManager.FindFiles(PageNames, *TempDirPath, L"jpg");

		//画像の読み込み
		UTexture2D* TextureTemp;
		for (FString PageName : PageNames)
		{
			if (LoadTexture2DFromFile(FPaths::Combine(TempDirPath, PageName), TextureTemp))
			{
				Buffer.Add(TextureTemp);
			}
			else
			{
				break;
			}
		}
	}

	//作業用ディレクトリを削除
	if (FileManager.DirectoryExists(*TempDirPath))
	{
		if (FileManager.DeleteDirectory(*TempDirPath, false, true))
		{
			UE_LOG(PDFImporter, Log, TEXT("The directory used for work was successfully deleted (%s)"), *TempDirPath);
		}
		else
		{
			UE_LOG(PDFImporter, Error, TEXT("The directory used for work could not be deleted (%s)"), *TempDirPath);
		}
	}

	if (bResult)
	{
		//PDFアセットを作成
		UPDF* PDFAsset = NewObject<UPDF>();
		PDFAsset->FileName = FPaths::GetBaseFilename(InputPath);
		PDFAsset->Pages = Buffer;
		PDFAsset->Dpi = Dpi;

		return PDFAsset;
	}
	else
	{
		return nullptr;
	}
}

bool FPDFImporterModule::ConvertPdfToJpeg(const FString& InputPath, const FString& OutputPath, int Dpi, int FirstPage, int LastPage, const FString& Locale)
{
	if (!(FirstPage > 0 && LastPage > 0 && FirstPage <= LastPage))
	{
		FirstPage = 1;
		LastPage = INT_MAX;
	}

	const char* Args[20] =
	{
		//Ghostscriptが標準出力に情報を出力しないように
		"-q",
		"-dQUIET",

		"-dPARANOIDSAFER",			//セーフモードで実行
		"-dBATCH",					//Ghostscriptがインタラクティブモードにならないように
		"-dNOPAUSE",				//ページごとの一時停止をしないように
		"-dNOPROMPT",				//コマンドプロンプトがでないように           
		"-dMaxBitmap=500000000",	//パフォーマンスを向上させる
		"-dNumRenderingThreads=4",	//マルチコアで実行

		//出力画像のアンチエイリアスや解像度など
		"-dAlignToPixels=0",
		"-dGridFitTT=0",
		"-dTextAlphaBits=4",
		"-dGraphicsAlphaBits=4",

		"-sDEVICE=jpeg",	//jpeg形式で出力
		"-sPAPERSIZE=a7",	//紙のサイズ

		"",	// 14 : 始めのページを指定
		"",	// 15 : 終わりのページを指定
		"",	// 16 : 横のDPI
		"",	// 17 : 縦のDPI
		"", // 18 : 出力パス
		""  // 19 : 入力パス
	};

	if (Locale != TEXT(""))
	{
		std::locale::global(std::locale(TCHAR_TO_ANSI(*Locale)));
		UE_LOG(PDFImporter, Log, TEXT("The locale has been set : %s"), *Locale);
	}

	FString FirstPageTemp(TEXT("-dFirstPage=") + FString::FromInt(FirstPage));
	int FirstPageSize = GetFStringSize(FirstPageTemp) + 1;
	TArray<char> FirstPageBuffer("", FirstPageSize);
	sprintf_s(FirstPageBuffer.GetData(), FirstPageSize, "%S", *FirstPageTemp);
	Args[14] = FirstPageBuffer.GetData();

	FString LastPageTemp(TEXT("-dLastPage=") + FString::FromInt(LastPage));
	int LastPageSize = GetFStringSize(LastPageTemp) + 1;
	TArray<char> LastPageBuffer("", LastPageSize);
	sprintf_s(LastPageBuffer.GetData(), LastPageSize, "%S", *LastPageTemp);
	Args[15] = LastPageBuffer.GetData();

	FString DpiXTemp(TEXT("-dDEVICEXRESOLUTION=") + FString::FromInt(Dpi));
	int DpiXSize = GetFStringSize(DpiXTemp) + 1;
	TArray<char> DpiXBuffer("", DpiXSize);
	sprintf_s(DpiXBuffer.GetData(), DpiXSize, "%S", *DpiXTemp);
	Args[16] = DpiXBuffer.GetData();

	FString DpiYTemp(TEXT("-dDEVICEYRESOLUTION=") + FString::FromInt(Dpi));
	int DpiYSize = GetFStringSize(DpiYTemp) + 1;
	TArray<char> DpiYBuffer("", DpiYSize);
	sprintf_s(DpiYBuffer.GetData(), DpiYSize, "%S", *DpiYTemp);
	Args[17] = DpiYBuffer.GetData();

	FString OutputPathTemp(TEXT("-sOutputFile=") + OutputPath);
	int OutputPathSize = GetFStringSize(OutputPathTemp) + 1;
	TArray<char> OutputPathBuffer("", OutputPathSize);
	sprintf_s(OutputPathBuffer.GetData(), OutputPathSize, "%S", *OutputPathTemp);
	Args[18] = OutputPathBuffer.GetData();

	int InputPathSize = GetFStringSize(InputPath) + 1;
	TArray<char> InputPathBuffer("", InputPathSize);
	sprintf_s(InputPathBuffer.GetData(), InputPathSize, "%S", *InputPath);
	Args[19] = InputPathBuffer.GetData();

	//Ghostscriptのインスタンスを作成
	void* GhostscriptInstance = nullptr;
	CreateInstance(&GhostscriptInstance, 0);
	if (GhostscriptInstance != nullptr)
	{
		//Ghostscriptを実行
		int Result = Init(GhostscriptInstance, 20, (char**)Args);

		//Ghostscriptを終了
		Exit(GhostscriptInstance);
		DeleteInstance(GhostscriptInstance);

		UE_LOG(PDFImporter, Log, TEXT("Ghostscript Return Code : %d"), Result);

		return Result == 0;
	}

	UE_LOG(PDFImporter, Error, TEXT("Failed to create Ghostscript instance"));
	return false;
}

bool FPDFImporterModule::LoadTexture2DFromFile(const FString& FilePath, UTexture2D* &LoadedTexture)
{
	//配列に画像を読み込む
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *FilePath)) return false;

	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		const TArray<uint8>* UncompressedRawData = NULL;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedRawData))
		{
			//Texture2Dを作成
			LoadedTexture = nullptr;
			LoadedTexture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
			if (!LoadedTexture) return false;

			void* TextureData = LoadedTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedRawData->GetData(), UncompressedRawData->Num());
			LoadedTexture->PlatformData->Mips[0].BulkData.Unlock();
			LoadedTexture->UpdateResource();
		}
	}

	return true;
}

int FPDFImporterModule::GetFStringSize(const FString& InString)
{
	int Size = 0;

	for (TCHAR Char : InString)
	{
		const char* Temp = TCHAR_TO_UTF8(*FString::Chr(Char));
		uint8 Code = static_cast<uint8>(*Temp);

		if ((Code >= 0x00) && (Code <= 0x7f))
		{
			Size += 1;
		}
		else if ((Code >= 0xc2) && (Code <= 0xdf))
		{
			Size += 2;
		}
		else if ((Code >= 0xe0) && (Code <= 0xef))
		{
			Size += 3;
		}
		else if ((Code >= 0xf0) && (Code <= 0xf7))
		{
			Size += 4;
		}
		else if ((Code >= 0xf8) && (Code <= 0xfb))
		{
			Size += 5;
		}
		else if ((Code >= 0xfc) && (Code <= 0xfd))
		{
			Size += 6;
		}
	}

	return Size;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPDFImporterModule, PDFImporter)