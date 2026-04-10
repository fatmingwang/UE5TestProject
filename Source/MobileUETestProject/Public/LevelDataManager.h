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
    UFUNCTION(BlueprintCallable, Category = "Level Save", meta = (WorldContext = "WorldContextObject"))
    static ALevelDataManager* GetOrCreateInstance(const UObject* WorldContextObject);

    // Save all actors with a specific tag to file
    UFUNCTION(BlueprintCallable, Category = "Level Save")
    bool SaveLevelActors(const FString& SlotName, const FName& ActorTag = "Saveable");

    // Load and spawn actors from save file
    UFUNCTION(BlueprintCallable, Category = "Level Save")
    bool LoadLevelActors(const FString& SlotName, bool bClearExistingActors = true);

    // Save to JSON file (alternative method)
    UFUNCTION(BlueprintCallable, Category = "Level Save")
    bool SaveLevelActorsToJSON(const FString& FileName, const FName& ActorTag = "Saveable");

    // Load from JSON file
    UFUNCTION(BlueprintCallable, Category = "Level Save")
    bool LoadLevelActorsFromJSON(const FString& FileName, bool bClearExistingActors = true);

    // Clear all actors with specific tag
    UFUNCTION(BlueprintCallable, Category = "Level Save")
    void ClearSavedActors(const FName& ActorTag = "Saveable");

private:
    // Helper function to serialize actor data
    FActorSaveData SerializeActor(AActor* Actor);

    // Helper function to deserialize and spawn actor
    AActor* DeserializeActor(const FActorSaveData& SaveData);
};
