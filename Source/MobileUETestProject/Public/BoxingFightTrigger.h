// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoxingFightTrigger.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class AFightDirector;

// Overlap trigger that starts a fight when the player pawn walks into it - the "collide box" path
// alongside AFightDirector's own hotkey (bBindTestHotkey/TestHotkey). Place one near a
// FightDirector's boxers and set FightDirectorRef in the Details panel.
UCLASS()
class MOBILEUETESTPROJECT_API ABoxingFightTrigger : public AActor
{
	GENERATED_BODY()

public:
	ABoxingFightTrigger();

	UPROPERTY(VisibleAnywhere, Category = "Fight")
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Fight")
	TObjectPtr<AFightDirector> FightDirectorRef;

	// If true, this trigger disables its own collision after firing once (a typical scripted
	// encounter). If false, it re-fires every time the player walks in - StartFight() is already a
	// no-op mid-fight, so this is only useful alongside AFightDirector::ResetFight() for repeat
	// testing.
	UPROPERTY(EditAnywhere, Category = "Fight")
	bool bTriggerOnce = true;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
