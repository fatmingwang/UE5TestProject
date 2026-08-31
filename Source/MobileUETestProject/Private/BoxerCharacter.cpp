// Fill out your copyright notice in the Description page of Project Settings.

#include "BoxerCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"

namespace
{
	UAnimSequence* PickRandomAnim(const TArray<TObjectPtr<UAnimSequence>>& Anims)
	{
		if (Anims.Num() == 0)
		{
			return nullptr;
		}
		return Anims[FMath::RandHelper(Anims.Num())];
	}
}

ABoxerCharacter::ABoxerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABoxerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState != EBoxerState::ChargingAttack && CurrentState != EBoxerState::ChargingGuard)
	{
		return;
	}

	const bool bAttack = CurrentState == EBoxerState::ChargingAttack;
	const float V0 = bAttack ? AttackV0 : DefenseV0;
	const float Vmax = bAttack ? AttackVmax : DefenseVmax;
	const float RampTime = bAttack ? AttackRampTime : DefenseRampTime;
	const FPowerBarZones& Zones = bAttack ? AttackZones : DefenseZones;

	BarElapsedTime += DeltaTime;
	const float Progress = RampTime > 0.0f ? FMath::Min(1.0f, BarElapsedTime / RampTime) : 1.0f;
	const float Speed = V0 + (Vmax - V0) * FMath::Pow(Progress, 3.0f);

	BarPosition += BarDirection * Speed * DeltaTime;

	const float Span = PowerBarSpan(Zones);
	if (BarPosition > Span)
	{
		BarPosition = 2.0f * Span - BarPosition;
		BarDirection = -1.0f;
	}
	else if (BarPosition < 0.0f)
	{
		BarPosition = -BarPosition;
		BarDirection = 1.0f;
	}
}

void ABoxerCharacter::StartChargingAttack()
{
	CurrentState = EBoxerState::ChargingAttack;
	BarPosition = 0.0f;
	BarDirection = 1.0f;
	BarElapsedTime = 0.0f;
	OnStateChanged.Broadcast(CurrentState);
}

float ABoxerCharacter::ReleaseAttack()
{
	const float Power = ComputePower(BarPosition, AttackZones);
	const EPowerZone Zone = GetPowerZone(BarPosition, AttackZones);
	const bool bPerfect = IsPerfectPower(BarPosition, AttackZones);

	CurrentState = EBoxerState::Punching;
	OnStateChanged.Broadcast(CurrentState);
	PlayPunchMontage(Zone, bPerfect);
	OnAttackReleased.Broadcast(Power, Zone, bPerfect);

	return Power;
}

void ABoxerCharacter::StartChargingGuard()
{
	CurrentState = EBoxerState::ChargingGuard;
	BarPosition = 0.0f;
	BarDirection = 1.0f;
	BarElapsedTime = 0.0f;
	EnterGuardPose();
	OnStateChanged.Broadcast(CurrentState);
}

float ABoxerCharacter::ReleaseGuard()
{
	const float Power = ComputePower(BarPosition, DefenseZones);
	const EPowerZone Zone = GetPowerZone(BarPosition, DefenseZones);
	const bool bPerfect = IsPerfectPower(BarPosition, DefenseZones);

	ExitGuardPose();
	CurrentState = EBoxerState::Guarding;
	OnStateChanged.Broadcast(CurrentState);
	OnGuardReleased.Broadcast(Power, Zone, bPerfect);

	return Power;
}

void ABoxerCharacter::ApplyDamage(float Amount)
{
	CurrentHP = FMath::Clamp(CurrentHP - Amount, 0.0f, MaxHP);
	OnHPChanged.Broadcast(CurrentHP, MaxHP);

	if (CurrentHP <= 0.0f)
	{
		CurrentState = EBoxerState::KO;
		OnStateChanged.Broadcast(CurrentState);
		PlayDeathMontage();
	}
}

void ABoxerCharacter::PlayHitReactMontage(EPowerZone AttackerZone, bool bAttackerPerfect)
{
	UAnimSequence* Seq = nullptr;

	if (bAttackerPerfect)
	{
		Seq = HitReactHeavyAnim;
	}
	else if (AttackerZone == EPowerZone::Blue)
	{
		Seq = PickRandomAnim(HitReactLightAnims);
	}
	else
	{
		Seq = PickRandomAnim(HitReactMediumAnims);
	}

	if (CurrentState != EBoxerState::KO)
	{
		CurrentState = EBoxerState::HitReacting;
		OnStateChanged.Broadcast(CurrentState);
	}

	PlayDynamicMontage(Seq);
}

float ABoxerCharacter::GetBarNormalizedPosition() const
{
	const FPowerBarZones Zones = GetActiveZones();
	const float Span = PowerBarSpan(Zones);
	return Span > 0.0f ? FMath::Clamp(BarPosition / Span, 0.0f, 1.0f) : 0.0f;
}

FPowerBarZones ABoxerCharacter::GetActiveZones() const
{
	return CurrentState == EBoxerState::ChargingGuard ? DefenseZones : AttackZones;
}

void ABoxerCharacter::PlayPunchMontage(EPowerZone Zone, bool bPerfect)
{
	UAnimSequence* Seq = nullptr;

	switch (Zone)
	{
	case EPowerZone::Green:
		Seq = PowerPunchAnim;
		break;
	case EPowerZone::Blue:
		Seq = (HookAnim && FMath::RandBool()) ? HookAnim : CrossAnim;
		break;
	case EPowerZone::Red:
	default:
		Seq = JabAnim;
		break;
	}

	PlayDynamicMontage(Seq);
}

void ABoxerCharacter::PlayDeathMontage()
{
	PlayDynamicMontage(PickRandomAnim(DeathAnims));
}

void ABoxerCharacter::EnterGuardPose()
{
	if (!JabAnim)
	{
		return;
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			GuardMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(JabAnim, MontageSlotName, 0.1f, 0.1f, 1.0f, 1);
			if (GuardMontage)
			{
				AnimInstance->Montage_Pause(GuardMontage);
			}
		}
	}
}

void ABoxerCharacter::ExitGuardPose()
{
	if (!GuardMontage)
	{
		return;
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.1f, GuardMontage);
		}
	}

	GuardMontage = nullptr;
}

UAnimMontage* ABoxerCharacter::PlayDynamicMontage(UAnimSequence* Sequence, float BlendInTime, float BlendOutTime)
{
	if (!Sequence)
	{
		return nullptr;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return nullptr;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		return nullptr;
	}

	return AnimInstance->PlaySlotAnimationAsDynamicMontage(Sequence, MontageSlotName, BlendInTime, BlendOutTime, 1.0f, 1);
}
