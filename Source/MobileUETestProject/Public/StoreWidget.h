// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoreItemData.h"
#include "StoreWidget.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;
class UStoreItemRowWidget;

// Store popup: visual layout is designed in a Widget Blueprint subclass (e.g. WBP_StoreWidget) via
// the UMG Designer; this C++ class only reads the catalog from UStoreSubsystem, lists each item with
// a Buy button, and handles clicks. Purchases spend gold via UMySaveGame and apply stats via the
// buyer's UPlayerStatsComponent (see UStoreSubsystem::TryPurchaseItem).
// The Designer must name its widgets to match the BindWidget properties below.
// Toggle visibility (or add/remove from viewport) to show/hide the popup.
UCLASS()
class MOBILEUETESTPROJECT_API UStoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Re-reads the catalog from UStoreSubsystem and rebuilds the item rows.
	UFUNCTION(BlueprintCallable, Category = "Store")
	void RefreshStoreItems();

	// Shows or hides the popup and switches the owning player's input mode between UI-only
	// (store open - mouse free, pawn stops receiving move/look input) and normal gameplay.
	UFUNCTION(BlueprintCallable, Category = "Store")
	void SetStoreOpen(bool bOpen);

	// Blueprint class used to instantiate each item row; defaults to the plain C++ class if unset,
	// but should be set to a WBP subclass (e.g. WBP_StoreItemRow) in the Designer's Class Defaults.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	TSubclassOf<UStoreItemRowWidget> ItemRowWidgetClass;

protected:
	virtual void NativeConstruct() override;

	// Scroll box the item rows are added to. Name a ScrollBox "ItemListBox" in the Designer.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UScrollBox> ItemListBox;

	// Name a TextBlock "GoldText" in the Designer.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldText;

	// Name a TextBlock "StatusText" in the Designer.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

	// Name a Button "CloseButton" in the Designer.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleItemPurchaseRequested(const FString& ItemID);

private:
	void AddItemRow(const FStoreItemData& Item, bool bCanAfford);
	void RefreshGoldText();

	// Restored on close, since bShowMouseCursor may already have been on for other reasons.
	bool bCursorWasVisibleBeforeOpen = false;
};
