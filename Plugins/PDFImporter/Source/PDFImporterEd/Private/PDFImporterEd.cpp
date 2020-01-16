// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PDFImporterEd.h"
#include "AssetTypeActions_PDF.h"

#define LOCTEXT_NAMESPACE "FPDFImporterModuleEd"

void FPDFImporterEdModule::StartupModule()
{
	// PDFアセットのAssetTypeActionsをAssetToolsモジュールに登録
	PDF_AssetTypeActions = MakeShareable(new FAssetTypeActions_PDF);
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(PDF_AssetTypeActions.ToSharedRef());
}

void FPDFImporterEdModule::ShutdownModule()
{
	if (PDF_AssetTypeActions.IsValid())
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(PDF_AssetTypeActions.ToSharedRef());
		}
		PDF_AssetTypeActions.Reset();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPDFImporterEdModule, PDFImporterEd)