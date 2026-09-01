// Fill out your copyright notice in the Description page of Project Settings.

#include "BoxingDataSubsystem.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	template <typename TDataStruct>
	bool LoadCatalogFromJson(const FString& FilePath, const TCHAR* CatalogName, TArray<TDataStruct>& OutCatalog)
	{
		FString JsonString;
		if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
		{
			UE_LOG(LogTemp, Warning, TEXT("BoxingDataSubsystem: could not read %s file '%s'"), CatalogName, *FilePath);
			return false;
		}

		TArray<TDataStruct> ParsedItems;
		if (!FJsonObjectConverter::JsonArrayStringToUStruct(JsonString, &ParsedItems, 0, 0))
		{
			UE_LOG(LogTemp, Warning, TEXT("BoxingDataSubsystem: failed to parse %s JSON '%s'"), CatalogName, *FilePath);
			return false;
		}

		OutCatalog = MoveTemp(ParsedItems);
		return true;
	}

	template <typename TDataStruct, typename TIdMember>
	const TDataStruct* FindById(const TArray<TDataStruct>& Catalog, TIdMember TDataStruct::* IdField, const FString& Id)
	{
		return Catalog.FindByPredicate([&Id, IdField](const TDataStruct& Entry)
		{
			return Entry.*IdField == Id;
		});
	}

	template <typename TDataStruct, typename TIdMember>
	bool TryFindById(const TArray<TDataStruct>& Catalog, TIdMember TDataStruct::* IdField, const FString& Id, TDataStruct& OutData)
	{
		if (const TDataStruct* Found = FindById(Catalog, IdField, Id))
		{
			OutData = *Found;
			return true;
		}
		return false;
	}
}

void UBoxingDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadGlovesFromJson(GetDefaultGlovesFilePath());
	LoadStancesFromJson(GetDefaultStancesFilePath());
	LoadPerksFromJson(GetDefaultPerksFilePath());
	LoadOpponentsFromJson(GetDefaultOpponentsFilePath());
}

bool UBoxingDataSubsystem::LoadGlovesFromJson(const FString& FilePath)
{
	return LoadCatalogFromJson(FilePath, TEXT("Gloves"), Gloves);
}

bool UBoxingDataSubsystem::LoadStancesFromJson(const FString& FilePath)
{
	return LoadCatalogFromJson(FilePath, TEXT("Stances"), Stances);
}

bool UBoxingDataSubsystem::LoadPerksFromJson(const FString& FilePath)
{
	return LoadCatalogFromJson(FilePath, TEXT("Perks"), Perks);
}

bool UBoxingDataSubsystem::LoadOpponentsFromJson(const FString& FilePath)
{
	return LoadCatalogFromJson(FilePath, TEXT("Opponents"), Opponents);
}

FString UBoxingDataSubsystem::GetDefaultGlovesFilePath()
{
	return FPaths::ProjectContentDir() / TEXT("BoxingData") / TEXT("Gloves.json");
}

FString UBoxingDataSubsystem::GetDefaultStancesFilePath()
{
	return FPaths::ProjectContentDir() / TEXT("BoxingData") / TEXT("Stances.json");
}

FString UBoxingDataSubsystem::GetDefaultPerksFilePath()
{
	return FPaths::ProjectContentDir() / TEXT("BoxingData") / TEXT("Perks.json");
}

FString UBoxingDataSubsystem::GetDefaultOpponentsFilePath()
{
	return FPaths::ProjectContentDir() / TEXT("BoxingData") / TEXT("Opponents.json");
}

bool UBoxingDataSubsystem::TryFindGlove(const FString& GloveID, FBoxerGloveData& OutData) const
{
	return TryFindById(Gloves, &FBoxerGloveData::GloveID, GloveID, OutData);
}

bool UBoxingDataSubsystem::TryFindStance(const FString& StanceID, FBoxerStanceData& OutData) const
{
	return TryFindById(Stances, &FBoxerStanceData::StanceID, StanceID, OutData);
}

bool UBoxingDataSubsystem::TryFindPerk(const FString& PerkID, FBoxerPerkData& OutData) const
{
	return TryFindById(Perks, &FBoxerPerkData::PerkID, PerkID, OutData);
}

bool UBoxingDataSubsystem::TryFindOpponent(const FString& OpponentID, FOpponentData& OutData) const
{
	return TryFindById(Opponents, &FOpponentData::OpponentID, OpponentID, OutData);
}

const FBoxerGloveData* UBoxingDataSubsystem::FindGlove(const FString& GloveID) const
{
	return FindById(Gloves, &FBoxerGloveData::GloveID, GloveID);
}

const FBoxerStanceData* UBoxingDataSubsystem::FindStance(const FString& StanceID) const
{
	return FindById(Stances, &FBoxerStanceData::StanceID, StanceID);
}

const FBoxerPerkData* UBoxingDataSubsystem::FindPerk(const FString& PerkID) const
{
	return FindById(Perks, &FBoxerPerkData::PerkID, PerkID);
}

const FOpponentData* UBoxingDataSubsystem::FindOpponent(const FString& OpponentID) const
{
	return FindById(Opponents, &FOpponentData::OpponentID, OpponentID);
}
