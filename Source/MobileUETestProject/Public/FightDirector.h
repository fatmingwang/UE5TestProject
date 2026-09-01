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
	// second visible bar. Overwritten by the chosen OpponentID's own OpponentSkill in ApplyLoadouts()
	// if that ID is found in UBoxingDataSubsystem, so this value only matters as a fallback/default.
	UPROPERTY(EditAnywhere, Category = "Fight", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OpponentSkill = 0.6f;

	// IDs looked up in UBoxingDataSubsystem's catalogs and applied to PlayerBoxer/OpponentBoxer in
	// BeginPlay via ApplyLoadouts() (design doc section 6). PlayerPerkID may be left empty - no perk
	// is applied in that case. IDs that aren't found in the catalog are silently skipped (that boxer
	// keeps whatever stats/tuning it already has, e.g. from Blueprint defaults).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight|Loadout")
	FString PlayerGloveID = TEXT("gloves_training");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight|Loadout")
	FString PlayerStanceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight|Loadout")
	FString PlayerPerkID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight|Loadout")
	FString OpponentID = TEXT("opp_rookie_ray");

	// If true, the fight starts immediately on BeginPlay (old behavior - useful for a level that
	// exists only to test the fight). If false (default), this actor sits dormant until something
	// calls StartFight() - a hotkey (bBindTestHotkey below), a trigger volume overlap
	// (ABoxingFightTrigger), or any other gameplay event - so the same FightDirector/boxers/camera
	// rig/HUD can live in a shared level (e.g. alongside the maze) without a fight kicking off the
	// moment the level loads.
	UPROPERTY(EditAnywhere, Category = "Fight")
	bool bAutoStartOnBeginPlay = false;

	// Binds TestHotkey to toggle StartFight()/ResetFight() on this actor for quick in-editor
	// testing without needing a trigger volume - press once to enter the fight, press again to back
	// out (restoring the player's previous pawn/camera - see StartFight()/ResetFight()). Safe to
	// leave on in a shipping level, but flip off if the key would conflict with other gameplay input.
	UPROPERTY(EditAnywhere, Category = "Fight|Debug")
	bool bBindTestHotkey = true;

	UPROPERTY(EditAnywhere, Category = "Fight|Debug")
	FKey TestHotkey = EKeys::F9;

	UPROPERTY(BlueprintAssignable, Category = "Fight|Events")
	FOnFightStateChanged OnFightStateChanged;

	// Starts the fight: applies loadouts, possesses PlayerBoxer with the local player's controller,
	// blends the view to CameraRig (so the boxing camera's own framing - PlayBellShot/
	// EnterReadyFraming, design doc section 7 - is actually what's on screen instead of just running
	// unseen), shows/creates the HUD, switches input to Game+UI, plays the Bell camera shot, and
	// starts the Bell timer. No-op if a fight is already in progress or has already ended - call
	// ResetFight() first to run it again. This is the single entry point meant to be called on
	// demand (hotkey, trigger volume, cutscene end, etc.) rather than relying on BeginPlay to kick
	// things off.
	UFUNCTION(BlueprintCallable, Category = "Fight")
	void StartFight();

	// Reverts a finished (or in-progress) fight back to a dormant, startable state - clears the Bell
	// timer, hides the HUD, restores game-only input, re-possesses whatever pawn/view target the
	// player controller had before StartFight() ran, and lets StartFight() run again.
	UFUNCTION(BlueprintCallable, Category = "Fight")
	void ResetFight();

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
	bool bFightStarted = false;

	// The local player controller's pawn/view target from just before StartFight() possessed
	// PlayerBoxer and blended the camera - restored by ResetFight().
	TWeakObjectPtr<APawn> PreviousPawn;
	TWeakObjectPtr<AActor> PreviousViewTarget;

	// Bound to TestHotkey - StartFight() if dormant, ResetFight() if not, so one key both enters
	// and exits the test fight.
	void ToggleFightForTesting();

	// Looks up PlayerGloveID/PlayerStanceID/PlayerPerkID/OpponentID in UBoxingDataSubsystem
	// (GameInstance subsystem, always available) and applies whichever entries are found to
	// PlayerBoxer/OpponentBoxer. Called once from StartFight() before the fight starts.
	void ApplyLoadouts();
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
