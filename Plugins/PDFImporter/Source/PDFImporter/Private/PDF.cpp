// Fill out your copyright notice in the Description page of Project Settings.

#include "PDF.h"
#include "GhostscriptCore.h"
#include "Misc/Paths.h"
#include "Serialization/CustomVersion.h"
#include "EditorFramework/AssetImportData.h"

static const int Version = 1;
static const FGuid GUID(2020, 1, 13, 16);
static FCustomVersionRegistration RegisterPDFCustomVersion(GUID, Version, TEXT("PDFVersion"));

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

	Ar.UsingCustomVersion(GUID);
	if (Ar.IsSaving() || (Ar.IsLoading() && (Version <= Ar.CustomVer(GUID))))
	{
		Ar << SourceDirectory << PageRange.FirstPage << PageRange.LastPage << Dpi;

		if (Ar.IsLoading())
		{
			FPDFImporterModule& PDFImporterModule = FModuleManager::LoadModuleChecked<FPDFImporterModule>(FName("PDFImporter"));
			TSharedPtr<FGhostscriptCore> GhostscriptCore = PDFImporterModule.GetGhostscriptCore();

			FString DirectoryFullPath = FPaths::Combine(FPaths::ProjectPluginsDir(), SourceDirectory);
			DirectoryFullPath = FPaths::ConvertRelativePathToFull(DirectoryFullPath);

			GhostscriptCore->LoadTexture2DsFromDirectory(DirectoryFullPath, Pages);
		}
	}
}

