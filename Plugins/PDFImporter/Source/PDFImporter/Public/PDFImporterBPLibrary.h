// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/ImageWrapper/Public/IImageWrapper.h"
#include "PDFImporterBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/
UCLASS()
class UPDFImporterBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	// Get the file path of the selected PDF file
	UFUNCTION(BlueprintCallable, Category = "PDFImporter | OpenFileDialog", meta = (AdvancedDisplay = "DefaultPath", DisplayName = "Open PDF Dialog"))
		static bool OpenPDFDialog(const FString& DefaultPath, FString& FileName);

	// Get the file paths of the selected PDF files
	UFUNCTION(BlueprintCallable, Category = "PDFImporter | OpenFileDialog", meta = (AdvancedDisplay = "DefaultPath", DisplayName = "Open PDF Dialog Multiple"))
		static bool OpenPDFDialogMultiple(const FString& DefaultPath, TArray<FString>& FileNames);

	// Get PDF page converted to Texture2D
	UFUNCTION(BlueprintCallable, Category = "PDFImporter | Convert", meta = (AdvancedDisplay = 2, DisplayName = "Convert PDF to Texture2D"))
		static bool ConvertPDFToTexture2D(TArray<UTexture2D*>& Pages, const FString& PDFFilePath, const FString& GhostscriptPath = TEXT("C:/Program Files/gs/gs9.27/bin/gswin64c.exe"), int32 Dpi = 150);

	// Convert PDF to JPG image with Ghostscript
	static bool ConvertPDFtoJPG(const FString& PDFFilePath, const FString& GhostscriptPath, const FString& OutDirPath, int32 Dpi = 150);
	
	// Import as UTexture2D from folder image file
	static bool LoadTexture2DFromFile(const FString& FilePath, EImageFormat ImageType, UTexture2D* &LoadedTexture);
};