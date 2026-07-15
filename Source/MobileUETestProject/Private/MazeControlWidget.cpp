// Fill out your copyright notice in the Description page of Project Settings.

#include "MazeControlWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

TSharedRef<SWidget> UMazeControlWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transactional);
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = RootCanvas;

	UBorder* PanelBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PanelBackground"));
	PanelBackground->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.8f));
	PanelBackground->SetPadding(FMargin(14.0f));

	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelBackground);
	PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
	PanelSlot->SetAlignment(FVector2D(0.0f, 0.0f));
	PanelSlot->SetPosition(FVector2D(24.0f, 24.0f));
	PanelSlot->SetAutoSize(false);
	PanelSlot->SetSize(FVector2D(320.0f, 470.0f));

	UVerticalBox* MainBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainBox"));
	PanelBackground->SetContent(MainBox);

	AddTitle(MainBox, TEXT("Maze Control"));

	WidthSlider = AddLabeledSlider(MainBox, WidthLabel, TEXT("WidthSlider"), TEXT("Width"), 2.0f, 60.0f);
	HeightSlider = AddLabeledSlider(MainBox, HeightLabel, TEXT("HeightSlider"), TEXT("Height"), 2.0f, 60.0f);
	StepIntervalSlider = AddLabeledSlider(MainBox, StepIntervalLabel, TEXT("StepIntervalSlider"), TEXT("Step Interval"), 0.0f, 0.5f);
	SeedSlider = AddLabeledSlider(MainBox, SeedLabel, TEXT("SeedSlider"), TEXT("Seed"), 0.0f, 9999.0f);

	FixedSeedCheckBox = AddLabeledCheckBox(MainBox, TEXT("FixedSeedCheckBox"), TEXT("Use Fixed Seed"));

	UHorizontalBox* ButtonRowTop = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ButtonRowTop"));
	UVerticalBoxSlot* ButtonRowTopSlot = MainBox->AddChildToVerticalBox(ButtonRowTop);
	ButtonRowTopSlot->SetPadding(FMargin(0.0f, 14.0f, 0.0f, 4.0f));
	ButtonRowTopSlot->SetHorizontalAlignment(HAlign_Fill);

	PlayButton = AddButton(ButtonRowTop, TEXT("PlayButton"), TEXT("Play"));
	PauseButton = AddButton(ButtonRowTop, TEXT("PauseButton"), TEXT("Pause"));

	UHorizontalBox* ButtonRowBottom = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ButtonRowBottom"));
	UVerticalBoxSlot* ButtonRowBottomSlot = MainBox->AddChildToVerticalBox(ButtonRowBottom);
	ButtonRowBottomSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
	ButtonRowBottomSlot->SetHorizontalAlignment(HAlign_Fill);

	StopButton = AddButton(ButtonRowBottom, TEXT("StopButton"), TEXT("Stop"));
	RestartButton = AddButton(ButtonRowBottom, TEXT("RestartButton"), TEXT("Restart"));

	ProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("ProgressBar"));
	UVerticalBoxSlot* ProgressSlot = MainBox->AddChildToVerticalBox(ProgressBar);
	ProgressSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 4.0f));
	ProgressSlot->SetHorizontalAlignment(HAlign_Fill);

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
	StatusText->SetJustification(ETextJustify::Center);
	UVerticalBoxSlot* StatusSlot = MainBox->AddChildToVerticalBox(StatusText);
	StatusSlot->SetHorizontalAlignment(HAlign_Fill);
	StatusSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));

	BindWidgetEvents();
	RefreshFromMazeActor();

	return Super::RebuildWidget();
}

void UMazeControlWidget::AddTitle(UVerticalBox* Parent, const FString& Title)
{
	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetText(FText::FromString(Title));
	TitleText->SetJustification(ETextJustify::Center);

	FSlateFontInfo Font = TitleText->GetFont();
	Font.Size = 20;
	TitleText->SetFont(Font);

	UVerticalBoxSlot* TitleSlot = Parent->AddChildToVerticalBox(TitleText);
	TitleSlot->SetHorizontalAlignment(HAlign_Fill);
	TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
}

USlider* UMazeControlWidget::AddLabeledSlider(UVerticalBox* Parent, TObjectPtr<UTextBlock>& OutLabel, FName WidgetName, const FString& LabelPrefix, float MinValue, float MaxValue)
{
	UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *(WidgetName.ToString() + TEXT("Label")));
	LabelText->SetText(FText::FromString(LabelPrefix));
	UVerticalBoxSlot* LabelSlot = Parent->AddChildToVerticalBox(LabelText);
	LabelSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	OutLabel = LabelText;

	USlider* Slider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), WidgetName);
	Slider->SetMinValue(MinValue);
	Slider->SetMaxValue(MaxValue);
	UVerticalBoxSlot* SliderSlot = Parent->AddChildToVerticalBox(Slider);
	SliderSlot->SetHorizontalAlignment(HAlign_Fill);

	return Slider;
}

