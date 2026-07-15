// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "MazeVisualizerActor.h"
#include "MazeEditorUtilityWidget.generated.h"

class UButton;
class USlider;
class UCheckBox;
class UTextBlock;
class UVerticalBox;
class UHorizontalBox;

// Editor-only tool: adjust an AMazeVisualizerActor's parameters and regenerate it instantly
// without entering Play mode. Run via right-click "Run Editor Utility Widget" on this asset in
// the Content Browser. Select a MazeVisualizerActor in the level, click "Refresh From Selection",
// then adjust sliders and click "Generate Instantly".
//
// NOTE: this depends on UEditorUtilityWidget (Blutility/UMGEditor), which is only linked into this
// module for the Editor target (see MobileUETestProject.Build.cs). If the MobileUETestProject
// (packaged Game) target is ever built, move this class into its own editor-only module first.
UCLASS()
class MOBILEUETESTPROJECT_API UMazeEditorUtilityWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(Transient)
	TObjectPtr<AMazeVisualizerActor> TargetActor;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TargetActorText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> RefreshButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> GenerateButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ClearButton;

	UPROPERTY(Transient)
	TObjectPtr<USlider> WidthSlider;

	UPROPERTY(Transient)
	TObjectPtr<USlider> HeightSlider;

	UPROPERTY(Transient)
	TObjectPtr<USlider> CellSizeSlider;

	UPROPERTY(Transient)
	TObjectPtr<USlider> WallHeightSlider;

	UPROPERTY(Transient)
	TObjectPtr<USlider> WallThicknessSlider;

	UPROPERTY(Transient)
	TObjectPtr<USlider> SeedSlider;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> FixedSeedCheckBox;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WidthLabel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HeightLabel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CellSizeLabel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WallHeightLabel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WallThicknessLabel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SeedLabel;

	UFUNCTION()
	void HandleRefreshClicked();

	UFUNCTION()
	void HandleGenerateClicked();

	UFUNCTION()
	void HandleClearClicked();

	UFUNCTION()
	void HandleWidthChanged(float NewValue);

	UFUNCTION()
	void HandleHeightChanged(float NewValue);

	UFUNCTION()
	void HandleCellSizeChanged(float NewValue);

	UFUNCTION()
	void HandleWallHeightChanged(float NewValue);

	UFUNCTION()
	void HandleWallThicknessChanged(float NewValue);

	UFUNCTION()
	void HandleSeedChanged(float NewValue);

	UFUNCTION()
	void HandleFixedSeedChanged(bool bIsChecked);

private:
	void BindWidgetEvents();
	void RefreshFromTargetActor();
	void SetTargetActor(AMazeVisualizerActor* NewTarget);

	void AddTitle(UVerticalBox* Parent, const FString& Title);
	USlider* AddLabeledSlider(UVerticalBox* Parent, TObjectPtr<UTextBlock>& OutLabel, FName WidgetName, const FString& LabelPrefix, float MinValue, float MaxValue);
	UCheckBox* AddLabeledCheckBox(UVerticalBox* Parent, FName WidgetName, const FString& Label);
	UButton* AddButton(UHorizontalBox* Parent, FName WidgetName, const FString& Label);
};
