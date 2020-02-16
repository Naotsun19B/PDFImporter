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
		SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("PDFImporter"))->GetBaseDir() / TEXT("Resources"));
		Set("BackPageButtonImage", new IMAGE_BRUSH("IconBackPage128", FVector2D(128.f, 128.f)));
		Set("NextPageButtonImage", new IMAGE_BRUSH("IconNextPage128", FVector2D(128.f, 128.f)));
		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

	~FPDFViewerStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}
};
#undef IMAGE_BRUSH
