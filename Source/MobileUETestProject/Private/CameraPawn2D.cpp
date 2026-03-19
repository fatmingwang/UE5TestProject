#include "CameraPawn2D.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

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

    PlayerInputComponent->BindAction("CameraPan", IE_Pressed, this, &ACameraPawn2D::OnPanPressed);
    PlayerInputComponent->BindAction("CameraPan", IE_Released, this, &ACameraPawn2D::OnPanReleased);
    PlayerInputComponent->BindAction("CameraClick", IE_Pressed, this, &ACameraPawn2D::OnClick);

    PlayerInputComponent->BindAxis("MouseX", this, &ACameraPawn2D::OnMouseX);
    PlayerInputComponent->BindAxis("MouseY", this, &ACameraPawn2D::OnMouseY);
    PlayerInputComponent->BindAxis("CameraZoom", this, &ACameraPawn2D::OnZoom);
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