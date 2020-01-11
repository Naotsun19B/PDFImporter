// Fill out your copyright notice in the Description page of Project Settings.

#include "PDFAsset/PDF.h"
#include "PDFImporter.h"

UTexture2D* UPDF::GetPageTexture(int Page) const
{
	if (Page < 1)
	{
		Page = 1;
		UE_LOG(PDFImporter, Warning, TEXT("Specified page is 0 or less"));
	}
	else if (Page > Pages.Num())
	{
		Page = Pages.Num();
		UE_LOG(PDFImporter, Warning, TEXT("The specified page exceeds the number of pages in the PDF"));
	}

	return Pages[Page - 1];
}
