// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PinActor.generated.h"

class USoundBase;


UCLASS()
class MOBILEUETESTPROJECT_API APinActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components",
    meta = (DisplayName = "Pin Mesh"))
	UStaticMeshComponent* PinMesh;

	// The force multiplier applied to the reflection impulse
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (DisplayName = "Reflection Force Multiplier"))
	float ReflectionForceMultiplier;

	// Maximum random deviation (degrees) applied to the reflected direction to make bounces less perfect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (DisplayName = "Random Bounce Angle (Deg)"))
	float RandomBounceAngleDegrees = 6.0f;

	// 0 = pure radial outward (arcade pinball feel), 1 = pure mirror reflection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (DisplayName = "Radial vs Reflect Blend", ClampMin = "0.0", ClampMax = "1.0"))
	float RadialReflectBlend = 0.3f;

	// Ball will always exit at least this fast (prevents ball getting stuck on a pin)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (DisplayName = "Min Exit Speed"))
	float MinExitSpeed = 400.0f;

	// Ignore re-hits from the same ball within this window (seconds) to prevent wedge-lock
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (DisplayName = "Hit Cooldown (sec)"))
	float HitCooldownSeconds = 0.15f;

	static float m_fGlobalReflectionForceMultiplier;

	UFUNCTION(BlueprintCallable, Category = "Physics", meta = (DisplayName = "Get Global Reflection Force Multiplier"))
	static float GetGlobalReflectionForceMultiplier();

	UFUNCTION(BlueprintCallable,  Category = "Physics", meta = (DisplayName = "Set Global Reflection Force Multiplier"))
	static void SetGlobalReflectionForceMultiplier(float NewValue);


	// Sets default values for this actor's properties
	APinActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnPinHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

private:
	// Per-ball last-hit timestamp to suppress rapid re-hit during wedge/overlap
	TMap<TWeakObjectPtr<AActor>, float> LastHitTimes;

public:
	// Sound to play when a ball collides with the pin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (DisplayName = "Hit Sound"))
	USoundBase* HitSound;
};