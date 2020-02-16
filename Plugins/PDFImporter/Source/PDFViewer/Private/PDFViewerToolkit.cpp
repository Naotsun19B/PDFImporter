// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PDFViewerToolkit.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Editor.h"
#include "Widgets/Layout/SBorder.h"
#include "Misc/FeedbackContext.h"
#include "Modules/ModuleManager.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "EditorStyleSet.h"
#include "EditorReimportHandler.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "Engine/LightMapTexture2D.h"
#include "Engine/ShadowMapTexture2D.h"
#include "Engine/Texture2DDynamic.h"
#include "Engine/TextureCube.h"
#include "Engine/VolumeTexture.h"
#include "Engine/TextureRenderTarget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureRenderTargetCube.h"
#include "Interfaces/IPDFViewerModule.h"
#include "PDFViewer.h"
#include "Slate/SceneViewport.h"
#include "PropertyEditorModule.h"
#include "PDFViewerConstants.h"
#include "Models/PDFViewerCommands.h"
#include "Widgets/SPDFViewerViewport.h"
#include "ISettingsModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "DeviceProfiles/DeviceProfile.h"
#include "Curves/CurveLinearColorAtlas.h"
#include "PDFViewerStyle.h"
#include "PDF.h"

#define LOCTEXT_NAMESPACE "FPDFViewerToolkit"

DEFINE_LOG_CATEGORY_STATIC(LogPDFViewer, Log, All);

#define MIPLEVEL_MIN 0
#define MIPLEVEL_MAX 15
#define EXPOSURE_MIN -10
#define EXPOSURE_MAX 10


const FName FPDFViewerToolkit::ViewportTabId(TEXT("PDFViewer_Viewport"));
const FName FPDFViewerToolkit::PropertiesTabId(TEXT("PDFViewer_Properties"));

UNREALED_API void GetBestFitForNumberOfTiles(int32 InSize, int32& OutRatioX, int32& OutRatioY);


/* FPDFViewerToolkit structors
 *****************************************************************************/

FPDFViewerToolkit::FPDFViewerToolkit()
	: Texture(nullptr)
	, VolumeOpacity(1.f)
	, VolumeOrientation(90, 0, -90)
{
	// Load styles used in pdf viewer
	Style = MakeShareable(new FPDFViewerStyle());
}

FPDFViewerToolkit::~FPDFViewerToolkit( )
{
	GEditor->UnregisterForUndo(this);
}


/* FAssetEditorToolkit interface
 *****************************************************************************/

FString FPDFViewerToolkit::GetDocumentationLink( ) const 
{
	return FString(TEXT("Engine/Content/Types/Textures/Properties/Interface"));
}


void FPDFViewerToolkit::RegisterTabSpawners( const TSharedRef<class FTabManager>& InTabManager )
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_PDFViewer", "Texture Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(ViewportTabId, FOnSpawnTab::CreateSP(this, &FPDFViewerToolkit::HandleTabSpawnerSpawnViewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(PropertiesTabId, FOnSpawnTab::CreateSP(this, &FPDFViewerToolkit::HandleTabSpawnerSpawnProperties))
		.SetDisplayName(LOCTEXT("PropertiesTab", "Details") )
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
}


void FPDFViewerToolkit::UnregisterTabSpawners( const TSharedRef<class FTabManager>& InTabManager )
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(ViewportTabId);
	InTabManager->UnregisterTabSpawner(PropertiesTabId);
}


