#pragma once

#include "CoreMinimal.h"
#include "PDFImporter.h"

typedef int(*CreateAPIInstance)(void** Instance, void* CallerHandle);
typedef void(*DeleteAPIInstance)(void* Instance);
typedef int(*InitAPI)(void* Instance, int Argc, char** Argv);
typedef int(*ExitAPI)(void* Instance);

class PDFIMPORTER_API FGhostscriptCore
{
private:
	// Ghostscript module
	void* GhostscriptModule;

	// Ghostscript function pointers
	CreateAPIInstance CreateInstance;
	DeleteAPIInstance DeleteInstance;
	InitAPI Init;
	ExitAPI Exit;

	TSharedPtr<class IImageWrapper> ImageWrapper;

public:
	// Convert PDF to PDF asset
	class UPDF* ConvertPdfToPdfAsset(const FString& InputPath, int Dpi, int FirstPage, int LastPage, const FString& Locale);

private:
	// Convert PDF to multiple jpeg images using Ghostscript API
	bool ConvertPdfToJpeg(const FString& InputPath, const FString& OutputPath, int Dpi, int FirstPage, int LastPage, const FString& Locale);

	// Import as UTexture2D from folder image file
	bool LoadTexture2DFromFile(const FString& FilePath, class UTexture2D* &LoadedTexture);

	// Get the size of FString data
	int GetFStringSize(const FString& Text);

private:
	// Only PDFImporterModule can create and destroy instances
	friend FPDFImporterModule;

	FGhostscriptCore();

public:
	// Destructor is open because it is managed by TSharedPtr
	~FGhostscriptCore();
};