UCheckBox* UMazeControlWidget::AddLabeledCheckBox(UVerticalBox* Parent, FName WidgetName, const FString& Label)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *(WidgetName.ToString() + TEXT("Row")));
	UVerticalBoxSlot* RowSlot = Parent->AddChildToVerticalBox(Row);
	RowSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));

	UCheckBox* CheckBox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), WidgetName);
	Row->AddChildToHorizontalBox(CheckBox);

	UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *(WidgetName.ToString() + TEXT("Label")));
	LabelText->SetText(FText::FromString(Label));
	UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(LabelText);
	LabelSlot->SetPadding(FMargin(6.0f, 0.0f, 0.0f, 0.0f));
	LabelSlot->SetVerticalAlignment(VAlign_Center);

	return CheckBox;
}

UButton* UMazeControlWidget::AddButton(UHorizontalBox* Parent, FName WidgetName, const FString& Label)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), WidgetName);

	UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *(WidgetName.ToString() + TEXT("Label")));
	ButtonText->SetText(FText::FromString(Label));
	ButtonText->SetJustification(ETextJustify::Center);

	FSlateFontInfo ButtonFont = ButtonText->GetFont();
	ButtonFont.Size = 12;
	ButtonText->SetFont(ButtonFont);

	Button->SetContent(ButtonText);

	UHorizontalBoxSlot* ButtonSlot = Parent->AddChildToHorizontalBox(Button);
	ButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
	ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
	ButtonSlot->SetVerticalAlignment(VAlign_Fill);
	ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	return Button;
}

void UMazeControlWidget::SetMazeActor(AMazeVisualizerActor* NewMazeActor)
{
	if (MazeActor)
	{
		MazeActor->OnPlayStateChanged.RemoveDynamic(this, &UMazeControlWidget::HandlePlayStateChanged);
	}

	MazeActor = NewMazeActor;

	if (MazeActor)
	{
		MazeActor->OnPlayStateChanged.AddUniqueDynamic(this, &UMazeControlWidget::HandlePlayStateChanged);
	}

	RefreshFromMazeActor();
}

void UMazeControlWidget::BindWidgetEvents()
{
	if (PlayButton)    PlayButton->OnClicked.AddUniqueDynamic(this, &UMazeControlWidget::HandlePlayClicked);
	if (PauseButton)   PauseButton->OnClicked.AddUniqueDynamic(this, &UMazeControlWidget::HandlePauseClicked);
	if (StopButton)    StopButton->OnClicked.AddUniqueDynamic(this, &UMazeControlWidget::HandleStopClicked);
	if (RestartButton) RestartButton->OnClicked.AddUniqueDynamic(this, &UMazeControlWidget::HandleRestartClicked);

	if (WidthSlider)        WidthSlider->OnValueChanged.AddUniqueDynamic(this, &UMazeControlWidget::HandleWidthChanged);
	if (HeightSlider)       HeightSlider->OnValueChanged.AddUniqueDynamic(this, &UMazeControlWidget::HandleHeightChanged);
	if (StepIntervalSlider) StepIntervalSlider->OnValueChanged.AddUniqueDynamic(this, &UMazeControlWidget::HandleStepIntervalChanged);
	if (SeedSlider)         SeedSlider->OnValueChanged.AddUniqueDynamic(this, &UMazeControlWidget::HandleSeedChanged);
	if (FixedSeedCheckBox)  FixedSeedCheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &UMazeControlWidget::HandleFixedSeedChanged);
}

void UMazeControlWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (MazeActor)
	{
		MazeActor->OnPlayStateChanged.AddUniqueDynamic(this, &UMazeControlWidget::HandlePlayStateChanged);
	}

	RefreshFromMazeActor();
}

void UMazeControlWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (MazeActor && ProgressBar)
	{
		ProgressBar->SetPercent(MazeActor->GetGenerationProgress());
	}
}

void UMazeControlWidget::HandlePlayClicked()
{
	if (MazeActor) MazeActor->Play();
}

void UMazeControlWidget::HandlePauseClicked()
{
	if (MazeActor) MazeActor->Pause();
}

void UMazeControlWidget::HandleStopClicked()
{
	if (MazeActor) MazeActor->Stop();
}

