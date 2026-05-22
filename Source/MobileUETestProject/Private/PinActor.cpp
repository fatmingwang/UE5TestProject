// Fill out your copyright notice in the Description page of Project Settings.


#include "PinActor.h"
float APinActor::m_fGlobalReflectionForceMultiplier = 1.0f; // Default value
float APinActor::GetGlobalReflectionForceMultiplier()
{
    return APinActor::m_fGlobalReflectionForceMultiplier;
}
void APinActor::SetGlobalReflectionForceMultiplier(float NewValue)
{
	APinActor::m_fGlobalReflectionForceMultiplier = NewValue;
}
// Sets default values
APinActor::APinActor()
{
    PrimaryActorTick.bCanEverTick = false;
    RandomBounceAngleDegrees = 0.6;

    PinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PinMesh"));
    RootComponent = PinMesh;
    PinMesh->SetMobility(EComponentMobility::Static);
    PinMesh->SetSimulatePhysics(false); // Pin is static by default
    PinMesh->SetNotifyRigidBodyCollision(true); // Enables OnComponentHit

    ReflectionForceMultiplier = 3.0f; // Default, editable in editor

    // Bind hit event (required for collision response)
    PinMesh->OnComponentHit.AddDynamic(this, &APinActor::OnPinHit);
}

// Called when the game starts or when spawned
void APinActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APinActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



void APinActor::OnPinHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit
)
{
    if (OtherActor && OtherActor != this && OtherComp && OtherComp->IsSimulatingPhysics())
    {
        FVector IncomingVelocity = OtherComp->GetPhysicsLinearVelocity();
        FVector SurfaceNormal = Hit.ImpactNormal.GetSafeNormal();

        // Reflect velocity direction around the hit surface normal
        FVector ReflectedDirection = IncomingVelocity.MirrorByVector(SurfaceNormal).GetSafeNormal();

        // Preserve the incoming speed and boost it by the multiplier for an arcade feel
        float IncomingSpeed = IncomingVelocity.Size();
        // Apply a small random deviation to the reflected direction to make bounces feel more natural.
          // RandomBounceAngleDegrees is editable in the editor.
        float MaxRandomAngleRad = FMath::DegreesToRadians(RandomBounceAngleDegrees);
        FVector PerturbedDirection = FMath::VRandCone(ReflectedDirection, MaxRandomAngleRad);
        //PerturbedDirection.Y = 0;
        if (PerturbedDirection.Z > 0)
        {
            PerturbedDirection *= -1;
        }
        PerturbedDirection = ReflectedDirection + PerturbedDirection;

        FVector NewVelocity = PerturbedDirection * IncomingSpeed * ReflectionForceMultiplier * m_fGlobalReflectionForceMultiplier;


        // Override the ball's velocity directly so it bounces cleanly without stacking forces
        OtherComp->SetPhysicsLinearVelocity(NewVelocity);

#if !UE_BUILD_SHIPPING
        //UE_LOG(LogTemp, Log, TEXT("Pin: Bounced %s | Incoming speed: %.1f | New velocity: %s"), *OtherActor->GetName(), IncomingSpeed, *NewVelocity.ToString());
#endif
    }
}