void FPDFViewerToolkit::InitPDFViewer( const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit )
{
	PDF = CastChecked<UPDF>(ObjectToEdit);
	CurrentPage = 1;
	Texture = PDF->GetPageTexture(CurrentPage);

	// Support undo/redo
	Texture->SetFlags(RF_Transactional);
	GEditor->RegisterForUndo(this);

	// initialize view options
	bIsRedChannel = true;
	bIsGreenChannel = true;
	bIsBlueChannel = true;
	bIsAlphaChannel = false;

	switch (Texture->CompressionSettings)
	{
	default:
		bIsAlphaChannel = !Texture->CompressionNoAlpha;
		break;
	case TC_Normalmap:
	case TC_Grayscale:
	case TC_Displacementmap:
	case TC_VectorDisplacementmap:
	case TC_DistanceFieldFont:
		bIsAlphaChannel = false;
		break;
	}

	bIsDesaturation = false;

	SpecifiedMipLevel = 0;
	bUseSpecifiedMipLevel = false;

	SavedCompressionSetting = false;

	Zoom = 1.0f;

	// Register our commands. This will only register them if not previously registered
	FPDFViewerCommands::Register();

	BindCommands();
	CreateInternalWidgets();

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_PDFViewer_Layout_v3")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()
						->SetOrientation(Orient_Vertical)
						->SetSizeCoefficient(0.66f)
						->Split
						(
							FTabManager::NewStack()
								->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.1f)
								
						)
						->Split
						(
							FTabManager::NewStack()
								->AddTab(ViewportTabId, ETabState::OpenedTab)
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.9f)
						)
				)
				->Split
				(
					FTabManager::NewStack()
						->AddTab(PropertiesTabId, ETabState::OpenedTab)
						->SetSizeCoefficient(0.33f)
				)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, PDFViewerAppIdentifier, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectToEdit);
	
	IPDFViewerModule* PDFViewerModule = &FModuleManager::LoadModuleChecked<IPDFViewerModule>("PDFViewer");
	AddMenuExtender(PDFViewerModule->GetMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));

	ExtendToolBar();

	RegenerateMenusAndToolbars();

	// @todo toolkit world centric editing
	/*if(IsWorldCentricAssetEditor())
	{
		SpawnToolkitTab(GetToolbarTabId(), FString(), EToolkitTabSpot::ToolBar);
		SpawnToolkitTab(ViewportTabId, FString(), EToolkitTabSpot::Viewport);
		SpawnToolkitTab(PropertiesTabId, FString(), EToolkitTabSpot::Details);
	}*/
}


/* IPDFViewerToolkit interface
 *****************************************************************************/

void FPDFViewerToolkit::CalculateTextureDimensions( uint32& Width, uint32& Height ) const
{
	uint32 ImportedWidth = Texture->Source.GetSizeX();
	uint32 ImportedHeight = Texture->Source.GetSizeY();

	// if Original Width and Height are 0, use the saved current width and height
	if ((ImportedWidth == 0) && (ImportedHeight == 0))
	{
		ImportedWidth = Texture->GetSurfaceWidth();
		ImportedHeight = Texture->GetSurfaceHeight();
	}

	Width = ImportedWidth;
	Height = ImportedHeight;


	// catch if the Width and Height are still zero for some reason
	if ((Width == 0) || (Height == 0))
	{
		Width = 0;
		Height= 0;

		return;
	}

	// See if we need to uniformly scale it to fit in viewport
	// Cap the size to effective dimensions
	uint32 ViewportW = TextureViewport->GetViewport()->GetSizeXY().X;
	uint32 ViewportH = TextureViewport->GetViewport()->GetSizeXY().Y;
	uint32 MaxWidth; 
	uint32 MaxHeight;

	const bool bFitToViewport = GetFitToViewport();
	if (bFitToViewport)
	{
		const UVolumeTexture* VolumeTexture = Cast<UVolumeTexture>(Texture);

		// Subtract off the viewport space devoted to padding (2 * PreviewPadding)
		// so that the texture is padded on all sides
		MaxWidth = ViewportW;
		MaxHeight = ViewportH;

		if (IsCubeTexture())
		{
			// Cubes are displayed 2:1. 2x width if the source exists and is not an unwrapped image.
			const bool bMultipleSourceImages = Texture->Source.GetNumSlices() > 1;
			const bool bNoSourceImage = Texture->Source.GetNumSlices() == 0;
			Width *= (bNoSourceImage || bMultipleSourceImages) ? 2 : 1;
		}
		else if (VolumeTexture)
		{
			UPDFViewerSettings& Settings = *GetMutableDefault<UPDFViewerSettings>();
			if (Settings.VolumeViewMode == EPDFViewerVolumeViewMode::PDFViewerVolumeViewMode_VolumeTrace)
			{
				Width  = Height;
			}
			else
			{
				Width = (uint32)((float)Height * (float)PreviewEffectiveTextureWidth / (float)PreviewEffectiveTextureHeight);
			}
		}

		// First, scale up based on the size of the viewport
		if (MaxWidth > MaxHeight)
		{
			Height = Height * MaxWidth / Width;
			Width = MaxWidth;
		}
		else
		{
			Width = Width * MaxHeight / Height;
			Height = MaxHeight;
		}

		// then, scale again if our width and height is impacted by the scaling
		if (Width > MaxWidth)
		{
			Height = Height * MaxWidth / Width;
			Width = MaxWidth;
		}
		if (Height > MaxHeight)
		{
			Width = Width * MaxHeight / Height;
			Height = MaxHeight;
		}
	}
	else
	{
		Width = PreviewEffectiveTextureWidth * Zoom;
		Height = PreviewEffectiveTextureHeight * Zoom;
	}
}


