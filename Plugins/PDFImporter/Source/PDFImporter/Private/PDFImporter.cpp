// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PDFImporter.h"
#include "GhostscriptCore.h"
#include "PDFAsset/AssetTypeActions_PDF.h"

#define LOCTEXT_NAMESPACE "FPDFImporterModule"

void FPDFImporterModule::StartupModule()
{
	// PDFアセットのAssetTypeActionsをAssetToolsモジュールに登録
	PDF_AssetTypeActions = MakeShareable(new FAssetTypeActions_PDF);
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(PDF_AssetTypeActions.ToSharedRef());


	UGhostscriptCore::CreateModule();
}

void FPDFImporterModule::ShutdownModule()
{
	if (PDF_AssetTypeActions.IsValid())
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(PDF_AssetTypeActions.ToSharedRef());
		}
		PDF_AssetTypeActions.Reset();
	}

	UGhostscriptCore::DestroyModule();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPDFImporterModule, PDFImporter)