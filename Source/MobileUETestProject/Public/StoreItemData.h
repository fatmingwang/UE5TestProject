// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
