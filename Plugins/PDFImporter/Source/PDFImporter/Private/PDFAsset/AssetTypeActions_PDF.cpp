#include "PDFAsset/AssetTypeActions_PDF.h"
#include "PDFAsset/PDF.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

UClass* FAssetTypeActions_PDF::GetSupportedClass() const
{
	return UPDF::StaticClass();
}

#undef LOCTEXT_NAMESPACE