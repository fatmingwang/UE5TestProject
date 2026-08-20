// Fill out your copyright notice in the Description page of Project Settings.

#include "StoreWidget.h"
#include "StoreSubsystem.h"
#include "StoreItemRowWidget.h"
#include "Engine/GameInstance.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

TSharedRef<SWidget> UStoreWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transactional);
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = RootCanvas;

	UBorder* PanelBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StorePanelBackground"));
	PanelBackground->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.9f));
	PanelBackground->SetPadding(FMargin(16.0f));

	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelBackground);
	PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
	PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	PanelSlot->SetPosition(FVector2D(0.0f, 0.0f));
	PanelSlot->SetAutoSize(false);
	PanelSlot->SetSize(FVector2D(480.0f, 420.0f));

	UVerticalBox* MainBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StoreMainBox"));
	PanelBackground->SetContent(MainBox);

	UHorizontalBox* HeaderRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("StoreHeaderRow"));
	UVerticalBoxSlot* HeaderRowSlot = MainBox->AddChildToVerticalBox(HeaderRow);
	HeaderRowSlot->SetHorizontalAlignment(HAlign_Fill);
	HeaderRowSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));

	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreTitleText"));
	TitleText->SetText(FText::FromString(TEXT("Store")));
	FSlateFontInfo TitleFont = TitleText->GetFont();
	TitleFont.Size = 22;
	TitleText->SetFont(TitleFont);
	UHorizontalBoxSlot* TitleSlot = HeaderRow->AddChildToHorizontalBox(TitleText);
	TitleSlot->SetHorizontalAlignment(HAlign_Left);
	TitleSlot->SetVerticalAlignment(VAlign_Center);
	TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	GoldText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreGoldText"));
	GoldText->SetJustification(ETextJustify::Right);
	UHorizontalBoxSlot* GoldSlot = HeaderRow->AddChildToHorizontalBox(GoldText);
	GoldSlot->SetHorizontalAlignment(HAlign_Right);
	GoldSlot->SetVerticalAlignment(VAlign_Center);

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreStatusText"));
	StatusText->SetJustification(ETextJustify::Center);
	StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.2f, 1.0f)));
	UVerticalBoxSlot* StatusSlot = MainBox->AddChildToVerticalBox(StatusText);
	StatusSlot->SetHorizontalAlignment(HAlign_Fill);
	StatusSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));

	ItemListBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("StoreItemListBox"));
	UVerticalBoxSlot* ListSlot = MainBox->AddChildToVerticalBox(ItemListBox);
	ListSlot->SetHorizontalAlignment(HAlign_Fill);
	ListSlot->SetVerticalAlignment(VAlign_Fill);
	ListSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StoreCloseButton"));
	UTextBlock* CloseText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreCloseButtonLabel"));
	CloseText->SetText(FText::FromString(TEXT("Close")));
	CloseText->SetJustification(ETextJustify::Center);
	CloseButton->SetContent(CloseText);
	CloseButton->OnClicked.AddUniqueDynamic(this, &UStoreWidget::HandleCloseClicked);

	UVerticalBoxSlot* CloseSlot = MainBox->AddChildToVerticalBox(CloseButton);
	CloseSlot->SetHorizontalAlignment(HAlign_Fill);
	CloseSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));

	RefreshStoreItems();

	return Super::RebuildWidget();
}

void UStoreWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshStoreItems();
}

void UStoreWidget::RefreshStoreItems()
{
	if (!ItemListBox)
	{
		return;
	}

	ItemListBox->ClearChildren();
	RefreshGoldText();

	const UStoreSubsystem* StoreSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UStoreSubsystem>() : nullptr;
	if (!StoreSubsystem)
	{
		return;
	}

	const int32 Gold = StoreSubsystem->GetPlayerGold();
	for (const FStoreItemData& Item : StoreSubsystem->GetStoreItems())
	{
		AddItemRow(Item, Gold >= Item.Price);
	}
}

void UStoreWidget::RefreshGoldText()
{
	if (!GoldText)
	{
		return;
	}

	const UStoreSubsystem* StoreSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UStoreSubsystem>() : nullptr;
	const int32 Gold = StoreSubsystem ? StoreSubsystem->GetPlayerGold() : 0;
	GoldText->SetText(FText::FromString(FString::Printf(TEXT("Gold: %d"), Gold)));
}

void UStoreWidget::AddItemRow(const FStoreItemData& Item, bool bCanAfford)
{
	UStoreItemRowWidget* Row = CreateWidget<UStoreItemRowWidget>(GetOwningPlayer(), UStoreItemRowWidget::StaticClass());
	if (!Row)
	{
		return;
	}

	Row->Setup(Item, bCanAfford);
	Row->OnBuyClicked.AddUniqueDynamic(this, &UStoreWidget::HandleItemPurchaseRequested);

	UScrollBoxSlot* RowSlot = Cast<UScrollBoxSlot>(ItemListBox->AddChild(Row));
	if (RowSlot)
	{
		RowSlot->SetHorizontalAlignment(HAlign_Fill);
		RowSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	}
}

void UStoreWidget::HandleCloseClicked()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UStoreWidget::HandleItemPurchaseRequested(const FString& ItemID)
{
	UStoreSubsystem* StoreSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UStoreSubsystem>() : nullptr;
	if (!StoreSubsystem)
	{
		return;
	}

	const FStoreItemData* Item = StoreSubsystem->FindStoreItem(ItemID);
	const FString ItemName = Item ? Item->ItemName : ItemID;

	if (StoreSubsystem->TryPurchaseItem(ItemID, GetOwningPlayer()))
	{
		if (StatusText)
		{
			StatusText->SetText(FText::FromString(FString::Printf(TEXT("Purchased %s!"), *ItemName)));
		}
		RefreshStoreItems();
	}
	else if (StatusText)
	{
		StatusText->SetText(FText::FromString(FString::Printf(TEXT("Can't afford %s."), *ItemName)));
	}
}
