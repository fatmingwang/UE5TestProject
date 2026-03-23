// Fill out your copyright notice in the Description page of Project Settings.


#include "My2DPlayerController.h"

AMy2DPlayerController::AMy2DPlayerController()
{
}

void AMy2DPlayerController::BeginPlay()
{
	Super::BeginPlay();
	//dump a log
	UE_LOG(LogTemp, Warning, TEXT("AMy2DPlayerController::BeginPlay() called"));
}
