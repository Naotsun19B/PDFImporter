// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PDFImporterBPLibrary.h"
#include "PDFImporter.h"
#include "Engine.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"

UPDFImporterBPLibrary::UPDFImporterBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UPDFImporterBPLibrary::OpenPDFDialog(const FString& DefaultPath, EOpenPDFDialogResult& OutputPin, FString& FileName)
{
	TArray<FString> FileNameTemp;
	OutputPin = ExecOpenPDFDialog(DefaultPath, FileNameTemp, false);
	FileName = FileNameTemp[0];
}

void UPDFImporterBPLibrary::OpenPDFDialogMultiple(const FString& DefaultPath, EOpenPDFDialogResult& OutputPin, TArray<FString>& FileNames)
{
	OutputPin = ExecOpenPDFDialog(DefaultPath, FileNames, true);
}

void* UPDFImporterBPLibrary::GetWindowHandle()
{
	// エディタの場合
	if (GIsEditor)
	{
		IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
		TSharedPtr<SWindow> MainWindow = MainFrameModule.GetParentWindow();

		if (MainWindow.IsValid() && MainWindow->GetNativeWindow().IsValid())
		{
			return MainWindow->GetNativeWindow()->GetOSWindowHandle();
		}
	}
	// 実行時の場合
	else
	{
		if (GEngine && GEngine->GameViewport)
		{
			return GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();
		}
	}

	return nullptr;
}

EOpenPDFDialogResult UPDFImporterBPLibrary::ExecOpenPDFDialog(const FString& DefaultPath, TArray<FString>& FileNames, bool bIsMultiple)
{
	void* WindowHandle = GetWindowHandle();
	if (WindowHandle)
	{
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		if (DesktopPlatform)
		{
			// ダイアログを開く
			bool bResult = DesktopPlatform->OpenFileDialog(
				GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle(),
				TEXT("Open PDF Dialog"),
				DefaultPath,
				TEXT(""),
				TEXT("PDF File (.pdf)|*.pdf"),
				(bIsMultiple ? EFileDialogFlags::Type::Multiple : EFileDialogFlags::Type::None),
				FileNames
			);

			if (bResult)
			{
				// 相対パスを絶対パスに変換
				for (FString& FileName : FileNames)
				{
					FileName = FPaths::ConvertRelativePathToFull(FileName);
					UE_LOG(PDFImporter, Log, TEXT("Open PDF Dialog : %s"), *FileName);
				}
				
				UE_LOG(PDFImporter, Log, TEXT("Open PDF Dialog : Successful"));
				return EOpenPDFDialogResult::Successful;
			}
		}
	}

	UE_LOG(PDFImporter, Log, TEXT("Open PDF Dialog : Cancelled"));
	return EOpenPDFDialogResult::Cancelled;
}