void UMazeControlWidget::HandleRestartClicked()
{
	if (MazeActor) MazeActor->Restart();
}

void UMazeControlWidget::HandleWidthChanged(float NewValue)
{
	const int32 Rounded = FMath::Max(2, FMath::RoundToInt(NewValue));

	if (MazeActor && MazeActor->MazeGenerator)
	{
		MazeActor->MazeGenerator->MazeWidth = Rounded;
	}

	if (WidthLabel)
	{
		WidthLabel->SetText(FText::FromString(FString::Printf(TEXT("Width: %d"), Rounded)));
	}
}

void UMazeControlWidget::HandleHeightChanged(float NewValue)
{
	const int32 Rounded = FMath::Max(2, FMath::RoundToInt(NewValue));

	if (MazeActor && MazeActor->MazeGenerator)
	{
		MazeActor->MazeGenerator->MazeHeight = Rounded;
	}

	if (HeightLabel)
	{
		HeightLabel->SetText(FText::FromString(FString::Printf(TEXT("Height: %d"), Rounded)));
	}
}

void UMazeControlWidget::HandleStepIntervalChanged(float NewValue)
{
	const float Clamped = FMath::Max(0.0f, NewValue);

	if (MazeActor)
	{
		MazeActor->StepInterval = Clamped;
	}

	if (StepIntervalLabel)
	{
		StepIntervalLabel->SetText(FText::FromString(FString::Printf(TEXT("Step Interval: %.2fs"), Clamped)));
	}
}

void UMazeControlWidget::HandleFixedSeedChanged(bool bIsChecked)
{
	if (MazeActor && MazeActor->MazeGenerator)
	{
		MazeActor->MazeGenerator->bUseFixedSeed = bIsChecked;
	}
}

void UMazeControlWidget::HandleSeedChanged(float NewValue)
{
	const int32 Rounded = FMath::RoundToInt(NewValue);

	if (MazeActor && MazeActor->MazeGenerator)
	{
		MazeActor->MazeGenerator->RandomSeed = Rounded;
	}

	if (SeedLabel)
	{
		SeedLabel->SetText(FText::FromString(FString::Printf(TEXT("Seed: %d"), Rounded)));
	}
}

void UMazeControlWidget::HandlePlayStateChanged(EMazePlayState NewState)
{
	RefreshStatusText(NewState);
}

void UMazeControlWidget::RefreshFromMazeActor()
{
	if (!MazeActor || !MazeActor->MazeGenerator)
	{
		return;
	}

	if (WidthSlider)        WidthSlider->SetValue((float)MazeActor->MazeGenerator->MazeWidth);
	if (HeightSlider)       HeightSlider->SetValue((float)MazeActor->MazeGenerator->MazeHeight);
	if (StepIntervalSlider) StepIntervalSlider->SetValue(MazeActor->StepInterval);
	if (FixedSeedCheckBox)  FixedSeedCheckBox->SetIsChecked(MazeActor->MazeGenerator->bUseFixedSeed);
	if (SeedSlider)         SeedSlider->SetValue((float)MazeActor->MazeGenerator->RandomSeed);

	if (WidthLabel)         WidthLabel->SetText(FText::FromString(FString::Printf(TEXT("Width: %d"), MazeActor->MazeGenerator->MazeWidth)));
	if (HeightLabel)        HeightLabel->SetText(FText::FromString(FString::Printf(TEXT("Height: %d"), MazeActor->MazeGenerator->MazeHeight)));
	if (StepIntervalLabel)  StepIntervalLabel->SetText(FText::FromString(FString::Printf(TEXT("Step Interval: %.2fs"), MazeActor->StepInterval)));
	if (SeedLabel)          SeedLabel->SetText(FText::FromString(FString::Printf(TEXT("Seed: %d"), MazeActor->MazeGenerator->RandomSeed)));

	RefreshStatusText(MazeActor->GetPlayState());
}

void UMazeControlWidget::RefreshStatusText(EMazePlayState State)
{
	if (!StatusText)
	{
		return;
	}

	switch (State)
	{
	case EMazePlayState::Stopped:   StatusText->SetText(FText::FromString(TEXT("Stopped")));   break;
	case EMazePlayState::Playing:   StatusText->SetText(FText::FromString(TEXT("Playing")));   break;
	case EMazePlayState::Paused:    StatusText->SetText(FText::FromString(TEXT("Paused")));    break;
	case EMazePlayState::Completed: StatusText->SetText(FText::FromString(TEXT("Completed"))); break;
	}
}
