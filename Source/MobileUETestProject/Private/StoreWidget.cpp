// Fill out your copyright notice in the Description page of Project Settings.

#include "StoreWidget.h"
#include "StoreSubsystem.h"
#include "StoreItemRowWidget.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UStoreWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UStoreWidget::HandleCloseClicked);
	}

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
	const TSubclassOf<UStoreItemRowWidget> RowClass = ItemRowWidgetClass ? ItemRowWidgetClass : TSubclassOf<UStoreItemRowWidget>(UStoreItemRowWidget::StaticClass());
	UStoreItemRowWidget* Row = CreateWidget<UStoreItemRowWidget>(GetOwningPlayer(), RowClass);
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
	SetStoreOpen(false);
}

void UStoreWidget::SetStoreOpen(bool bOpen)
{
	SetVisibility(bOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	if (bOpen)
	{
		RefreshStoreItems();

		bCursorWasVisibleBeforeOpen = PC->bShowMouseCursor;
		PC->SetShowMouseCursor(true);

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
	else
	{
		PC->SetShowMouseCursor(bCursorWasVisibleBeforeOpen);
		PC->SetInputMode(FInputModeGameOnly());
	}
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
