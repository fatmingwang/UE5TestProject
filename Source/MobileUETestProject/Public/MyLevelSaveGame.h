#pragma once
#pragma once

#include "CoreMinimal.h"
#include "SaveGameRegistry.h"
#include "MyLevelSaveGame.generated.h"

// Struct to store actor information
USTRUCT(BlueprintType)
struct FActorSaveData
{
    GENERATED_BODY()

    UPROPERTY()
    TSubclassOf<AActor> ActorClass;

    UPROPERTY()
    FTransform Transform;

    UPROPERTY()
    FString ActorName;

    // Add any custom properties your actors need to save
    UPROPERTY()
    TMap<FString, FString> CustomProperties;
};

/**
 * Save game class to store level actor information
 */
UCLASS()
class MOBILEUETESTPROJECT_API UMyLevelSaveGame : public URegisteredSaveGame
{
    GENERATED_BODY()

public:
    UMyLevelSaveGame();

    UPROPERTY()
    TArray<FActorSaveData> SavedActors;

    UPROPERTY()
    FString LevelName;

    UPROPERTY()
    FDateTime SaveTime;
};
