// Fill out your copyright notice in the Description page of Project Settings.

#include "StoreItemRowWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

TSharedRef<SWidget> UStoreItemRowWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transactional);
	}

	UBorder* RowBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StoreItemRowBackground"));
	RowBackground->SetBrushColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.06f));
	RowBackground->SetPadding(FMargin(10.0f, 8.0f));
	WidgetTree->RootWidget = RowBackground;

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("StoreItemRow"));
	RowBackground->SetContent(Row);

	UVerticalBox* InfoBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StoreItemInfoBox"));
	UHorizontalBoxSlot* InfoSlot = Row->AddChildToHorizontalBox(InfoBox);
	InfoSlot->SetHorizontalAlignment(HAlign_Fill);
	InfoSlot->SetVerticalAlignment(VAlign_Center);
	InfoSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreItemNameText"));
	FSlateFontInfo NameFont = NameText->GetFont();
	NameFont.Size = 16;
	NameText->SetFont(NameFont);
	InfoBox->AddChildToVerticalBox(NameText);

	DescText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreItemDescText"));
	DescText->SetAutoWrapText(true);
	UVerticalBoxSlot* DescSlot = InfoBox->AddChildToVerticalBox(DescText);
	DescSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 0.0f));

	StatsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreItemStatsText"));
	UVerticalBoxSlot* StatsSlot = InfoBox->AddChildToVerticalBox(StatsText);
	StatsSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 0.0f));

	UVerticalBox* BuyBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StoreItemBuyBox"));
	UHorizontalBoxSlot* BuyBoxSlot = Row->AddChildToHorizontalBox(BuyBox);
	BuyBoxSlot->SetHorizontalAlignment(HAlign_Right);
	BuyBoxSlot->SetVerticalAlignment(VAlign_Center);
	BuyBoxSlot->SetPadding(FMargin(10.0f, 0.0f, 0.0f, 0.0f));

	BuyButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreItemBuyButtonText"));
	BuyButtonText->SetJustification(ETextJustify::Center);

	BuyButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StoreItemBuyButton"));
	BuyButton->SetContent(BuyButtonText);
	BuyButton->OnClicked.AddUniqueDynamic(this, &UStoreItemRowWidget::HandleBuyButtonClicked);
	UVerticalBoxSlot* BuyButtonSlot = BuyBox->AddChildToVerticalBox(BuyButton);
	BuyButtonSlot->SetHorizontalAlignment(HAlign_Fill);

	return Super::RebuildWidget();
}

void UStoreItemRowWidget::Setup(const FStoreItemData& InItem, bool bCanAfford)
{
	Item = InItem;

	// Ensure RebuildWidget() has run so NameText/DescText/etc. exist to populate below.
	TakeWidget();

	NameText->SetText(FText::FromString(Item.ItemName));
	DescText->SetText(FText::FromString(Item.Description));
	StatsText->SetText(FText::FromString(FString::Printf(TEXT("HP +%d   EXP +%d   PWR +%d   DEF +%d"),
		Item.HP, Item.EXP, Item.Power, Item.Def)));
	BuyButtonText->SetText(FText::FromString(FString::Printf(TEXT("Buy\n%d G"), Item.Price)));

	BuyButton->SetIsEnabled(bCanAfford);
}

void UStoreItemRowWidget::HandleBuyButtonClicked()
{
	OnBuyClicked.Broadcast(Item.ItemID);
}
