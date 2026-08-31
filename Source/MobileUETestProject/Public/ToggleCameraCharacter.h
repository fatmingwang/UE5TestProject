// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ToggleCameraCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

// Adds a third-person camera boom to a Character and lets it toggle between first-person
// (camera pulled into the head) and third-person (camera pulled back on a boom) views.
// Intended as a native parent for BP_FirstPersonCharacter in MazeLevel.
UCLASS()
class MOBILEUETESTPROJECT_API AToggleCameraCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AToggleCameraCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> ThirdPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	bool bIsFirstPerson = true;

	// Boom length/offset used when bIsFirstPerson is true (camera pulled into the head).
	UPROPERTY(EditAnywhere, Category = "Camera")
	float FirstPersonArmLength = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector FirstPersonSocketOffset = FVector(0.0f, 0.0f, 70.0f);

	// Boom length/offset used when bIsFirstPerson is false (camera pulled back behind the character).
	UPROPERTY(EditAnywhere, Category = "Camera")
	float ThirdPersonArmLength = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector ThirdPersonSocketOffset = FVector(0.0f, 50.0f, 60.0f);

	// Switches between first-person and third-person camera placement.
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ToggleCameraView();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void ApplyCameraView();
};
