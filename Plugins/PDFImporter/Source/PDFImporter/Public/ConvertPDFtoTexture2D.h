// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "ConvertPDFtoTexture2D.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLoadingCompletedPin, class UPDF*, PDF);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFailedToLoadPin);

UCLASS()
class PDFIMPORTER_API UConvertPDFtoTexture2D : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	// Execution pin called when loading is complete
	UPROPERTY(BlueprintAssignable)
	FLoadingCompletedPin Completed;

	// Execution pin called when loading fails
	UPROPERTY(BlueprintAssignable)
	FFailedToLoadPin Failed;

private:
	TSharedPtr<class FGhostscriptCore> GhostscriptCore;
	const UObject* WorldContextObject;
	bool bIsActive;

	// For argument passing
	FString PDFFilePath;
	int Dpi;
	int FirstPage;
	int LastPage;
	FString Locale;

public:
	// Constructor
	UConvertPDFtoTexture2D(const FObjectInitializer& ObjectInitializer);

	// ConvertPDFtoTexture2D node
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = 3, BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Convert PDF to Texture2D"), Category = "PDFImporter")
	static UConvertPDFtoTexture2D* ConvertPDFtoTexture2D(
		const UObject* WorldContextObject, 
		const FString& PDFFilePath, 
		int Dpi = 150,
		int FirstPage = 0,
		int LastPage = 0,
		const FString& Locale = ""
	);

	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;

private:
	// Execute convert process
	void ExecConversion();
};