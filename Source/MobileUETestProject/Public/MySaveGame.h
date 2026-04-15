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
	
	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FString SavedLevelName;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FVector SavedPlayerPosition;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FRotator SavedPlayerRotation;

	// Static helper functions
	UFUNCTION(BlueprintCallable, Category = "SaveGame", meta = (WorldContext = "WorldContextObject"))
	static void SaveGame(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "SaveGame", meta = (WorldContext = "WorldContextObject"))
	static void LoadGame(UObject* WorldContextObject);
};
