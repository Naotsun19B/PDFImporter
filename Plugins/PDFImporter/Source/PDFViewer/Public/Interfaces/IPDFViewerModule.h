// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Interfaces/IPDFViewerToolkit.h"
#include "Modules/ModuleInterface.h"

/**
 * Interface for pdf viewer modules.
 */
class IPDFViewerModule
	: public IModuleInterface
	, public IHasMenuExtensibility
	, public IHasToolBarExtensibility
{
public:

	/**
	 * Creates a new pdf viewer.
	 *
	 * @param Mode 
	 * @param InitToolkitHost 
	 * @param Texture 
	 */
	virtual TSharedRef<IPDFViewerToolkit> CreatePDFViewer( const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, class UPDF* PDF ) = 0;
};
