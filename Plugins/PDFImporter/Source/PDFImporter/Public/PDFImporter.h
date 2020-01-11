// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

typedef int(*CreateAPIInstance)(void** Instance, void* CallerHandle);
typedef void(*DeleteAPIInstance)(void* Instance);
typedef int(*InitAPI)(void* Instance, int Argc, char** Argv);
typedef int(*ExitAPI)(void* Instance);

class FPDFImporterModule : public IModuleInterface
{
private:
	// Ghostscript module
	void* APIModule;

	// Ghostscript function pointers
	CreateAPIInstance CreateInstance;
	DeleteAPIInstance DeleteInstance;
	InitAPI Init;
	ExitAPI Exit;

	TSharedPtr<class IImageWrapper> ImageWrapper;

public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	// Convert PDF to PDF asset
	class UPDF* ConvertPdfToPdfAsset(const FString& InputPath, int Dpi, int FirstPage, int LastPage, const FString& Locale);

private:
	// Convert PDF to multiple jpeg images using Ghostscript API
	bool ConvertPdfToJpeg(const FString& InputPath, const FString& OutputPath, int Dpi, int FirstPage, int LastPage, const FString& Locale);

	// Import as UTexture2D from folder image file
	bool LoadTexture2DFromFile(const FString& FilePath, UTexture2D* &LoadedTexture);

	// Get the size of FString data
	int GetFStringSize(const FString& Text);
};

DEFINE_LOG_CATEGORY_STATIC(PDFImporter, Log, All);