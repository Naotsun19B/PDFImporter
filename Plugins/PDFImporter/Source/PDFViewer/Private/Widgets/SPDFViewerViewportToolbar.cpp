// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Widgets/SPDFViewerViewportToolbar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "Menus/PDFViewerViewOptionsMenu.h"


#define LOCTEXT_NAMESPACE "SPDFViewerViewportToolbar"


/* SPDFViewerViewportToolbar interface
 *****************************************************************************/

void SPDFViewerViewportToolbar::Construct( const FArguments& InArgs, const TSharedRef<FUICommandList>& InToolkitCommands )
{
	ToolkitCommands = InToolkitCommands;

	bIsVolumeTexture = InArgs._IsVolumeTexture;

	ChildSlot
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(ViewOptionsMenuAnchor, SMenuAnchor)
					.OnGetMenuContent(this, &SPDFViewerViewportToolbar::GenerateViewOptionsMenu)
					.Placement(MenuPlacement_ComboBox)
					[
						SNew(SButton)
							.ClickMethod(EButtonClickMethod::MouseDown)
							.ContentPadding(FMargin(5.0f, 2.0f))
							.VAlign(VAlign_Center)
							.ButtonStyle(FEditorStyle::Get(), "ViewPortMenu.Button")
							.OnClicked(this, &SPDFViewerViewportToolbar::HandleViewOptionsMenuButtonClicked)
							[
								SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(STextBlock)
										.Text(LOCTEXT("ViewButtonText", "View"))
										.TextStyle(FEditorStyle::Get(), "ViewportMenu.Label")
									]

								+ SHorizontalBox::Slot()
									.AutoWidth()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									.Padding(2.0f)
									[
										SNew(SImage)
										.Image(FEditorStyle::GetBrush("ComboButton.Arrow"))
										.ColorAndOpacity(FLinearColor::Black)
									]
							]
					]
			]
	];
}


/* SPDFViewerViewportToolbar implementation
 *****************************************************************************/

TSharedRef<SWidget> SPDFViewerViewportToolbar::GenerateViewOptionsMenu( ) const
{
	FMenuBuilder MenuBuilder(true, ToolkitCommands);
	FPDFViewerViewOptionsMenu::MakeMenu(MenuBuilder, bIsVolumeTexture);

	return MenuBuilder.MakeWidget();
}


FReply SPDFViewerViewportToolbar::HandleViewOptionsMenuButtonClicked( )
{
	if (ViewOptionsMenuAnchor->ShouldOpenDueToClick())
	{
		ViewOptionsMenuAnchor->SetIsOpen(true);
	}
	else
	{
		ViewOptionsMenuAnchor->SetIsOpen(false);
	}

	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE
