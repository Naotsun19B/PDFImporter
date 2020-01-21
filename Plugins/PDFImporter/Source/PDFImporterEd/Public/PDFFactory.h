#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "PDFFactory.generated.h"

UCLASS()
class PDFIMPORTERED_API UPDFFactory : public UFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

private:
	TSharedPtr<class FGhostscriptCore> GhostscriptCore;

public:
	// UFactory interface
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual UClass* ResolveSupportedClass() override;
	virtual UObject* FactoryCreateFile(
		UClass* InClass,
		UObject* InParent,
		FName InName,
		EObjectFlags Flags,
		const FString& Filename,
		const TCHAR* Parms,
		FFeedbackContext* Warn,
		bool& bOutOperationCanceled
	)override;
	// End of UFactory interface

	// FReimportHandler interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	// End of FReimportHandler interface

	// Called before the asset is deleted in the editor
	void OnAssetsPreDelete(const TArray<UObject*>& AssetsToDelete);

private:
	// Display a dialog to enter import options
	void ShowImportOptionWindow(TSharedPtr<class SPDFImportOptions>& Options, const FString& Filename, class UPDFImportOptions* &Result);

	// Delete all corresponding texture assets of PDF asset
	bool DeletePageTextures(class UPDF* PdfToDelete);
};
