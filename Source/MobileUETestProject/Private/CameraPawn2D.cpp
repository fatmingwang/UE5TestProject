#include "CameraPawn2D.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ACameraPawn2D::ACameraPawn2D()
{
    PrimaryActorTick.bCanEverTick = false;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    RootComponent = CameraComponent;
    CameraComponent->SetProjectionMode(ECameraProjectionMode::Orthographic);
    CameraComponent->SetOrthoWidth(2048.f);
}

void ACameraPawn2D::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UE_LOG(LogTemp, Warning, TEXT("=== SetupPlayerInputComponent Called ==="));
    UE_LOG(LogTemp, Warning, TEXT("Pawn: %s, Controller: %s"), *GetName(), GetController() ? *GetController()->GetName() : TEXT("NULL"));
    
    if (!InputConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("InputConfig is NULL! Assign it in the Blueprint/Details panel!"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("InputConfig is valid"));
    
    if (!InputConfig->ActionMove)
    {
        UE_LOG(LogTemp, Error, TEXT("InputConfig->ActionMove is NULL!"));
    }
    
    if (!InputConfig->CameraContext)
    {
        UE_LOG(LogTemp, Error, TEXT("InputConfig->CameraContext is NULL!"));
    }
    
    // 1. Add Default Mapping Context
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController found: %s"), *PC->GetName());
        
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            UE_LOG(LogTemp, Warning, TEXT("Enhanced Input Subsystem found"));
            
            if (InputConfig->CameraContext)
            {
                Subsystem->AddMappingContext(InputConfig->CameraContext, 0);
                UE_LOG(LogTemp, Warning, TEXT("CameraContext added, NumMappings: %d"), InputConfig->CameraContext->GetMappings().Num());

                // Debug: Print what actions are in the mapping context
                for (const FEnhancedActionKeyMapping& Mapping : InputConfig->CameraContext->GetMappings())
                {
                    if (Mapping.Action)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("  Mapped Action: %s, Key: %s"), *Mapping.Action->GetName(), *Mapping.Key.ToString());
                    }
                }
            }

            // Debug: Print what action we're trying to bind
            if (InputConfig->ActionMove)
            {
                UE_LOG(LogTemp, Warning, TEXT("ActionMove to bind: %s"), *InputConfig->ActionMove->GetName());
            }
            
            if (InputConfig->ToolContext)
            {
                Subsystem->AddMappingContext(InputConfig->ToolContext, 1);
                UE_LOG(LogTemp, Warning, TEXT("ToolContext added"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Enhanced Input Subsystem NOT found!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController NOT found!"));
    }

    // 2. Bind Actions
    if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent found"));
        
        if (InputConfig->ActionMove)
        {
            EIC->BindAction(InputConfig->ActionMove, ETriggerEvent::Triggered, this, &ACameraPawn2D::HandleMove);
            UE_LOG(LogTemp, Warning, TEXT("ActionMove bound successfully"));
        }
        
        if (InputConfig->ActionClick)
        {
            EIC->BindAction(InputConfig->ActionClick, ETriggerEvent::Started, this, &ACameraPawn2D::HandleClick);
            UE_LOG(LogTemp, Warning, TEXT("ActionClick bound successfully"));
        }
    }
}

void ACameraPawn2D::HandleMove(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT(">>> HandleMove CALLED! <<<"));
    FVector2D MoveVec = Value.Get<FVector2D>();
    UE_LOG(LogTemp, Warning, TEXT("Move Vector: X=%f, Y=%f"), MoveVec.X, MoveVec.Y);
    // Move along XY plane (Z remains constant)
    AddActorWorldOffset(FVector(MoveVec.Y * 10.f, MoveVec.X * 10.f, 0.f));
}

void ACameraPawn2D::HandleClick()
{
    if (!PinClass)
    {
        return;
    }
    APlayerController* PC = Cast<APlayerController>(GetController());
    FHitResult Hit;
    // Trace against a plane at Z=0 or specific collision channel
    if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        // Simple Grid Snap (e.g., 50 units)
        FVector SpawnPos = Hit.Location;
        SpawnPos.X = FMath::GridSnap(SpawnPos.X, 50.f);
        SpawnPos.Y = FMath::GridSnap(SpawnPos.Y, 50.f);
        SpawnPos.Z = 0.f;

        GetWorld()->SpawnActor<AActor>(PinClass, SpawnPos, FRotator::ZeroRotator);
    }
}

void ACameraPawn2D::OnPanPressed()
{
    bIsPanning = true;
    LastMousePosition = GetMousePosition();
}

void ACameraPawn2D::OnPanReleased()
{
    bIsPanning = false;
}

void ACameraPawn2D::OnMouseX(float AxisValue)
{
    if (bIsPanning)
    {
        FVector2D CurrentMousePosition = GetMousePosition();
        FVector2D Delta = CurrentMousePosition - LastMousePosition;
        LastMousePosition = CurrentMousePosition;

        // Move camera opposite to mouse drag for natural pan
        FVector Location = GetActorLocation();
        Location -= FVector(Delta.X * PanSpeed, Delta.Y * PanSpeed, 0.f);
        SetActorLocation(Location);
    }
}

void ACameraPawn2D::OnMouseY(float AxisValue)
{
    // Handled together with MouseX inside OnMouseX for panning
}

void ACameraPawn2D::OnZoom(float AxisValue)
{
    if (FMath::Abs(AxisValue) > KINDA_SMALL_NUMBER)
    {
        float NewWidth = CameraComponent->OrthoWidth - AxisValue * ZoomStep;
        NewWidth = FMath::Clamp(NewWidth, MinOrthoWidth, MaxOrthoWidth);
        CameraComponent->SetOrthoWidth(NewWidth);
    }
}

void ACameraPawn2D::OnClick()
{
    FVector2D MousePos = GetMousePosition();
    FVector WorldPos = GetWorldPositionFromScreen(MousePos);

    UE_LOG(LogTemp, Log, TEXT("Clicked world position: %s"), *WorldPos.ToString());
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

void ACameraPawn2D::PawnClientRestart()
{
    Super::PawnClientRestart();

    // Note: Don't clear mappings here - SetupPlayerInputComponent handles adding contexts
    // Clearing here would remove contexts added in SetupPlayerInputComponent
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