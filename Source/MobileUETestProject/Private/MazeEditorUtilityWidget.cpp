// Fill out your copyright notice in the Description page of Project Settings.

#include "MazeEditorUtilityWidget.h"
#include "Editor.h"
#include "Selection.h"
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

TSharedRef<SWidget> UMazeEditorUtilityWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transactional);
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = RootCanvas;

	UBorder* PanelBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PanelBackground"));
	PanelBackground->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.9f));
	PanelBackground->SetPadding(FMargin(14.0f));

	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelBackground);
	PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
	PanelSlot->SetAlignment(FVector2D(0.0f, 0.0f));
	PanelSlot->SetPosition(FVector2D(0.0f, 0.0f));
	PanelSlot->SetAutoSize(false);
	PanelSlot->SetSize(FVector2D(360.0f, 560.0f));

	UVerticalBox* MainBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainBox"));
	PanelBackground->SetContent(MainBox);

	AddTitle(MainBox, TEXT("Maze Editor Tools"));

	TargetActorText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TargetActorText"));
	TargetActorText->SetText(FText::FromString(TEXT("No target actor - select one and click Refresh")));
	TargetActorText->SetAutoWrapText(true);
	UVerticalBoxSlot* TargetTextSlot = MainBox->AddChildToVerticalBox(TargetActorText);
	TargetTextSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));

	UHorizontalBox* RefreshRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RefreshRow"));
	UVerticalBoxSlot* RefreshRowSlot = MainBox->AddChildToVerticalBox(RefreshRow);
	RefreshRowSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	RefreshRowSlot->SetHorizontalAlignment(HAlign_Fill);
	RefreshButton = AddButton(RefreshRow, TEXT("RefreshButton"), TEXT("Refresh From Selection"));

	WidthSlider = AddLabeledSlider(MainBox, WidthLabel, TEXT("WidthSlider"), TEXT("Width"), 2.0f, 60.0f);
	HeightSlider = AddLabeledSlider(MainBox, HeightLabel, TEXT("HeightSlider"), TEXT("Height"), 2.0f, 60.0f);
	CellSizeSlider = AddLabeledSlider(MainBox, CellSizeLabel, TEXT("CellSizeSlider"), TEXT("Cell Size"), 50.0f, 1000.0f);
	WallHeightSlider = AddLabeledSlider(MainBox, WallHeightLabel, TEXT("WallHeightSlider"), TEXT("Wall Height"), 50.0f, 1000.0f);
	WallThicknessSlider = AddLabeledSlider(MainBox, WallThicknessLabel, TEXT("WallThicknessSlider"), TEXT("Wall Thickness"), 5.0f, 100.0f);
	SeedSlider = AddLabeledSlider(MainBox, SeedLabel, TEXT("SeedSlider"), TEXT("Seed"), 0.0f, 9999.0f);

	FixedSeedCheckBox = AddLabeledCheckBox(MainBox, TEXT("FixedSeedCheckBox"), TEXT("Use Fixed Seed"));

	UHorizontalBox* ActionRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ActionRow"));
	UVerticalBoxSlot* ActionRowSlot = MainBox->AddChildToVerticalBox(ActionRow);
	ActionRowSlot->SetPadding(FMargin(0.0f, 14.0f, 0.0f, 4.0f));
	ActionRowSlot->SetHorizontalAlignment(HAlign_Fill);

	GenerateButton = AddButton(ActionRow, TEXT("GenerateButton"), TEXT("Generate Instantly"));
	ClearButton = AddButton(ActionRow, TEXT("ClearButton"), TEXT("Clear"));

	BindWidgetEvents();
	RefreshFromTargetActor();

	return Super::RebuildWidget();
}

void UMazeEditorUtilityWidget::AddTitle(UVerticalBox* Parent, const FString& Title)
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

USlider* UMazeEditorUtilityWidget::AddLabeledSlider(UVerticalBox* Parent, TObjectPtr<UTextBlock>& OutLabel, FName WidgetName, const FString& LabelPrefix, float MinValue, float MaxValue)
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

UCheckBox* UMazeEditorUtilityWidget::AddLabeledCheckBox(UVerticalBox* Parent, FName WidgetName, const FString& Label)
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

