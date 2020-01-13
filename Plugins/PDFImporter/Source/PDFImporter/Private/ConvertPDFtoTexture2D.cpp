// Fill out your copyright notice in the Description page of Project Settings.

#include "ConvertPDFtoTexture2D.h"
#include "GhostscriptCore.h"
#include "PDF.h"
#include "AsyncExecTask.h"
#include "Misc/Paths.h"

UConvertPDFtoTexture2D::UConvertPDFtoTexture2D(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr), bIsActive(false), 
	  PDFFilePath(""), Dpi(0), FirstPage(0), LastPage(0), Locale("")
{
	FPDFImporterModule& PDFImporterModule = FModuleManager::LoadModuleChecked<FPDFImporterModule>(FName("PDFImporter"));
	GhostscriptCore = PDFImporterModule.GetGhostscriptCore();
}

UConvertPDFtoTexture2D* UConvertPDFtoTexture2D::ConvertPDFtoTexture2D(
	const UObject* WorldContextObject, 
	const FString& PDFFilePath, 
	int Dpi,
	int FirstPage,
	int LastPage,
	const FString& Locale
){
	UConvertPDFtoTexture2D* Node = NewObject<UConvertPDFtoTexture2D>();
	Node->WorldContextObject = WorldContextObject;
	Node->PDFFilePath = PDFFilePath;
	Node->Dpi = Dpi;
	Node->FirstPage = FirstPage;
	Node->LastPage = LastPage;
	Node->Locale = Locale;
	return Node;
}

void UConvertPDFtoTexture2D::Activate()
{
	if (!WorldContextObject)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid WorldContextObject. Cannot execute ConvertPDFtoTexture2D."), ELogVerbosity::Error);
		return;
	}
	if (bIsActive)
	{
		FFrame::KismetExecutionMessage(TEXT("ConvertPDFtoTexture2D is already running."), ELogVerbosity::Warning);
		return;
	}
	
	//•ÏŠ·ŠJŽn
	auto ConvertTask = new FAutoDeleteAsyncTask<FAsyncExecTask>([this]() { ExecConversion(); });
	ConvertTask->StartBackgroundTask();
}

void UConvertPDFtoTexture2D::ExecConversion()
{
	UPDF* PDFAsset = GhostscriptCore->ConvertPdfToPdfAsset(PDFFilePath, Dpi, FirstPage, LastPage, Locale);
	if (PDFAsset != nullptr)
	{
		Completed.Broadcast(PDFAsset);
	}
	else
	{
		Failed.Broadcast();
	}
}
