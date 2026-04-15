// Fill out your copyright notice in the Description page of Project Settings.


#include "My2DGameModeBase.h"
#include "CameraPawn2D.h"
#include "My2DPlayerController.h"
#include "SaveGameRegistry.h"
#include "Kismet/GameplayStatics.h"

AMy2DGameModeBase::AMy2DGameModeBase()
{
    UE_LOG(LogTemp, Warning, TEXT("AMy2DGameModeBase::AMy2DGameModeBase() called"));
    // Set the default pawn to your 2D Camera Pawn
    DefaultPawnClass = ACameraPawn2D::StaticClass();

    // Set the player controller to your 2D Controller
    PlayerControllerClass = AMy2DPlayerController::StaticClass();

    // Log all entries currently stored in the save game registry
    if (UGameplayStatics::DoesSaveGameExist(USaveGameRegistry::RegistrySlotName, 0))
    {
        USaveGameRegistry* Registry = Cast<USaveGameRegistry>(
            UGameplayStatics::LoadGameFromSlot(USaveGameRegistry::RegistrySlotName, 0));

        if (Registry && Registry->SaveSlots.Num() > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("=== SaveGameRegistry: %d slot(s) found ==="), Registry->SaveSlots.Num());
            for (const FSaveSlotInfo& Slot : Registry->SaveSlots)
            {
                UE_LOG(LogTemp, Log, TEXT("  Slot='%s'  Display='%s'  Level='%s'  Actors=%d  Time=%s"),
                    *Slot.SlotName, *Slot.DisplayName, *Slot.LevelName,
                    Slot.ActorCount, *Slot.SaveTimeString);
            }
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("=== SaveGameRegistry: registry exists but contains no slots ==="));
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("=== SaveGameRegistry: no registry file found ==="));
    }
}
