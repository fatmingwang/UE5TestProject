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