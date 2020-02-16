// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture.h"
#include "PDF.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Interfaces/IPDFViewerToolkit.h"
#include "Interfaces/IPDFViewerModule.h"
#include "PropertyEditorModule.h"
#include "PDFViewerSettings.h"
#include "PDFViewerToolkit.h"
#include "ISettingsModule.h"
#include "Customizations/TextureDetailsCustomization.h"
#include "Customizations/CurveLinearColorAtlasDetailsCustomization.h"

#define LOCTEXT_NAMESPACE "FPDFViewerModule"


extern const FName PDFViewerAppIdentifier = FName(TEXT("PDFViewerApp"));


/*-----------------------------------------------------------------------------
   FPDFViewerModule
-----------------------------------------------------------------------------*/

class FPDFViewerModule : public IPDFViewerModule
{
public:

	// IPDFViewerModule interface

	virtual TSharedRef<IPDFViewerToolkit> CreatePDFViewer( const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UPDF* PDF ) override
	{
		TSharedRef<FPDFViewerToolkit> NewPDFViewer(new FPDFViewerToolkit());
		NewPDFViewer->InitPDFViewer(Mode, InitToolkitHost, PDF);

		return NewPDFViewer;
	}

	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager( ) override
	{
		return MenuExtensibilityManager;
	}

	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager( ) override
	{
		return ToolBarExtensibilityManager;
	}

public:

	// IModuleInterface interface

	virtual void StartupModule( ) override
	{
		// register menu extensions
		MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
		ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);

		// register settings
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->RegisterSettings("Editor", "ContentEditors", "PDFViewer",
				LOCTEXT("PDFViewerSettingsName", "PDF Viewer"),
				LOCTEXT("PDFViewerSettingsDescription", "Configure the look and feel of the Texture Editor."),
				GetMutableDefault<UPDFViewerSettings>()
			);
		}

		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout("Texture", FOnGetDetailCustomizationInstance::CreateStatic(&FTextureDetails::MakeInstance));
		PropertyModule.RegisterCustomClassLayout("CurveLinearColorAtlas", FOnGetDetailCustomizationInstance::CreateStatic(&FCurveLinearColorAtlasDetails::MakeInstance));
	}

	virtual void ShutdownModule( ) override
	{
		// unregister settings
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->UnregisterSettings("Editor", "ContentEditors", "PDFViewer");
		}

		// unregister menu extensions
		MenuExtensibilityManager.Reset();
		ToolBarExtensibilityManager.Reset();

		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("Texture");
		PropertyModule.UnregisterCustomClassLayout("CurveLinearColorAtlas");
	}

private:

	// Holds the menu extensibility manager.
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;

	// Holds the tool bar extensibility manager.
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;
};


IMPLEMENT_MODULE(FPDFViewerModule, PDFViewer);


#undef LOCTEXT_NAMESPACE
