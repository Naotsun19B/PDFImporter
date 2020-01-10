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

public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	// Convert PDF to multiple jpeg images using Ghostscript API
	bool ConvertPdfToJpeg(const FString& InputPath, const FString& OutputPath, int Dpi, int FirstPage, int LastPage);
};

DEFINE_LOG_CATEGORY_STATIC(PDFImporter, Log, All);