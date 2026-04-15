#pragma once

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGameRegistry.generated.h"

#define SAVE_SLOT_CATEGORY "Save Slot"

/**
 * Struct to hold metadata about a save slot
 */
USTRUCT(BlueprintType)
struct FSaveSlotInfo
{
    GENERATED_BODY()

    // The slot name used to save/load
    UPROPERTY(BlueprintReadOnly, Category = SAVE_SLOT_CATEGORY)
    FString SlotName;

    // When the save was created
    UPROPERTY(BlueprintReadOnly, Category = SAVE_SLOT_CATEGORY)
    FDateTime SaveTime;

    // Display-friendly save time string
    UPROPERTY(BlueprintReadOnly, Category = SAVE_SLOT_CATEGORY)
    FString SaveTimeString;

    // The level/map name when saved
    UPROPERTY(BlueprintReadOnly, Category = SAVE_SLOT_CATEGORY)
    FString LevelName;

    // Number of actors saved
    UPROPERTY(BlueprintReadOnly, Category = SAVE_SLOT_CATEGORY)
    int32 ActorCount;

    // Optional user-defined display name
    UPROPERTY(BlueprintReadWrite, Category = SAVE_SLOT_CATEGORY)
    FString DisplayName;

    // Default constructor
    FSaveSlotInfo()
        : SlotName(TEXT(""))
        , SaveTime(FDateTime::Now())
        , SaveTimeString(TEXT(""))
        , LevelName(TEXT(""))
        , ActorCount(0)
        , DisplayName(TEXT(""))
    {
    }

    // Constructor with parameters
    FSaveSlotInfo(const FString& InSlotName, const FString& InLevelName, int32 InActorCount, const FString& InDisplayName = TEXT(""))
        : SlotName(InSlotName)
        , SaveTime(FDateTime::Now())
        , LevelName(InLevelName)
        , ActorCount(InActorCount)
        , DisplayName(InDisplayName)
    {
        // Format: "Dec 26, 2024 - 3:45 PM"
        SaveTimeString = SaveTime.ToString(TEXT("%b %d, %Y - %I:%M %p"));
    }

    // Update save time to now
    void UpdateSaveTime()
    {
        SaveTime = FDateTime::Now();
        SaveTimeString = SaveTime.ToString(TEXT("%b %d, %Y - %I:%M %p"));
    }
};

/**
 * Registry that tracks all save slots
 * This is saved separately from the actual save data
 */
UCLASS()
class MOBILEUETESTPROJECT_API USaveGameRegistry : public USaveGame
{
    GENERATED_BODY()

public:
    USaveGameRegistry();

    // All registered save slots
    UPROPERTY()
    TArray<FSaveSlotInfo> SaveSlots;

    // The slot name used for the registry itself
    static const FString RegistrySlotName;

    // Add or update a save slot entry
    void RegisterSaveSlot(const FSaveSlotInfo& SlotInfo);

    // Remove a save slot entry
    bool UnregisterSaveSlot(const FString& SlotName);

    // Find a save slot by name
    FSaveSlotInfo* FindSaveSlot(const FString& SlotName);

    // Check if a slot exists
    bool DoesSaveSlotExist(const FString& SlotName) const;

    // Get total number of saves
    int32 GetSaveCount() const { return SaveSlots.Num(); }

    // Sort saves by time (newest first)
    void SortByNewest();

    // Sort saves by time (oldest first)
    void SortByOldest();

    // Sort saves by name
    void SortByName();

    // Register info from a save game object and persist the registry.
    // Call this after every successful SaveGameToSlot().
    UFUNCTION(BlueprintCallable, Category = "Save Registry",
        meta = (WorldContext = "WorldContextObject"))
    static void UpdateRegistry(UObject* WorldContextObject, const FSaveSlotInfo& SlotInfo);

    // Drop-in replacement for UGameplayStatics::SaveGameToSlot.
    // Saves the object to disk and automatically updates the registry.
    // SlotInfo metadata (LevelName, ActorCount) is gathered automatically
    // from the save object when possible; pass a custom DisplayName optionally.
    UFUNCTION(BlueprintCallable, Category = "Save Registry",
        meta = (WorldContext = "WorldContextObject"))
    static bool SaveGame(UObject* WorldContextObject, USaveGame* SaveGameObject,
        const FString& SlotName, int32 UserIndex = 0,
        const FString& DisplayName = TEXT(""));
};

// -----------------------------------------------------------------------
// Base class that every custom USaveGame must inherit from.
// Use USaveGameRegistry::SaveGame() to save - no overrides required.
// -----------------------------------------------------------------------
UCLASS(Abstract)
class MOBILEUETESTPROJECT_API URegisteredSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // The slot name this save game will be stored under.
    UPROPERTY(BlueprintReadWrite, Category = "Save Registry")
    FString SlotName;

    // The user index passed to SaveGameToSlot / LoadGameFromSlot.
    UPROPERTY(BlueprintReadWrite, Category = "Save Registry")
    int32 UserIndex = 0;
};
