// Fill out your copyright notice in the Description page of Project Settings.

#include "PDFImportOptions.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "PDFImportOptions"

SPDFImportOptions::SPDFImportOptions()
	: ImportOptions(nullptr)
	, bShouldImport(false)
{
}

void SPDFImportOptions::Construct(const FArguments& InArgs)
{
	ImportOptions = InArgs._ImportOptions;
	WidgetWindow = InArgs._WidgetWindow;

	check(ImportOptions)

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(ImportOptions);

	this->ChildSlot
	[
		SNew(SVerticalBox)
		// ファイルパス
		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			[
				SNew(SBorder)
				.Padding(FMargin(3))
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Font(FEditorStyle::GetFontStyle("CurveEd.LabelFont"))
						.Text(LOCTEXT("Import_CurrentFileTitle", "Current File: "))
					]
					+ SHorizontalBox::Slot()
					.Padding(5, 0, 0, 0)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Font(FEditorStyle::GetFontStyle("CurveEd.InfoFont"))
						.Text(InArgs._Filename)
					]
				]
			]
		// 入力欄
		+ SVerticalBox::Slot()
			.Padding(2)
			.MaxHeight(500.0f)
			[
				DetailsView->AsShared()
			]
		// インポートとキャンセルのボタン
		+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(2)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(2)
				+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(LOCTEXT("PDFImportOptions_Import", "Import"))
						.OnClicked(this, &SPDFImportOptions::OnImport)
					]

				+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(LOCTEXT("PDFImportOptions_Cancel", "Cancel"))
						.ToolTipText(LOCTEXT("PDFImportOptions_Cancel_ToolTip", "Cancels importing this PDF file"))
						.OnClicked(this, &SPDFImportOptions::OnCancel)
					]
			]
	];
}

FReply SPDFImportOptions::OnImport()
{
	bShouldImport = true;

	if (!ImportOptions->SpecifyPageRange)
	{
		ImportOptions->FirstPage = 0;
		ImportOptions->LastPage = 0;
	}

	if (WidgetWindow.IsValid())
	{
		WidgetWindow.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FReply SPDFImportOptions::OnCancel()
{
	bShouldImport = false;
	if (WidgetWindow.IsValid())
	{
		WidgetWindow.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