ESimpleElementBlendMode FPDFViewerToolkit::GetColourChannelBlendMode( ) const
{
	if (Texture && (Texture->CompressionSettings == TC_Grayscale || Texture->CompressionSettings == TC_Alpha)) 
	{
		return SE_BLEND_Opaque;
	}

	// Add the red, green, blue, alpha and desaturation flags to the enum to identify the chosen filters
	uint32 Result = (uint32)SE_BLEND_RGBA_MASK_START;
	Result += bIsRedChannel ? (1 << 0) : 0;
	Result += bIsGreenChannel ? (1 << 1) : 0;
	Result += bIsBlueChannel ? (1 << 2) : 0;
	Result += bIsAlphaChannel ? (1 << 3) : 0;
	
	// If we only have one color channel active, enable color desaturation by default
	const int32 NumColorChannelsActive = (bIsRedChannel ? 1 : 0) + (bIsGreenChannel ? 1 : 0) + (bIsBlueChannel ? 1 : 0);
	const bool bIsDesaturationLocal = bIsDesaturation ? true : (NumColorChannelsActive==1);
	Result += bIsDesaturationLocal ? (1 << 4) : 0;

	return (ESimpleElementBlendMode)Result;
}


bool FPDFViewerToolkit::GetFitToViewport( ) const
{
	const UPDFViewerSettings& Settings = *GetDefault<UPDFViewerSettings>();
	return Settings.FitToViewport;
}


int32 FPDFViewerToolkit::GetMipLevel( ) const
{
	return GetUseSpecifiedMip() ? SpecifiedMipLevel : 0;
}


UTexture* FPDFViewerToolkit::GetTexture( ) const
{
	return Texture;
}


bool FPDFViewerToolkit::HasValidTextureResource( ) const
{
	return Texture != nullptr && Texture->Resource != nullptr;
}


bool FPDFViewerToolkit::GetUseSpecifiedMip( ) const
{
	return false;
}


double FPDFViewerToolkit::GetZoom( ) const
{
	return Zoom;
}


