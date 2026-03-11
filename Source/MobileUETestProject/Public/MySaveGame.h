// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MySaveGame.generated.h"

/**
 * 
 */
UCLASS()
class MOBILEUETESTPROJECT_API UMySaveGame : public USaveGame
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
