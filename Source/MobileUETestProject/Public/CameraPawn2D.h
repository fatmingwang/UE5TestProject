#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "MyEditorInputConfig.h"
#include "CameraPawn2D.generated.h"

UCLASS()
class MOBILEUETESTPROJECT_API ACameraPawn2D : public APawn
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    UMyEditorInputConfig* InputConfig;

    UPROPERTY(EditAnywhere, Category = "Setup")
    TSubclassOf<AActor> PinClass; // The Actor to spawn
public:
    ACameraPawn2D();

protected:
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    void HandleMove(const FInputActionValue& Value);
    void HandleClick();

private:
    UPROPERTY(VisibleAnywhere)
    UCameraComponent* CameraComponent;

    bool bIsPanning = false;
    FVector2D LastMousePosition;

    UPROPERTY(EditAnywhere)
    float PanSpeed = 1.0f;

    UPROPERTY(EditAnywhere)
    float ZoomStep = 100.0f;

    UPROPERTY(EditAnywhere)
    float MinOrthoWidth = 512.0f;

    UPROPERTY(EditAnywhere)
    float MaxOrthoWidth = 4096.0f;

    // Input handlers
    void OnPanPressed();
    void OnPanReleased();
    void OnMouseX(float AxisValue);
    void OnMouseY(float AxisValue);
    void OnZoom(float AxisValue);
    void OnClick();

    FVector2D GetMousePosition() const;
    FVector GetWorldPositionFromScreen(const FVector2D& ScreenPosition) const;

    void PawnClientRestart();
    void SetEditorMode(bool bIsPlacementMode);
};