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

    // 1. Add Default Mapping Context
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(InputConfig->CameraContext, 0);
            Subsystem->AddMappingContext(InputConfig->ToolContext, 1);
        }
    }

    // 2. Bind Actions
    if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(InputConfig->ActionMove, ETriggerEvent::Triggered, this, &ACameraPawn2D::HandleMove);
        EIC->BindAction(InputConfig->ActionClick, ETriggerEvent::Started, this, &ACameraPawn2D::HandleClick);
    }
}

void ACameraPawn2D::HandleMove(const FInputActionValue& Value)
{
    FVector2D MoveVec = Value.Get<FVector2D>();
    // Move along XY plane (Z remains constant)
    AddActorWorldOffset(FVector(MoveVec.Y * 10.f, MoveVec.X * 10.f, 0.f));
}

void ACameraPawn2D::HandleClick()
{
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

    // 1. Get the Player Controller
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        // 2. Get the Enhanced Input Local Player Subsystem
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            // 3. Clear old contexts so they don't overlap
            Subsystem->ClearAllMappings();

        //    // 4. Add your Default IMC (WASD movement)
        //    if (DefaultMappingContext)
        //    {
        //        Subsystem->AddMappingContext(DefaultMappingContext, 0);
        //    }
        }
    }
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