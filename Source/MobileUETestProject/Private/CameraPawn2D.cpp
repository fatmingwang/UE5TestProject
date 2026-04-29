#include "CameraPawn2D.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "LevelDataManager.h"

ACameraPawn2D::ACameraPawn2D()
{
    m_bUseXYNotXZ = false;
    PanSpeed = 50.f;
    PrimaryActorTick.bCanEverTick = true;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    RootComponent = CameraComponent;
    CameraComponent->SetProjectionMode(ECameraProjectionMode::Orthographic);
    CameraComponent->SetOrthoWidth(2048.f);
    m_bFlipXYIfXIsRotation90Degree = false;
    DraggedActor = nullptr;
    DragOffset = FVector::ZeroVector;
}

void ACameraPawn2D::SetPlayerControllerMouseBehivor()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }
    //PC->bShowMouseCursor = true;
    PC->bEnableClickEvents = true;
    PC->bShowMouseCursor = true;                    // Hide real cursor
    PC->SetInputMode(FInputModeGameOnly());

    if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
    {
        ViewportClient->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
        ViewportClient->SetMouseLockMode(EMouseLockMode::LockAlways);
    }
    //get X rotation  if its 90 then make m_bFlipXYIfXIsRotation90Degree  to true
    float XRotation = GetActorRotation().Pitch; // Roll = rotation around X axis
    m_bFlipXYIfXIsRotation90Degree = FMath::IsNearlyEqual(XRotation, -90.f, 1.f);
	UE_LOG(LogTemp, Warning, TEXT("CameraPawn2D Rotation Pitch: %f, m_bFlipXYIfXIsRotation90Degree: %s"), XRotation, m_bFlipXYIfXIsRotation90Degree ? TEXT("true") : TEXT("false"));

    //this->getcompo
}

void ACameraPawn2D::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("ACameraPawn2D::BeginPlay"));
    SetPlayerControllerMouseBehivor();
}

void ACameraPawn2D::PawnClientRestart()
{
    Super::PawnClientRestart();
    // 1. Get the Player Controller
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    // 2. Get the Subsystem
    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
        return;
    }
    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

    if (Subsystem && InputConfig)    
    {
        // 3. Clear old contexts to avoid "input ghosting" from the previous pawn
        Subsystem->ClearAllMappings();

        // 4. Add your new contexts
        if (InputConfig->m_CameraIMC)
        {
            Subsystem->AddMappingContext(InputConfig->m_CameraIMC, 0);
            UE_LOG(LogTemp, Warning, TEXT("CameraContext injected via PawnClientRestart"));
        }

    }
    SetPlayerControllerMouseBehivor();
}

