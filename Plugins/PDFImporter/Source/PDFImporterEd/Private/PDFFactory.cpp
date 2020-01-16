#include "PDFFactory.h"
#include "GhostscriptCore.h"
#include "PDF.h"
#include "HAL/FileManager.h"
#include "EditorFramework/AssetImportData.h"

UPDFFactory::UPDFFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UPDF::StaticClass();
	bEditorImport = true;
	bText = true;
	Formats.Add(TEXT("pdf;PDF File"));

	FPDFImporterModule& PDFImporterModule = FModuleManager::LoadModuleChecked<FPDFImporterModule>(FName("PDFImporter"));
	GhostscriptCore = PDFImporterModule.GetGhostscriptCore();
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
	UPDF* LoadedPDF = GhostscriptCore->ConvertPdfToPdfAsset(Filename, 150, 0, 0, TEXT("ja"), true);

	if (LoadedPDF != nullptr)
	{
		NewPDF->PageRange = LoadedPDF->PageRange;
		NewPDF->Dpi = LoadedPDF->Dpi;
		NewPDF->Pages = LoadedPDF->Pages;

#if WITH_EDITORONLY_DATA
		NewPDF->AssetImportData = NewObject<UAssetImportData>();
		NewPDF->AssetImportData->SourceData.Insert({ Filename, IFileManager::Get().GetTimeStamp(*Filename) });
		NewPDF->Filename = Filename;
		NewPDF->TimeStamp = IFileManager::Get().GetTimeStamp(*Filename);
#endif
	}

	return NewPDF;
}

bool UPDFFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UPDF* PDF = Cast<UPDF>(Obj);
	if (PDF && PDF->AssetImportData)
	{
		for (auto SourceFile : PDF->AssetImportData->SourceData.SourceFiles)
		{
			OutFilenames.Add(SourceFile.RelativeFilename);
		}
		
		return true;
	}

	return false;
}

void UPDFFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UPDF* PDF = Cast<UPDF>(Obj);
	if (PDF && ensure(NewReimportPaths.Num() == 1))
	{
		PDF->AssetImportData->SourceData.SourceFiles[0].RelativeFilename = NewReimportPaths[0];
	}
}

EReimportResult::Type UPDFFactory::Reimport(UObject* Obj)
{
	UPDF* PDF = Cast<UPDF>(Obj);
	if (!PDF)
	{
		return EReimportResult::Failed;
	}

	const FString Filename = PDF->AssetImportData->SourceData.SourceFiles[0].RelativeFilename;
	if (!Filename.Len() || IFileManager::Get().FileSize(*Filename) == INDEX_NONE)
	{
		return EReimportResult::Failed;
	}

	EReimportResult::Type Result = EReimportResult::Failed;
	if (UFactory::StaticImportObject(
		PDF->GetClass(), PDF->GetOuter(),
		*PDF->GetName(), RF_Public | RF_Standalone, *Filename, NULL, this))
	{
		if (PDF->GetOuter())
		{
			PDF->GetOuter()->MarkPackageDirty();
		}
		else
		{
			PDF->MarkPackageDirty();
		}

		return EReimportResult::Succeeded;
	}

	return EReimportResult::Failed;
}
