#include "GhostscriptCore.h"
#include "PDF.h"
#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/FileManager.h"
#include "AssetRegistryModule.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "IPluginManager.h"

#include "AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "HideWindowsPlatformTypes.h"

const FString FGhostscriptCore::PagesDirectoryPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("PDFImporter"))->GetBaseDir(), TEXT("Content")));

FGhostscriptCore::FGhostscriptCore()
{
	// dllファイルのパスを取得
	FString GhostscriptDllPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("PDFImporter"), TEXT("ThirdParty")));
#ifdef _WIN64
	GhostscriptDllPath = FPaths::Combine(GhostscriptDllPath, TEXT("Win64"));
#elif _WIN32
	GhostscriptDllPath = FPaths::Combine(GhostscriptDllPath, TEXT("Win32"));
#endif
	GhostscriptDllPath = FPaths::Combine(GhostscriptDllPath, TEXT("gsdll.dll"));

	// モジュールをロード
	GhostscriptModule = FPlatformProcess::GetDllHandle(*GhostscriptDllPath);
	if (GhostscriptModule == nullptr)
	{
		UE_LOG(PDFImporter, Fatal, TEXT("Failed to load Ghostscript module"));
	}

	// 関数ポインタを取得
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

FGhostscriptCore::~FGhostscriptCore()
{
	FPlatformProcess::FreeDllHandle(GhostscriptModule);
	UE_LOG(PDFImporter, Log, TEXT("Ghostscript dll unloaded"));
}

UPDF* FGhostscriptCore::ConvertPdfToPdfAsset(const FString& InputPath, int Dpi, int FirstPage, int LastPage, bool bIsImportIntoEditor)
{
	IFileManager& FileManager = IFileManager::Get();
	
	// PDFがあるか確認
	if (!FileManager.FileExists(*InputPath))
	{
		UE_LOG(PDFImporter, Error, TEXT("File not found : %s"), *InputPath);
		return nullptr;
	}

	// 作業用のディレクトリを作成
	FString TempDirPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ConvertTemp"));
	TempDirPath = FPaths::ConvertRelativePathToFull(TempDirPath);
	if (FileManager.DirectoryExists(*TempDirPath))
	{
		FileManager.DeleteDirectory(*TempDirPath);
	}
	FileManager.MakeDirectory(*TempDirPath);
	UE_LOG(PDFImporter, Log, TEXT("A working directory has been created (%s)"), *TempDirPath);

	// Ghostscriptを用いてPDFからjpg画像を作成
	FString OutputPath = FPaths::Combine(TempDirPath, FPaths::GetBaseFilename(InputPath) + TEXT("%010d.jpg"));
	TArray<UTexture2D*> Buffer; 
	UPDF* PDFAsset = nullptr;

	if (ConvertPdfToJpeg(InputPath, OutputPath, Dpi, FirstPage, LastPage))
	{
		// 画像のファイルパスを取得
		TArray<FString> PageNames;
		IFileManager::Get().FindFiles(PageNames, *TempDirPath, L"jpg");
		
		// 作成したjpg画像を読み込む
		UTexture2D* TextureTemp;
		for (const FString& PageName : PageNames)
		{
			bool bResult = false;
			if (bIsImportIntoEditor)
			{
#if WITH_EDITORONLY_DATA
				bResult = CreateTextureAssetFromFile(FPaths::Combine(TempDirPath, PageName), TextureTemp);
#endif
			}
			else
			{
				bResult = LoadTexture2DFromFile(FPaths::Combine(TempDirPath, PageName), TextureTemp);
			}
			
			if (bResult)
			{
				Buffer.Add(TextureTemp);
			}
		}

		// PDFアセットを作成
		PDFAsset = NewObject<UPDF>();

		if (FirstPage <= 0 || LastPage <= 0 || FirstPage > LastPage)
		{
			FirstPage = 1;
			LastPage = Buffer.Num();
		}

		PDFAsset->PageRange = FPageRange(FirstPage, LastPage);
		PDFAsset->Dpi = Dpi;
		PDFAsset->Pages = Buffer;
	}

	// 作業ディレクトリを削除
	if (FileManager.DirectoryExists(*TempDirPath))
	{
		FileManager.DeleteDirectory(*TempDirPath, true, true);
		UE_LOG(PDFImporter, Log, TEXT("Successfully deleted working directory (%s)"), *TempDirPath);
	}

	return PDFAsset;
}