void FPDFViewerToolkit::PopulateQuickInfo( )
{
	UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
	UTextureRenderTarget2D* Texture2DRT = Cast<UTextureRenderTarget2D>(Texture);
	UTextureRenderTargetCube* TextureCubeRT = Cast<UTextureRenderTargetCube>(Texture);
	UTextureCube* TextureCube = Cast<UTextureCube>(Texture);
	UTexture2DDynamic* Texture2DDynamic = Cast<UTexture2DDynamic>(Texture);
	UVolumeTexture* VolumeTexture = Cast<UVolumeTexture>(Texture);

	const uint32 SurfaceWidth = (uint32)Texture->GetSurfaceWidth();
	const uint32 SurfaceHeight = (uint32)Texture->GetSurfaceHeight();
	const uint32 SurfaceDepth =  VolumeTexture ? (uint32)VolumeTexture->GetSizeZ() : 1;

	const uint32 ImportedWidth = FMath::Max<uint32>(SurfaceWidth, Texture->Source.GetSizeX());
	const uint32 ImportedHeight =  FMath::Max<uint32>(SurfaceHeight, Texture->Source.GetSizeY());
	const uint32 ImportedDepth =  FMath::Max<uint32>(SurfaceDepth, VolumeTexture ? Texture->Source.GetNumSlices() : 1);

	const int32 ActualMipBias = Texture2D ? (Texture2D->GetNumMips() - Texture2D->GetNumResidentMips()) : Texture->GetCachedLODBias();
	const uint32 ActualWidth = FMath::Max<uint32>(SurfaceWidth >> ActualMipBias, 1);
	const uint32 ActualHeight = FMath::Max<uint32>(SurfaceHeight >> ActualMipBias, 1);
	const uint32 ActualDepth =  FMath::Max<uint32>(SurfaceDepth >> ActualMipBias, 1);

	// Editor dimensions (takes user specified mip setting into account)
	const int32 MipLevel = GetMipLevel();
	PreviewEffectiveTextureWidth = FMath::Max<uint32>(ActualWidth >> MipLevel, 1);
	PreviewEffectiveTextureHeight = FMath::Max<uint32>(ActualHeight >> MipLevel, 1);;
	uint32 PreviewEffectiveTextureDepth = FMath::Max<uint32>(ActualDepth >> MipLevel, 1);

	// In game max bias and dimensions
	const int32 MaxResMipBias = Texture2D ? (Texture2D->GetNumMips() - Texture2D->GetNumMipsAllowed(true)) : Texture->GetCachedLODBias();
	const uint32 MaxInGameWidth = FMath::Max<uint32>(SurfaceWidth >> MaxResMipBias, 1);
	const uint32 MaxInGameHeight = FMath::Max<uint32>(SurfaceHeight >> MaxResMipBias, 1);
	const uint32 MaxInGameDepth = FMath::Max<uint32>(SurfaceDepth >> MaxResMipBias, 1);

	// Texture asset size
	const uint32 Size = (Texture->GetResourceSizeBytes(EResourceSizeMode::Exclusive) + 512) / 1024;

	FNumberFormattingOptions SizeOptions;
	SizeOptions.UseGrouping = false;
	SizeOptions.MaximumFractionalDigits = 0;

	// Cubes are previewed as unwrapped 2D textures.
	// These have 2x the width of a cube face.
	PreviewEffectiveTextureWidth *= IsCubeTexture() ? 2 : 1;

	FNumberFormattingOptions Options;
	Options.UseGrouping = false;


	if (VolumeTexture)
	{
		ImportedText->SetText(FText::Format( NSLOCTEXT("PDFViewer", "QuickInfo_Imported_3x", "Imported: {0}x{1}x{2}"), FText::AsNumber(ImportedWidth, &Options), FText::AsNumber(ImportedHeight, &Options), FText::AsNumber(ImportedDepth, &Options)));
		CurrentText->SetText(FText::Format( NSLOCTEXT("PDFViewer", "QuickInfo_Displayed_3x", "Displayed: {0}x{1}x{2}"), FText::AsNumber(PreviewEffectiveTextureWidth, &Options ), FText::AsNumber(PreviewEffectiveTextureHeight, &Options), FText::AsNumber(PreviewEffectiveTextureDepth, &Options)));
		MaxInGameText->SetText(FText::Format( NSLOCTEXT("PDFViewer", "QuickInfo_MaxInGame_3x", "Max In-Game: {0}x{1}x{2}"), FText::AsNumber(MaxInGameWidth, &Options), FText::AsNumber(MaxInGameHeight, &Options), FText::AsNumber(MaxInGameDepth, &Options)));

		UPDFViewerSettings& Settings = *GetMutableDefault<UPDFViewerSettings>();
		if (Settings.VolumeViewMode == EPDFViewerVolumeViewMode::PDFViewerVolumeViewMode_VolumeTrace)
		{
			PreviewEffectiveTextureWidth = PreviewEffectiveTextureHeight = FMath::Max(PreviewEffectiveTextureWidth, PreviewEffectiveTextureHeight);
		}
		else
		{
			int32 NumTilesX = 0;
			int32 NumTilesY = 0;
			GetBestFitForNumberOfTiles(PreviewEffectiveTextureDepth, NumTilesX, NumTilesY);
			PreviewEffectiveTextureWidth *= (uint32)NumTilesX;
			PreviewEffectiveTextureHeight *= (uint32)NumTilesY;
		}
	}
	else
	{
	    FText CubemapAdd;
	    if(TextureCube)
	    {
		    CubemapAdd = NSLOCTEXT("PDFViewer", "QuickInfo_PerCubeSide", "x6 (CubeMap)");
	    }
    
	    ImportedText->SetText(FText::Format( NSLOCTEXT("PDFViewer", "QuickInfo_Imported_2x", "Imported: {0}x{1}"), FText::AsNumber(ImportedWidth, &Options), FText::AsNumber(ImportedHeight, &Options)));
		CurrentText->SetText(FText::Format( NSLOCTEXT("PDFViewer", "QuickInfo_Displayed_2x", "Displayed: {0}x{1}{2}"), FText::AsNumber(PreviewEffectiveTextureWidth, &Options ), FText::AsNumber(PreviewEffectiveTextureHeight, &Options), CubemapAdd));
		MaxInGameText->SetText(FText::Format( NSLOCTEXT("PDFViewer", "QuickInfo_MaxInGame_2x", "Max In-Game: {0}x{1}{2}"), FText::AsNumber(MaxInGameWidth, &Options), FText::AsNumber(MaxInGameHeight, &Options), CubemapAdd));
	}

	SizeText->SetText(FText::Format(NSLOCTEXT("PDFViewer", "QuickInfo_ResourceSize", "Resource Size: {0} Kb"), FText::AsNumber(Size, &SizeOptions)));
	MethodText->SetText(FText::Format(NSLOCTEXT("PDFViewer", "QuickInfo_Method", "Method: {0}"), Texture->NeverStream ? NSLOCTEXT("PDFViewer", "QuickInfo_MethodNotStreamed", "Not Streamed") : NSLOCTEXT("PDFViewer", "QuickInfo_MethodStreamed", "Streamed")));
	LODBiasText->SetText(FText::Format(NSLOCTEXT("PDFViewer", "QuickInfo_LODBias", "Combined LOD Bias: {0}"), FText::AsNumber(Texture->GetCachedLODBias())));

	int32 TextureFormatIndex = PF_MAX;
	
	if (Texture2D)
	{
		TextureFormatIndex = Texture2D->GetPixelFormat();
	}
	else if (TextureCube)
	{
		TextureFormatIndex = TextureCube->GetPixelFormat();
	}
	else if (Texture2DRT)
	{
		TextureFormatIndex = Texture2DRT->GetFormat();
	}
	else if (Texture2DDynamic)
	{
		TextureFormatIndex = Texture2DDynamic->Format;
	}
	else if (VolumeTexture)
	{
		TextureFormatIndex = VolumeTexture->GetPixelFormat();
	}

	if (TextureFormatIndex != PF_MAX)
	{
		FormatText->SetText(FText::Format(NSLOCTEXT("PDFViewer", "QuickInfo_Format", "Format: {0}"), FText::FromString(GPixelFormats[TextureFormatIndex].Name)));
	}

	int32 NumMips = 1;
	if (Texture2D)
	{
		NumMips = Texture2D->GetNumMips();
	}
	else if (TextureCube)
	{
		NumMips = TextureCube->GetNumMips();
	}
	else if (Texture2DRT)
	{
		NumMips = Texture2DRT->GetNumMips();
	}
	else if (Texture2DDynamic)
	{
		NumMips = Texture2DDynamic->NumMips;
	}
	else if (VolumeTexture)
	{
		NumMips = VolumeTexture->GetNumMips();
	}

	NumMipsText->SetText(FText::Format(NSLOCTEXT("PDFViewer", "QuickInfo_NumMips", "Number of Mips: {0}"), FText::AsNumber(NumMips)));

	if (Texture2D)
	{
		HasAlphaChannelText->SetText(FText::Format(NSLOCTEXT("PDFViewer", "QuickInfo_HasAlphaChannel", "Has Alpha Channel: {0}"),
			Texture2D->HasAlphaChannel() ? NSLOCTEXT("PDFViewer", "True", "True") : NSLOCTEXT("PDFViewer", "False", "False")));
	}

	HasAlphaChannelText->SetVisibility(Texture2D ? EVisibility::Visible : EVisibility::Collapsed);
}


