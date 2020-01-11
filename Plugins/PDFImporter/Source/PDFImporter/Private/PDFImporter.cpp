// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PDFImporter.h"
#include "Misc/Paths.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Kismet/KismetInternationalizationLibrary.h"

#include <string>

#define LOCTEXT_NAMESPACE "FPDFImporterModule"

void FPDFImporterModule::StartupModule()
{
	//dllファイルのパスを取得
	FString APIDllPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("ThirdParty"), TEXT("Ghostscript")));
#ifdef _WIN64
	APIDllPath = FPaths::Combine(APIDllPath, TEXT("Win64"));
#elif _WIN32
	APIDllPath = FPaths::Combine(APIDllPath, TEXT("Win32"));
#endif
	APIDllPath = FPaths::Combine(APIDllPath, TEXT("gsdll.dll"));

	//モジュールをロード
	APIModule = FPlatformProcess::GetDllHandle(*APIDllPath);
	if (APIModule == nullptr)
	{
		UE_LOG(PDFImporter, Fatal, TEXT("Failed to load Ghostscript module"));
	}

	//関数ポインタを取得
	CreateInstance = (CreateAPIInstance)FPlatformProcess::GetDllExport(APIModule, TEXT("gsapi_new_instance"));
	DeleteInstance = (DeleteAPIInstance)FPlatformProcess::GetDllExport(APIModule, TEXT("gsapi_delete_instance"));
	Init = (InitAPI)FPlatformProcess::GetDllExport(APIModule, TEXT("gsapi_init_with_args"));
	Exit = (ExitAPI)FPlatformProcess::GetDllExport(APIModule, TEXT("gsapi_exit"));
	if (CreateInstance == nullptr || DeleteInstance == nullptr || Init == nullptr || Exit == nullptr)
	{
		UE_LOG(PDFImporter, Fatal, TEXT("Failed to get Ghostscript function pointer"));
	}
}

void FPDFImporterModule::ShutdownModule()
{
	FPlatformProcess::FreeDllHandle(APIModule);
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
	void* APIInstance = nullptr;
	CreateInstance(&APIInstance, 0);
	if (APIInstance != nullptr)
	{
		//Ghostscriptを実行
		int Result = Init(APIInstance, 20, (char**)Args);
		
		//Ghostscriptを終了
		Exit(APIInstance);
		DeleteInstance(APIInstance);

		UE_LOG(PDFImporter, Log, TEXT("Ghostscript Return Code : %d"), Result);

		return Result == 0;
	}

	UE_LOG(PDFImporter, Error, TEXT("Failed to create Ghostscript instance"));
	return false;
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