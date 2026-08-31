// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PowerBarMath.generated.h"

// Which third of the bar the marker landed in when released. See Doc/BattleConecpt/
// fight_scene_demo.html and MAIN_BATTLE_SCENE_DESIGN.md section 5 for the design this ports.
UENUM(BlueprintType)
enum class EPowerZone : uint8
{
	Green,
	Blue,
	Red
};

// Three radii measured outward from the bar's center. RedOuter also defines the bar's own
// half-width/edge, so the marker's total travel range is [0, 2*RedOuter] with the center at
// RedOuter. Defaults match the HTML prototype's baseline weapon/armor (wand/robe).
USTRUCT(BlueprintType)
struct FPowerBarZones
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerBar")
	float GreenHalf = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerBar")
	float BlueOuter = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerBar")
	float RedOuter = 60.0f;
};

FORCEINLINE float PowerBarCenter(const FPowerBarZones& Zones)
{
	return Zones.RedOuter;
}

FORCEINLINE float PowerBarSpan(const FPowerBarZones& Zones)
{
	return Zones.RedOuter * 2.0f;
}

// Piecewise-linear power (0-100) as a function of distance from the bar's center: GREEN is
// always 71-100, BLUE always 31-70, RED always 0-30, whatever the three radii are.
FORCEINLINE float ComputePower(float Pos, const FPowerBarZones& Zones)
{
	const float Distance = FMath::Abs(Pos - PowerBarCenter(Zones));

	if (Distance <= Zones.GreenHalf)
	{
		const float T = Zones.GreenHalf <= 0.0f ? 0.0f : Distance / Zones.GreenHalf;
		return 100.0f - 29.0f * T;
	}
	else if (Distance <= Zones.BlueOuter)
	{
		const float T = (Distance - Zones.GreenHalf) / (Zones.BlueOuter - Zones.GreenHalf);
		return 70.0f - 39.0f * T;
	}
	else
	{
		const float T = FMath::Min(1.0f, (Distance - Zones.BlueOuter) / (Zones.RedOuter - Zones.BlueOuter));
		return 30.0f - 30.0f * T;
	}
}

FORCEINLINE EPowerZone GetPowerZone(float Pos, const FPowerBarZones& Zones)
{
	const float Distance = FMath::Abs(Pos - PowerBarCenter(Zones));

	if (Distance <= Zones.GreenHalf)
	{
		return EPowerZone::Green;
	}
	if (Distance <= Zones.BlueOuter)
	{
		return EPowerZone::Blue;
	}
	return EPowerZone::Red;
}

// GREEN-PERFECT: dead-center enough that power reads >= 97.
FORCEINLINE bool IsPerfectPower(float Pos, const FPowerBarZones& Zones)
{
	return GetPowerZone(Pos, Zones) == EPowerZone::Green && ComputePower(Pos, Zones) >= 97.0f;
}
