// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PowerBarMath.h"
#include "BoxingLoadoutData.h"
#include "BoxerCharacter.generated.h"

class UAnimSequence;
class UAnimMontage;

UENUM(BlueprintType)
enum class EBoxerState : uint8
{
	Idle,
	ChargingAttack,
	ChargingGuard,
	Punching,
	Guarding,
	HitReacting,
	KO,
	Victorious
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBoxerBarReleased, float, Power, EPowerZone, Zone, bool, bPerfect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBoxerHPChanged, float, NewHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoxerStateChanged, EBoxerState, NewState);

// Shared class for both boxers (player and opponent). Owns HP/state and the hold-to-charge
// Attack/Defense power bars (see PowerBarMath.h - ported from
// Doc/BattleConecpt/fight_scene_demo.html). Does NOT know about its opponent or apply damage
// across actors - AFightDirector reads the released power/zone from OnAttackReleased/
// OnGuardReleased and resolves damage itself, so this class stays usable/testable standalone,
// the same way UWilsonMazeGenerator only ever knows about its own grid.
UCLASS(Blueprintable)
class MOBILEUETESTPROJECT_API ABoxerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABoxerCharacter();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boxer|Stats")
	float MaxHP = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Boxer|Stats")
	float CurrentHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boxer|Stats")
	float ATK = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boxer|Stats")
	float DEF = 4.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Boxer|State")
	EBoxerState CurrentState = EBoxerState::Idle;

	// Attack bar tuning - this boxer's "Gloves" (design doc section 6).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxer|Attack Bar")
	FPowerBarZones AttackZones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxer|Attack Bar")
	float AttackV0 = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxer|Attack Bar")
	float AttackVmax = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxer|Attack Bar")
	float AttackRampTime = 1.8f;

	// Damage multiplier at 100% power: 0.2 + (AttackMaxMult - 0.2) * power/100 (HTML prototype formula).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxer|Attack Bar")
	float AttackMaxMult = 2.0f;

	// Defense bar tuning - this boxer's "Stance" (design doc section 6).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxer|Defense Bar")
	FPowerBarZones DefenseZones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxer|Defense Bar")
	float DefenseV0 = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxer|Defense Bar")
	float DefenseVmax = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boxer|Defense Bar")
	float DefenseRampTime = 1.4f;

	// Animation clips (existing Content/Characters/Mannequins/Anims/Unarmed/* assets) - played as
	// one-off dynamic montages via UAnimInstance::PlaySlotAnimationAsDynamicMontage(), so no Montage
	// assets need to be hand-authored. See design doc section 4 for the zone -> clip mapping.
	UPROPERTY(EditAnywhere, Category = "Boxer|Animation")
	TObjectPtr<UAnimSequence> JabAnim;

	UPROPERTY(EditAnywhere, Category = "Boxer|Animation")
	TObjectPtr<UAnimSequence> CrossAnim;

	UPROPERTY(EditAnywhere, Category = "Boxer|Animation")
	TObjectPtr<UAnimSequence> HookAnim;

	UPROPERTY(EditAnywhere, Category = "Boxer|Animation")
	TObjectPtr<UAnimSequence> PowerPunchAnim;

	UPROPERTY(EditAnywhere, Category = "Boxer|Animation")
	TArray<TObjectPtr<UAnimSequence>> HitReactLightAnims;

	UPROPERTY(EditAnywhere, Category = "Boxer|Animation")
	TArray<TObjectPtr<UAnimSequence>> HitReactMediumAnims;

	UPROPERTY(EditAnywhere, Category = "Boxer|Animation")
	TObjectPtr<UAnimSequence> HitReactHeavyAnim;

	UPROPERTY(EditAnywhere, Category = "Boxer|Animation")
	TArray<TObjectPtr<UAnimSequence>> DeathAnims;

	// Anim slot the dynamic montages play on. ABP_Unarmed's locomotion state machine needs a Slot
	// node with this name for punches/hit-reacts/guard to blend over movement instead of fully
	// overriding it - verify this in the editor after Stage 2 and adjust here if it's named differently.
	UPROPERTY(EditAnywhere, Category = "Boxer|Animation")
	FName MontageSlotName = TEXT("DefaultSlot");

	UPROPERTY(BlueprintAssignable, Category = "Boxer|Events")
	FOnBoxerBarReleased OnAttackReleased;

	UPROPERTY(BlueprintAssignable, Category = "Boxer|Events")
	FOnBoxerBarReleased OnGuardReleased;

	UPROPERTY(BlueprintAssignable, Category = "Boxer|Events")
	FOnBoxerHPChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boxer|Events")
	FOnBoxerStateChanged OnStateChanged;

	// Starts the Attack bar bouncing (call on button-press).
	UFUNCTION(BlueprintCallable, Category = "Boxer")
	void StartChargingAttack();

	// Locks in the Attack bar's current power (call on button-release), plays the matching punch
	// montage, and broadcasts OnAttackReleased. Does not apply damage - the caller (AFightDirector)
	// does that, since this class doesn't know its opponent.
	UFUNCTION(BlueprintCallable, Category = "Boxer")
	float ReleaseAttack();

	// Starts the Defense bar bouncing and holds a raised-fists guard pose (call on button-press).
	UFUNCTION(BlueprintCallable, Category = "Boxer")
	void StartChargingGuard();

	// Locks in the Defense bar's current power (call on button-release), drops the guard pose, and
	// broadcasts OnGuardReleased. Does not apply damage - see ReleaseAttack().
	UFUNCTION(BlueprintCallable, Category = "Boxer")
	float ReleaseGuard();

	// Reduces CurrentHP (clamped to [0, MaxHP]), broadcasts OnHPChanged, and transitions to KO
	// (playing a death montage) if HP reaches 0.
	UFUNCTION(BlueprintCallable, Category = "Boxer")
	void ApplyDamage(float Amount);

	// Plays a hit-reaction montage picked by the tier of the punch that just landed on this boxer
	// (design doc section 4): AttackerZone/bAttackerPerfect describe the incoming punch, not this
	// boxer's own guard roll.
	UFUNCTION(BlueprintCallable, Category = "Boxer")
	void PlayHitReactMontage(EPowerZone AttackerZone, bool bAttackerPerfect);

	// [0,1] marker position across whichever bar is currently charging - for the HUD to poll.
	UFUNCTION(BlueprintPure, Category = "Boxer")
	float GetBarNormalizedPosition() const;

	// AttackZones or DefenseZones, whichever bar is currently charging - for the HUD to poll.
	UFUNCTION(BlueprintPure, Category = "Boxer")
	FPowerBarZones GetActiveZones() const;

	UFUNCTION(BlueprintPure, Category = "Boxer")
	bool IsCharging() const { return CurrentState == EBoxerState::ChargingAttack || CurrentState == EBoxerState::ChargingGuard; }

	// Loadout appliers (design doc section 6) - read a catalog entry off UBoxingDataSubsystem and
	// configure this boxer with it. Call once during fight setup (e.g. from BP_FightDirector or
	// AFightDirector::BeginPlay), not repeatedly - ATKBonus/DEFBonus are added on top of the
	// current ATK/DEF each call, everything else (bar tuning/zones) is overwritten outright since a
	// boxer only ever wears one glove/stance at a time.
	UFUNCTION(BlueprintCallable, Category = "Boxer|Loadout")
	void ApplyGloveData(const FBoxerGloveData& Glove);

	UFUNCTION(BlueprintCallable, Category = "Boxer|Loadout")
	void ApplyStanceData(const FBoxerStanceData& Stance);

	// Sets MaxHP/CurrentHP/ATK/DEF and both bars' tuning wholesale - meant for configuring an
	// opponent-side boxer from a chosen UBoxingDataSubsystem opponent entry, not the player boxer
	// (which is built from Gloves/Stance/Perk instead).
	UFUNCTION(BlueprintCallable, Category = "Boxer|Loadout")
	void ApplyOpponentData(const FOpponentData& Opponent);

	// Stores Perk and applies its RampTimeMultiplier to both bars. MitigationFloorBonus/
	// AttackPowerBonusPct/bZeroDamageOnRedGuard are not applied here - GetActivePerk() exposes them
	// for whoever resolves damage (AFightDirector) to read at release time.
	UFUNCTION(BlueprintCallable, Category = "Boxer|Loadout")
	void ApplyPerkData(const FBoxerPerkData& Perk);

	UFUNCTION(BlueprintPure, Category = "Boxer|Loadout")
	const FBoxerPerkData& GetActivePerk() const { return ActivePerk; }

private:
	UPROPERTY(Transient)
	FBoxerPerkData ActivePerk;

	float BarPosition = 0.0f;
	float BarDirection = 1.0f;
	float BarElapsedTime = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> GuardMontage;

	void PlayPunchMontage(EPowerZone Zone, bool bPerfect);
	void PlayDeathMontage();
	void EnterGuardPose();
	void ExitGuardPose();
	UAnimMontage* PlayDynamicMontage(UAnimSequence* Sequence, float BlendInTime = 0.15f, float BlendOutTime = 0.15f);
};
