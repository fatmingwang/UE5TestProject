// Fill out your copyright notice in the Description page of Project Settings.

#include "StoreButtonWidget.h"
#include "StoreWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

TSharedRef<SWidget> UStoreButtonWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transactional);
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("StoreButtonRootCanvas"));
	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = RootCanvas;

	StoreButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StoreButton"));

	UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreButtonLabel"));
	ButtonText->SetText(FText::FromString(TEXT("Store")));
	ButtonText->SetJustification(ETextJustify::Center);
	StoreButton->SetContent(ButtonText);
	StoreButton->OnClicked.AddUniqueDynamic(this, &UStoreButtonWidget::HandleStoreButtonClicked);

	// Bottom-right corner - top-right is already used by UMazeMinimapWidget in maze levels.
	UCanvasPanelSlot* ButtonSlot = RootCanvas->AddChildToCanvas(StoreButton);
	ButtonSlot->SetAnchors(FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
	ButtonSlot->SetAlignment(FVector2D(1.0f, 1.0f));
	ButtonSlot->SetPosition(FVector2D(-24.0f, -24.0f));
	ButtonSlot->SetAutoSize(false);
	ButtonSlot->SetSize(FVector2D(100.0f, 40.0f));

	return Super::RebuildWidget();
}

void UStoreButtonWidget::EnsureStorePopupExists()
{
	if (StorePopupInstance)
	{
		return;
	}

	TSubclassOf<UStoreWidget> ClassToUse = StorePopupWidgetClass;
	if (!ClassToUse)
	{
		ClassToUse = UStoreWidget::StaticClass();
	}
	StorePopupInstance = CreateWidget<UStoreWidget>(GetOwningPlayer(), ClassToUse);
	if (StorePopupInstance)
	{
		StorePopupInstance->SetVisibility(ESlateVisibility::Collapsed);
		StorePopupInstance->AddToViewport(10);
	}
}

void UStoreButtonWidget::HandleStoreButtonClicked()
{
	EnsureStorePopupExists();
	if (!StorePopupInstance)
	{
		return;
	}

	const bool bCurrentlyOpen = StorePopupInstance->GetVisibility() != ESlateVisibility::Collapsed;
	StorePopupInstance->SetStoreOpen(!bCurrentlyOpen);
}
