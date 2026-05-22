// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PinActor.generated.h"


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
};