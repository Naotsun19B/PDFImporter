#pragma once

#include "UnrealEd.h"
#include "SlateStyle.h"
#include "IPluginManager.h"
#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

class FPDFViewerStyle : public FSlateStyleSet
{
public:
	FPDFViewerStyle() : FSlateStyleSet("PDFViewerStyle")
	{
		// Register pdf viewer style set
		SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("PDFImporter"))->GetBaseDir() / TEXT("Resources"));
		Set("PDFViewer.BackButton", new IMAGE_BRUSH("IconBackPage128", FVector2D(40.f, 40.f)));
		Set("PDFViewer.NextButton", new IMAGE_BRUSH("IconNextPage128", FVector2D(40.f, 40.f)));
		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

	~FPDFViewerStyle()
	{
		// Unregister pdf viewer style set
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}
};
#undef IMAGE_BRUSH
