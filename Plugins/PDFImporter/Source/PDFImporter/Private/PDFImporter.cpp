// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PDFImporter.h"
#include "Misc/Paths.h"
#include "GenericPlatform/GenericPlatformProcess.h"

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
#pragma warning(push)
#pragma warning( disable : 4191 )
	CreateInstance = (CreateAPIInstance)FPlatformProcess::GetDllExport(APIModule, TEXT("gsapi_new_instance"));
	DeleteInstance = (DeleteAPIInstance)FPlatformProcess::GetDllExport(APIModule, TEXT("gsapi_delete_instance"));
	Init = (InitAPI)FPlatformProcess::GetDllExport(APIModule, TEXT("gsapi_init_with_args"));
	Exit = (ExitAPI)FPlatformProcess::GetDllExport(APIModule, TEXT("gsapi_exit"));
#pragma warning(pop)
	if (CreateInstance == nullptr || DeleteInstance == nullptr || Init == nullptr || Exit == nullptr)
	{
		UE_LOG(PDFImporter, Fatal, TEXT("Failed to get Ghostscript function pointer"));
	}
}

void FPDFImporterModule::ShutdownModule()
{
	FPlatformProcess::FreeDllHandle(APIModule);
}

bool FPDFImporterModule::ConvertPdfToJpeg(const FString& InputPath, const FString& OutputPath, int Dpi, int FirstPage, int LastPage)
{
	if (!(FirstPage > 0 && LastPage > 0 && FirstPage <= LastPage))
	{
		FirstPage = 1;
		LastPage = INT_MAX;
	}

	FString ParamOutputPath(TEXT("-sOutputFile=") + OutputPath);

	const char* Args[20] =
	{
		//Ghostscriptが標準出力に情報を出力しないように
		"-q",
		"-dQUIET",

		"-dPARANOIDSAFER",			//セーフモードで実行
		"-dBATCH",					//Ghostscriptがインタラクティブモードにならないように
		"-dNOPAUSE",					//ページごとの一時停止をしないように
		"-dNOPROMPT",				//コマンドプロンプトがでないように           
		"-dMaxBitmap=500000000",		//パフォーマンスを向上させる
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
	
	std::wstring FirstPage_wstr(*FString(TEXT("-dFirstPage=") + FString::FromInt(FirstPage)));
	std::string FirstPage_str(FirstPage_wstr.begin(), FirstPage_wstr.end());
	Args[14] = FirstPage_str.c_str();

	std::wstring LastPage_wstr(*FString(TEXT("-dLastPage=") + FString::FromInt(LastPage)));
	std::string LastPage_str(LastPage_wstr.begin(), LastPage_wstr.end());
	Args[15] = LastPage_str.c_str();

	std::wstring DpiX_wstr(*FString(TEXT("-dDEVICEXRESOLUTION=") + FString::FromInt(Dpi)));
	std::string DpiX_str(DpiX_wstr.begin(), DpiX_wstr.end());
	Args[16] = DpiX_str.c_str();

	std::wstring DpiY_wstr(*FString(TEXT("-dDEVICEYRESOLUTION=") + FString::FromInt(Dpi)));
	std::string DpiY_str(DpiY_wstr.begin(), DpiY_wstr.end());
	Args[17] = DpiY_str.c_str();

	std::wstring OutputPath_wstr(*FString(TEXT("-sOutputFile=") + OutputPath));
	std::string OutputPath_str(OutputPath_wstr.begin(), OutputPath_wstr.end());
	Args[18] = OutputPath_str.c_str();

	std::wstring InputPath_wstr(*InputPath);
	std::string InputPath_str(InputPath_wstr.begin(), InputPath_wstr.end());
	Args[19] = InputPath_str.c_str();

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

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPDFImporterModule, PDFImporter)