bool FGhostscriptCore::ConvertPdfToJpeg(const FString& InputPath, const FString& OutputPath, int Dpi, int FirstPage, int LastPage)
{
	if (!(FirstPage > 0 && LastPage > 0 && FirstPage <= LastPage))
	{
		FirstPage = 1;
		LastPage = INT_MAX;
	}

	TArray<char> FirstPageBuffer = FStringToCharPtr(FString(TEXT("-dFirstPage=") + FString::FromInt(FirstPage)));
	TArray<char> LastPageBuffer = FStringToCharPtr(FString(TEXT("-dLastPage=") + FString::FromInt(LastPage)));
	TArray<char> DpiXBuffer = FStringToCharPtr(FString(TEXT("-dDEVICEXRESOLUTION=") + FString::FromInt(Dpi)));
	TArray<char> DpiYBuffer = FStringToCharPtr(FString(TEXT("-dDEVICEYRESOLUTION=") + FString::FromInt(Dpi)));
	TArray<char> OutputPathBuffer = FStringToCharPtr(FString(TEXT("-sOutputFile=") + OutputPath));
	TArray<char> InputPathBuffer = FStringToCharPtr(InputPath);

	const char* Args[20] =
	{
		// Ghostscriptが標準出力に情報を出力しないように
		"-q",
		"-dQUIET",

		"-dPARANOIDSAFER",			// セーフモードで実行
		"-dBATCH",					// Ghostscriptがインタラクティブモードにならないように
		"-dNOPAUSE",				// ページごとの一時停止をしないように
		"-dNOPROMPT",				// コマンドプロンプトがでないように           
		"-dMaxBitmap=500000000",	// パフォーマンスを向上させる
		"-dNumRenderingThreads=4",	// マルチコアで実行

		// 出力画像のアンチエイリアスや解像度など
		"-dAlignToPixels=0",
		"-dGridFitTT=0",
		"-dTextAlphaBits=4",
		"-dGraphicsAlphaBits=4",

		"-sDEVICE=jpeg",	// jpeg形式で出力
		"-sPAPERSIZE=a7",	// 紙のサイズ

		FirstPageBuffer.GetData(),	// 14 : 始めのページを指定
		LastPageBuffer.GetData(),	// 15 : 終わりのページを指定
		DpiXBuffer.GetData(),		// 16 : 横のDPI
		DpiYBuffer.GetData(),		// 17 : 縦のDPI
		OutputPathBuffer.GetData(), // 18 : 出力パス
		InputPathBuffer.GetData()	// 19 : 入力パス
	};

	// Ghostscriptのインスタンスを作成
	void* GhostscriptInstance = nullptr;
	CreateInstance(&GhostscriptInstance, 0);
	if (GhostscriptInstance != nullptr)
	{
		// Ghostscriptを実行
		int Result = Init(GhostscriptInstance, 20, (char**)Args);

		// Ghostscriptを終了
		Exit(GhostscriptInstance);
		DeleteInstance(GhostscriptInstance);

		UE_LOG(PDFImporter, Log, TEXT("Ghostscript Return Code : %d"), Result);

		return Result == 0;
	}
	else
	{
		UE_LOG(PDFImporter, Error, TEXT("Failed to create Ghostscript instance"));
		return false;
	}
}

