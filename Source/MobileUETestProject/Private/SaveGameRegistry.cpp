#include "SaveGameRegistry.h"
#include "SaveGameRegistry.h"
#include "Kismet/GameplayStatics.h"

const FString USaveGameRegistry::RegistrySlotName = TEXT("SaveGameRegistry");

USaveGameRegistry::USaveGameRegistry()
{
}

void USaveGameRegistry::RegisterSaveSlot(const FSaveSlotInfo& SlotInfo)
{
    for (FSaveSlotInfo& Existing : SaveSlots)
    {
        if (Existing.SlotName == SlotInfo.SlotName)
        {
            Existing = SlotInfo;
            return;
        }
    }
    SaveSlots.Add(SlotInfo);
}

bool USaveGameRegistry::UnregisterSaveSlot(const FString& InSlotName)
{
    for (int32 i = 0; i < SaveSlots.Num(); ++i)
    {
        if (SaveSlots[i].SlotName == InSlotName)
        {
            SaveSlots.RemoveAt(i);
            return true;
        }
    }
    return false;
}

FSaveSlotInfo* USaveGameRegistry::FindSaveSlot(const FString& InSlotName)
{
    for (FSaveSlotInfo& Slot : SaveSlots)
    {
        if (Slot.SlotName == InSlotName)
        {
            return &Slot;
        }
    }
    return nullptr;
}

bool USaveGameRegistry::DoesSaveSlotExist(const FString& InSlotName) const
{
    for (const FSaveSlotInfo& Slot : SaveSlots)
    {
        if (Slot.SlotName == InSlotName)
        {
            return true;
        }
    }
    return false;
}

void USaveGameRegistry::SortByNewest()
{
    SaveSlots.Sort([](const FSaveSlotInfo& A, const FSaveSlotInfo& B)
    {
        return A.SaveTime > B.SaveTime;
    });
}

void USaveGameRegistry::SortByOldest()
{
    SaveSlots.Sort([](const FSaveSlotInfo& A, const FSaveSlotInfo& B)
    {
        return A.SaveTime < B.SaveTime;
    });
}

void USaveGameRegistry::SortByName()
{
    SaveSlots.Sort([](const FSaveSlotInfo& A, const FSaveSlotInfo& B)
    {
        return A.SlotName < B.SlotName;
    });
}

void USaveGameRegistry::UpdateRegistry(UObject* WorldContextObject, const FSaveSlotInfo& SlotInfo)
{
    if (!WorldContextObject)
    {
        return;
    }

    // Load existing registry or create a new one
    USaveGameRegistry* Registry = nullptr;
    if (UGameplayStatics::DoesSaveGameExist(RegistrySlotName, 0))
    {
        Registry = Cast<USaveGameRegistry>(
            UGameplayStatics::LoadGameFromSlot(RegistrySlotName, 0));
    }

    if (!Registry)
    {
        Registry = Cast<USaveGameRegistry>(
            UGameplayStatics::CreateSaveGameObject(USaveGameRegistry::StaticClass()));
    }

    if (!Registry)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateRegistry: Failed to get or create registry"));
        return;
    }

    Registry->RegisterSaveSlot(SlotInfo);

    if (UGameplayStatics::SaveGameToSlot(Registry, RegistrySlotName, 0))
    {
        UE_LOG(LogTemp, Log, TEXT("UpdateRegistry: Registry saved with %d entries"), Registry->SaveSlots.Num());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateRegistry: Failed to save registry"));
    }
}

bool USaveGameRegistry::SaveGame(UObject* WorldContextObject, USaveGame* SaveGameObject,
    const FString& SlotName, int32 UserIndex, const FString& DisplayName)
{
    if (!WorldContextObject || !SaveGameObject || SlotName.IsEmpty())
    {
        return false;
    }

    if (!UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, UserIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("SaveGame: Failed to save slot '%s'"), *SlotName);
        return false;
    }

    // Build registry metadata automatically from the save object.
    FString LevelName;
    int32 ActorCount = 0;

    // Try to read known fields from UMyLevelSaveGame or any URegisteredSaveGame subclass.
    if (URegisteredSaveGame* Registered = Cast<URegisteredSaveGame>(SaveGameObject))
    {
        // LevelName property may exist on the concrete subclass via reflection.
        if (FStrProperty* Prop = FindFProperty<FStrProperty>(SaveGameObject->GetClass(), TEXT("LevelName")))
        {
            LevelName = Prop->GetPropertyValue_InContainer(SaveGameObject);
        }
        // ActorCount from a TArray property named "SavedActors".
        if (FArrayProperty* Prop = FindFProperty<FArrayProperty>(SaveGameObject->GetClass(), TEXT("SavedActors")))
        {
            FScriptArrayHelper Helper(Prop, Prop->ContainerPtrToValuePtr<void>(SaveGameObject));
            ActorCount = Helper.Num();
        }
        if (LevelName.IsEmpty() && WorldContextObject)
        {
            LevelName = UGameplayStatics::GetCurrentLevelName(WorldContextObject);
        }
    }

    UpdateRegistry(WorldContextObject, FSaveSlotInfo(SlotName, LevelName, ActorCount, DisplayName));
    UE_LOG(LogTemp, Log, TEXT("SaveGame: Slot '%s' saved and registered"), *SlotName);
    return true;
}
