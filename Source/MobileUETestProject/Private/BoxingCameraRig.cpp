// Fill out your copyright notice in the Description page of Project Settings.

#include "BoxingCameraRig.h"
#include "BoxerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ABoxingCameraRig::ABoxingCameraRig()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	SetRootComponent(CameraBoom);
	CameraBoom->TargetArmLength = ReadyArmLength;
	CameraBoom->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
}

void ABoxingCameraRig::PlayBellShot(ABoxerCharacter* BoxerA, ABoxerCharacter* BoxerB)
{
	bDollyActive = false;
	bOrbiting = false;
	FocusTarget = nullptr;
	TargetArmLength = BellArmLength;
	CameraBoom->TargetArmLength = BellArmLength;

	if (BoxerA && BoxerB)
	{
		const FVector LocationA = BoxerA->GetActorLocation();
		const FVector LocationB = BoxerB->GetActorLocation();
		const FVector Midpoint = (LocationA + LocationB) * 0.5f;

		FVector Side = FVector::CrossProduct(LocationB - LocationA, FVector::UpVector).GetSafeNormal();
		if (Side.IsNearlyZero())
		{
			Side = FVector::ForwardVector;
		}

		const FVector DesiredLocation = Midpoint + Side * 10.0f + FVector(0.0f, 0.0f, 60.0f);
		SetActorLocation(DesiredLocation);
		SetActorRotation((Midpoint + FVector(0.0f, 0.0f, 60.0f) - DesiredLocation).Rotation());
	}
}

void ABoxingCameraRig::EnterReadyFraming(ABoxerCharacter* FocusBoxer)
{
	bDollyActive = false;
	bOrbiting = false;
	FocusTarget = FocusBoxer;
	TargetArmLength = ReadyArmLength;
}

void ABoxingCameraRig::BeginChargeDolly(float RampTime)
{
	bDollyActive = true;
	DollyElapsed = 0.0f;
	DollyRampTime = FMath::Max(0.1f, RampTime);
	DollyStartArmLength = CameraBoom->TargetArmLength;
}

void ABoxingCameraRig::PunchImpactShake(float Power)
{
	ShakeIntensity = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 100.0f), FVector2D(4.0f, 24.0f), Power);
	ShakeDuration = 0.15f;
	ShakeTimeRemaining = ShakeDuration;
}

void ABoxingCameraRig::GuardImpactShake(float Power)
{
	// Near-zero on a strong guard (reads as "nothing got through") - inverse of the punch shake.
	ShakeIntensity = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 100.0f), FVector2D(10.0f, 1.0f), Power);
	ShakeDuration = 0.1f;
	ShakeTimeRemaining = ShakeDuration;
}

void ABoxingCameraRig::PlayKOOrbit(ABoxerCharacter* Fallen)
{
	bDollyActive = false;
	FocusTarget = nullptr;
	OrbitTarget = Fallen;
	bOrbiting = true;
	OrbitElapsed = 0.0f;
	TargetArmLength = ChargeArmLength;
}

void ABoxingCameraRig::PlayVictoryOrbit(ABoxerCharacter* Winner)
{
	bDollyActive = false;
	FocusTarget = nullptr;
	OrbitTarget = Winner;
	bOrbiting = true;
	OrbitElapsed = 0.0f;
	TargetArmLength = ChargeArmLength;
}

void ABoxingCameraRig::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDollyActive)
	{
		DollyElapsed += DeltaTime;
		const float Alpha = FMath::Clamp(DollyElapsed / DollyRampTime, 0.0f, 1.0f);
		CameraBoom->TargetArmLength = FMath::Lerp(DollyStartArmLength, ChargeArmLength, Alpha);
	}
	else
	{
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, FramingLerpSpeed);
	}

	if (bOrbiting && OrbitTarget.IsValid())
	{
		OrbitElapsed += DeltaTime;
		const FVector Center = OrbitTarget->GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
		const float Yaw = OrbitElapsed * OrbitYawPerSecond;
		const FVector Offset = FVector(FMath::Cos(FMath::DegreesToRadians(Yaw)), FMath::Sin(FMath::DegreesToRadians(Yaw)), 0.0f) * 250.0f;
		const FVector DesiredLocation = Center + Offset;

		SetActorLocation(FMath::VInterpTo(GetActorLocation(), DesiredLocation, DeltaTime, 2.0f));
		SetActorRotation((Center - GetActorLocation()).Rotation());
	}
	else if (FocusTarget.IsValid())
	{
		FrameOn(FocusTarget.Get(), DeltaTime);
	}

	if (ShakeTimeRemaining > 0.0f)
	{
		ShakeTimeRemaining -= DeltaTime;
		const float DecayAlpha = ShakeDuration > 0.0f ? FMath::Clamp(ShakeTimeRemaining / ShakeDuration, 0.0f, 1.0f) : 0.0f;
		const float Offset = FMath::Sin(ShakeTimeRemaining * 60.0f) * ShakeIntensity * DecayAlpha;
		CameraBoom->SocketOffset = FVector(0.0f, Offset, Offset * 0.5f);
	}
	else
	{
		CameraBoom->SocketOffset = FVector::ZeroVector;
	}
}

void ABoxingCameraRig::FrameOn(const AActor* Target, float DeltaTime)
{
	if (!Target)
	{
		return;
	}

	const FVector DesiredLocation = Target->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
	const FRotator DesiredRotation = (DesiredLocation - GetActorLocation()).Rotation();
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, FramingLerpSpeed));
}
