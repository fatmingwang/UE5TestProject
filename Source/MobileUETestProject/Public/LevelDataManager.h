#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyLevelSaveGame.h"
#include "LevelDataManager.generated.h"

// Define tag name for saveable actors
#define SAVEABLE_ACTOR_TAG TEXT("Saveable")

UCLASS()
class MOBILEUETESTPROJECT_API ALevelDataManager : public AActor
{
    GENERATED_BODY()

public:    
    ALevelDataManager();

    // Get or create the singleton instance of LevelDataManager
    UFUNCTION(BlueprintCallable, Category = "Level Save", 
        meta = (WorldContext = "WorldContextObject", 
               DisplayName = "Get Level Data Manager Instance"))
    static ALevelDataManager* GetOrCreateInstance(const UObject* WorldContextObject);
    // Returns the saveable actor tag constant - use this instead of hardcoding "Saveable"
    UFUNCTION(BlueprintPure, Category = "Level Save",
              meta = (DisplayName = "Get Saveable Tag", CompactNodeTitle = "SaveableTag"))

    static FName GetSaveableTag()
    {
        return FName(SAVEABLE_ACTOR_TAG);
    }
    // Save all actors with a specific tag to file
    UFUNCTION(BlueprintCallable, Category = "Level Save",
        meta = (DisplayName = "Save Level Actors (Binary)"))
    bool SaveLevelActors(const FString& SlotName, const FName& ActorTag = "Saveable");

    // Load and spawn actors from save file
    UFUNCTION(BlueprintCallable, Category = "Level Save",
        meta = (DisplayName = "Load Level Actors (Binary)"))
    bool LoadLevelActors(const FString& SlotName, bool bClearExistingActors = true);

    // Save to JSON file (alternative method)
    UFUNCTION(BlueprintCallable, Category = "Level Save",
        meta = (DisplayName = "Save Level Actors (JSON)"))
    bool SaveLevelActorsToJSON(const FString& FileName, const FName& ActorTag = "Saveable");

    // Load from JSON file
    UFUNCTION(BlueprintCallable, Category = "Level Save",
        meta = (DisplayName = "Load Level Actors (JSON)"))
    bool LoadLevelActorsFromJSON(const FString& FileName, bool bClearExistingActors = true);

    // Clear all actors with specific tag
    UFUNCTION(BlueprintCallable, Category = "Level Save",
        meta = (DisplayName = "Clear Saved Actors"))
    void ClearSavedActors(const FName& ActorTag = "Saveable");

private:
    // Helper function to serialize actor data
    FActorSaveData SerializeActor(AActor* Actor);

    // Helper function to deserialize and spawn actor
    AActor* DeserializeActor(const FActorSaveData& SaveData);
};
