// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoxingCameraRig.generated.h"

class USpringArmComponent;
class UCameraComponent;
class ABoxerCharacter;

// Hand-rolled fight camera (design doc section 7): per-tick Lerps drive framing/dolly/shake, no
// Sequencer or CameraShake assets - matches AMazeVisualizerActor::UpdateMinimapView()'s style.
// Sits at a fixed vantage to the side of the mat (set once by PlayBellShot); "dolly in" is done by
// shortening the spring arm, not by physically moving the rig.
UCLASS(Blueprintable)
class MOBILEUETESTPROJECT_API ABoxingCameraRig : public AActor
{
	GENERATED_BODY()

public:
	ABoxingCameraRig();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float ReadyArmLength = 450.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float ChargeArmLength = 320.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float BellArmLength = 700.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float FramingLerpSpeed = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float OrbitYawPerSecond = 40.0f;

	// Wide static two-shot with both boxers in frame (design doc section 7, "Bell").
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void PlayBellShot(ABoxerCharacter* BoxerA, ABoxerCharacter* BoxerB);

	// Medium shot favoring FocusBoxer, same side-on axis as the Bell shot (design doc section 7, "Ready").
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void EnterReadyFraming(ABoxerCharacter* FocusBoxer);

	// Starts shortening the boom toward ChargeArmLength over RampTime seconds (design doc section 7,
	// "Charging punch-in").
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void BeginChargeDolly(float RampTime);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void PunchImpactShake(float Power);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void GuardImpactShake(float Power);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void PlayKOOrbit(ABoxerCharacter* Fallen);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void PlayVictoryOrbit(ABoxerCharacter* Winner);

private:
	TWeakObjectPtr<ABoxerCharacter> FocusTarget;
	TWeakObjectPtr<ABoxerCharacter> OrbitTarget;

	float TargetArmLength = 450.0f;

	bool bDollyActive = false;
	float DollyElapsed = 0.0f;
	float DollyRampTime = 1.0f;
	float DollyStartArmLength = 0.0f;

	bool bOrbiting = false;
	float OrbitElapsed = 0.0f;

	// Hand-rolled shake: decaying sine offset applied on top of the boom's socket offset.
	float ShakeTimeRemaining = 0.0f;
	float ShakeDuration = 0.0f;
	float ShakeIntensity = 0.0f;

	void FrameOn(const AActor* Target, float DeltaTime);
};
