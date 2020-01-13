#include "AssetTypeActions_PDF.h"
#include "PDF.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

UClass* FAssetTypeActions_PDF::GetSupportedClass() const
{
	return UPDF::StaticClass();
}

#undef LOCTEXT_NAMESPACE