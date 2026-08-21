// Fill out your copyright notice in the Description page of Project Settings.

#include "StoreItemRowWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UStoreItemRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BuyButton)
	{
		BuyButton->OnClicked.AddUniqueDynamic(this, &UStoreItemRowWidget::HandleBuyButtonClicked);
	}
}

void UStoreItemRowWidget::Setup(const FStoreItemData& InItem, bool bCanAfford)
{
	Item = InItem;

	if (IconImage)
	{
		if (!Item.Icon.IsNull())
		{
			IconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			IconImage->SetBrushFromSoftTexture(Item.Icon);
		}
		else
		{
			IconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

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
