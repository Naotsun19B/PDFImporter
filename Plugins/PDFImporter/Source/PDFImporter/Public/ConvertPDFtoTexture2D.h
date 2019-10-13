// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Runtime/ImageWrapper/Public/IImageWrapper.h"
#include "ConvertPDFtoTexture2D.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLoadingCompletedPin, const TArray<UTexture2D*>&, Pages);
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
	const UObject* mWorldContextObject;
	bool bIsActive;
	TSharedPtr<IImageWrapper> mImageWrapper;

	// For argument passing
	FString mPDFFilePath;
	FString mGhostscriptPath;
	int32 mDpi;

public:
	// Constructor
	UConvertPDFtoTexture2D(const FObjectInitializer& ObjectInitializer);

	// ConvertPDFtoTexture2D node
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = 2, BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Convert PDF to Texture2D"), Category = "PDFImporter")
		static UConvertPDFtoTexture2D* ConvertPDFtoTexture2D(
			const UObject* WorldContextObject, 
			const FString& PDFFilePath, 
			const FString& GhostscriptPath = TEXT("C:/Program Files/gs/gs9.27/bin/gswin64c.exe"), 
			int32 Dpi = 150
		);

	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;

private:
	// Execute ConvertPDFtoJPG function
	void ExecConversion();

	// Convert PDF to JPG image with Ghostscript
	bool ConvertPDFtoJPG(const FString& PDFFilePath, const FString& GhostscriptPath, const FString& OutDirPath, int32 Dpi);

	// Import as UTexture2D from folder image file
	bool LoadTexture2DFromFile(const FString& FilePath, UTexture2D* &LoadedTexture);
};