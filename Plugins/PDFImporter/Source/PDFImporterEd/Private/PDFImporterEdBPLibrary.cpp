// Fill out your copyright notice in the Description page of Project Settings.

#include "PDFImporterEdBPLibrary.h"
#include "PDFImporterEd.h"
#include "Engine.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

void UPDFImporterEdBPLibrary::OpenPDFDialogEd(const FString& DefaultPath, EOpenPDFDialogResult& OutputPin, FString& FileName)
{
	TSharedPtr<SWindow> MainWindow;
	if (GIsEditor)
	{
		auto& MainFrameModule = IMainFrameModule::Get();
		MainWindow = MainFrameModule.GetParentWindow();

		if (MainWindow.IsValid() && MainWindow->GetNativeWindow().IsValid())
		{
			IDesktopPlatform* desktopPlatform = FDesktopPlatformModule::Get();
			if (desktopPlatform)
			{
				TArray<FString> resultTemp;
				bool result = desktopPlatform->OpenFileDialog(
					MainWindow->GetNativeWindow()->GetOSWindowHandle(),
					TEXT("Open PDF Dialog"),
					DefaultPath,
					TEXT(""),
					TEXT("PDF File (.pdf)|*.pdf"),
					EFileDialogFlags::Type::None,
					resultTemp
				);

				if (result)
				{
					//相対パスを絶対パスに変換
					FileName = FPaths::ConvertRelativePathToFull(resultTemp[0]);
					UE_LOG(PDFImporterEd, Log, TEXT("Open PDF Dialog : %s"), *FileName);
					OutputPin = EOpenPDFDialogResult::Successful;
					return;
				}
			}
		}
	}
	UE_LOG(PDFImporterEd, Log, TEXT("Open PDF Dialog : Cancelled"));
	OutputPin = EOpenPDFDialogResult::Cancelled;
}