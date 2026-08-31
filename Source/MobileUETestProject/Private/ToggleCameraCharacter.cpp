// Fill out your copyright notice in the Description page of Project Settings.

#include "ToggleCameraCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AToggleCameraCharacter::AToggleCameraCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->bUsePawnControlRotation = true;

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	ThirdPersonCamera->bUsePawnControlRotation = false;
}

void AToggleCameraCharacter::BeginPlay()
{
	Super::BeginPlay();

	// BP_FirstPersonCharacter (and similar) may already have their own dedicated first-person
	// camera component; make sure only ThirdPersonCamera drives the view so toggling is unambiguous.
	TArray<UCameraComponent*> Cameras;
	GetComponents<UCameraComponent>(Cameras);
	for (UCameraComponent* Cam : Cameras)
	{
		Cam->SetActive(Cam == ThirdPersonCamera);
	}

	ApplyCameraView();
}

void AToggleCameraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::V, IE_Pressed, this, &AToggleCameraCharacter::ToggleCameraView);
}

void AToggleCameraCharacter::ToggleCameraView()
{
	bIsFirstPerson = !bIsFirstPerson;
	ApplyCameraView();
}

void AToggleCameraCharacter::ApplyCameraView()
{
	if (bIsFirstPerson)
	{
		CameraBoom->TargetArmLength = FirstPersonArmLength;
		CameraBoom->SocketOffset = FirstPersonSocketOffset;
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		CameraBoom->TargetArmLength = ThirdPersonArmLength;
		CameraBoom->SocketOffset = ThirdPersonSocketOffset;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}
