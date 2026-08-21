// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoreItemData.h"
#include "StoreItemRowWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoreItemBuyClicked, const FString&, ItemID);

// Single row in the store popup: name/description/stats/price plus a Buy button. Visual layout is
// designed in a Widget Blueprint subclass (e.g. WBP_StoreItemRow) via the UMG Designer, naming its
// widgets to match the BindWidget properties below. UStoreWidget calls Setup() to populate the row
// and binds OnBuyClicked to react to purchases.
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
	virtual void NativeConstruct() override;

	// Optional: not every row layout needs an icon. Name an Image "IconImage" in the Designer to use one.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> IconImage;

	// Name a TextBlock "NameText" in the Designer.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	// Name a TextBlock "DescText" in the Designer.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> DescText;

	// Name a TextBlock "StatsText" in the Designer.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> StatsText;

	// Name a Button "BuyButton" in the Designer.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> BuyButton;

	// Name a TextBlock "BuyButtonText" in the Designer (typically placed as BuyButton's content).
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> BuyButtonText;

	UFUNCTION()
	void HandleBuyButtonClicked();

private:
	FStoreItemData Item;
};
