// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PDF.generated.h"

USTRUCT(BlueprintType)
struct FPageRange
{
	GENERATED_BODY()

public:
	FPageRange(int InFirstPage, int InLastPage) : FirstPage(InFirstPage), LastPage(InLastPage) {}
	FPageRange() : FPageRange(0, 0) {}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PageRange")
	int FirstPage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PageRange")
	int LastPage;
};

UCLASS(BlueprintType)
class PDFIMPORTER_API UPDF : public UObject
{
	GENERATED_BODY()

public:
	// PDF page range
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PDF")
	FPageRange PageRange;

	// PDF resolution
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PDF")
	int Dpi;

	// PDF page textures
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PDF")
	TArray<class UTexture2D*> Pages;

	// Data for import setting
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Instanced, Category = "ImportSettings")
	class UAssetImportData* AssetImportData;
#endif
	UPROPERTY()
	FString Filename;

	UPROPERTY()
	FDateTime TimeStamp;

public:
	// Get the texture of the specified page
	UFUNCTION(BlueprintCallable, Category = "PDF")
	UTexture2D* GetPageTexture(int Page) const;

	// Get number of pages in PDF
	UFUNCTION(BlueprintCallable, Category = "PDF")
	int GetPageCount() const { return Pages.Num(); }

public:
	// UObject interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
#if WITH_EDITORONLY_DATA
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
#endif
	// End of UObject interface
};
