#include "MySaveGame.h"
#include "SaveGameRegistry.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

// Define the save slot name
const FString UMySaveGame::SaveSlotName = TEXT("MyGameSlot");

// Save game function
void UMySaveGame::SaveGame(UObject* WorldContextObject)
{
	if (!WorldContextObject) return;

	UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));

	if (SaveGameInstance)
	{
		// Save current level name
		SaveGameInstance->SavedLevelName = UGameplayStatics::GetCurrentLevelName(WorldContextObject);

		// Get player pawn and save position
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(WorldContextObject, 0);
		if (PlayerPawn)
		{
			SaveGameInstance->SavedPlayerPosition = PlayerPawn->GetActorLocation();
			SaveGameInstance->SavedPlayerRotation = PlayerPawn->GetActorRotation();
		}

		// Save to slot and register in one call
		if (USaveGameRegistry::SaveGame(WorldContextObject, SaveGameInstance, SaveSlotName, 0))
		{
			UE_LOG(LogTemp, Log, TEXT("Game saved successfully!"));
		}
	}
}

// Load game function
void UMySaveGame::LoadGame(UObject* WorldContextObject)
{
	if (!WorldContextObject) return;

	UMySaveGame* LoadGameInstance = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));

	if (LoadGameInstance)
	{
		// Load level if different
		FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(WorldContextObject);
		if (CurrentLevel != LoadGameInstance->SavedLevelName)
		{
			UGameplayStatics::OpenLevel(WorldContextObject, FName(*LoadGameInstance->SavedLevelName));
		}

		// Restore player position
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(WorldContextObject, 0);
		if (PlayerPawn)
		{
			PlayerPawn->SetActorLocation(LoadGameInstance->SavedPlayerPosition);
			PlayerPawn->SetActorRotation(LoadGameInstance->SavedPlayerRotation);
		}

		UE_LOG(LogTemp, Log, TEXT("Game loaded successfully!"));
	}
}

UMySaveGame* UMySaveGame::LoadOrCreateSaveGame()
{
	if (UMySaveGame* Loaded = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)))
	{
		return Loaded;
	}
	return Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));
}

int32 UMySaveGame::GetGold(UObject* WorldContextObject)
{
	UMySaveGame* SaveGameInstance = LoadOrCreateSaveGame();
	return SaveGameInstance ? SaveGameInstance->Gold : DefaultStartingGold;
}

bool UMySaveGame::TrySpendGold(UObject* WorldContextObject, int32 Amount)
{
	if (!WorldContextObject || Amount <= 0)
	{
		return false;
	}

	UMySaveGame* SaveGameInstance = LoadOrCreateSaveGame();
	if (!SaveGameInstance || SaveGameInstance->Gold < Amount)
	{
		return false;
	}

	SaveGameInstance->Gold -= Amount;
	return USaveGameRegistry::SaveGame(WorldContextObject, SaveGameInstance, SaveSlotName, 0);
}

void UMySaveGame::AddGold(UObject* WorldContextObject, int32 Amount)
{
	if (!WorldContextObject || Amount <= 0)
	{
		return;
	}

	UMySaveGame* SaveGameInstance = LoadOrCreateSaveGame();
	if (!SaveGameInstance)
	{
		return;
	}

	SaveGameInstance->Gold += Amount;
	USaveGameRegistry::SaveGame(WorldContextObject, SaveGameInstance, SaveSlotName, 0);
}

void UMySaveGame::GetPlayerStats(UObject* WorldContextObject, int32& OutHP, int32& OutEXP, int32& OutPower, int32& OutDef)
{
	UMySaveGame* SaveGameInstance = LoadOrCreateSaveGame();
	OutHP = SaveGameInstance ? SaveGameInstance->PlayerHP : 0;
	OutEXP = SaveGameInstance ? SaveGameInstance->PlayerEXP : 0;
	OutPower = SaveGameInstance ? SaveGameInstance->PlayerPower : 0;
	OutDef = SaveGameInstance ? SaveGameInstance->PlayerDef : 0;
}

void UMySaveGame::SavePlayerStats(UObject* WorldContextObject, int32 HP, int32 EXP, int32 Power, int32 Def)
{
	if (!WorldContextObject)
	{
		return;
	}

	UMySaveGame* SaveGameInstance = LoadOrCreateSaveGame();
	if (!SaveGameInstance)
	{
		return;
	}

	SaveGameInstance->PlayerHP = HP;
	SaveGameInstance->PlayerEXP = EXP;
	SaveGameInstance->PlayerPower = Power;
	SaveGameInstance->PlayerDef = Def;
	USaveGameRegistry::SaveGame(WorldContextObject, SaveGameInstance, SaveSlotName, 0);
}