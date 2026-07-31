// Fill out your copyright notice in the Description page of Project Settings.

#include "MazeMinimapWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

TSharedRef<SWidget> UMazeMinimapWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transactional);
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = RootCanvas;

	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MinimapFrame"));
	Frame->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.8f));
	Frame->SetPadding(FMargin(10.0f));

	UCanvasPanelSlot* FrameSlot = RootCanvas->AddChildToCanvas(Frame);
	FrameSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
	FrameSlot->SetAlignment(FVector2D(1.0f, 0.0f));
	FrameSlot->SetPosition(FVector2D(-24.0f, 24.0f));
	FrameSlot->SetAutoSize(true);

	UVerticalBox* MainBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MinimapMainBox"));
	Frame->SetContent(MainBox);

	USizeBox* MapSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MapSizeBox"));
	MapSizeBox->SetWidthOverride(MinimapPanelSize);
	MapSizeBox->SetHeightOverride(MinimapPanelSize);
	UVerticalBoxSlot* MapSizeBoxSlot = MainBox->AddChildToVerticalBox(MapSizeBox);
	MapSizeBoxSlot->SetHorizontalAlignment(HAlign_Fill);

	UCanvasPanel* MapCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MapCanvas"));
	MapSizeBox->SetContent(MapCanvas);

	MapImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MapImage"));
	UCanvasPanelSlot* MapImageSlot = MapCanvas->AddChildToCanvas(MapImage);
	MapImageSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	MapImageSlot->SetOffsets(FMargin(0.0f));

	PlayerIconWidget = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PlayerIconWidget"));
	PlayerIconWidget->SetBrushColor(PlayerIconColor);
	PlayerIconSlot = MapCanvas->AddChildToCanvas(PlayerIconWidget);
	PlayerIconSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
	PlayerIconSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	PlayerIconSlot->SetAutoSize(false);
	PlayerIconSlot->SetSize(FVector2D(10.0f, 10.0f));

	UHorizontalBox* ScaleRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ScaleRow"));
	UVerticalBoxSlot* ScaleRowSlot = MainBox->AddChildToVerticalBox(ScaleRow);
	ScaleRowSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	ScaleRowSlot->SetHorizontalAlignment(HAlign_Fill);

	ScaleLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ScaleLabel"));
	UHorizontalBoxSlot* ScaleLabelSlot = ScaleRow->AddChildToHorizontalBox(ScaleLabel);
	ScaleLabelSlot->SetVerticalAlignment(VAlign_Center);
	ScaleLabelSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));

	ScaleSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("ScaleSlider"));
	ScaleSlider->SetMinValue(0.05f);
	ScaleSlider->SetMaxValue(1.0f);
	ScaleSlider->SetValue(MinimapScale);
	UHorizontalBoxSlot* ScaleSliderSlot = ScaleRow->AddChildToHorizontalBox(ScaleSlider);
	ScaleSliderSlot->SetHorizontalAlignment(HAlign_Fill);
	ScaleSliderSlot->SetVerticalAlignment(VAlign_Center);
	ScaleSliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	ScaleSlider->OnValueChanged.AddUniqueDynamic(this, &UMazeMinimapWidget::HandleScaleChanged);

	RefreshScaleLabel();
	RefreshMapBrush();

	return Super::RebuildWidget();
}

void UMazeMinimapWidget::SetMazeActor(AMazeVisualizerActor* NewMazeActor)
{
	MazeActor = NewMazeActor;
	RefreshMapBrush();
}

void UMazeMinimapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!MazeActor)
	{
		return;
	}

	if (MapImage && MapImage->GetBrush().GetResourceObject() != MazeActor->MinimapRenderTarget)
	{
		RefreshMapBrush();
	}

	UpdatePlayerIcon();
}

void UMazeMinimapWidget::HandleScaleChanged(float NewValue)
{
	MinimapScale = FMath::Clamp(NewValue, 0.05f, 1.0f);
	RefreshScaleLabel();
}

void UMazeMinimapWidget::RefreshMapBrush()
{
	if (!MapImage || !MazeActor || !MazeActor->MinimapRenderTarget)
	{
		return;
	}

	FSlateBrush NewBrush;
	NewBrush.SetResourceObject(MazeActor->MinimapRenderTarget);
	NewBrush.ImageSize = FVector2D(MazeActor->MinimapRenderTarget->SizeX, MazeActor->MinimapRenderTarget->SizeY);
	NewBrush.DrawAs = ESlateBrushDrawType::Image;
	MapImage->SetBrush(NewBrush);
}

void UMazeMinimapWidget::UpdatePlayerIcon()
{
	if (!MazeActor)
	{
		return;
	}

	const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Pawn)
	{
		return;
	}

	const FVector PlayerLocation = Pawn->GetActorLocation();
	MazeActor->UpdateMinimapView(PlayerLocation, MinimapScale);

	if (PlayerIconSlot)
	{
		const FVector2D UV = MazeActor->WorldToMinimapUV(PlayerLocation);
		const FVector2D Clamped(FMath::Clamp(UV.X, 0.0f, 1.0f), FMath::Clamp(UV.Y, 0.0f, 1.0f));
		PlayerIconSlot->SetPosition(Clamped * MinimapPanelSize);
	}

	if (PlayerIconWidget)
	{
		PlayerIconWidget->SetRenderTransformAngle(Pawn->GetActorRotation().Yaw);
	}
}

void UMazeMinimapWidget::RefreshScaleLabel()
{
	if (ScaleLabel)
	{
		ScaleLabel->SetText(FText::FromString(FString::Printf(TEXT("Zoom: %d%%"), FMath::RoundToInt(MinimapScale * 100.0f))));
	}
}
