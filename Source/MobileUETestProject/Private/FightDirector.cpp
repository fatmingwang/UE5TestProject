// Fill out your copyright notice in the Description page of Project Settings.

#include "FightDirector.h"
#include "BoxerCharacter.h"
#include "BoxingCameraRig.h"
#include "BoxingHUDWidget.h"
#include "BoxingDataSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/InputComponent.h"
#include "TimerManager.h"

AFightDirector::AFightDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFightDirector::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerBoxer)
	{
		PlayerBoxer->OnAttackReleased.AddDynamic(this, &AFightDirector::HandlePlayerAttackReleased);
		PlayerBoxer->OnGuardReleased.AddDynamic(this, &AFightDirector::HandlePlayerGuardReleased);
	}

	if (bBindTestHotkey)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			EnableInput(PC);
		}

		if (InputComponent)
		{
			InputComponent->BindKey(TestHotkey, IE_Pressed, this, &AFightDirector::ToggleFightForTesting);
		}
	}

	if (bAutoStartOnBeginPlay)
	{
		StartFight();
	}
}

void AFightDirector::StartFight()
{
	if (bFightStarted || !PlayerBoxer || !OpponentBoxer)
	{
		return;
	}
	bFightStarted = true;

	ApplyLoadouts();

	if (CameraRig)
	{
		CameraRig->PlayBellShot(PlayerBoxer, OpponentBoxer);
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PreviousPawn = PC->GetPawn();
		PreviousViewTarget = PC->GetViewTarget();

		PC->Possess(PlayerBoxer);

		if (CameraRig)
		{
			PC->SetViewTargetWithBlend(CameraRig, 0.5f, VTBlend_Cubic);
		}

		if (!HUDWidgetInstance && HUDWidgetClass)
		{
			HUDWidgetInstance = CreateWidget<UBoxingHUDWidget>(PC, HUDWidgetClass);
			if (HUDWidgetInstance)
			{
				HUDWidgetInstance->SetFightDirector(this);
				HUDWidgetInstance->AddToViewport();
			}
		}
		else if (HUDWidgetInstance)
		{
			HUDWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		}

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	SetState(EFightState::Bell);
	GetWorldTimerManager().SetTimer(BellTimerHandle, this, &AFightDirector::StartPlayerAttackPhase, BellDuration, false);
}

void AFightDirector::ResetFight()
{
	bFightStarted = false;
	GetWorldTimerManager().ClearTimer(BellTimerHandle);
	CurrentState = EFightState::Bell;

	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);

		if (APawn* PawnToRestore = PreviousPawn.Get())
		{
			PC->Possess(PawnToRestore);
		}

		if (AActor* ViewTargetToRestore = PreviousViewTarget.Get())
		{
			PC->SetViewTargetWithBlend(ViewTargetToRestore, 0.5f, VTBlend_Cubic);
		}
	}

	PreviousPawn = nullptr;
	PreviousViewTarget = nullptr;
}

void AFightDirector::ToggleFightForTesting()
{
	if (bFightStarted)
	{
		ResetFight();
	}
	else
	{
		StartFight();
	}
}

void AFightDirector::ApplyLoadouts()
{
	const UBoxingDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UBoxingDataSubsystem>() : nullptr;
	if (!DataSubsystem)
	{
		return;
	}

	if (PlayerBoxer)
	{
		if (const FBoxerGloveData* Glove = DataSubsystem->FindGlove(PlayerGloveID))
		{
			PlayerBoxer->ApplyGloveData(*Glove);
		}

		if (const FBoxerStanceData* Stance = DataSubsystem->FindStance(PlayerStanceID))
		{
			PlayerBoxer->ApplyStanceData(*Stance);
		}

		if (!PlayerPerkID.IsEmpty())
		{
			if (const FBoxerPerkData* Perk = DataSubsystem->FindPerk(PlayerPerkID))
			{
				PlayerBoxer->ApplyPerkData(*Perk);
			}
		}
	}

	if (OpponentBoxer)
	{
		if (const FOpponentData* Opponent = DataSubsystem->FindOpponent(OpponentID))
		{
			OpponentBoxer->ApplyOpponentData(*Opponent);
			OpponentSkill = Opponent->OpponentSkill;
		}
	}
}

void AFightDirector::SetState(EFightState NewState)
{
	CurrentState = NewState;
	OnFightStateChanged.Broadcast(CurrentState);
}

void AFightDirector::StartPlayerAttackPhase()
{
	if (!PlayerBoxer || !OpponentBoxer)
	{
		return;
	}

	SetState(EFightState::PlayerAttack);

	if (CameraRig)
	{
		CameraRig->EnterReadyFraming(PlayerBoxer);
	}
}

void AFightDirector::StartPlayerDefendPhase()
{
	SetState(EFightState::PlayerDefend);

	if (CameraRig && OpponentBoxer)
	{
		CameraRig->EnterReadyFraming(OpponentBoxer);
	}
}

