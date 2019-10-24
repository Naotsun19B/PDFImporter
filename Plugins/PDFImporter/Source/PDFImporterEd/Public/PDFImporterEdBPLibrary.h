// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PDFImporter/Public/PDFImporterBPLibrary.h"
#include "PDFImporterEdBPLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PDFIMPORTERED_API UPDFImporterEdBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Get the file path of the selected PDF file (editor only)
	UFUNCTION(BlueprintCallable, Category = "PDFImporterEd | OpenFileDialog", meta = (AdvancedDisplay = "DefaultPath", DisplayName = "Open PDF Dialog (Editor Only)", ExpandEnumAsExecs = "OutputPin"))
		static void OpenPDFDialogEd(const FString& DefaultPath, EOpenPDFDialogResult& OutputPin, FString& FileName);
};
