// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerBarMath.h"
#include "FightDirector.generated.h"

class ABoxerCharacter;
class ABoxingCameraRig;
class UBoxingHUDWidget;

UENUM(BlueprintType)
enum class EFightState : uint8
{
	Bell,
	PlayerAttack,
	PlayerDefend,
	Win,
	Lose
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFightStateChanged, EFightState, NewState);

// Owns the boxing match's turn loop (design doc section 9): player throws a punch (Attack bar) ->
// opponent swings back and the player must guard it (Defense bar) -> repeat until KO. Also
// creates/owns the HUD widget and drives the fight camera's beats (design doc section 7).
UCLASS(Blueprintable)
class MOBILEUETESTPROJECT_API AFightDirector : public AActor
{
	GENERATED_BODY()

public:
	AFightDirector();

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Fight")
	TObjectPtr<ABoxerCharacter> PlayerBoxer;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Fight")
	TObjectPtr<ABoxerCharacter> OpponentBoxer;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Fight")
	TObjectPtr<ABoxingCameraRig> CameraRig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight|UI")
	TSubclassOf<UBoxingHUDWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Fight|UI")
	TObjectPtr<UBoxingHUDWidget> HUDWidgetInstance;

	UPROPERTY(BlueprintReadOnly, Category = "Fight")
	EFightState CurrentState = EFightState::Bell;

	// Seconds the Bell shot holds before the fight starts (design doc section 7).
	UPROPERTY(EditAnywhere, Category = "Fight")
	float BellDuration = 2.5f;

	// How far from its own Attack bar's center the opponent's simulated incoming punch is allowed
	// to land, as a fraction of the bar's edge (0 = always dead-center/best, 1 = anywhere up to the
	// full bar) - design doc section 3/6's simplified AI release-timing policy, without needing a
	// second visible bar.
	UPROPERTY(EditAnywhere, Category = "Fight", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OpponentSkill = 0.6f;

	UPROPERTY(BlueprintAssignable, Category = "Fight|Events")
	FOnFightStateChanged OnFightStateChanged;

	// Called by the HUD's Attack button OnPressed/OnReleased.
	UFUNCTION(BlueprintCallable, Category = "Fight")
	void HandleAttackButtonPressed();

	UFUNCTION(BlueprintCallable, Category = "Fight")
	void HandleAttackButtonReleased();

	// Called by the HUD's Guard button OnPressed/OnReleased.
	UFUNCTION(BlueprintCallable, Category = "Fight")
	void HandleGuardButtonPressed();

	UFUNCTION(BlueprintCallable, Category = "Fight")
	void HandleGuardButtonReleased();

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle BellTimerHandle;

	void SetState(EFightState NewState);
	void StartPlayerAttackPhase();
	void StartPlayerDefendPhase();
	void ResolveOpponentPunch(float& OutPower, EPowerZone& OutZone, bool& OutPerfect) const;
	void EndFight(bool bPlayerWon);

	UFUNCTION()
	void HandlePlayerAttackReleased(float Power, EPowerZone Zone, bool bPerfect);

	UFUNCTION()
	void HandlePlayerGuardReleased(float Power, EPowerZone Zone, bool bPerfect);
};