UButton* UMazeEditorUtilityWidget::AddButton(UHorizontalBox* Parent, FName WidgetName, const FString& Label)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), WidgetName);

	UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *(WidgetName.ToString() + TEXT("Label")));
	ButtonText->SetText(FText::FromString(Label));
	ButtonText->SetJustification(ETextJustify::Center);
	ButtonText->SetAutoWrapText(true);

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

void UMazeEditorUtilityWidget::BindWidgetEvents()
{
	if (RefreshButton) RefreshButton->OnClicked.AddUniqueDynamic(this, &UMazeEditorUtilityWidget::HandleRefreshClicked);
	if (GenerateButton) GenerateButton->OnClicked.AddUniqueDynamic(this, &UMazeEditorUtilityWidget::HandleGenerateClicked);
	if (ClearButton) ClearButton->OnClicked.AddUniqueDynamic(this, &UMazeEditorUtilityWidget::HandleClearClicked);

	if (WidthSlider)         WidthSlider->OnValueChanged.AddUniqueDynamic(this, &UMazeEditorUtilityWidget::HandleWidthChanged);
	if (HeightSlider)        HeightSlider->OnValueChanged.AddUniqueDynamic(this, &UMazeEditorUtilityWidget::HandleHeightChanged);
	if (CellSizeSlider)      CellSizeSlider->OnValueChanged.AddUniqueDynamic(this, &UMazeEditorUtilityWidget::HandleCellSizeChanged);
	if (WallHeightSlider)    WallHeightSlider->OnValueChanged.AddUniqueDynamic(this, &UMazeEditorUtilityWidget::HandleWallHeightChanged);
	if (WallThicknessSlider) WallThicknessSlider->OnValueChanged.AddUniqueDynamic(this, &UMazeEditorUtilityWidget::HandleWallThicknessChanged);
	if (SeedSlider)          SeedSlider->OnValueChanged.AddUniqueDynamic(this, &UMazeEditorUtilityWidget::HandleSeedChanged);
	if (FixedSeedCheckBox)   FixedSeedCheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &UMazeEditorUtilityWidget::HandleFixedSeedChanged);
}

void UMazeEditorUtilityWidget::SetTargetActor(AMazeVisualizerActor* NewTarget)
{
	TargetActor = NewTarget;
	RefreshFromTargetActor();
}

void UMazeEditorUtilityWidget::HandleRefreshClicked()
{
	if (!GEditor)
	{
		return;
	}

	TArray<AActor*> SelectedActors;
	GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);

	for (AActor* Actor : SelectedActors)
	{
		if (AMazeVisualizerActor* MazeActor = Cast<AMazeVisualizerActor>(Actor))
		{
			SetTargetActor(MazeActor);
			return;
		}
	}

	SetTargetActor(nullptr);
}

void UMazeEditorUtilityWidget::HandleGenerateClicked()
{
	if (TargetActor)
	{
		TargetActor->GenerateInstantly();
	}
}

void UMazeEditorUtilityWidget::HandleClearClicked()
{
	if (TargetActor)
	{
		TargetActor->Stop();
	}
}

void UMazeEditorUtilityWidget::HandleWidthChanged(float NewValue)
{
	const int32 Rounded = FMath::Max(2, FMath::RoundToInt(NewValue));
	if (TargetActor && TargetActor->MazeGenerator)
	{
		TargetActor->MazeGenerator->MazeWidth = Rounded;
	}
	if (WidthLabel)
	{
		WidthLabel->SetText(FText::FromString(FString::Printf(TEXT("Width: %d"), Rounded)));
	}
}

void UMazeEditorUtilityWidget::HandleHeightChanged(float NewValue)
{
	const int32 Rounded = FMath::Max(2, FMath::RoundToInt(NewValue));
	if (TargetActor && TargetActor->MazeGenerator)
	{
		TargetActor->MazeGenerator->MazeHeight = Rounded;
	}
	if (HeightLabel)
	{
		HeightLabel->SetText(FText::FromString(FString::Printf(TEXT("Height: %d"), Rounded)));
	}
}

