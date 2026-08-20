// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StoreItemData.h"
#include "StoreSubsystem.generated.h"

class APlayerController;

// Holds the store's item catalog and loads it from a JSON file. Lives on the GameInstance so it
// is created automatically and can be reached from anywhere (UStoreWidget queries it to populate
// the popup). Loads its default catalog on Initialize(), so it's ready before any UI needs it.
UCLASS()
class MOBILEUETESTPROJECT_API UStoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Parses FilePath (a JSON array of store items) and replaces the current catalog on success.
	// Existing catalog is left untouched if the file is missing or fails to parse.
	UFUNCTION(BlueprintCallable, Category = "Store")
	bool LoadStoreItemsFromJson(const FString& FilePath);

	UFUNCTION(BlueprintPure, Category = "Store")
	const TArray<FStoreItemData>& GetStoreItems() const { return StoreItems; }

	// Content/StoreData/StoreItems.json - ships with the project, editable by designers.
	UFUNCTION(BlueprintPure, Category = "Store")
	static FString GetDefaultStoreItemsFilePath();

	// Not UFUNCTION: UHT can't expose a raw pointer to a USTRUCT return value. C++-only lookup.
	const FStoreItemData* FindStoreItem(const FString& ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Store")
	int32 GetPlayerGold() const;

	// Spends the item's price (if affordable) and applies its stats to PurchasingPlayer's
	// UPlayerStatsComponent. Returns false and leaves gold untouched if the item is unknown,
	// PurchasingPlayer is null, or the player can't afford it.
	UFUNCTION(BlueprintCallable, Category = "Store")
	bool TryPurchaseItem(const FString& ItemID, APlayerController* PurchasingPlayer);

private:
	UPROPERTY()
	TArray<FStoreItemData> StoreItems;
};
