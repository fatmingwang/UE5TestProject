// Fill out your copyright notice in the Description page of Project Settings.

#include "StoreSubsystem.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "MySaveGame.h"
#include "PlayerStatsComponent.h"
#include "GameFramework/PlayerController.h"

void UStoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadStoreItemsFromJson(GetDefaultStoreItemsFilePath());
}

FString UStoreSubsystem::GetDefaultStoreItemsFilePath()
{
	return FPaths::ProjectContentDir() / TEXT("StoreData") / TEXT("StoreItems.json");
}

bool UStoreSubsystem::LoadStoreItemsFromJson(const FString& FilePath)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("StoreSubsystem: could not read store file '%s'"), *FilePath);
		return false;
	}

	TArray<FStoreItemData> ParsedItems;
	if (!FJsonObjectConverter::JsonArrayStringToUStruct(JsonString, &ParsedItems, 0, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("StoreSubsystem: failed to parse store JSON '%s'"), *FilePath);
		return false;
	}

	StoreItems = MoveTemp(ParsedItems);
	return true;
}

const FStoreItemData* UStoreSubsystem::FindStoreItem(const FString& ItemID) const
{
	return StoreItems.FindByPredicate([&ItemID](const FStoreItemData& Item)
	{
		return Item.ItemID == ItemID;
	});
}

int32 UStoreSubsystem::GetPlayerGold() const
{
	return UMySaveGame::GetGold(GetGameInstance());
}

bool UStoreSubsystem::TryPurchaseItem(const FString& ItemID, APlayerController* PurchasingPlayer)
{
	if (!PurchasingPlayer)
	{
		return false;
	}

	const FStoreItemData* Item = FindStoreItem(ItemID);
	if (!Item)
	{
		return false;
	}

	if (!UMySaveGame::TrySpendGold(GetGameInstance(), Item->Price))
	{
		return false;
	}

	if (UPlayerStatsComponent* Stats = PurchasingPlayer->FindComponentByClass<UPlayerStatsComponent>())
	{
		Stats->ApplyStoreItem(*Item);
	}

	return true;
}