void ACameraPawn2D::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UE_LOG(LogTemp, Warning, TEXT("=== SetupPlayerInputComponent Called ==="));
    
    if (!InputConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("InputConfig is NULL! Assign it in the Blueprint/Details panel!"));
        return;
    }

    // Bind Actions ONLY - Do NOT add mapping contexts here
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent found - Binding actions"));
        
        if (InputConfig->m_KeyboardActionMove)
        {
            EIC->BindAction(InputConfig->m_KeyboardActionMove, ETriggerEvent::Triggered, this, &ACameraPawn2D::HandleMoveByKeyboardWASD);
            UE_LOG(LogTemp, Warning, TEXT("ActionMove bound successfully"));
        }
        
        if (InputConfig->m_MouseLeftButtonActionMove)
        {
            EIC->BindAction(InputConfig->m_MouseLeftButtonActionMove, ETriggerEvent::Started, this, &ACameraPawn2D::HandleClick);
            EIC->BindAction(InputConfig->m_MouseLeftButtonActionMove, ETriggerEvent::Completed, this, &ACameraPawn2D::HandleClickReleased);
            UE_LOG(LogTemp, Warning, TEXT("ActionClick bound successfully"));
        }
        
        if (InputConfig->m_MouseWheelActionZoom)
        {
            EIC->BindAction(InputConfig->m_MouseWheelActionZoom, ETriggerEvent::Triggered, this, &ACameraPawn2D::HandleZoom);
            UE_LOG(LogTemp, Warning, TEXT("ActionZoom bound successfully"));
        }
        
        // Bind Mouse Movement for panning
        if (InputConfig->m_MouseActionMove)
        {
            EIC->BindAction(InputConfig->m_MouseActionMove, ETriggerEvent::Triggered, this, &ACameraPawn2D::HandleCameraPan);
            UE_LOG(LogTemp, Warning, TEXT("ActionMouseMove bound successfully"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast to EnhancedInputComponent"));
    }
}

FVector2D ACameraPawn2D::GetMovedVec(FVector2D e_MoveVec)
{
    if(0)
    if (m_bFlipXYIfXIsRotation90Degree)
    {
        FVector2D l_Original = e_MoveVec;
        e_MoveVec.X = l_Original.Y;
        e_MoveVec.Y = -l_Original.X;
    }
	return e_MoveVec;
}

void ACameraPawn2D::HandleMoveByKeyboardWASD(const FInputActionValue& Value)
{
    //UE_LOG(LogTemp, Warning, TEXT(">>> HandleMoveByKeyboardWASD CALLED! <<<"));
    FVector2D MoveVec = Value.Get<FVector2D>();
    MoveVec = GetMovedVec(MoveVec);
    //UE_LOG(LogTemp, Warning, TEXT("Move Vector: X=%f, Y=%f"), MoveVec.X, MoveVec.Y);
    // Move along XY plane (Z remains constant)
    if (m_bUseXYNotXZ)
    {
        AddActorWorldOffset(FVector(MoveVec.Y * 20.f, -MoveVec.X * 20.f, 0.f));
    }
    else
    {
        AddActorWorldOffset(FVector(MoveVec.Y * 20.f, 0.f, MoveVec.X * 20.f));
    }
    
}

void ACameraPawn2D::GetActorPlaneDepth(FVector& e_Input) const
{
    // XY mode: depth is on Z axis; XZ mode: depth is on Y axis
    if (m_bUseXYNotXZ)
        e_Input.Z = m_ActorPlaneDepthXY;
    else
        e_Input.Y = m_ActorPlaneDepthXZ;
}

void ACameraPawn2D::HandleClick()
{
    UE_LOG(LogTemp, Warning, TEXT(">>> HandleClick CALLED <<<"));

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
        return;

    float MouseX, MouseY;
    if (!PC->GetMousePosition(MouseX, MouseY))
        return;

    FVector WorldLocation, WorldDirection;
    PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);

    // First: check if an existing actor is under the cursor
    FHitResult HitResult;
    FVector TraceEnd = WorldLocation + WorldDirection * 100000.f;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_Visibility, Params))
    {
        AActor* HitActor = HitResult.GetActor();
        if (HitActor && HitActor != this)
        {
            // Only allow dragging if the hit actor is the same type as m_TargetPlaceActor
            if (m_TargetPlaceActor && HitActor->IsA(m_TargetPlaceActor))
            {
                DraggedActor = HitActor;
                // Use the hit component directly; if it is a non-primitive DefaultSceneRoot,
                // fall back to the first UPrimitiveComponent child on the actor
                UPrimitiveComponent* TargetComp = Cast<UPrimitiveComponent>(HitResult.GetComponent());
                if (!TargetComp)
                {
                    TArray<UPrimitiveComponent*> PrimComps;
                    HitActor->GetComponents<UPrimitiveComponent>(PrimComps);
                    TargetComp = PrimComps.Num() > 0 ? PrimComps[0] : nullptr;
                    UE_LOG(LogTemp, Warning, TEXT("CameraPawn2D: HitComponent is not primitive, using child: %s"),
                        TargetComp ? *TargetComp->GetName() : TEXT("none"));
                }
                // Use component world position (not actor origin) for correct offset
                // when root is DefaultSceneRoot and the visual mesh is a child component
                FVector ClickPlanePos = WorldLocation;
                GetActorPlaneDepth(ClickPlanePos);
                FVector ComponentPos = TargetComp ? TargetComp->GetComponentLocation() : DraggedActor->GetActorLocation();
                DragOffset = ComponentPos - ClickPlanePos;
                DraggedActorOriginalState.Capture(TargetComp);
                UE_LOG(LogTemp, Log, TEXT("CameraPawn2D: Grabbed %s (matches TargetPlaceActor type)"), *DraggedActor->GetName());
                return; // dragging — do not spawn
            }
            else
            {
                // Hit actor is a different type — ignore it and fall through to spawn
                UE_LOG(LogTemp, Log, TEXT("CameraPawn2D: Clicked %s but type does not match TargetPlaceActor, spawning instead"), *HitActor->GetName());
            }
        }
    }

    // No actor hit — spawn a new one
    if (!m_TargetPlaceActor)
    {
        UE_LOG(LogTemp, Error, TEXT("PinClass is NULL!"));
        return;
    }
    
    FVector SpawnPos = WorldLocation;
    GetActorPlaneDepth(SpawnPos);
    
    
    
    // Spawn the actor
    AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(m_TargetPlaceActor, SpawnPos, FRotator::ZeroRotator);

    if (SpawnedActor)
    {
        // Prevent spawned actor from capturing input
        SpawnedActor->DisableInput(PC);

        // Add force to shoot the actor along +Y axis
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SpawnedActor->GetRootComponent()))
        {
            PrimComp->SetSimulatePhysics(true);
            PrimComp->AddImpulse(FVector(0.f, 3000.f, 0.f), NAME_None, /*bVelChange=*/true);
        }


        // Add tag so it can be saved/loaded
        SpawnedActor->Tags.AddUnique(SAVEABLE_ACTOR_TAG);

        // Enable physics on the root primitive component
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SpawnedActor->GetRootComponent()))
        {
            //PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            //PrimComp->SetSimulatePhysics(true);
            //PrimComp->SetCollisionProfileName(TEXT("PhysicsActor")); // required for physics to simulate
            //PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            //PrimComp->SetSimulatePhysics(true);

            //PrimComp->SetConstraintMode(EDOFMode::XZPlane); // or YZPlane depending on your camera axis
            //PrimComp->SetEnableGravity(false); // disable if you don't want falling
        }

        // Log actor's position
        UE_LOG(LogTemp, Log, TEXT("Actor Position: %s"), *SpawnedActor->GetActorLocation().ToString());

        UE_LOG(LogTemp, Warning, TEXT("✅ Spawned: %s at %s"), 
            *SpawnedActor->GetName(), *SpawnPos.ToString());
    }
    

}

