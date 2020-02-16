#pragma once

#include "UnrealEd.h"
#include "AssetTypeActions_Base.h"

class PDFIMPORTERED_API FAssetTypeActions_PDF : public FAssetTypeActions_Base
{
public:
	virtual void OpenAssetEditor(
		const TArray<UObject*>& InObjects,
		TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()
	) override;
	virtual FText GetName() const override
	{
		return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_PDF", "PDF");
	}
	virtual FColor GetTypeColor() const override { return FColor(255, 196, 128); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
	virtual UClass* GetSupportedClass() const override;
	virtual bool IsImportedAsset() const override { return true; }
};

