#include "AssetTypeActions_PDF.h"
#include "PDF.h"
#include "IPDFViewerModule.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

void FAssetTypeActions_PDF::OpenAssetEditor(
	const TArray<UObject*>& InObjects,
	TSharedPtr<class IToolkitHost> EditWithinLevelEditor
)
{
	for (int i = 0; i < InObjects.Num(); ++i)
	{
		IPDFViewerModule* PDFViewerModule = &FModuleManager::LoadModuleChecked<IPDFViewerModule>("PDFViewer");
		PDFViewerModule->CreatePDFViewer(EToolkitMode::Standalone, EditWithinLevelEditor, Cast<UPDF>(InObjects[i]));
	}
}

UClass* FAssetTypeActions_PDF::GetSupportedClass() const
{
	return UPDF::StaticClass();
}

#undef LOCTEXT_NAMESPACE