void FPDFViewerToolkit::SetFitToViewport( const bool bFitToViewport )
{
	UPDFViewerSettings& Settings = *GetMutableDefault<UPDFViewerSettings>();
	Settings.FitToViewport = bFitToViewport;
	Settings.PostEditChange();
}


void FPDFViewerToolkit::SetZoom( double ZoomValue )
{
	Zoom = FMath::Clamp(ZoomValue, MinZoom, MaxZoom);
	SetFitToViewport(false);
}


void FPDFViewerToolkit::ZoomIn( )
{
	SetZoom(Zoom + ZoomStep);
}


void FPDFViewerToolkit::ZoomOut( )
{
	SetZoom(Zoom - ZoomStep);
}

float FPDFViewerToolkit::GetVolumeOpacity() const
{
	return VolumeOpacity;
}

void FPDFViewerToolkit::SetVolumeOpacity(float ZoomValue)
{
	VolumeOpacity = FMath::Clamp(ZoomValue, 0.f, 1.f);
}

const FRotator& FPDFViewerToolkit::GetVolumeOrientation() const
{
	return VolumeOrientation;
}

void FPDFViewerToolkit::SetVolumeOrientation(const FRotator& InOrientation)
{
	VolumeOrientation = InOrientation;
}

/* IToolkit interface
 *****************************************************************************/

FText FPDFViewerToolkit::GetBaseToolkitName( ) const
{
	return LOCTEXT("AppLabel", "PDF Viewer");
}


FName FPDFViewerToolkit::GetToolkitFName( ) const
{
	return FName("PDFViewer");
}


FLinearColor FPDFViewerToolkit::GetWorldCentricTabColorScale( ) const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}


FString FPDFViewerToolkit::GetWorldCentricTabPrefix( ) const
{
	return LOCTEXT("WorldCentricTabPrefix", "PDF ").ToString();
}


/* FGCObject interface
 *****************************************************************************/

void FPDFViewerToolkit::AddReferencedObjects( FReferenceCollector& Collector )
{
	Collector.AddReferencedObject(Texture);
	TextureViewport->AddReferencedObjects(Collector);
}


