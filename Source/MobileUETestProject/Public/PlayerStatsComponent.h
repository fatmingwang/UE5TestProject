// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StoreItemData.h"
#include "PlayerStatsComponent.generated.h"

// Minimal holder for the stat totals store purchases feed into. Attach to the player controller
// (or pawn) and call ApplyStoreItem() whenever a purchase succeeds.
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOBILEUETESTPROJECT_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Store")
	void ApplyStoreItem(const FStoreItemData& Item);

	UFUNCTION(BlueprintPure, Category = "Store")
	int32 GetHP() const { return HP; }

	UFUNCTION(BlueprintPure, Category = "Store")
	int32 GetEXP() const { return EXP; }

	UFUNCTION(BlueprintPure, Category = "Store")
	int32 GetPower() const { return Power; }

	UFUNCTION(BlueprintPure, Category = "Store")
	int32 GetDef() const { return Def; }

protected:
	// Loads previously saved stat totals so purchases survive a relaunch.
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Store")
	int32 HP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Store")
	int32 EXP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Store")
	int32 Power = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Store")
	int32 Def = 0;
};
