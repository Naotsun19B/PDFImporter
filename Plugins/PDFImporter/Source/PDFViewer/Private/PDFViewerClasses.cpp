// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "PDFViewerSettings.h"


UPDFViewerSettings::UPDFViewerSettings( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
	, Background(PDFViewerBackground_Checkered)
	, BackgroundColor(FColor::Black)
	, CheckerColorOne(FColor(128, 128, 128))
	, CheckerColorTwo(FColor(64, 64, 64))
	, CheckerSize(32)
	, FitToViewport(true)
	, TextureBorderColor(FColor::White)
	, TextureBorderEnabled(true)
{ }
