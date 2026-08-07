// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MazeVisualizerActor.h"
#include "MazeMinimapWidget.generated.h"

class UImage;
class UBorder;
class UCanvasPanel;
class UCanvasPanelSlot;
class USlider;
class UTextBlock;

// Self-building minimap: displays AMazeVisualizerActor's top-down MinimapRenderTarget with a
// "you are here" icon that tracks the locally controlled pawn's position and facing. Builds its
// own UMG layout in C++ (RebuildWidget), so no UMG Designer work is required. Assign this class
// (or a Blueprint child of it) to AMazeVisualizerActor::MinimapWidgetClass and it just works.
//
// A Zoom slider (0.05-1.0) drives AMazeVisualizerActor::UpdateMinimapView() every tick: at 1.0 the
// whole maze is shown; as it's lowered the view zooms in and pans to follow the player, showing
// only that fraction of the maze's extent around them.
UCLASS()
class MOBILEUETESTPROJECT_API UMazeMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Actor this widget displays. Assign in the Blueprint details panel, or call SetMazeActor() at runtime.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyMaze", meta = (DisplayName = "Maze Actor"))
	TObjectPtr<AMazeVisualizerActor> MazeActor;

	// 1.0 = whole maze visible. 0.1 = only ~10% of the maze's extent, centered on the player, visible.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyMaze|Minimap", meta = (ClampMin = "0.05", ClampMax = "1.0", DisplayName = "Minimap Scale"))
	float MinimapScale = 1.0f;

	// Screen-space size (both edges) of the minimap panel, in pixels.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyMaze|Minimap", meta = (DisplayName = "Minimap Panel Size"))
	float MinimapPanelSize = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyMaze|Minimap", meta = (DisplayName = "Player Icon Color"))
	FLinearColor PlayerIconColor = FLinearColor(1.0f, 0.15f, 0.15f, 1.0f);

	// Length (pixels) of the facing indicator that sticks out from the player icon.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyMaze|Minimap", meta = (ClampMin = "0.0", DisplayName = "Player Direction Length"))
	float PlayerDirectionLength = 16.0f;

	UFUNCTION(BlueprintCallable, Category = "MyMaze")
	void SetMazeActor(AMazeVisualizerActor* NewMazeActor);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(Transient)
	TObjectPtr<UImage> MapImage;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> PlayerIconWidget;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanelSlot> PlayerIconSlot;

	// Thin bar pivoting at the player icon's center, rotated to the pawn's yaw - reads as a
	// needle/arrow sticking out of the dot in the direction the player is facing. (A rotating
	// square icon has no visible asymmetry, so the icon alone can't show facing - this does.)
	UPROPERTY(Transient)
	TObjectPtr<UBorder> PlayerDirectionIndicator;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanelSlot> PlayerDirectionSlot;

	UPROPERTY(Transient)
	TObjectPtr<USlider> ScaleSlider;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ScaleLabel;

	UFUNCTION()
	void HandleScaleChanged(float NewValue);

private:
	void RefreshMapBrush();
	void UpdatePlayerIcon();
	void RefreshScaleLabel();
};