/* FEditorUndoClient interface
 *****************************************************************************/

void FPDFViewerToolkit::PostUndo( bool bSuccess )
{
}


void FPDFViewerToolkit::PostRedo( bool bSuccess )
{
	PostUndo(bSuccess);
}


/* FPDFViewerToolkit implementation
 *****************************************************************************/

void FPDFViewerToolkit::BindCommands( )
{
	const FPDFViewerCommands& Commands = FPDFViewerCommands::Get();

	ToolkitCommands->MapAction(
		Commands.RedChannel,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleRedChannelActionExecute),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FPDFViewerToolkit::HandleRedChannelActionIsChecked));

	ToolkitCommands->MapAction(
		Commands.GreenChannel,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleGreenChannelActionExecute),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FPDFViewerToolkit::HandleGreenChannelActionIsChecked));

	ToolkitCommands->MapAction(
		Commands.BlueChannel,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleBlueChannelActionExecute),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FPDFViewerToolkit::HandleBlueChannelActionIsChecked));

	ToolkitCommands->MapAction(
		Commands.AlphaChannel,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleAlphaChannelActionExecute),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FPDFViewerToolkit::HandleAlphaChannelActionIsChecked));

	ToolkitCommands->MapAction(
		Commands.Desaturation,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleDesaturationChannelActionExecute),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FPDFViewerToolkit::HandleDesaturationChannelActionIsChecked));

	ToolkitCommands->MapAction(
		Commands.FitToViewport,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleFitToViewportActionExecute),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FPDFViewerToolkit::HandleFitToViewportActionIsChecked));

	ToolkitCommands->MapAction(
		Commands.CheckeredBackground,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleCheckeredBackgroundActionExecute, PDFViewerBackground_Checkered),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FPDFViewerToolkit::HandleCheckeredBackgroundActionIsChecked, PDFViewerBackground_Checkered));

	ToolkitCommands->MapAction(
		Commands.CheckeredBackgroundFill,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleCheckeredBackgroundActionExecute, PDFViewerBackground_CheckeredFill),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FPDFViewerToolkit::HandleCheckeredBackgroundActionIsChecked, PDFViewerBackground_CheckeredFill));

	ToolkitCommands->MapAction(
		Commands.SolidBackground,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleCheckeredBackgroundActionExecute, PDFViewerBackground_SolidColor),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FPDFViewerToolkit::HandleCheckeredBackgroundActionIsChecked, PDFViewerBackground_SolidColor));

	// Begin - Volume Texture Specifics
	ToolkitCommands->MapAction(
		Commands.DepthSlices,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleVolumeViewModeActionExecute, PDFViewerVolumeViewMode_DepthSlices),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FPDFViewerToolkit::HandleVolumeViewModeActionIsChecked, PDFViewerVolumeViewMode_DepthSlices));

	ToolkitCommands->MapAction(
		Commands.TraceIntoVolume,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleVolumeViewModeActionExecute, PDFViewerVolumeViewMode_VolumeTrace),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FPDFViewerToolkit::HandleVolumeViewModeActionIsChecked, PDFViewerVolumeViewMode_VolumeTrace));
	// End - Volume Texture Specifics

	ToolkitCommands->MapAction(
		Commands.TextureBorder,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleTextureBorderActionExecute),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FPDFViewerToolkit::HandleTextureBorderActionIsChecked));

	ToolkitCommands->MapAction(
		Commands.BackPage,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleBackPage),
		FCanExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleIsBackPageButtonEnable));

	ToolkitCommands->MapAction(
		Commands.NextPage,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleNextPage),
		FCanExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleIsNextPageButtonEnable));

	ToolkitCommands->MapAction(
		Commands.Settings,
		FExecuteAction::CreateSP(this, &FPDFViewerToolkit::HandleSettingsActionExecute));
}


TSharedRef<SWidget> FPDFViewerToolkit::BuildTexturePropertiesWidget( )
{
	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TexturePropertiesWidget = PropertyModule.CreateDetailView(Args);
	TexturePropertiesWidget->SetObject(PDF);

	return TexturePropertiesWidget.ToSharedRef();
}

