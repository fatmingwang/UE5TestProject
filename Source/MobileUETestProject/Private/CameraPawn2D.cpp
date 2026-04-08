#include "CameraPawn2D.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ACameraPawn2D::ACameraPawn2D()
{
    PanSpeed = 10.f;
    PrimaryActorTick.bCanEverTick = false;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    RootComponent = CameraComponent;
    CameraComponent->SetProjectionMode(ECameraProjectionMode::Orthographic);
    CameraComponent->SetOrthoWidth(2048.f);
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
        if (InputConfig->CameraContext)
        {
            Subsystem->AddMappingContext(InputConfig->CameraContext, 0);
            UE_LOG(LogTemp, Warning, TEXT("CameraContext injected via PawnClientRestart"));
        }

        if (InputConfig->ToolContext)
        {
            Subsystem->AddMappingContext(InputConfig->ToolContext, 1);
        }
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
        
        if (InputConfig->ActionMove)
        {
            EIC->BindAction(InputConfig->ActionMove, ETriggerEvent::Triggered, this, &ACameraPawn2D::HandleMoveByKeyboardWASD);
            UE_LOG(LogTemp, Warning, TEXT("ActionMove bound successfully"));
        }
        
        if (InputConfig->ActionClick)
        {
            EIC->BindAction(InputConfig->ActionClick, ETriggerEvent::Completed, this, &ACameraPawn2D::HandleClick);
            UE_LOG(LogTemp, Warning, TEXT("ActionClick bound successfully"));
        }
        
        if (InputConfig->ActionZoom)
        {
            EIC->BindAction(InputConfig->ActionZoom, ETriggerEvent::Triggered, this, &ACameraPawn2D::HandleZoom);
            UE_LOG(LogTemp, Warning, TEXT("ActionZoom bound successfully"));
        }
        
        // Bind Right Mouse Button for panning
        if (InputConfig->ActionRightMouseClick)
        {
            EIC->BindAction(InputConfig->ActionRightMouseClick, ETriggerEvent::Started, this, &ACameraPawn2D::HandleRightMousePressed);
            EIC->BindAction(InputConfig->ActionRightMouseClick, ETriggerEvent::Completed, this, &ACameraPawn2D::HandleRightMouseReleased);
            UE_LOG(LogTemp, Warning, TEXT("ActionRightMouseClick bound successfully"));
        }
        
        // Bind Mouse Movement for panning
        if (InputConfig->ActionMouseMove)
        {
            EIC->BindAction(InputConfig->ActionMouseMove, ETriggerEvent::Triggered, this, &ACameraPawn2D::HandleMouseMove);
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
        

        // Log actor's position
        UE_LOG(LogTemp, Log, TEXT("Actor Position: %s"), *SpawnedActor->GetActorLocation().ToString());

        UE_LOG(LogTemp, Warning, TEXT("✅ Spawned: %s at %s"), 
            *SpawnedActor->GetName(), *SpawnPos.ToString());

    }
    
    // CRITICAL: Reset input mode to ensure continued mouse input
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    PC->SetInputMode(InputMode);
    
    // Ensure mouse cursor stays visible
    PC->bShowMouseCursor = true;
    PC->bEnableClickEvents = true;
}

void ACameraPawn2D::HandleRightMousePressed()
{
    bIsPanning = true;
    LastMousePosition = GetMousePosition();
    //UE_LOG(LogTemp, Warning, TEXT(">>> Panning Started <<<"));
}

void ACameraPawn2D::HandleRightMouseReleased()
{
    bIsPanning = false;
    //UE_LOG(LogTemp, Warning, TEXT(">>> Panning Stopped <<<"));
}

void ACameraPawn2D::HandleMouseMove(const FInputActionValue& Value)
{
    if (bIsPanning)
    {
        FVector2D MouseDelta = Value.Get<FVector2D>();
        
        // For 2D camera panning, move the camera in the opposite direction of mouse movement
        FVector CurrentLocation = GetActorLocation();
        
        // Negative delta for natural panning (drag right = camera moves right)
        // Adjust based on your camera orientation and desired panning feel
        CurrentLocation.X += MouseDelta.X * PanSpeed;
        CurrentLocation.Y += -MouseDelta.Y * PanSpeed;
        
        SetActorLocation(CurrentLocation);
        
        //UE_LOG(LogTemp, Log, TEXT("Panning - Delta: (%f, %f), New Position: %s"), MouseDelta.X, MouseDelta.Y, *CurrentLocation.ToString());
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