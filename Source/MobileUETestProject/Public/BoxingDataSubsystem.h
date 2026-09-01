// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BoxingLoadoutData.h"
#include "BoxingDataSubsystem.generated.h"

// Holds the boxing match's Gloves/Stances/Perks/Opponents catalogs and loads them from JSON.
// Lives on the GameInstance so it's created automatically and reachable from anywhere (a BP_
// FightDirector setup queries it to fetch a chosen loadout/opponent before applying it to an
// ABoxerCharacter). Loads its default catalogs on Initialize(), so it's ready before any fight
// setup needs it. Mirrors UStoreSubsystem/FStoreItemData's JSON-catalog pattern exactly.
UCLASS()
class MOBILEUETESTPROJECT_API UBoxingDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Boxing")
	bool LoadGlovesFromJson(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Boxing")
	bool LoadStancesFromJson(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Boxing")
	bool LoadPerksFromJson(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Boxing")
	bool LoadOpponentsFromJson(const FString& FilePath);

	UFUNCTION(BlueprintPure, Category = "Boxing")
	const TArray<FBoxerGloveData>& GetGloves() const { return Gloves; }

	UFUNCTION(BlueprintPure, Category = "Boxing")
	const TArray<FBoxerStanceData>& GetStances() const { return Stances; }

	UFUNCTION(BlueprintPure, Category = "Boxing")
	const TArray<FBoxerPerkData>& GetPerks() const { return Perks; }

	UFUNCTION(BlueprintPure, Category = "Boxing")
	const TArray<FOpponentData>& GetOpponents() const { return Opponents; }

	// Content/BoxingData/*.json - ship with the project, editable by designers.
	UFUNCTION(BlueprintPure, Category = "Boxing")
	static FString GetDefaultGlovesFilePath();

	UFUNCTION(BlueprintPure, Category = "Boxing")
	static FString GetDefaultStancesFilePath();

	UFUNCTION(BlueprintPure, Category = "Boxing")
	static FString GetDefaultPerksFilePath();

	UFUNCTION(BlueprintPure, Category = "Boxing")
	static FString GetDefaultOpponentsFilePath();

	// BlueprintCallable "TryFind" variants (UHT can't expose a raw-pointer return, so these copy
	// out into OutData and return whether the ID was found) - the BP-friendly counterparts of the
	// C++-only Find*() below.
	UFUNCTION(BlueprintCallable, Category = "Boxing")
	bool TryFindGlove(const FString& GloveID, FBoxerGloveData& OutData) const;

	UFUNCTION(BlueprintCallable, Category = "Boxing")
	bool TryFindStance(const FString& StanceID, FBoxerStanceData& OutData) const;

	UFUNCTION(BlueprintCallable, Category = "Boxing")
	bool TryFindPerk(const FString& PerkID, FBoxerPerkData& OutData) const;

	UFUNCTION(BlueprintCallable, Category = "Boxing")
	bool TryFindOpponent(const FString& OpponentID, FOpponentData& OutData) const;

	// Not UFUNCTION: UHT can't expose a raw pointer to a USTRUCT return value. C++-only lookup.
	const FBoxerGloveData* FindGlove(const FString& GloveID) const;
	const FBoxerStanceData* FindStance(const FString& StanceID) const;
	const FBoxerPerkData* FindPerk(const FString& PerkID) const;
	const FOpponentData* FindOpponent(const FString& OpponentID) const;

private:
	UPROPERTY()
	TArray<FBoxerGloveData> Gloves;

	UPROPERTY()
	TArray<FBoxerStanceData> Stances;

	UPROPERTY()
	TArray<FBoxerPerkData> Perks;

	UPROPERTY()
	TArray<FOpponentData> Opponents;
};