void FPDFViewerToolkit::CreateInternalWidgets( )
{
	TextureViewport = SNew(SPDFViewerViewport, SharedThis(this));

	TextureProperties = SNew(SVerticalBox)

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(2.0f)
	[
		SNew(SBorder)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(ImportedText, STextBlock)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(CurrentText, STextBlock)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(MaxInGameText, STextBlock)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(SizeText, STextBlock)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(HasAlphaChannelText, STextBlock)
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(MethodText, STextBlock)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(FormatText, STextBlock)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(LODBiasText, STextBlock)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(4.0f)
				[
					SAssignNew(NumMipsText, STextBlock)
				]
			]
		]
	]

	+ SVerticalBox::Slot()
	.FillHeight(1.0f)
	.Padding(2.0f)
	[
		SNew(SBorder)
		.Padding(4.0f)
		[
			BuildTexturePropertiesWidget()
		]
	];
}


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FPDFViewerToolkit::ExtendToolBar( )
{
	TSharedRef<SWidget> PageControl = SNew(SBox)
		.WidthOverride(100.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.MaxWidth(100.0f)
			.Padding(0.0f, 0.0f, 0.0f, 0.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.MaxHeight(50.f)
				.Padding(5.0f, 0.0f, 5.0f, 5.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SNumericEntryBox<int32>)
					.AllowSpin(true)
					.MinSliderValue(1)
					.MaxSliderValue(PDF->GetPageCount())
					.Value(this, &FPDFViewerToolkit::HandleCurrentPageEntryBoxValue)
					.OnValueChanged(this, &FPDFViewerToolkit::HandleCurrentPageEntryBoxChanged)
					.ToolTipText(LOCTEXT("PageControl_ToolTip", "Go to a page by entering the number of pages."))
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.MaxHeight(50.f)
				.Padding(0.0f, 0.0f, 0.0f, 0.0f)
				.VAlign(VAlign_Bottom)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PageControl_Text", "Current Page"))
					.Justification(ETextJustify::Center)
				]
			]
		];

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FPDFViewerToolkit::FillToolbar, GetToolkitCommands(), PageControl)
	);

	AddToolbarExtender(ToolbarExtender);

	IPDFViewerModule* PDFViewerModule = &FModuleManager::LoadModuleChecked<IPDFViewerModule>("PDFViewer");
	AddToolbarExtender(PDFViewerModule->GetToolBarExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));
}

