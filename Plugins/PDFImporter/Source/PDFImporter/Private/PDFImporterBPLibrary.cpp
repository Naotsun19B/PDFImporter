// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PDFImporterBPLibrary.h"
#include "PDFImporter.h"
#include "Engine.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Runtime/Core/Public/Templates/SharedPointer.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatform.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"

UPDFImporterBPLibrary::UPDFImporterBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

bool UPDFImporterBPLibrary::OpenPDFDialog(const FString& DefaultPath, FString& FileName)
{
	if (GEngine) 
	{
		if (GEngine->GameViewport)
		{
			IDesktopPlatform* desktopPlatform = FDesktopPlatformModule::Get();
			if(desktopPlatform)
			{
				TArray<FString> resultTemp;
				bool result = desktopPlatform->OpenFileDialog(
					GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle(),
					TEXT("Open PDF Dialog"),
					DefaultPath,
					TEXT(""),
					TEXT("PDF File (.pdf)|*.pdf"),
					EFileDialogFlags::Type::None,
					resultTemp
				);

				if (result)FileName = resultTemp[0];
				return result;
			}
		}
	}
	return false;
}

bool UPDFImporterBPLibrary::OpenPDFDialogMultiple(const FString& DefaultPath, TArray<FString>& FileNames)
{
	if (GEngine)
	{
		if (GEngine->GameViewport)
		{
			IDesktopPlatform* desktopPlatform = FDesktopPlatformModule::Get();
			if (desktopPlatform)
			{
				return desktopPlatform->OpenFileDialog(
					GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle(),
					TEXT("Open PDF Dialog"),
					DefaultPath,
					TEXT(""),
					TEXT("PDF File (.pdf)|*.pdf"),
					EFileDialogFlags::Type::Multiple,
					FileNames
				);
			}
		}
	}
	return false;
}

bool UPDFImporterBPLibrary::ConvertPDFToTexture2D(TArray<UTexture2D*>& Pages, const FString& PDFFilePath, const FString& GhostscriptPath, int32 Dpi)
{
	IFileManager& fileManager = IFileManager::Get();

	//PDFがあるか確認
	if (!fileManager.FileExists(*PDFFilePath)) 
	{
		UE_LOG(PDFImporter, Error, TEXT("File not found : %s"), *PDFFilePath);
		return false;
	}
	//Ghostscriptの実行ファイルがあるか確認
	if (!fileManager.FileExists(*GhostscriptPath))
	{
		UE_LOG(PDFImporter, Error, TEXT("File not found : %s"), *GhostscriptPath);
		return false;
	}

	//作業用のディレクトリを作成
	FString tempDirPath = FPaths::Combine(FPaths::GameSavedDir(), TEXT("ConvertTemp"), TEXT(""));
	tempDirPath = FPaths::ConvertRelativePathToFull(tempDirPath);
	if (!fileManager.DirectoryExists(*tempDirPath)) 
	{
		fileManager.MakeDirectory(*tempDirPath);
		UE_LOG(PDFImporter, Log, TEXT("A working directory has been created (%s)"), *tempDirPath);
	}
	else
	{
		UE_LOG(PDFImporter, Error, TEXT("There was already a directory with the same name (%s)"), *tempDirPath);
		return false;
	}
	
	//Ghostscriptを用いてPDFからjpg画像を作成
	bool result = ConvertPDFtoJPG(PDFFilePath, GhostscriptPath, tempDirPath, Dpi);

	//作成したjpg画像を読み込む
	if (result)
	{
		//画像のファイルパスを取得
		TArray<FString> pageNames;
		fileManager.FindFiles(pageNames, *tempDirPath, L"jpg");

		//画像の読み込み
		UTexture2D* textureTemp;
		for (FString pageName : pageNames)
		{
			result = LoadTexture2DFromFile(FPaths::Combine(tempDirPath, pageName), EImageFormat::JPEG, textureTemp);
			if (result)Pages.Add(textureTemp);
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
	return result;
}

bool UPDFImporterBPLibrary::ConvertPDFtoJPG(const FString& PDFFilePath, const FString& GhostscriptPath, const FString& OutDirPath, int32 Dpi)
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
	params += TEXT("%03d.jpg\" ");
	params += PDFFilePath;

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

bool UPDFImporterBPLibrary::LoadTexture2DFromFile(const FString& FilePath, EImageFormat ImageType, UTexture2D* &LoadedTexture)
{
	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> imageWrapper = imageWrapperModule.CreateImageWrapper(ImageType);

	//配列に画像を読み込む
	TArray<uint8> rawFileData;
	if (!FFileHelper::LoadFileToArray(rawFileData, *FilePath)) return false;

	if (imageWrapper.IsValid() && imageWrapper->SetCompressed(rawFileData.GetData(), rawFileData.Num()))
	{
		const TArray<uint8>* uncompressedRawData = NULL;
		if (imageWrapper->GetRaw(ERGBFormat::BGRA, 8, uncompressedRawData))
		{
			//Texture2Dを作成
			LoadedTexture = nullptr;
			LoadedTexture = UTexture2D::CreateTransient(imageWrapper->GetWidth(), imageWrapper->GetHeight(), PF_B8G8R8A8);
			if (!LoadedTexture) return false;

			void* textureData = LoadedTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(textureData, uncompressedRawData->GetData(), uncompressedRawData->Num());
			LoadedTexture->PlatformData->Mips[0].BulkData.Unlock();
			LoadedTexture->UpdateResource();
		}
	}

	return true;
}