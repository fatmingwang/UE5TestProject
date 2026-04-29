#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "MyEditorInputConfig.h"
#include "DragActorState.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (DisplayName = "Target Place Actor Class"))
	TSubclassOf<AActor> m_TargetPlaceActor; // The Actor to spawn
	bool m_bFlipXYIfXIsRotation90Degree ; // Whether to flip X and Y axes when rotation is 90 degrees
public:
	ACameraPawn2D();

	UFUNCTION(BlueprintCallable, Category = "Camera2D", meta = (DisplayName = "Set Target Place Actor Class"))
	void BP_SetTargetPlaceActor(TSubclassOf<AActor> NewActorClass);

	UFUNCTION(BlueprintPure, Category = "Camera2D", meta = (DisplayName = "Get Target Place Actor Class"))
	TSubclassOf<AActor> BP_GetTargetPlaceActor() const;

protected:
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void PawnClientRestart() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    void HandleMoveByKeyboardWASD(const FInputActionValue& Value);
    void HandleClick();
    void HandleClickReleased();
    void HandleZoom(const FInputActionValue& Value);
    void SetPlayerControllerMouseBehivor();
    FVector2D GetMovedVec(FVector2D e_MoveVec);
    // Sets the fixed depth on the correct axis of e_Input based on camera mode
    void GetActorPlaneDepth(FVector& e_Input) const;

private:
    UPROPERTY(VisibleAnywhere)
    UCameraComponent* CameraComponent;

    // The actor currently being dragged; nullptr when nothing is selected
    AActor* DraggedActor;
    // Saved original state of DraggedActor's root component before drag
    FDragActorState DraggedActorOriginalState;
    // World-space offset from actor origin to mouse hit point at grab time
    FVector DragOffset;

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

    // Fixed depth on the non-movement axis where actors are placed/dragged
    UPROPERTY(EditAnywhere, Category = "Setup", meta = (DisplayName = "Actor Plane Depth (XY mode Z)"))
    float m_ActorPlaneDepthXY = 250.0f;

    UPROPERTY(EditAnywhere, Category = "Setup", meta = (DisplayName = "Actor Plane Depth (XZ mode Y)"))
    float m_ActorPlaneDepthXZ = 700.0f;

    // Input handlers
    //void HandleRightMousePressed();
    //void HandleRightMouseReleased();
    void HandleCameraPan(const FInputActionValue& Value);

    FVector2D GetMousePosition() const;
    FVector GetWorldPositionFromScreen(const FVector2D& ScreenPosition) const;

    void SetEditorMode(bool bIsPlacementMode);
};