// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "StoreItemData.generated.h"

// One purchasable entry in the store. Field names match the JSON keys loaded by
// UStoreSubsystem::LoadStoreItemsFromJson() one-for-one (see Content/StoreData/StoreItems.json).
USTRUCT(BlueprintType)
struct MOBILEUETESTPROJECT_API FStoreItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	FString ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	FString Description;

	// Store-row thumbnail. JSON stores this as the texture's soft path string (e.g.
	// "/Game/StoreData/Icons/T_Icon_Sword_Iron.T_Icon_Sword_Iron"); left unset, the row hides the icon slot.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	int32 HP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	int32 EXP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	int32 Power = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	int32 Def = 0;
};
