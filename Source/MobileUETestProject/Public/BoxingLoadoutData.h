// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PowerBarMath.h"
#include "BoxingLoadoutData.generated.h"

// One equippable Attack-bar loadout (design doc section 6, "Gloves" - was Weapon in the
// prototype). Field names match the JSON keys loaded by UBoxingDataSubsystem::LoadGlovesFromJson()
// one-for-one (see Content/BoxingData/Gloves.json). Applied to a boxer via
// ABoxerCharacter::ApplyGloveData().
USTRUCT(BlueprintType)
struct FBoxerGloveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Gloves")
	FString GloveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Gloves")
	FString GloveName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Gloves")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Gloves")
	float ATKBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Gloves")
	float AttackV0 = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Gloves")
	float AttackVmax = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Gloves")
	float AttackRampTime = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Gloves")
	float AttackMaxMult = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Gloves")
	FPowerBarZones AttackZones;
};

// One equippable Defense-bar loadout (design doc section 6, "Stance" - was Armor). Field names
// match Content/BoxingData/Stances.json. Applied via ABoxerCharacter::ApplyStanceData().
USTRUCT(BlueprintType)
struct FBoxerStanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Stance")
	FString StanceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Stance")
	FString StanceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Stance")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Stance")
	float DEFBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Stance")
	float DefenseV0 = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Stance")
	float DefenseVmax = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Stance")
	float DefenseRampTime = 1.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Stance")
	FPowerBarZones DefenseZones;
};

// One equippable Perk (design doc section 6 - was Talent). Not auto-applied by
// ABoxerCharacter::ApplyPerkData() beyond the numeric knobs below - ZeroDamageOnRedGuard and any
// flourish/SFX flavor stay the fight loop's (AFightDirector's) responsibility to read and act on.
USTRUCT(BlueprintType)
struct FBoxerPerkData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Perk")
	FString PerkID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Perk")
	FString PerkName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Perk")
	FString Description;

	// "Iron Chin": flat mitigation floor added on top of Defense-bar mitigation (0.1 = +10%).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Perk")
	float MitigationFloorBonus = 0.0f;

	// "Counter Puncher": extra Attack power percentage added on release (0.15 = +15%).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Perk")
	float AttackPowerBonusPct = 0.0f;

	// "Counter Puncher": takes 0 damage on a RED-zone guard instead of the usual heavy hit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Perk")
	bool bZeroDamageOnRedGuard = false;

	// "Focus": multiplies both AttackRampTime and DefenseRampTime (1.25 = 25% slower ramp).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Perk")
	float RampTimeMultiplier = 1.0f;
};

// One AI opponent (design doc section 6 - was Enemy). Field names match
// Content/BoxingData/Opponents.json. Applied via ABoxerCharacter::ApplyOpponentData(); OpponentSkill
// maps 1:1 onto AFightDirector::OpponentSkill.
USTRUCT(BlueprintType)
struct FOpponentData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	FString OpponentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	FString OpponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	float MaxHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	float ATK = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	float DEF = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	float AttackV0 = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	float AttackVmax = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	float AttackRampTime = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	float AttackMaxMult = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	FPowerBarZones AttackZones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	float DefenseV0 = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	float DefenseVmax = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	float DefenseRampTime = 1.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent")
	FPowerBarZones DefenseZones;

	// How close to its own bar's center this opponent's simulated punch lands - maps 1:1 onto
	// AFightDirector::OpponentSkill (0 = always dead-center/best, 1 = anywhere up to the full bar).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxing|Opponent", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OpponentSkill = 0.6f;
};
