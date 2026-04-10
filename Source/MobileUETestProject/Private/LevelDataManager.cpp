#include "LevelDataManager.h"
#include "LevelDataManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"

ALevelDataManager::ALevelDataManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

ALevelDataManager* ALevelDataManager::GetOrCreateInstance(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        UE_LOG(LogTemp, Error, TEXT("GetOrCreateInstance: WorldContextObject is null"));
        return nullptr;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("GetOrCreateInstance: Failed to get world"));
        return nullptr;
    }

    // Try to find existing instance
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALevelDataManager::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        ALevelDataManager* Manager = Cast<ALevelDataManager>(FoundActors[0]);
        if (Manager)
        {
            UE_LOG(LogTemp, Log, TEXT("GetOrCreateInstance: Found existing LevelDataManager"));
            return Manager;
        }
    }

    // No instance exists, create one
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName(TEXT("LevelDataManager_Singleton"));
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ALevelDataManager* NewManager = World->SpawnActor<ALevelDataManager>(
        ALevelDataManager::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (NewManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetOrCreateInstance: Created new LevelDataManager instance"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GetOrCreateInstance: Failed to spawn LevelDataManager"));
    }

    return NewManager;
}

bool ALevelDataManager::SaveLevelActors(const FString& SlotName, const FName& ActorTag)
{
    UMyLevelSaveGame* SaveGameInstance = Cast<UMyLevelSaveGame>(UGameplayStatics::CreateSaveGameObject(UMyLevelSaveGame::StaticClass()));

    if (!SaveGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create save game object"));
        return false;
    }

    // Clear previous data
    SaveGameInstance->SavedActors.Empty();
    SaveGameInstance->LevelName = GetWorld()->GetMapName();
    SaveGameInstance->SaveTime = FDateTime::Now();

    // Find all actors with the specified tag
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), ActorTag, FoundActors);

    UE_LOG(LogTemp, Warning, TEXT("Found %d actors with tag '%s' to save"), FoundActors.Num(), *ActorTag.ToString());

    // Serialize each actor
    for (AActor* Actor : FoundActors)
    {
        if (Actor && !Actor->IsPendingKillPending())
        {
            FActorSaveData SaveData = SerializeActor(Actor);
            SaveGameInstance->SavedActors.Add(SaveData);

            UE_LOG(LogTemp, Log, TEXT("Saved actor: %s at location %s"), 
                *Actor->GetName(), *Actor->GetActorLocation().ToString());
        }
    }

    // Save to disk
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("? Successfully saved %d actors to slot '%s'"), 
            SaveGameInstance->SavedActors.Num(), *SlotName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("? Failed to save game to slot '%s'"), *SlotName);
    }

    return bSuccess;
}

bool ALevelDataManager::LoadLevelActors(const FString& SlotName, bool bClearExistingActors)
{
    // Check if save file exists
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogTemp, Error, TEXT("Save game slot '%s' does not exist"), *SlotName);
        return false;
    }

    // Load save game
    UMyLevelSaveGame* LoadedGame = Cast<UMyLevelSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!LoadedGame)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load save game from slot '%s'"), *SlotName);
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("Loaded save game with %d actors from '%s' (saved at %s)"), 
        LoadedGame->SavedActors.Num(), *LoadedGame->LevelName, *LoadedGame->SaveTime.ToString());

    // Clear existing actors if requested
    if (bClearExistingActors)
    {
        ClearSavedActors(SAVEABLE_ACTOR_TAG);
    }

    // Spawn actors from saved data
    int32 SpawnedCount = 0;
    for (const FActorSaveData& SaveData : LoadedGame->SavedActors)
    {
        AActor* SpawnedActor = DeserializeActor(SaveData);
        if (SpawnedActor)
        {
            SpawnedCount++;
            UE_LOG(LogTemp, Log, TEXT("Spawned actor: %s at location %s"), 
                *SpawnedActor->GetName(), *SpawnedActor->GetActorLocation().ToString());
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("? Successfully spawned %d actors from save"), SpawnedCount);
    return SpawnedCount > 0;
}

bool ALevelDataManager::SaveLevelActorsToJSON(const FString& FileName, const FName& ActorTag)
{
    // Find all actors with the specified tag
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), ActorTag, FoundActors);

    // Create JSON object
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

    // Add metadata
    RootObject->SetStringField(TEXT("LevelName"), GetWorld()->GetMapName());
    RootObject->SetStringField(TEXT("SaveTime"), FDateTime::Now().ToString());

    // Create array of actors
    TArray<TSharedPtr<FJsonValue>> ActorArray;

    for (AActor* Actor : FoundActors)
    {
        if (Actor && !Actor->IsPendingKillPending())
        {
            TSharedPtr<FJsonObject> ActorObject = MakeShareable(new FJsonObject);

            // Store actor class
            ActorObject->SetStringField(TEXT("ClassName"), Actor->GetClass()->GetPathName());

            // Store transform
            FTransform Transform = Actor->GetActorTransform();
            TSharedPtr<FJsonObject> TransformObject = MakeShareable(new FJsonObject);
            TransformObject->SetStringField(TEXT("Location"), Transform.GetLocation().ToString());
            TransformObject->SetStringField(TEXT("Rotation"), Transform.Rotator().ToString());
            TransformObject->SetStringField(TEXT("Scale"), Transform.GetScale3D().ToString());
            ActorObject->SetObjectField(TEXT("Transform"), TransformObject);

            // Store actor name
            ActorObject->SetStringField(TEXT("Name"), Actor->GetName());

            ActorArray.Add(MakeShareable(new FJsonValueObject(ActorObject)));
        }
    }

    RootObject->SetArrayField(TEXT("Actors"), ActorArray);

    // Serialize to string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    // Save to file
    FString FilePath = FPaths::ProjectSavedDir() + TEXT("LevelData/") + FileName + TEXT(".json");

    // Create directory if it doesn't exist
    FString Directory = FPaths::GetPath(FilePath);
    if (!FPaths::DirectoryExists(Directory))
    {
        IFileManager::Get().MakeDirectory(*Directory, true);
    }

    bool bSuccess = FFileHelper::SaveStringToFile(OutputString, *FilePath);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("? Saved %d actors to JSON file: %s"), FoundActors.Num(), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("? Failed to save to JSON file: %s"), *FilePath);
    }

    return bSuccess;
}

