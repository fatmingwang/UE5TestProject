// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoreItemData.h"
#include "StoreWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UScrollBox;
class UStoreItemRowWidget;

// Self-building store popup: constructs its own UMG layout in C++ (RebuildWidget), reads its
// catalog from UStoreSubsystem, and lists each item with a Buy button. Purchases spend gold via
// UMySaveGame and apply stats via the buyer's UPlayerStatsComponent (see UStoreSubsystem::TryPurchaseItem).
// Toggle visibility (or add/remove from viewport) to show/hide the popup.
UCLASS()
class MOBILEUETESTPROJECT_API UStoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Re-reads the catalog from UStoreSubsystem and rebuilds the item rows.
	UFUNCTION(BlueprintCallable, Category = "Store")
	void RefreshStoreItems();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> ItemListBox;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GoldText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleItemPurchaseRequested(const FString& ItemID);

private:
	void AddItemRow(const FStoreItemData& Item, bool bCanAfford);
	void RefreshGoldText();
};
