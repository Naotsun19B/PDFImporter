// Fill out your copyright notice in the Description page of Project Settings.

#include "ConvertPdfToPdfAsset.h"
#include "GhostscriptCore.h"
#include "PDF.h"
#include "AsyncExecTask.h"
#include "Misc/Paths.h"

UConvertPdfToPdfAsset::UConvertPdfToPdfAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr), bIsActive(false), 
	  PDFFilePath(""), Dpi(0), FirstPage(0), LastPage(0)
{
	FPDFImporterModule& PDFImporterModule = FModuleManager::LoadModuleChecked<FPDFImporterModule>(FName("PDFImporter"));
	GhostscriptCore = PDFImporterModule.GetGhostscriptCore();
}

UConvertPdfToPdfAsset* UConvertPdfToPdfAsset::ConvertPdfToPdfAsset(
	const UObject* WorldContextObject, 
	const FString& PDF_FilePath, 
	int Dpi,
	int FirstPage,
	int LastPage
){
	UConvertPdfToPdfAsset* Node = NewObject<UConvertPdfToPdfAsset>();
	Node->WorldContextObject = WorldContextObject;
	Node->PDFFilePath = PDF_FilePath;
	Node->Dpi = Dpi;
	Node->FirstPage = FirstPage;
	Node->LastPage = LastPage;
	return Node;
}

void UConvertPdfToPdfAsset::Activate()
{
	if (WorldContextObject == nullptr)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid WorldContextObject. Cannot execute ConvertPDFtoPDFAsset."), ELogVerbosity::Error);
		return;
	}

	if (bIsActive)
	{
		FFrame::KismetExecutionMessage(TEXT("ConvertPDFtoPDFAsset is already running."), ELogVerbosity::Warning);
		return;
	}
	
	// •ÏŠ·ŠJŽn
	auto ConvertTask = new FAutoDeleteAsyncTask<FAsyncExecTask>([this]() 
	{
		UPDF* PDFAsset = GhostscriptCore->ConvertPdfToPdfAsset(PDFFilePath, Dpi, FirstPage, LastPage);
		if (PDFAsset != nullptr)
		{
			Completed.Broadcast(PDFAsset);
		}
		else
		{
			Failed.Broadcast();
		}
	});

	ConvertTask->StartBackgroundTask();
}
