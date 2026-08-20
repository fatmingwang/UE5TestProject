// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveGameRegistry.h"
#include "MySaveGame.generated.h"

/**
 * Save game for player transform.
 * Inherits URegisteredSaveGame so it is automatically tracked by the registry.
 */
UCLASS()
class MOBILEUETESTPROJECT_API UMySaveGame : public URegisteredSaveGame
{
	GENERATED_BODY()
public:
	// Save slot name constant
	static const FString SaveSlotName;
	
	UPROPERTY(VisibleAnywhere, Category = "MySaveGame", meta = (DisplayName = "Saved Level Name"))
	FString SavedLevelName;

	UPROPERTY(VisibleAnywhere, Category = "MySaveGame", meta = (DisplayName = "Saved Player Position"))
	FVector SavedPlayerPosition;

	UPROPERTY(VisibleAnywhere, Category = "MySaveGame", meta = (DisplayName = "Saved Player Rotation"))
	FRotator SavedPlayerRotation;

	UPROPERTY(VisibleAnywhere, Category = "MySaveGame", meta = (DisplayName = "Gold"))
	int32 Gold = DefaultStartingGold;

	static const int32 DefaultStartingGold = 100;

	// Stat totals accumulated from store purchases (see UPlayerStatsComponent).
	UPROPERTY(VisibleAnywhere, Category = "MySaveGame", meta = (DisplayName = "Player HP"))
	int32 PlayerHP = 0;

	UPROPERTY(VisibleAnywhere, Category = "MySaveGame", meta = (DisplayName = "Player EXP"))
	int32 PlayerEXP = 0;

	UPROPERTY(VisibleAnywhere, Category = "MySaveGame", meta = (DisplayName = "Player Power"))
	int32 PlayerPower = 0;

	UPROPERTY(VisibleAnywhere, Category = "MySaveGame", meta = (DisplayName = "Player Def"))
	int32 PlayerDef = 0;

	// Static helper functions
	UFUNCTION(BlueprintCallable, Category = "MySaveGame", meta = (WorldContext = "WorldContextObject"))
	static void SaveGame(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "MySaveGame", meta = (WorldContext = "WorldContextObject"))
	static void LoadGame(UObject* WorldContextObject);

	// Reads the player's current gold balance (falls back to DefaultStartingGold if no save exists yet).
	UFUNCTION(BlueprintCallable, Category = "MySaveGame", meta = (WorldContext = "WorldContextObject"))
	static int32 GetGold(UObject* WorldContextObject);

	// Deducts Amount from the player's gold and persists it. Returns false (no change) if the
	// balance is insufficient.
	UFUNCTION(BlueprintCallable, Category = "MySaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool TrySpendGold(UObject* WorldContextObject, int32 Amount);

	// Adds Amount to the player's gold and persists it.
	UFUNCTION(BlueprintCallable, Category = "MySaveGame", meta = (WorldContext = "WorldContextObject"))
	static void AddGold(UObject* WorldContextObject, int32 Amount);

	// Reads the player's saved stat totals (0s if no save exists yet).
	UFUNCTION(BlueprintCallable, Category = "MySaveGame", meta = (WorldContext = "WorldContextObject"))
	static void GetPlayerStats(UObject* WorldContextObject, int32& OutHP, int32& OutEXP, int32& OutPower, int32& OutDef);

	// Overwrites and persists the player's stat totals. Called by UPlayerStatsComponent after
	// every store purchase so totals survive a relaunch.
	UFUNCTION(BlueprintCallable, Category = "MySaveGame", meta = (WorldContext = "WorldContextObject"))
	static void SavePlayerStats(UObject* WorldContextObject, int32 HP, int32 EXP, int32 Power, int32 Def);

private:
	// Loads the existing save slot, or creates a fresh default one if none exists yet.
	static UMySaveGame* LoadOrCreateSaveGame();
};
