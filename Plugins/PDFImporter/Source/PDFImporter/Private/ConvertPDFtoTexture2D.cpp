// Fill out your copyright notice in the Description page of Project Settings.

#include "ConvertPDFtoTexture2D.h"
#include "PDFImporter.h"
#include "AsyncExecTask.h"
#include "Runtime/Core/Public/Templates/SharedPointer.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatform.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"

UConvertPDFtoTexture2D::UConvertPDFtoTexture2D(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), mWorldContextObject(nullptr), bIsActive(false), 
	  mPDFFilePath(""), mGhostscriptPath(""), mDpi(0)
{
	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	mImageWrapper = imageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
}

UConvertPDFtoTexture2D* UConvertPDFtoTexture2D::ConvertPDFtoTexture2D(
	const UObject* WorldContextObject, 
	const FString& PDFFilePath, 
	const FString& GhostscriptPath, 
	int32 Dpi
){
	UConvertPDFtoTexture2D* node = NewObject<UConvertPDFtoTexture2D>();
	node->mWorldContextObject = WorldContextObject;
	node->mPDFFilePath = PDFFilePath;
	node->mGhostscriptPath = GhostscriptPath;
	node->mDpi = Dpi;
	return node;
}

void UConvertPDFtoTexture2D::Activate()
{
	if (!mWorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid WorldContextObject. Cannot execute ConvertPDFtoTexture2D."), ELogVerbosity::Error);
		return;
	}
	if (bIsActive)
	{
		FFrame::KismetExecutionMessage(TEXT("ConvertPDFtoTexture2D is already running."), ELogVerbosity::Warning);
		return;
	}
	
	//変換開始
	auto convertTask = new FAutoDeleteAsyncTask<FAsyncExecTask>([this]() { ExecConversion(); });
	convertTask->StartBackgroundTask();
}

void UConvertPDFtoTexture2D::ExecConversion()
{
	IFileManager& fileManager = IFileManager::Get();

	//PDFがあるか確認
	if (!fileManager.FileExists(*mPDFFilePath))
	{
		UE_LOG(PDFImporter, Error, TEXT("File not found : %s"), *mPDFFilePath);
		Failed.Broadcast();
	}
	//Ghostscriptの実行ファイルがあるか確認
	if (!fileManager.FileExists(*mGhostscriptPath))
	{
		UE_LOG(PDFImporter, Error, TEXT("File not found : %s"), *mGhostscriptPath);
		Failed.Broadcast();
	}

	//作業用のディレクトリを作成
	FString tempDirPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ConvertTemp"), TEXT(""));
	tempDirPath = FPaths::ConvertRelativePathToFull(tempDirPath);
	if (!fileManager.DirectoryExists(*tempDirPath))
	{
		fileManager.MakeDirectory(*tempDirPath);
		UE_LOG(PDFImporter, Log, TEXT("A working directory has been created (%s)"), *tempDirPath);
	}
	else
	{
		UE_LOG(PDFImporter, Error, TEXT("There was already a directory with the same name (%s)"), *tempDirPath);
		Failed.Broadcast();
	}

	//Ghostscriptを用いてPDFからjpg画像を作成
	bool result = ConvertPDFtoJPG(mPDFFilePath, mGhostscriptPath, tempDirPath, mDpi);

	//作成したjpg画像を読み込む
	TArray<UTexture2D*> buffer;
	if (result)
	{
		//画像のファイルパスを取得
		TArray<FString> pageNames;
		fileManager.FindFiles(pageNames, *tempDirPath, L"jpg");

		//画像の読み込み
		UTexture2D* textureTemp;
		for (FString pageName : pageNames)
		{
			result = LoadTexture2DFromFile(FPaths::Combine(tempDirPath, pageName), textureTemp);
			if (result) buffer.Add(textureTemp);
			else break;
		}
	}

	//作業用ディレクトリを削除
	if (fileManager.DirectoryExists(*tempDirPath))
	{
		if (fileManager.DeleteDirectory(*tempDirPath, false, true))
		{
			UE_LOG(PDFImporter, Log, TEXT("The directory used for work was successfully deleted (%s)"), *tempDirPath);
		}
		else UE_LOG(PDFImporter, Error, TEXT("The directory used for work could not be deleted (%s)"), *tempDirPath);
	}
	
	if (result) Completed.Broadcast(buffer);
	else Failed.Broadcast();
}

bool UConvertPDFtoTexture2D::ConvertPDFtoJPG(const FString& PDFFilePath, const FString& GhostscriptPath, const FString& OutDirPath, int32 Dpi)
{
	//コマンドライン引数を設定
	FString params = TEXT("-dSAFER");
	params += TEXT(" -dBATCH");
	params += TEXT(" -dNOPAUSE");
	params += TEXT(" -sDEVICE=jpeg");
	params += TEXT(" -r");
	params += FString::FromInt(Dpi);
	params += TEXT(" -sOutputFile=\"");
	params += OutDirPath;
	params += FPaths::GetBaseFilename(PDFFilePath);
	params += TEXT("%03d.jpg\" \"");
	params += PDFFilePath;
	params += TEXT("\"");

	//Ghostscriptを実行（終了まで待機）
	int32 returnCode = 0;
	FString stdOut = "";
	FString stdErr = "";
	bool processResult = FPlatformProcess::ExecProcess(
		*GhostscriptPath,
		*params,
		&returnCode,
		&stdOut,
		&stdErr
	);

	//実行結果をログ出力
	if (processResult)
	{
		UE_LOG(PDFImporter, Log, TEXT("StdOut : %s"), *stdOut);
		UE_LOG(PDFImporter, Log, TEXT("Return Code : %d"), returnCode);
		if (returnCode != 0)
		{
			UE_LOG(PDFImporter, Error, TEXT("StdErr : %s"), *stdErr);
			processResult = false;
		}
	}

	return processResult;
}

bool UConvertPDFtoTexture2D::LoadTexture2DFromFile(const FString& FilePath, UTexture2D* &LoadedTexture)
{
	//配列に画像を読み込む
	TArray<uint8> rawFileData;
	if (!FFileHelper::LoadFileToArray(rawFileData, *FilePath)) return false;

	if (mImageWrapper.IsValid() && mImageWrapper->SetCompressed(rawFileData.GetData(), rawFileData.Num()))
	{
		const TArray<uint8>* uncompressedRawData = NULL;
		if (mImageWrapper->GetRaw(ERGBFormat::BGRA, 8, uncompressedRawData))
		{
			//Texture2Dを作成
			LoadedTexture = nullptr;
			LoadedTexture = UTexture2D::CreateTransient(mImageWrapper->GetWidth(), mImageWrapper->GetHeight(), PF_B8G8R8A8);
			if (!LoadedTexture) return false;

			void* textureData = LoadedTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(textureData, uncompressedRawData->GetData(), uncompressedRawData->Num());
			LoadedTexture->PlatformData->Mips[0].BulkData.Unlock();
			LoadedTexture->UpdateResource();
		}
	}

	return true;
}