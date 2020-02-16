// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"

/**
 * Holds the UI commands for the PDFViewerToolkit widget.
 */
class FPDFViewerCommands : public TCommands<FPDFViewerCommands>
{
public:

	/**
	 * Default constructor.
	 */
	FPDFViewerCommands( ) 
		: TCommands<FPDFViewerCommands>("PDFViewer", NSLOCTEXT("Contexts", "PDFViewer", "PDF Viewer"), NAME_None, FEditorStyle::GetStyleSetName())
	{ }

public:

	// TCommands interface

	virtual void RegisterCommands() override;
	
public:

	/** Toggles the red channel */
	TSharedPtr<FUICommandInfo> RedChannel;
	
	/** Toggles the green channel */
	TSharedPtr<FUICommandInfo> GreenChannel;
	
	/** Toggles the blue channel */
	TSharedPtr<FUICommandInfo> BlueChannel;
	
	/** Toggles the alpha channel */
	TSharedPtr<FUICommandInfo> AlphaChannel;

	/** Toggles color saturation */
	TSharedPtr<FUICommandInfo> Desaturation;

	/** If enabled, the texture will be scaled to fit the viewport */
	TSharedPtr<FUICommandInfo> FitToViewport;

	/** Sets the checkered background pattern */
	TSharedPtr<FUICommandInfo> CheckeredBackground;

	/** Sets the checkered background pattern (filling the view port) */
	TSharedPtr<FUICommandInfo> CheckeredBackgroundFill;

	/** Sets the solid color background */
	TSharedPtr<FUICommandInfo> SolidBackground;

// Begin - Volume Texture Specifics
	/** For volume texture, shows each depth slice side by side */
	TSharedPtr<FUICommandInfo> DepthSlices;

	/** For volume texture, shows a trace into the volume */
	TSharedPtr<FUICommandInfo> TraceIntoVolume;
// End - Volume Texture Specifics

	/** If enabled, a border is drawn around the texture */
	TSharedPtr<FUICommandInfo> TextureBorder;

	/** To the previous page */
	TSharedPtr<FUICommandInfo> BackPage;

	/** To the next page */
	TSharedPtr<FUICommandInfo> NextPage;

	/** Open the texture editor settings. */
	TSharedPtr< FUICommandInfo > Settings;
};