void FPDFViewerToolkit::FillToolbar(FToolBarBuilder& ToolbarBuilder, const TSharedRef< FUICommandList > InToolkitCommands, TSharedRef<SWidget> PageControl)
{
	UCurveLinearColorAtlas* Atlas = Cast<UCurveLinearColorAtlas>(GetTexture());
	if (!Atlas)
	{
		ToolbarBuilder.BeginSection("PdfMisc");
		{
			ToolbarBuilder.AddToolBarButton(FPDFViewerCommands::Get().BackPage, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(TEXT("PdfViewerStyle"), TEXT("PDFViewer.BackButton")));
			ToolbarBuilder.AddToolBarButton(FPDFViewerCommands::Get().NextPage, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(TEXT("PdfViewerStyle"), TEXT("PDFViewer.NextButton")));
		}
		ToolbarBuilder.EndSection();

		ToolbarBuilder.BeginSection("PdfPageControl");
		{
			ToolbarBuilder.AddWidget(PageControl);
		}
		ToolbarBuilder.EndSection();
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION


TOptional<int32> FPDFViewerToolkit::GetMaxMipLevel( ) const
{
	return MIPLEVEL_MAX;
}


bool FPDFViewerToolkit::IsCubeTexture( ) const
{
	return (Texture->IsA(UTextureCube::StaticClass()) || Texture->IsA(UTextureRenderTargetCube::StaticClass()));
}


/* FPDFViewerToolkit callbacks
 *****************************************************************************/

bool FPDFViewerToolkit::HandleAlphaChannelActionCanExecute( ) const
{
	const UTexture2D* Texture2D = Cast<UTexture2D>(Texture);

	if (Texture2D == NULL)
	{
		return false;
	}

	return Texture2D->HasAlphaChannel();
}


void FPDFViewerToolkit::HandleAlphaChannelActionExecute( )
{
	bIsAlphaChannel = !bIsAlphaChannel;
}


bool FPDFViewerToolkit::HandleAlphaChannelActionIsChecked( ) const
{
	return bIsAlphaChannel;
}


void FPDFViewerToolkit::HandleBlueChannelActionExecute( )
{
	 bIsBlueChannel = !bIsBlueChannel;
}


bool FPDFViewerToolkit::HandleBlueChannelActionIsChecked( ) const
{
	return bIsBlueChannel;
}


void FPDFViewerToolkit::HandleCheckeredBackgroundActionExecute( EPDFViewerBackgrounds Background )
{
	UPDFViewerSettings& Settings = *GetMutableDefault<UPDFViewerSettings>();
	Settings.Background = Background;
	Settings.PostEditChange();
}


bool FPDFViewerToolkit::HandleCheckeredBackgroundActionIsChecked( EPDFViewerBackgrounds Background )
{
	const UPDFViewerSettings& Settings = *GetDefault<UPDFViewerSettings>();

	return (Background == Settings.Background);
}

// Callback for toggling the volume display action.
void FPDFViewerToolkit::HandleVolumeViewModeActionExecute(EPDFViewerVolumeViewMode InViewMode)
{
	UPDFViewerSettings& Settings = *GetMutableDefault<UPDFViewerSettings>();
	Settings.VolumeViewMode = InViewMode;
	Settings.PostEditChange();
}

// Callback for getting the checked state of the volume display action.
bool FPDFViewerToolkit::HandleVolumeViewModeActionIsChecked(EPDFViewerVolumeViewMode InViewMode)
{
	const UPDFViewerSettings& Settings = *GetDefault<UPDFViewerSettings>();

	return (InViewMode == Settings.VolumeViewMode);
}


void FPDFViewerToolkit::HandleFitToViewportActionExecute( )
{
	ToggleFitToViewport();
}


bool FPDFViewerToolkit::HandleFitToViewportActionIsChecked( ) const
{
	return GetFitToViewport();
}


void FPDFViewerToolkit::HandleGreenChannelActionExecute( )
{
	 bIsGreenChannel = !bIsGreenChannel;
}


bool FPDFViewerToolkit::HandleGreenChannelActionIsChecked( ) const
{
	return bIsGreenChannel;
}


void FPDFViewerToolkit::HandleRedChannelActionExecute( )
{
	bIsRedChannel = !bIsRedChannel;
}


bool FPDFViewerToolkit::HandleRedChannelActionIsChecked( ) const
{
	return bIsRedChannel;
}


void FPDFViewerToolkit::HandleDesaturationChannelActionExecute( )
{
	bIsDesaturation = !bIsDesaturation;
}


bool FPDFViewerToolkit::HandleDesaturationChannelActionIsChecked( ) const
{
	return bIsDesaturation;
}


void FPDFViewerToolkit::HandleSettingsActionExecute( )
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Editor", "ContentEditors", "PDFViewer");
}


TSharedRef<SDockTab> FPDFViewerToolkit::HandleTabSpawnerSpawnProperties( const FSpawnTabArgs& Args )
{
	check(Args.GetTabId() == PropertiesTabId);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("TextureEditor.Tabs.Properties"))
		.Label(LOCTEXT("TexturePropertiesTitle", "Details"))
		[
			TextureProperties.ToSharedRef()
		];

	PopulateQuickInfo();

	return SpawnedTab;
}


TSharedRef<SDockTab> FPDFViewerToolkit::HandleTabSpawnerSpawnViewport( const FSpawnTabArgs& Args )
{
	check(Args.GetTabId() == ViewportTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("TextureViewportTitle", "Viewport"))
		[
			TextureViewport.ToSharedRef()
		];
}


void FPDFViewerToolkit::HandleTextureBorderActionExecute( )
{
	UPDFViewerSettings& Settings = *GetMutableDefault<UPDFViewerSettings>();
	Settings.TextureBorderEnabled = !Settings.TextureBorderEnabled;
	Settings.PostEditChange();
}


bool FPDFViewerToolkit::HandleTextureBorderActionIsChecked( ) const
{
	const UPDFViewerSettings& Settings = *GetDefault<UPDFViewerSettings>();

	return Settings.TextureBorderEnabled;
}


void FPDFViewerToolkit::HandleCurrentPageEntryBoxChanged(int32 NewPageCount)
{
	if (NewPageCount >= 1 && NewPageCount <= PDF->GetPageCount())
	{
		CurrentPage = NewPageCount;
		Texture = PDF->GetPageTexture(CurrentPage);
	}
}


TOptional<int32> FPDFViewerToolkit::HandleCurrentPageEntryBoxValue() const
{
	return CurrentPage;
}


void FPDFViewerToolkit::HandleBackPage()
{
	CurrentPage--;
	Texture = PDF->GetPageTexture(CurrentPage);
}


void FPDFViewerToolkit::HandleNextPage()
{
	CurrentPage++;
	Texture = PDF->GetPageTexture(CurrentPage);
}


bool FPDFViewerToolkit::HandleIsBackPageButtonEnable() const
{
	return CurrentPage > 1;
}


bool FPDFViewerToolkit::HandleIsNextPageButtonEnable() const
{
	return CurrentPage < PDF->GetPageCount();
}


#undef LOCTEXT_NAMESPACE
