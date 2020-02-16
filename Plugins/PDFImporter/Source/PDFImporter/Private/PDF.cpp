// Fill out your copyright notice in the Description page of Project Settings.

#include "PDF.h"
#include "PDFImporter.h"
#include "Misc/Paths.h"
#include "Engine//Texture2D.h"
#include "Serialization/CustomVersion.h"

#if WITH_EDITORONLY_DATA
#include "EditorFramework/AssetImportData.h"
#endif

static const int PDF_Version = 1;
static const FGuid PDF_GUID(2020, 1, 13, 16);
static FCustomVersionRegistration RegisterPDFCustomVersion(PDF_GUID, PDF_Version, TEXT("PDFVersion"));

UTexture2D* UPDF::GetPageTexture(int Page) const
{
	if (Page < 1)
	{
		Page = 1;
		UE_LOG(PDFImporter, Warning, TEXT("Specified page is 0 or less"));
	}
	else if (Page > Pages.Num())
	{
		Page = Pages.Num();
		UE_LOG(PDFImporter, Warning, TEXT("The specified page exceeds the number of pages in the PDF"));
	}

	return Pages[Page - 1];
}

void UPDF::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(PDF_GUID);
	if (Ar.IsSaving() || (Ar.IsLoading() && (PDF_Version <= Ar.CustomVer(PDF_GUID))))
	{
		Ar << PageRange.FirstPage << PageRange.LastPage << Dpi << Pages << Filename << TimeStamp;
	}
}

void UPDF::PostInitProperties()
{
#if WITH_EDITORONLY_DATA
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
#endif
	Super::PostInitProperties();
}

void UPDF::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITORONLY_DATA
	if (AssetImportData == nullptr)
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
		AssetImportData->SourceData.Insert({ Filename, TimeStamp });
	}
#endif
}

#if WITH_EDITORONLY_DATA
void UPDF::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	if (AssetImportData)
	{
		OutTags.Add(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}
	
	Super::GetAssetRegistryTags(OutTags);
}
#endif
