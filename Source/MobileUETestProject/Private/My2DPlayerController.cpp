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
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	// Keep mouse cursor visible
	bShowMouseCursor = true;

	UE_LOG(LogTemp, Warning, TEXT("AMy2DPlayerController::BeginPlay() called"));
}