void UMazeEditorUtilityWidget::HandleCellSizeChanged(float NewValue)
{
	if (TargetActor)
	{
		TargetActor->CellSize = NewValue;
	}
	if (CellSizeLabel)
	{
		CellSizeLabel->SetText(FText::FromString(FString::Printf(TEXT("Cell Size: %.0f"), NewValue)));
	}
}

void UMazeEditorUtilityWidget::HandleWallHeightChanged(float NewValue)
{
	if (TargetActor)
	{
		TargetActor->WallHeight = NewValue;
	}
	if (WallHeightLabel)
	{
		WallHeightLabel->SetText(FText::FromString(FString::Printf(TEXT("Wall Height: %.0f"), NewValue)));
	}
}

void UMazeEditorUtilityWidget::HandleWallThicknessChanged(float NewValue)
{
	if (TargetActor)
	{
		TargetActor->WallThickness = NewValue;
	}
	if (WallThicknessLabel)
	{
		WallThicknessLabel->SetText(FText::FromString(FString::Printf(TEXT("Wall Thickness: %.0f"), NewValue)));
	}
}

void UMazeEditorUtilityWidget::HandleSeedChanged(float NewValue)
{
	const int32 Rounded = FMath::RoundToInt(NewValue);
	if (TargetActor && TargetActor->MazeGenerator)
	{
		TargetActor->MazeGenerator->RandomSeed = Rounded;
	}
	if (SeedLabel)
	{
		SeedLabel->SetText(FText::FromString(FString::Printf(TEXT("Seed: %d"), Rounded)));
	}
}

void UMazeEditorUtilityWidget::HandleFixedSeedChanged(bool bIsChecked)
{
	if (TargetActor && TargetActor->MazeGenerator)
	{
		TargetActor->MazeGenerator->bUseFixedSeed = bIsChecked;
	}
}

void UMazeEditorUtilityWidget::RefreshFromTargetActor()
{
	if (TargetActorText)
	{
		TargetActorText->SetText(TargetActor
			? FText::FromString(FString::Printf(TEXT("Target: %s"), *TargetActor->GetActorLabel()))
			: FText::FromString(TEXT("No target actor - select one and click Refresh")));
	}

	if (!TargetActor || !TargetActor->MazeGenerator)
	{
		return;
	}

	if (WidthSlider)         WidthSlider->SetValue((float)TargetActor->MazeGenerator->MazeWidth);
	if (HeightSlider)        HeightSlider->SetValue((float)TargetActor->MazeGenerator->MazeHeight);
	if (CellSizeSlider)      CellSizeSlider->SetValue(TargetActor->CellSize);
	if (WallHeightSlider)    WallHeightSlider->SetValue(TargetActor->WallHeight);
	if (WallThicknessSlider) WallThicknessSlider->SetValue(TargetActor->WallThickness);
	if (SeedSlider)          SeedSlider->SetValue((float)TargetActor->MazeGenerator->RandomSeed);
	if (FixedSeedCheckBox)   FixedSeedCheckBox->SetIsChecked(TargetActor->MazeGenerator->bUseFixedSeed);

	if (WidthLabel)         WidthLabel->SetText(FText::FromString(FString::Printf(TEXT("Width: %d"), TargetActor->MazeGenerator->MazeWidth)));
	if (HeightLabel)        HeightLabel->SetText(FText::FromString(FString::Printf(TEXT("Height: %d"), TargetActor->MazeGenerator->MazeHeight)));
	if (CellSizeLabel)      CellSizeLabel->SetText(FText::FromString(FString::Printf(TEXT("Cell Size: %.0f"), TargetActor->CellSize)));
	if (WallHeightLabel)    WallHeightLabel->SetText(FText::FromString(FString::Printf(TEXT("Wall Height: %.0f"), TargetActor->WallHeight)));
	if (WallThicknessLabel) WallThicknessLabel->SetText(FText::FromString(FString::Printf(TEXT("Wall Thickness: %.0f"), TargetActor->WallThickness)));
	if (SeedLabel)          SeedLabel->SetText(FText::FromString(FString::Printf(TEXT("Seed: %d"), TargetActor->MazeGenerator->RandomSeed)));
}
