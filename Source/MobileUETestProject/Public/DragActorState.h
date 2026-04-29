#pragma once

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"

/** Captures and restores a primitive component's mobility and physics state around a drag operation. */
struct FDragActorState
{
    FDragActorState()
        : CapturedComponent(nullptr)
        , Mobility(EComponentMobility::Static)
        , bSimulatePhysics(false)
    {}

    /** Capture state from the exact component that was hit, then prepare it for dragging. */
    void Capture(UPrimitiveComponent* PrimComp)
    {
        if (!PrimComp)
        {
            UE_LOG(LogTemp, Error, TEXT("DragActorState::Capture — null PrimitiveComponent"));
            return;
        }

        CapturedComponent = PrimComp;
        Mobility          = PrimComp->GetMobility();
        bSimulatePhysics  = PrimComp->IsSimulatingPhysics();
        UE_LOG(LogTemp, Warning, TEXT("DragActorState::Capture [%s] Mobility=%d SimPhysics=%s"),
            *PrimComp->GetName(), (int32)Mobility, bSimulatePhysics ? TEXT("true") : TEXT("false"));

        // Set Movable FIRST to avoid warnings and prevent physics body re-init
        if (Mobility != EComponentMobility::Movable)
            PrimComp->SetMobility(EComponentMobility::Movable);

        // Disable physics AFTER mobility is set
        if (bSimulatePhysics)
            PrimComp->SetSimulatePhysics(false);

        UE_LOG(LogTemp, Warning, TEXT("DragActorState::Capture END [%s] CurrentMobility=%d IsSimulatingNow=%s"),
            *PrimComp->GetName(), (int32)PrimComp->GetMobility(),
            PrimComp->IsSimulatingPhysics() ? TEXT("true") : TEXT("false"));
    }

    /** Move the captured component directly to a world position, bypassing root hierarchy. */
    void SetPosition(const FVector& NewWorldLocation)
    {
        if (!CapturedComponent)
        {
            UE_LOG(LogTemp, Error, TEXT("DragActorState::SetPosition - no captured component"));
            return;
        }
        // Move the exact physics component directly - works regardless of root hierarchy
        CapturedComponent->SetWorldLocation(NewWorldLocation, false, nullptr, ETeleportType::TeleportPhysics);
    }

    /** Restore the exact state saved by Capture(). */
    void Restore()
    {
        if (!CapturedComponent)
        {
            UE_LOG(LogTemp, Error, TEXT("DragActorState::Restore — no captured component to restore"));
            return;
        }

        UE_LOG(LogTemp, Warning, TEXT("DragActorState::Restore [%s] Mobility=%d SimPhysics=%s"),
            *CapturedComponent->GetName(), (int32)Mobility, bSimulatePhysics ? TEXT("true") : TEXT("false"));
        CapturedComponent->SetMobility(Mobility);
        CapturedComponent->SetSimulatePhysics(bSimulatePhysics);
        CapturedComponent = nullptr;
    }

private:
    UPrimitiveComponent*     CapturedComponent;
    EComponentMobility::Type Mobility;
    bool                     bSimulatePhysics;
};