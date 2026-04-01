// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "MyEditorInputConfig.generated.h"

/**
 * 
 */
UCLASS()
class MOBILEUETESTPROJECT_API UMyEditorInputConfig : public UDataAsset
{
	GENERATED_BODY()
    public:
    UPROPERTY(EditAnywhere, Category = "Contexts")
    UInputMappingContext* CameraContext; // WASD Move

    UPROPERTY(EditAnywhere, Category = "Contexts")
    UInputMappingContext* ToolContext;   // Click to Place

    UPROPERTY(EditAnywhere, Category = "Actions")
    UInputAction* ActionMove;            // Axis2D


    UPROPERTY(EditAnywhere, Category = "Actions")
    UInputAction* ActionMouseMove;            // Axis2D

    UPROPERTY(EditAnywhere, Category = "Actions")
    UInputAction* ActionRightMouseClick;           // Digital (bool)	

    UPROPERTY(EditAnywhere, Category = "Actions")
    UInputAction* ActionClick;           // Digital (bool)	

    UPROPERTY(EditAnywhere, Category = "Actions")
    UInputAction* ActionZoom;            // axis 1D
};