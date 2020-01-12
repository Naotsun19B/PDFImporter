#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GhostscriptCore.generated.h"

typedef int(*CreateAPIInstance)(void** Instance, void* CallerHandle);
typedef void(*DeleteAPIInstance)(void* Instance);
typedef int(*InitAPI)(void* Instance, int Argc, char** Argv);
typedef int(*ExitAPI)(void* Instance);

UCLASS()
class PDFIMPORTER_API UGhostscriptCore : public UObject
{
	GENERATED_BODY()

private:
	// Ghostscript module
	void* APIModule;

	// Ghostscript function pointers
	CreateAPIInstance CreateInstance;
	DeleteAPIInstance DeleteInstance;
	InitAPI Init;
	ExitAPI Exit;

	TSharedPtr<class IImageWrapper> ImageWrapper;

public:
	// Constructor
	UGhostscriptCore();

	// Destructor
	virtual void BeginDestroy() override;

	// Convert PDF to PDF asset
	class UPDF* ConvertPdfToPdfAsset(const FString& InputPath, int Dpi, int FirstPage, int LastPage, const FString& Locale);

private:
	// Convert PDF to multiple jpeg images using Ghostscript API
	bool ConvertPdfToJpeg(const FString& InputPath, const FString& OutputPath, int Dpi, int FirstPage, int LastPage, const FString& Locale);

	// Import as UTexture2D from folder image file
	bool LoadTexture2DFromFile(const FString& FilePath, class UTexture2D* &LoadedTexture);

	// Get the size of FString data
	int GetFStringSize(const FString& Text);

// ----- Singleton interface -----
protected:
	// One and only existence
	static UGhostscriptCore* Instance;

public:
	// Get instance of this class
	static UGhostscriptCore* Get();

	static void CreateModule();
	static void DestroyModule();
};