bool ALevelDataManager::LoadLevelActorsFromJSON(const FString& FileName, bool bClearExistingActors)
{
    FString FilePath = FPaths::ProjectSavedDir() + TEXT("LevelData/") + FileName + TEXT(".json");

    // Check if file exists
    if (!FPaths::FileExists(FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("JSON file does not exist: %s"), *FilePath);
        return false;
    }

    // Load file
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load JSON file: %s"), *FilePath);
        return false;
    }

    // Parse JSON
    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON from file: %s"), *FilePath);
        return false;
    }

    // Clear existing actors if requested
    if (bClearExistingActors)
    {
        ClearSavedActors(SAVEABLE_ACTOR_TAG);
    }

    // Get actor array
    const TArray<TSharedPtr<FJsonValue>>* ActorArray;
    if (!RootObject->TryGetArrayField(TEXT("Actors"), ActorArray))
    {
        UE_LOG(LogTemp, Error, TEXT("No 'Actors' array found in JSON"));
        return false;
    }

    int32 SpawnedCount = 0;

    for (const TSharedPtr<FJsonValue>& ActorValue : *ActorArray)
    {
        const TSharedPtr<FJsonObject>* ActorObject;
        if (!ActorValue->TryGetObject(ActorObject))
            continue;

        // Get actor class
        FString ClassName;
        if (!(*ActorObject)->TryGetStringField(TEXT("ClassName"), ClassName))
            continue;

        UClass* ActorClass = LoadObject<UClass>(nullptr, *ClassName);
        if (!ActorClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load class: %s"), *ClassName);
            continue;
        }

        // Get transform
        const TSharedPtr<FJsonObject>* TransformObject;
        if (!(*ActorObject)->TryGetObjectField(TEXT("Transform"), TransformObject))
            continue;

        FString LocationStr, RotationStr, ScaleStr;
        (*TransformObject)->TryGetStringField(TEXT("Location"), LocationStr);
        (*TransformObject)->TryGetStringField(TEXT("Rotation"), RotationStr);
        (*TransformObject)->TryGetStringField(TEXT("Scale"), ScaleStr);

        FVector Location;
        FRotator Rotation;
        FVector Scale(1.0f);

        Location.InitFromString(LocationStr);
        Rotation.InitFromString(RotationStr);
        Scale.InitFromString(ScaleStr);

        FTransform Transform(Rotation, Location, Scale);

        // Spawn actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, Transform, SpawnParams);

        if (SpawnedActor)
        {
            // Add the saveable tag
            SpawnedActor->Tags.AddUnique(SAVEABLE_ACTOR_TAG);

            SpawnedCount++;
            UE_LOG(LogTemp, Log, TEXT("Spawned actor: %s at location %s"), 
                *SpawnedActor->GetName(), *SpawnedActor->GetActorLocation().ToString());
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("? Successfully spawned %d actors from JSON"), SpawnedCount);
    return SpawnedCount > 0;
}

void ALevelDataManager::ClearSavedActors(const FName& ActorTag)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), ActorTag, FoundActors);

    int32 DestroyedCount = 0;
    for (AActor* Actor : FoundActors)
    {
        if (Actor && !Actor->IsPendingKillPending())
        {
            Actor->Destroy();
            DestroyedCount++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Cleared %d actors with tag '%s'"), DestroyedCount, *ActorTag.ToString());
}

FActorSaveData ALevelDataManager::SerializeActor(AActor* Actor)
{
    FActorSaveData SaveData;

    SaveData.ActorClass = Actor->GetClass();
    SaveData.Transform = Actor->GetActorTransform();
    SaveData.ActorName = Actor->GetName();

    // You can add custom property serialization here
    // For example, if your actor has specific variables you want to save

    return SaveData;
}

AActor* ALevelDataManager::DeserializeActor(const FActorSaveData& SaveData)
{
    if (!SaveData.ActorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid actor class in save data"));
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SaveData.ActorClass, SaveData.Transform, SpawnParams);

    if (SpawnedActor)
    {
        // Add the saveable tag so it can be saved again
        SpawnedActor->Tags.AddUnique(SAVEABLE_ACTOR_TAG);

        // Restore any custom properties here
    }

    return SpawnedActor;
}
