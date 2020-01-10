// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
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

UENUM()
enum class EOpenPDFDialogResult : uint8
{
	Successful, Cancelled
};

UCLASS()
class UPDFImporterBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	// Get the file path of the selected PDF file
	UFUNCTION(BlueprintCallable, Category = "PDFImporter", meta = (AdvancedDisplay = "DefaultPath", DisplayName = "Open PDF Dialog", ExpandEnumAsExecs = "OutputPin"))
	static void OpenPDFDialog(const FString& DefaultPath, EOpenPDFDialogResult& OutputPin, FString& FileName);

	// Get the file paths of the selected PDF files
	UFUNCTION(BlueprintCallable, Category = "PDFImporter", meta = (AdvancedDisplay = "DefaultPath", DisplayName = "Open PDF Dialog Multiple", ExpandEnumAsExecs = "OutputPin"))
	static void OpenPDFDialogMultiple(const FString& DefaultPath, EOpenPDFDialogResult& OutputPin, TArray<FString>& FileNames);

private:
	// Get window handle
	static void* GetWindowHandle();

	// Run open file dialog
	static EOpenPDFDialogResult ExecOpenPDFDialog(const FString& DefaultPath, TArray<FString>& FileNames, bool bIsMultiple);
};