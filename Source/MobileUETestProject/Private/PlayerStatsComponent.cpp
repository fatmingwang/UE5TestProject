// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStatsComponent.h"
#include "MySaveGame.h"

void UPlayerStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	UMySaveGame::GetPlayerStats(this, HP, EXP, Power, Def);
}

void UPlayerStatsComponent::ApplyStoreItem(const FStoreItemData& Item)
{
	HP += Item.HP;
	EXP += Item.EXP;
	Power += Item.Power;
	Def += Item.Def;

	UMySaveGame::SavePlayerStats(this, HP, EXP, Power, Def);
}
