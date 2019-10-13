// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PDFImporterBPLibrary.h"
#include "PDFImporter.h"
#include "Engine.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

UPDFImporterBPLibrary::UPDFImporterBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UPDFImporterBPLibrary::OpenPDFDialog(const FString& DefaultPath, EOpenPDFDialogResult& OutputPin, FString& FileName)
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

				if (result) 
				{
					FileName = resultTemp[0];
					OutputPin = EOpenPDFDialogResult::Successful;
					return;
				}
			}
		}
	}
	OutputPin = EOpenPDFDialogResult::Cancelled;
}

void UPDFImporterBPLibrary::OpenPDFDialogMultiple(const FString& DefaultPath, EOpenPDFDialogResult& OutputPin, TArray<FString>& FileNames)
{
	if (GEngine)
	{
		if (GEngine->GameViewport)
		{
			IDesktopPlatform* desktopPlatform = FDesktopPlatformModule::Get();
			if (desktopPlatform)
			{
				bool result = desktopPlatform->OpenFileDialog(
					GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle(),
					TEXT("Open PDF Dialog"),
					DefaultPath,
					TEXT(""),
					TEXT("PDF File (.pdf)|*.pdf"),
					EFileDialogFlags::Type::Multiple,
					FileNames
				);

				if (result) 
				{
					OutputPin = EOpenPDFDialogResult::Successful;
					return;
				}
			}
		}
	}
	OutputPin = EOpenPDFDialogResult::Cancelled;
}