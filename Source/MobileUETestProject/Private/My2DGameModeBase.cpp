// Fill out your copyright notice in the Description page of Project Settings.


#include "My2DGameModeBase.h"
#include "CameraPawn2D.h"
#include "My2DPlayerController.h"

AMy2DGameModeBase::AMy2DGameModeBase()
{
    UE_LOG(LogTemp, Warning, TEXT("AMy2DGameModeBase::AMy2DGameModeBase() called"));
    // Set the default pawn to your 2D Camera Pawn
    DefaultPawnClass = ACameraPawn2D::StaticClass();

    // Set the player controller to your 2D Controller
    PlayerControllerClass = AMy2DPlayerController::StaticClass();
}
