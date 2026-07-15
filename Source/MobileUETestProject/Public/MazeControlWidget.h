// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MazeVisualizerActor.h"
#include "MazeControlWidget.generated.h"

class UButton;
class USlider;
class UCheckBox;
class UTextBlock;
class UProgressBar;
class UVerticalBox;
class UHorizontalBox;

// Self-building maze control panel: it constructs its own UMG layout in C++ (RebuildWidget),
// so no UMG Designer work is required. Assign this class (or a Blueprint child of it) to
// AMazeVisualizerActor::ControlWidgetClass and it just works.
UCLASS()
class MOBILEUETESTPROJECT_API UMazeControlWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Actor this widget drives. Assign in the Blueprint details panel, or call SetMazeActor() at runtime.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze")
	TObjectPtr<AMazeVisualizerActor> MazeActor;

	UFUNCTION(BlueprintCallable, Category = "Maze")
	void SetMazeActor(AMazeVisualizerActor* NewMazeActor);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PlayButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PauseButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> StopButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(Transient)
	TObjectPtr<USlider> WidthSlider;

	UPROPERTY(Transient)
	TObjectPtr<USlider> HeightSlider;

	UPROPERTY(Transient)
	TObjectPtr<USlider> StepIntervalSlider;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> FixedSeedCheckBox;

	UPROPERTY(Transient)
	TObjectPtr<USlider> SeedSlider;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WidthLabel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HeightLabel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StepIntervalLabel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SeedLabel;

	UFUNCTION()
	void HandlePlayClicked();

	UFUNCTION()
	void HandlePauseClicked();

	UFUNCTION()
	void HandleStopClicked();

	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleWidthChanged(float NewValue);

	UFUNCTION()
	void HandleHeightChanged(float NewValue);

	UFUNCTION()
	void HandleStepIntervalChanged(float NewValue);

	UFUNCTION()
	void HandleFixedSeedChanged(bool bIsChecked);

	UFUNCTION()
	void HandleSeedChanged(float NewValue);

	UFUNCTION()
	void HandlePlayStateChanged(EMazePlayState NewState);

private:
	void BindWidgetEvents();
	void RefreshFromMazeActor();
	void RefreshStatusText(EMazePlayState State);

	// Layout helpers used by RebuildWidget() to construct the UI purely in code.
	void AddTitle(UVerticalBox* Parent, const FString& Title);
	USlider* AddLabeledSlider(UVerticalBox* Parent, UTextBlock*& OutLabel, FName WidgetName, const FString& LabelPrefix, float MinValue, float MaxValue);
	UCheckBox* AddLabeledCheckBox(UVerticalBox* Parent, FName WidgetName, const FString& Label);
	UButton* AddButton(UHorizontalBox* Parent, FName WidgetName, const FString& Label);
};
