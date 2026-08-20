// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "My2DPlayerController.generated.h"

class UStoreButtonWidget;
class UPlayerStatsComponent;

/**
 *
 */
UCLASS()
class MOBILEUETESTPROJECT_API AMy2DPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AMy2DPlayerController();
	virtual void BeginPlay() override;

	// If set, a UStoreButtonWidget instance is created and added to the viewport on BeginPlay -
	// clicking it pops up the store UI. Assign a WBP_ child of UStoreButtonWidget here.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store", meta = (DisplayName = "Store Button Widget Class"))
	TSubclassOf<UStoreButtonWidget> StoreButtonWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store", meta = (DisplayName = "Auto Create Store Button"))
	bool bAutoCreateStoreButton = true;

	UPROPERTY(BlueprintReadOnly, Category = "Store", meta = (DisplayName = "Store Button Widget Instance"))
	TObjectPtr<UStoreButtonWidget> StoreButtonWidgetInstance;

	// Stat totals accumulated from store purchases.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Store", meta = (DisplayName = "Player Stats"))
	TObjectPtr<UPlayerStatsComponent> PlayerStatsComponent;
};
