#include "CameraPawn2D.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "LevelDataManager.h"

ACameraPawn2D::ACameraPawn2D()
{
    PanSpeed = 100.f;
    PrimaryActorTick.bCanEverTick = true;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    RootComponent = CameraComponent;
    CameraComponent->SetProjectionMode(ECameraProjectionMode::Orthographic);
    CameraComponent->SetOrthoWidth(2048.f);
}

void ACameraPawn2D::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    //PC->bShowMouseCursor = true;
    PC->bEnableClickEvents = true;
    PC->bShowMouseCursor = false;                    // Hide real cursor
    PC->SetInputMode(FInputModeGameOnly());
    //PC->SetInputMode(FInputModeGameAndUI());

    // CapturePermanently: mouse input ALWAYS reaches Enhanced Input regardless of
    // which button is pressed first. DoNotLock keeps cursor freely movable.
    if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
    {
        ViewportClient->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
        ViewportClient->SetMouseLockMode(EMouseLockMode::DoNotLock);
    }
}

void ACameraPawn2D::PawnClientRestart()
{
    Super::PawnClientRestart();

    // 1. Get the Player Controller
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    // 2. Get the Subsystem
    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer) return;

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

    //PC->bShowMouseCursor = true;
    PC->bEnableClickEvents = true;
    //PC->SetInputMode(FInputModeGameAndUI());
    PC->bShowMouseCursor = false;                    // Hide real cursor
    PC->SetInputMode(FInputModeGameOnly());
    // CapturePermanently: mouse input ALWAYS reaches Enhanced Input regardless of
    // which button is pressed first. DoNotLock keeps cursor freely movable.
    if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
    {
        ViewportClient->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
        ViewportClient->SetMouseLockMode(EMouseLockMode::DoNotLock);
    }
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
            UE_LOG(LogTemp, Warning, TEXT("ActionClick bound successfully"));
        }
        
        if (InputConfig->m_MouseWheelActionZoom)
        {
            EIC->BindAction(InputConfig->m_MouseWheelActionZoom, ETriggerEvent::Triggered, this, &ACameraPawn2D::HandleZoom);
            UE_LOG(LogTemp, Warning, TEXT("ActionZoom bound successfully"));
        }
        
        // Bind Right Mouse Button for panning
        if (InputConfig->m_MouseRightButtonAction)
        {
            //EIC->BindAction(InputConfig->ActionRightMouseClick, ETriggerEvent::Started, this, &ACameraPawn2D::HandleRightMousePressed);
            //EIC->BindAction(InputConfig->ActionRightMouseClick, ETriggerEvent::Completed, this, &ACameraPawn2D::HandleRightMouseReleased);
            //UE_LOG(LogTemp, Warning, TEXT("ActionRightMouseClick bound successfully"));
            //EIC->BindAction(InputConfig->ActionRightMouseClick, ETriggerEvent::Triggered, this, &ACameraPawn2D::HandleCameraPan);
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

void ACameraPawn2D::HandleMoveByKeyboardWASD(const FInputActionValue& Value)
{
    //UE_LOG(LogTemp, Warning, TEXT(">>> HandleMoveByKeyboardWASD CALLED! <<<"));
    FVector2D MoveVec = Value.Get<FVector2D>();
    //UE_LOG(LogTemp, Warning, TEXT("Move Vector: X=%f, Y=%f"), MoveVec.X, MoveVec.Y);
    // Move along XY plane (Z remains constant)
    AddActorWorldOffset(FVector(MoveVec.Y * 20.f, -MoveVec.X * 20.f, 0.f));
}

void ACameraPawn2D::HandleClick()
{
    UE_LOG(LogTemp, Warning, TEXT(">>> HandleClick CALLED <<<"));
    if (!PinClass)
    {
        UE_LOG(LogTemp, Error, TEXT("PinClass is NULL!"));
        return;
    }
    
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController is NULL!"));
        return;
    }
    
    // Get mouse position
    float MouseX, MouseY;
    if (!PC->GetMousePosition(MouseX, MouseY))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get mouse position!"));
        return;
    }
    
    // Deproject to world
    FVector WorldLocation, WorldDirection;
    PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);
    
    FVector SpawnPos = WorldLocation;
    SpawnPos.Z = 0.0f;
    
    // Spawn the actor
    AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(PinClass, SpawnPos, FRotator::ZeroRotator);

    if (SpawnedActor)
    {
        // Prevent spawned actor from capturing input
        SpawnedActor->DisableInput(PC);

        // Add tag so it can be saved/loaded
        SpawnedActor->Tags.AddUnique(SAVEABLE_ACTOR_TAG);

        // Log actor's position
        UE_LOG(LogTemp, Log, TEXT("Actor Position: %s"), *SpawnedActor->GetActorLocation().ToString());

        UE_LOG(LogTemp, Warning, TEXT("✅ Spawned: %s at %s"), 
            *SpawnedActor->GetName(), *SpawnPos.ToString());

    }
    

}

//void ACameraPawn2D::HandleRightMousePressed()
//{
//    //bIsPanning = true;
//    LastMousePosition = GetMousePosition();
//    UE_LOG(LogTemp, Warning, TEXT(">>> RMB PRESSED - bIsPanning=true, Pos=(%f,%f)"), LastMousePosition.X, LastMousePosition.Y);
//}
//
//void ACameraPawn2D::HandleRightMouseReleased()
//{
//    //bIsPanning = false;
//    UE_LOG(LogTemp, Warning, TEXT(">>> RMB RELEASED - bIsPanning=false"));
//}

void ACameraPawn2D::HandleCameraPan(const FInputActionValue& Value)
{
    //FVector2D Delta = Value.Get<FVector2D>();
    //UE_LOG(LogTemp, Warning, TEXT("[HandleCameraPan] Delta=(%f,%f)"), Delta.X, Delta.Y);

    //if (!Delta.IsNearlyZero())
    //{
    //    FVector CurrentLocation = GetActorLocation();
    //    CurrentLocation.X -= Delta.X * PanSpeed;
    //    CurrentLocation.Y += Delta.Y * PanSpeed;
    //    SetActorLocation(CurrentLocation);
    //    UE_LOG(LogTemp, Warning, TEXT("[HandleCameraPan] Moved to: %s"), *CurrentLocation.ToString());
    //}

    FVector2D Delta = Value.Get<FVector2D>();

    if (Delta.IsNearlyZero()) return;

    FVector Loc = GetActorLocation();
    Loc.X -= Delta.X * PanSpeed ;
    Loc.Y += Delta.Y * PanSpeed ;

    SetActorLocation(Loc);
}

void ACameraPawn2D::Tick(float DeltaTime)
{
    //Super::Tick(DeltaTime);

    //if (bIsPanning)
    //{
    //    FVector2D CurrentMousePos = GetMousePosition();
    //    FVector2D Delta = CurrentMousePos - LastMousePosition;
    //    LastMousePosition = CurrentMousePos;

    //    UE_LOG(LogTemp, Warning, TEXT("[Tick Panning] Delta=(%f,%f)"), Delta.X, Delta.Y);

    //    if (!Delta.IsNearlyZero())
    //    {
    //        FVector CurrentLocation = GetActorLocation();
    //        CurrentLocation.X -= Delta.X * PanSpeed;
    //        CurrentLocation.Y += Delta.Y * PanSpeed;
    //        SetActorLocation(CurrentLocation);
    //        UE_LOG(LogTemp, Warning, TEXT("[Tick Panning] Moved to: %s"), *CurrentLocation.ToString());
    //    }
    //}
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