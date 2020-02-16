// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FPDFImporterEdModule : public IModuleInterface
{
public:
	TSharedPtr<class FAssetTypeActions_PDF> PDF_AssetTypeActions;
	TSharedPtr<class FSlateStyleSet> StyleSet;

public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

DEFINE_LOG_CATEGORY_STATIC(PDFImporterEd, Log, All);