// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoreItemData.h"
#include "StoreItemRowWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoreItemBuyClicked, const FString&, ItemID);

// Self-building single row in the store popup: name/description/stats/price plus a Buy button.
// UStoreWidget calls Setup() to populate the row and binds OnBuyClicked to react to purchases.
UCLASS()
class MOBILEUETESTPROJECT_API UStoreItemRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Populates the row's text/price from Item and enables the Buy button only if bCanAfford.
	void Setup(const FStoreItemData& InItem, bool bCanAfford);

	UPROPERTY(BlueprintAssignable, Category = "Store")
	FOnStoreItemBuyClicked OnBuyClicked;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DescText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatsText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> BuyButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BuyButtonText;

	UFUNCTION()
	void HandleBuyButtonClicked();

private:
	FStoreItemData Item;
};