bool FGhostscriptCore::LoadTexture2DFromFile(const FString& FilePath, class UTexture2D*& LoadedTexture)
{
	// 画像データを読み込む
	TArray<uint8> RawFileData;
	if (FFileHelper::LoadFileToArray(RawFileData, *FilePath) &&
		ImageWrapper.IsValid() && 
		ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num())
		)
	{
		// 非圧縮の画像データを取得
		const TArray<uint8>* UncompressedRawData = nullptr;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedRawData))
		{
			// Texture2Dを作成
			UTexture2D* NewTexture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
			if (!NewTexture)
			{
				return false;
			}

			// ピクセルデータをテクスチャに書き込む
			void* TextureData = NewTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedRawData->GetData(), UncompressedRawData->Num());
			NewTexture->PlatformData->Mips[0].BulkData.Unlock();
			NewTexture->UpdateResource();

			LoadedTexture = NewTexture;

			return true;
		}
	}
	
	return false;
}

#if WITH_EDITORONLY_DATA
bool FGhostscriptCore::CreateTextureAssetFromFile(const FString& FilePath, class UTexture2D*& LoadedTexture)
{
	// 画像データを読み込む
	TArray<uint8> RawFileData;
	if (FFileHelper::LoadFileToArray(RawFileData, *FilePath))
	{
		// 非圧縮の画像データを取得
		const TArray<uint8>* UncompressedRawData = nullptr;
		if (ImageWrapper.IsValid() &&
			ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()) &&
			ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedRawData)
			)
		{
			FString Filename = FPaths::GetBaseFilename(FilePath);
			Filename = Filename.Left(Filename.Len() - 10);
			int Width = ImageWrapper->GetWidth();
			int Height = ImageWrapper->GetHeight();

			// パッケージを作成
			FString PackagePath(TEXT("/PDFImporter/") + Filename + TEXT("/"));
			FString AbsolutePackagePath = PagesDirectoryPath + TEXT("/") + Filename + TEXT("/");

			FPackageName::RegisterMountPoint(PackagePath, AbsolutePackagePath);

			PackagePath += Filename;

			UPackage* Package = CreatePackage(nullptr, *PackagePath);
			Package->FullyLoad();

			// テクスチャを作成
			FName TextureName = MakeUniqueObjectName(Package, UTexture2D::StaticClass(), FName(*Filename));
			UTexture2D* NewTexture = NewObject<UTexture2D>(Package, TextureName, RF_Public | RF_Standalone);

			// テクスチャの設定
			NewTexture->PlatformData = new FTexturePlatformData();
			NewTexture->PlatformData->SizeX = Width;
			NewTexture->PlatformData->SizeY = Height;
			NewTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
			NewTexture->NeverStream = false;

			// ピクセルデータをテクスチャに書き込む
			FTexture2DMipMap* Mip = new FTexture2DMipMap();
			NewTexture->PlatformData->Mips.Add(Mip);
			Mip->SizeX = Width;
			Mip->SizeY = Height;
			Mip->BulkData.Lock(LOCK_READ_WRITE);
			uint8* TextureData = (uint8*)Mip->BulkData.Realloc(UncompressedRawData->Num());
			FMemory::Memcpy(TextureData, UncompressedRawData->GetData(), UncompressedRawData->Num());
			Mip->BulkData.Unlock();

			// テクスチャを更新
			NewTexture->AddToRoot();
			NewTexture->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_BGRA8, UncompressedRawData->GetData());
			NewTexture->UpdateResource();

			// パッケージを保存
			Package->MarkPackageDirty();
			FAssetRegistryModule::AssetCreated(NewTexture);
			LoadedTexture = NewTexture;

			FString PackageFilename = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
			return UPackage::SavePackage(Package, NewTexture, RF_Public | RF_Standalone, *PackageFilename, GError, nullptr, true, true, SAVE_NoError);
		}
	}

	return false;
}
#endif

TArray<char> FGhostscriptCore::FStringToCharPtr(const FString& Text)
{
	int Size = GetFStringSize(Text) + 1;
	TArray<char> Buffer("", Size);
	WideCharToMultiByte(CP_ACP, 0, *Text, Size, Buffer.GetData(), Buffer.Num(), NULL, NULL);
	return Buffer;
}

int FGhostscriptCore::GetFStringSize(const FString& InString)
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
