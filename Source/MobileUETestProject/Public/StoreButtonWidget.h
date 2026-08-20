// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoreButtonWidget.generated.h"

class UButton;
class UStoreWidget;

// Self-building "Store" button. On click, lazily creates a UStoreWidget (or a Blueprint child of
// it, via StorePopupWidgetClass), adds it to the viewport, and toggles its visibility - the
// popup itself reads its item list from UStoreSubsystem. Drop this widget into a HUD/PlayerController
// and it just works with no extra Blueprint wiring.
UCLASS()
class MOBILEUETESTPROJECT_API UStoreButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Widget class used for the store popup. Defaults to UStoreWidget if left unset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store", meta = (DisplayName = "Store Popup Widget Class"))
	TSubclassOf<UStoreWidget> StorePopupWidgetClass;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(Transient)
	TObjectPtr<UButton> StoreButton;

	UPROPERTY(Transient)
	TObjectPtr<UStoreWidget> StorePopupInstance;

	UFUNCTION()
	void HandleStoreButtonClicked();

private:
	void EnsureStorePopupExists();
};
