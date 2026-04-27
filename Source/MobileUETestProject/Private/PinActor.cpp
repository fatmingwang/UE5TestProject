// Fill out your copyright notice in the Description page of Project Settings.


#include "PinActor.h"

// Sets default values
APinActor::APinActor()
{
    PrimaryActorTick.bCanEverTick = false;

    PinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PinMesh"));
    RootComponent = PinMesh;
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

        // Reflect the incoming Ball velocity around the collision normal
        FVector ReflectedVector = FVector::VectorPlaneProject(IncomingVelocity, SurfaceNormal) - FVector::DotProduct(IncomingVelocity, SurfaceNormal) * SurfaceNormal;
        ReflectedVector = IncomingVelocity.MirrorByVector(SurfaceNormal);

        // Scale by the ReflectionForceMultiplier (higher means 'gibber' bounce)
        FVector Impulse = ReflectedVector * ReflectionForceMultiplier;

        // Apply the impulse; use VelocityChange to ignore object mass for more arcade feeling
        OtherComp->AddImpulse(Impulse, NAME_None, true);

#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Log, TEXT("Pin: Applied reflection force %s to %s"), *Impulse.ToString(), *OtherActor->GetName());
#endif
    }
}