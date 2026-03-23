// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "My2DPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MOBILEUETESTPROJECT_API AMy2DPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AMy2DPlayerController();
	virtual void BeginPlay() override;
};
