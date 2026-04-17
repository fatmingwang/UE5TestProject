// Fill out your copyright notice in the Description page of Project Settings.


#include "My2DPlayerController.h"

AMy2DPlayerController::AMy2DPlayerController()
{
	bShowMouseCursor = true;  // Show mouse cursor for clicking
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AMy2DPlayerController::BeginPlay()
{
	Super::BeginPlay();
	// Set input mode to Game Only so keyboard input works immediately
	//FInputModeGameOnly InputMode;
	//SetInputMode(InputMode);
	//log input mode change for debugging
	if (UGameViewportClient* VC = GetWorld()->GetGameViewport())
	{
		EMouseCaptureMode CaptureMode = VC->GetMouseCaptureMode();
		EMouseLockMode LockMode = VC->GetMouseLockMode();

		UE_LOG(LogTemp, Warning, TEXT("CaptureMode=%d  LockMode=%d"),
				(int32)CaptureMode, (int32)LockMode);
		// CaptureMode: 0=NoCapture, 1=CapturePermanently, 2=CapturePermanently_IncludingInitialMouseDown, 3=CaptureDuringMouseDown, 4=CaptureDuringRightMouseDown
		// LockMode:    0=DoNotLock, 1=LockOnCapture, 2=LockAlways, 3=LockInFullscreen
	}
	//UE_LOG(LogTemp, Warning, TEXT( ));

	// Keep mouse cursor visible
	//bShowMouseCursor = true;

	UE_LOG(LogTemp, Warning, TEXT("AMy2DPlayerController::BeginPlay() called"));
}
