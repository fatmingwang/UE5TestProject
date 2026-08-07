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
    UPROPERTY(EditAnywhere, Category = "MyActions", meta = (DisplayName = "Camera Input Mapping Cotext"))
    UInputMappingContext* m_CameraIMC; // WASD Move

    UPROPERTY(EditAnywhere, Category = "MyActions", meta = (DisplayName = "KeyBoard WASD"))
    UInputAction* m_KeyboardActionMove;            // Axis2D

    UPROPERTY(EditAnywhere, Category = "MyActions", meta = (DisplayName = "Mouse Move"))
    UInputAction* m_MouseActionMove;      // Axis2D

    UPROPERTY(EditAnywhere, Category = "MyActions", meta = (DisplayName = "Right Mouse Button Move"))
    UInputAction* m_MouseRightButtonAction;// Digital (bool)	

    UPROPERTY(EditAnywhere, Category = "MyActions", meta = (DisplayName = "Left Mouse Button Move"))
    UInputAction* m_MouseLeftButtonActionMove;           // Digital (bool)	

    UPROPERTY(EditAnywhere, Category = "MyActions", meta = (DisplayName = "Mouse Wheel Zoom"))
    UInputAction* m_MouseWheelActionZoom;            // axis 1D
};