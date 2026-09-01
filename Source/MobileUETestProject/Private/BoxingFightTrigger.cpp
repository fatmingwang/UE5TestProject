// Fill out your copyright notice in the Description page of Project Settings.

#include "BoxingFightTrigger.h"
#include "FightDirector.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ABoxingFightTrigger::ABoxingFightTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = TriggerVolume;
}

void ABoxingFightTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ABoxingFightTrigger::HandleBeginOverlap);
}

void ABoxingFightTrigger::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!FightDirectorRef || !OtherActor || OtherActor != UGameplayStatics::GetPlayerPawn(this, 0))
	{
		return;
	}

	FightDirectorRef->StartFight();

	if (bTriggerOnce)
	{
		TriggerVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
