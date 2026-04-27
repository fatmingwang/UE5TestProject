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