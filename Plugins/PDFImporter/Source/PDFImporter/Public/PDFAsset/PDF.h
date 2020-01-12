// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PDF.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PDFIMPORTER_API UPDF : public UObject
{
	GENERATED_BODY()
	
public:
	// PDF file name
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PDF")
	FString FileName;

	// PDF page textures
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PDF")
	TArray<class UTexture2D*> Pages;

	// PDF resolution
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PDF")
	int Dpi;

#if WITH_EDITORONLY_DATA
	// Data for re-import
	UPROPERTY(VisibleAnywhere, Instanced, Category = "Reimport")
	class UAssetImportData* AssetImportData;
#endif

private:
	FString FilePath;

public:
	// Get the texture of the specified page
	UFUNCTION(BlueprintCallable, Category = "PDF")
	UTexture2D* GetPageTexture(int Page) const;

	// Get number of pages in PDF
	UFUNCTION(BlueprintCallable, Category = "PDF")
	int GetPageCount() const { return Pages.Num(); }
};
