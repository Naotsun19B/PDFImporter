#include "PDFAsset/PDFFactory.h"
#include "PDFImporter.h"
#include "PDFAsset/PDF.h"

UPDFFactory::UPDFFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UPDF::StaticClass();
	bEditorImport = true;
	bText = true;
	Formats.Add(TEXT("pdf;PDF File"));
}

bool UPDFFactory::DoesSupportClass(UClass* Class)
{
	return (Class == UPDF::StaticClass());
}

UClass* UPDFFactory::ResolveSupportedClass()
{
	return UPDF::StaticClass();
}

UObject* UPDFFactory::FactoryCreateFile(
	UClass* InClass,
	UObject* InParent,
	FName InName,
	EObjectFlags Flags,
	const FString& Filename,
	const TCHAR* Parms,
	FFeedbackContext* Warn,
	bool& bOutOperationCanceled
)
{
	UPDF* NewPDF = CastChecked<UPDF>(StaticConstructObject_Internal(InClass, InParent, InName, Flags));
	FPDFImporterModule & PDFImporterModule = FModuleManager::GetModuleChecked<FPDFImporterModule>(FName("PDFImporter"));
	UPDF* LoadedPDF = PDFImporterModule.ConvertPdfToPdfAsset(Filename, 150, 0, 0, "ja");

	if (LoadedPDF != nullptr)
	{
		NewPDF->FileName = LoadedPDF->FileName;
		NewPDF->Pages = LoadedPDF->Pages;
		NewPDF->Dpi = LoadedPDF->Dpi;
	}

	return NewPDF;
}