void AFightDirector::HandleAttackButtonPressed()
{
	if (CurrentState != EFightState::PlayerAttack || !PlayerBoxer)
	{
		return;
	}

	PlayerBoxer->StartChargingAttack();

	if (CameraRig)
	{
		CameraRig->BeginChargeDolly(PlayerBoxer->AttackRampTime);
	}
}

void AFightDirector::HandleAttackButtonReleased()
{
	if (CurrentState != EFightState::PlayerAttack || !PlayerBoxer || !PlayerBoxer->IsCharging())
	{
		return;
	}

	PlayerBoxer->ReleaseAttack();
}

void AFightDirector::HandlePlayerAttackReleased(float Power, EPowerZone Zone, bool bPerfect)
{
	if (!PlayerBoxer || !OpponentBoxer)
	{
		return;
	}

	const float BaseHit = FMath::Max(1.0f, PlayerBoxer->ATK - OpponentBoxer->DEF);
	float Mult = 0.2f + (PlayerBoxer->AttackMaxMult - 0.2f) * (Power / 100.0f);
	float Damage = FMath::CeilToFloat(BaseHit * Mult);
	if (bPerfect)
	{
		Damage = FMath::CeilToFloat(Damage * 1.25f);
	}

	OpponentBoxer->ApplyDamage(Damage);

	if (CameraRig)
	{
		CameraRig->PunchImpactShake(Power);
	}

	if (OpponentBoxer->CurrentHP <= 0.0f)
	{
		EndFight(true);
		return;
	}

	StartPlayerDefendPhase();
}

void AFightDirector::HandleGuardButtonPressed()
{
	if (CurrentState != EFightState::PlayerDefend || !PlayerBoxer)
	{
		return;
	}

	PlayerBoxer->StartChargingGuard();

	if (CameraRig)
	{
		CameraRig->BeginChargeDolly(PlayerBoxer->DefenseRampTime);
	}
}

void AFightDirector::HandleGuardButtonReleased()
{
	if (CurrentState != EFightState::PlayerDefend || !PlayerBoxer || !PlayerBoxer->IsCharging())
	{
		return;
	}

	PlayerBoxer->ReleaseGuard();
}

void AFightDirector::HandlePlayerGuardReleased(float GuardPower, EPowerZone GuardZone, bool bPerfectGuard)
{
	if (!PlayerBoxer || !OpponentBoxer)
	{
		return;
	}

	float OpponentPower = 0.0f;
	EPowerZone OpponentZone = EPowerZone::Red;
	bool bOpponentPerfect = false;
	ResolveOpponentPunch(OpponentPower, OpponentZone, bOpponentPerfect);

	const float BaseHit = FMath::Max(1.0f, OpponentBoxer->ATK - PlayerBoxer->DEF);
	float Mult = 0.2f + (OpponentBoxer->AttackMaxMult - 0.2f) * (OpponentPower / 100.0f);
	float IncomingDamage = FMath::CeilToFloat(BaseHit * Mult);
	if (bOpponentPerfect)
	{
		IncomingDamage = FMath::CeilToFloat(IncomingDamage * 1.25f);
	}

	const float Mitigation = GuardPower / 100.0f;
	const float FinalDamage = bPerfectGuard ? 0.0f : FMath::CeilToFloat(IncomingDamage * (1.0f - Mitigation));

	PlayerBoxer->ApplyDamage(FinalDamage);
	if (FinalDamage > 0.0f)
	{
		PlayerBoxer->PlayHitReactMontage(OpponentZone, bOpponentPerfect);
	}

	if (CameraRig)
	{
		CameraRig->GuardImpactShake(GuardPower);
	}

	if (PlayerBoxer->CurrentHP <= 0.0f)
	{
		EndFight(false);
		return;
	}

	StartPlayerAttackPhase();
}

void AFightDirector::ResolveOpponentPunch(float& OutPower, EPowerZone& OutZone, bool& OutPerfect) const
{
	if (!OpponentBoxer)
	{
		OutPower = 50.0f;
		OutZone = EPowerZone::Blue;
		OutPerfect = false;
		return;
	}

	const FPowerBarZones& Zones = OpponentBoxer->AttackZones;
	// OpponentSkill in [0,1]: 0 always samples near dead-center (best), 1 samples anywhere up to
	// the bar's own edge.
	const float MaxDistance = FMath::Lerp(Zones.GreenHalf * 0.25f, Zones.RedOuter, OpponentSkill);
	const float Distance = FMath::FRandRange(0.0f, FMath::Max(0.0f, MaxDistance));
	const float Position = PowerBarCenter(Zones) + Distance;

	OutPower = ComputePower(Position, Zones);
	OutZone = GetPowerZone(Position, Zones);
	OutPerfect = IsPerfectPower(Position, Zones);
}

void AFightDirector::EndFight(bool bPlayerWon)
{
	SetState(bPlayerWon ? EFightState::Win : EFightState::Lose);

	if (!CameraRig)
	{
		return;
	}

	if (bPlayerWon)
	{
		CameraRig->PlayVictoryOrbit(PlayerBoxer);
	}
	else
	{
		CameraRig->PlayKOOrbit(PlayerBoxer);
	}
}
