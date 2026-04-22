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
protected:
    UPROPERTY(EditAnywhere, Category = "Input")
    UMyEditorInputConfig* InputConfig;
    UPROPERTY(EditAnywhere, Category = "Input", meta = (DisplayName = "Use 2D XY(not XZ)"))
    bool m_bUseXYNotXZ;

    UPROPERTY(EditAnywhere, Category = "Setup")
    TSubclassOf<AActor> PinClass; // The Actor to spawn
	bool m_bFlipXYIfXIsRotation90Degree ; // Whether to flip X and Y axes when rotation is 90 degrees
public:
    ACameraPawn2D();

protected:
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void PawnClientRestart() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    void HandleMoveByKeyboardWASD(const FInputActionValue& Value);
    void HandleClick();
    void HandleZoom(const FInputActionValue& Value);
    void SetPlayerControllerMouseBehivor();
    FVector2D GetMovedVec(FVector2D e_MoveVec);

private:
    UPROPERTY(VisibleAnywhere)
    UCameraComponent* CameraComponent;

    //bool bIsPanning = false;
    FVector2D LastMousePosition;

    UPROPERTY(EditAnywhere)
    float PanSpeed = 5.0f;

    UPROPERTY(EditAnywhere)
    float ZoomStep = 100.0f;

    UPROPERTY(EditAnywhere)
    float MinOrthoWidth = 512.0f;

    UPROPERTY(EditAnywhere)
    float MaxOrthoWidth = 4096.0f;

    // Input handlers
    //void HandleRightMousePressed();
    //void HandleRightMouseReleased();
    void HandleCameraPan(const FInputActionValue& Value);

    FVector2D GetMousePosition() const;
    FVector GetWorldPositionFromScreen(const FVector2D& ScreenPosition) const;

    void SetEditorMode(bool bIsPlacementMode);
};