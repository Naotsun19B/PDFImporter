// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "EditorUndoClient.h"
#include "Toolkits/IToolkitHost.h"
#include "Interfaces/IPDFViewerToolkit.h"
#include "IDetailsView.h"
#include "PDFViewerSettings.h"

class SDockableTab;
class STextBlock;
class SPDFViewerViewport;
class UFactory;
class UTexture;

/**
 * Implements an Editor toolkit for textures.
 */
class FPDFViewerToolkit
	: public IPDFViewerToolkit
	, public FEditorUndoClient
	, public FGCObject
{
public:
	FPDFViewerToolkit();

	/**
	 * Destructor.
	 */
	virtual ~FPDFViewerToolkit( );

public:

	/**
	 * Edits the specified Texture object.
	 *
	 * @param Mode The tool kit mode.
	 * @param InitToolkitHost 
	 * @param ObjectToEdit The texture object to edit.
	 */
	void InitPDFViewer( const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UObject* ObjectToEdit );

public:

	// FAssetEditorToolkit interface

	virtual FString GetDocumentationLink( ) const override;
	virtual void RegisterTabSpawners( const TSharedRef<class FTabManager>& TabManager ) override;
	virtual void UnregisterTabSpawners( const TSharedRef<class FTabManager>& TabManager ) override;

public:

	// IPDFViewerToolkit interface

	virtual void CalculateTextureDimensions( uint32& Width, uint32& Height ) const override;
	virtual ESimpleElementBlendMode GetColourChannelBlendMode( ) const override;
	virtual bool GetFitToViewport( ) const override;
	virtual int32 GetMipLevel( ) const override;
	virtual UTexture* GetTexture( ) const override;
	virtual bool HasValidTextureResource( ) const override;
	virtual bool GetUseSpecifiedMip( ) const override;
	virtual double GetZoom( ) const override;
	virtual void PopulateQuickInfo( ) override;
	virtual void SetFitToViewport( const bool bFitToViewport ) override;
	virtual void SetZoom( double ZoomValue ) override;
	virtual void ZoomIn( ) override;
	virtual void ZoomOut( ) override;
	virtual float GetVolumeOpacity( ) const override;
	virtual void SetVolumeOpacity( float ZoomValue ) override;
	virtual const FRotator& GetVolumeOrientation( ) const override;
	virtual void SetVolumeOrientation( const FRotator& InOrientation ) override;

public:

	// IToolkit interface

	virtual FText GetBaseToolkitName( ) const override;
	virtual FName GetToolkitFName( ) const override;
	virtual FLinearColor GetWorldCentricTabColorScale( ) const override;
	virtual FString GetWorldCentricTabPrefix( ) const override;

public:

	// FGCObject interface

	virtual void AddReferencedObjects( FReferenceCollector& Collector ) override;
	
protected:

	// FEditorUndoClient interface

	virtual void PostUndo( bool bSuccess ) override;
	virtual void PostRedo( bool bSuccess ) override;

protected:

	/**
	 * Binds the UI commands to delegates.
	 */
	void BindCommands( );

	/**
	 * Creates the texture properties details widget.
	 *
	 * @return The widget.
	 */
	TSharedRef<SWidget> BuildTexturePropertiesWidget( );

	/**
	 * Creates all internal widgets for the tabs to point at.
	 */
	void CreateInternalWidgets( );

	/**
	 * Builds the toolbar widget for the Texture editor.
	 */
	void ExtendToolBar( );

	void FillToolbar(FToolBarBuilder& ToolbarBuilder, const TSharedRef< FUICommandList > ToolkitCommands, TSharedRef<SWidget> PageControl);
	/**
	 * Gets the highest mip map level that this texture supports.
	 *
	 * @return Mip map level.
	 */
	TOptional<int32> GetMaxMipLevel( ) const;

	/**
	 * Checks whether the texture being edited is a cube map texture.
	 */
	bool IsCubeTexture( ) const;

private:

	// Callback for toggling the Alpha channel action.
	void HandleAlphaChannelActionExecute( );

	// Callback for getting the checked state of the Alpha channel action.
	bool HandleAlphaChannelActionIsChecked( ) const;

	// Callback for getting the enabled state of the Alpha channel action.
	bool HandleAlphaChannelActionCanExecute( ) const;

	// Callback for toggling the Blue channel action.
	void HandleBlueChannelActionExecute( );

	// Callback for getting the checked state of the Blue channel action.
	bool HandleBlueChannelActionIsChecked( ) const;

	// Callback for toggling the Checkered Background action.
	void HandleCheckeredBackgroundActionExecute( EPDFViewerBackgrounds Background );

	// Callback for getting the checked state of the Checkered Background action.
	bool HandleCheckeredBackgroundActionIsChecked( EPDFViewerBackgrounds Background );

	// Callback for toggling the volume display action.
	void HandleVolumeViewModeActionExecute( EPDFViewerVolumeViewMode InViewMode );

	// Callback for getting the checked state of the volume display action.
	bool HandleVolumeViewModeActionIsChecked( EPDFViewerVolumeViewMode InViewMode );

	// Callback for toggling the Fit To Viewport action.
	void HandleFitToViewportActionExecute( );

	// Callback for getting the checked state of the Fit To Viewport action.
	bool HandleFitToViewportActionIsChecked( ) const;

	// Callback for toggling the Green channel action.
	void HandleGreenChannelActionExecute( );

	// Callback for getting the checked state of the Green Channel action.
	bool HandleGreenChannelActionIsChecked( ) const;

	// Callback for toggling the Red channel action.
	void HandleRedChannelActionExecute( );

	// Callback for getting the checked state of the Red Channel action.
	bool HandleRedChannelActionIsChecked( ) const;

	// Callback for toggling the Desaturation channel action.
	void HandleDesaturationChannelActionExecute( );

	// Callback for getting the checked state of the Desaturation channel action.
	bool HandleDesaturationChannelActionIsChecked( ) const;

	// Callback for determining whether the Settings action can execute.
	void HandleSettingsActionExecute( );

	// Callback for spawning the Properties tab.
	TSharedRef<SDockTab> HandleTabSpawnerSpawnProperties( const FSpawnTabArgs& Args );

	// Callback for spawning the Viewport tab.
	TSharedRef<SDockTab> HandleTabSpawnerSpawnViewport( const FSpawnTabArgs& Args );

	// Callback for toggling the Texture Border action.
	void HandleTextureBorderActionExecute( );

	// Callback for getting the checked state of the Texture Border action.
	bool HandleTextureBorderActionIsChecked( ) const;

	// Callback that updates the current page
	void HandleCurrentPageEntryBoxChanged(int32 NewPageCount);

	// Callback that returns the current page
	TOptional<int32> HandleCurrentPageEntryBoxValue() const;

	// Callback that fires when the back page button is pressed.
	void HandleBackPage();

	// Callback that fires when the next page button is pressed.
	void HandleNextPage();

	// Back page button determines activation
	bool HandleIsBackPageButtonEnable() const;

	// Next page button determines activation
	bool HandleIsNextPageButtonEnable() const;

private:
	/** The pdf asset being inspected **/
	class UPDF* PDF;

	/** Number of pages currently viewed **/
	int32 CurrentPage;

	/** The Texture asset being inspected */
	UTexture* Texture;

	/** Style used in pdf viewer **/
	class TSharedPtr<class FPDFViewerStyle> Style;

	/** List of open tool panels; used to ensure only one exists at any one time */
	TMap<FName, TWeakPtr<SDockableTab>> SpawnedToolPanels;

	/** Viewport */
	TSharedPtr<SPDFViewerViewport> TextureViewport;

	/** Properties tab */
	TSharedPtr<SVerticalBox> TextureProperties;

	/** Properties tree view */
	TSharedPtr<class IDetailsView> TexturePropertiesWidget;

	/** Quick info text blocks */
	TSharedPtr<STextBlock> ImportedText;
	TSharedPtr<STextBlock> CurrentText;
	TSharedPtr<STextBlock> MaxInGameText;
	TSharedPtr<STextBlock> SizeText;
	TSharedPtr<STextBlock> MethodText;
	TSharedPtr<STextBlock> FormatText;
	TSharedPtr<STextBlock> LODBiasText;
	TSharedPtr<STextBlock> HasAlphaChannelText;
	TSharedPtr<STextBlock> NumMipsText;

	/** If true, displays the red channel */
	bool bIsRedChannel;

	/** If true, displays the green channel */
	bool bIsGreenChannel;

	/** If true, displays the blue channel */
	bool bIsBlueChannel;

	/** If true, displays the alpha channel */
	bool bIsAlphaChannel;

	/** If true, desaturates the texture */
	bool bIsDesaturation;

	/** The maximum width/height at which the texture will render in the preview window */
	uint32 PreviewEffectiveTextureWidth;
	uint32 PreviewEffectiveTextureHeight;

	/** Which mip level should be shown */
	int32 SpecifiedMipLevel;
	/* When true, the specified mip value is used. Top mip is used when false.*/
	bool bUseSpecifiedMipLevel;

	/** During re-import, cache this setting so it can be restored if necessary */
	bool SavedCompressionSetting;

	/** The texture's zoom factor. */
	double Zoom;

	// For volume texture, defines an opacity to see through the volume when tracing.
	float VolumeOpacity;

	// For volume texture, the orientation when tracing.
	FRotator VolumeOrientation;

private:

	// The name of the Viewport tab.
	static const FName ViewportTabId;

	// The name of the Properties tab.
	static const FName PropertiesTabId;
};