void ACameraPawn2D::BP_SetTargetPlaceActor(TSubclassOf<AActor> NewActorClass)
{
    m_TargetPlaceActor = NewActorClass;
}

TSubclassOf<AActor> ACameraPawn2D::BP_GetTargetPlaceActor() const
{
    return m_TargetPlaceActor;
}

void ACameraPawn2D::HandleCameraPan(const FInputActionValue& Value)
{

    FVector2D Delta = Value.Get<FVector2D>();

    if (Delta.IsNearlyZero())
    {
        return;
    }
    Delta = GetMovedVec(Delta);
    FVector Loc = GetActorLocation();
    Loc.X -= Delta.X * PanSpeed ;
    if (m_bUseXYNotXZ)
    {
        Loc.Y += Delta.Y * PanSpeed;
    }
    else
    {
        Loc.Z -= Delta.Y * PanSpeed;
    }
    

    SetActorLocation(Loc);
}

void ACameraPawn2D::HandleClickReleased()
{
    if (DraggedActor)
    {
        // Restore the exact original state saved at grab time
        DraggedActorOriginalState.Restore();
        UE_LOG(LogTemp, Log, TEXT("CameraPawn2D: Released %s"), *DraggedActor->GetName());
        DraggedActor = nullptr;
    }
}

void ACameraPawn2D::Tick(float DeltaTime)
{
    // Move dragged actor with the mouse cursor
    if (DraggedActor)
    {
        APlayerController* PC = Cast<APlayerController>(GetController());
        if (!PC)
        {
            UE_LOG(LogTemp, Error, TEXT("CameraPawn2D::Tick — DraggedActor set but PlayerController is null"));
            return;
        }

        float MouseX, MouseY;
        if (!PC->GetMousePosition(MouseX, MouseY))
        {
            UE_LOG(LogTemp, Warning, TEXT("CameraPawn2D::Tick — GetMousePosition failed"));
            return;
        }

        FVector WorldLocation, WorldDirection;
        PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);

        FVector NewLocation = WorldLocation;
        GetActorPlaneDepth(NewLocation);
        FVector FinalLocation = NewLocation + DragOffset;

        // Move the exact captured component directly - works for any root hierarchy
        DraggedActorOriginalState.SetPosition(FinalLocation);
    }
}

void ACameraPawn2D::HandleZoom(const FInputActionValue& Value)
{
    float AxisValue = Value.Get<float>();
    //UE_LOG(LogTemp, Warning, TEXT(">>> HandleZoom CALLED! Zoom Value: %f <<<"), AxisValue);
    
    if (FMath::Abs(AxisValue) > KINDA_SMALL_NUMBER)
    {
        float OldWidth = CameraComponent->OrthoWidth;
        float NewWidth = OldWidth - AxisValue * ZoomStep;
        NewWidth = FMath::Clamp(NewWidth, MinOrthoWidth, MaxOrthoWidth);
        CameraComponent->SetOrthoWidth(NewWidth);
        
        //UE_LOG(LogTemp, Warning, TEXT("Zoom: %f -> %f (Delta: %f)"), OldWidth, NewWidth, AxisValue);
    }
}

FVector2D ACameraPawn2D::GetMousePosition() const
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        float X, Y;
        if (PC->GetMousePosition(X, Y))
        {
            return FVector2D(X, Y);
        }
    }
    return FVector2D::ZeroVector;
}

FVector ACameraPawn2D::GetWorldPositionFromScreen(const FVector2D& ScreenPosition) const
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        FVector WorldOrigin, WorldDirection;
        if (PC->DeprojectScreenPositionToWorld(ScreenPosition.X, ScreenPosition.Y, WorldOrigin, WorldDirection))
        {
            // Project onto world Z=0 plane for 2D
            float Distance = -WorldOrigin.Z / WorldDirection.Z;
            return WorldOrigin + WorldDirection * Distance;
        }
    }
    return FVector::ZeroVector;
}

void ACameraPawn2D::SetEditorMode(bool bIsPlacementMode)
{
    //APlayerController* l_pPC = Cast<APlayerController>(GetController());
    //if (!l_pPC)
    //{
    //    return;
    //}

    //UEnhancedInputLocalPlayerSubsystem* l_pSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(l_pPC->GetLocalPlayer());
    //if (!l_pSubsystem)
    //{
    //    return;
    //}

    //if (bIsPlacementMode)
    //{
    //    // Add placement rules on top of movement (Priority 1)
    //    l_pSubsystem->AddMappingContext(PlacementMappingContext, 1);
    //}
    //else
    //{
    //    // Remove placement rules
    //    l_pSubsystem->RemoveMappingContext(PlacementMappingContext);
    //}
}