// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Models/PDFViewerCommands.h"

#define LOCTEXT_NAMESPACE "PDFViewerViewOptionsMenu"

/**
 * Static helper class for populating the "View Options" menu in the texture editor's view port.
 */
class FPDFViewerViewOptionsMenu
{
public:

	/**
	 * Creates the menu.
	 *
	 * @param MenuBuilder The builder for the menu that owns this menu.
	 */
	static void MakeMenu( FMenuBuilder& MenuBuilder, bool bIsVolumeTexture )
	{
		// color channel options
		MenuBuilder.BeginSection("ChannelSection", LOCTEXT("ChannelsSectionHeader", "Color Channels"));
		{
			MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().RedChannel);
			MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().GreenChannel);
			MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().BlueChannel);
			MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().AlphaChannel);
			MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().Desaturation);
		}
		MenuBuilder.EndSection();

		// view port options
		MenuBuilder.BeginSection("ViewportSection", LOCTEXT("ViewportSectionHeader", "Viewport Options"));
		{
			MenuBuilder.AddSubMenu(
				LOCTEXT("Background", "Background"),
				LOCTEXT("BackgroundTooltip", "Set the viewport's background"),
				FNewMenuDelegate::CreateStatic(&FPDFViewerViewOptionsMenu::GenerateBackgroundMenuContent)
			);

			if (bIsVolumeTexture)
			{
				MenuBuilder.AddSubMenu(
					LOCTEXT("ViewMode", "View Mode"),
					LOCTEXT("ViewModeTooltip", "Set the view mode"),
					FNewMenuDelegate::CreateStatic(&FPDFViewerViewOptionsMenu::GenerateVolumeDisplayModeMenuContent)
				);
			}

			MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().TextureBorder);
			MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().FitToViewport);
		}
		MenuBuilder.EndSection();

		MenuBuilder.AddMenuSeparator();
		MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().Settings);
	}

protected:

	/**
	 * Creates the 'Background' sub-menu.
	 *
	 * @param MenuBuilder The builder for the menu that owns this menu.
	 */
	static void GenerateBackgroundMenuContent( FMenuBuilder& MenuBuilder )
	{
		MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().CheckeredBackground);
		MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().CheckeredBackgroundFill);
		MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().SolidBackground);
	}

	/**
	 * Creates the 'Display Mode' sub-menu.
	 *
	 * @param MenuBuilder The builder for the menu that owns this menu.
	 */
	static void GenerateVolumeDisplayModeMenuContent( FMenuBuilder& MenuBuilder )
	{
		MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().DepthSlices);
		MenuBuilder.AddMenuEntry(FPDFViewerCommands::Get().TraceIntoVolume);
	}
};


#undef LOCTEXT_NAMESPACE
