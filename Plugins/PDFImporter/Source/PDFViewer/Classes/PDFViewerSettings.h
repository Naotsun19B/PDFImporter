// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "PDFViewerSettings.generated.h"


/**
 * Enumerates background for the texture editor view port.
 */
UENUM()
enum EPDFViewerBackgrounds
{
	PDFViewerBackground_SolidColor UMETA(DisplayName="Solid Color"),
	PDFViewerBackground_Checkered UMETA(DisplayName="Checkered"),
	PDFViewerBackground_CheckeredFill UMETA(DisplayName="Checkered (Fill)")
};

UENUM()
enum EPDFViewerVolumeViewMode
{
	PDFViewerVolumeViewMode_DepthSlices UMETA(DisplayName="Depth Slices"),
	PDFViewerVolumeViewMode_VolumeTrace UMETA(DisplayName="Trace Into Volume"),
};

/**
 * Implements the Editor's user settings.
 */
UCLASS(config=EditorPerProjectUserSettings)
class PDFVIEWER_API UPDFViewerSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/** The type of background to draw in the texture editor view port. */
	UPROPERTY(config)
	TEnumAsByte<EPDFViewerBackgrounds> Background;

	/** The type of display when viewing volume textures. */
	UPROPERTY(config)
	TEnumAsByte<EPDFViewerVolumeViewMode> VolumeViewMode;

	/** Background and foreground color used by Texture preview view ports. */
	UPROPERTY(config, EditAnywhere, Category=Background)
	FColor BackgroundColor;

	/** The first color of the checkered background. */
	UPROPERTY(config, EditAnywhere, Category=Background)
	FColor CheckerColorOne;

	/** The second color of the checkered background. */
	UPROPERTY(config, EditAnywhere, Category=Background)
	FColor CheckerColorTwo;

	/** The size of the checkered background tiles. */
	UPROPERTY(config, EditAnywhere, Category=Background, meta=(ClampMin="2", ClampMax="4096"))
	int32 CheckerSize;

public:

	/** Whether the texture should scale to fit the view port. */
	UPROPERTY(config)
	bool FitToViewport;

	/** Color to use for the texture border, if enabled. */
	UPROPERTY(config, EditAnywhere, Category=TextureBorder)
	FColor TextureBorderColor;

	/** If true, displays a border around the texture. */
	UPROPERTY(config)
	bool TextureBorderEnabled;
};
