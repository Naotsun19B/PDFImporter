// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PDFImporter.h"
#include "GhostscriptCore.h"

#define LOCTEXT_NAMESPACE "FPDFImporterModule"

void FPDFImporterModule::StartupModule()
{
	GhostscriptCore = MakeShareable(new FGhostscriptCore());
}

void FPDFImporterModule::ShutdownModule()
{
	GhostscriptCore.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPDFImporterModule, PDFImporter)