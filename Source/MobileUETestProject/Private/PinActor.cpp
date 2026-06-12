// Fill out your copyright notice in the Description page of Project Settings.


#include "PinActor.h"
float APinActor::m_fGlobalReflectionForceMultiplier = 0.2f; // Default value
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
    RandomBounceAngleDegrees = 20.0f;

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
    if (!OtherActor || OtherActor == this || !OtherComp || !OtherComp->IsSimulatingPhysics())
        return;

    // Suppress rapid re-hits from the same ball while it's still overlapping the pin
    float Now = GetWorld()->GetTimeSeconds();
    TWeakObjectPtr<AActor> Key(OtherActor);
    float* LastTime = LastHitTimes.Find(Key);
    if (LastTime && (Now - *LastTime) < HitCooldownSeconds)
        return;
    LastHitTimes.Add(Key, Now);

    FVector IncomingVelocity = OtherComp->GetPhysicsLinearVelocity();
    float IncomingSpeed = IncomingVelocity.Size();

    // ImpactNormal points radially outward from the pin surface — the natural exit direction for a round pin.
    // Blending toward reflection (RadialReflectBlend=1) makes it mirror-like; toward 0 is pure arcade pinball.
    FVector RadialDir = Hit.ImpactNormal.GetSafeNormal();
    FVector ReflectedDir = IncomingVelocity.MirrorByVector(RadialDir).GetSafeNormal();
    FVector BlendedDir = FMath::Lerp(RadialDir, ReflectedDir, RadialReflectBlend).GetSafeNormal();

    // Rotate BlendedDir by a random angle in the XZ plane (around Y axis).
    // This gives true 2D spread without needing to zero Y afterward.
    float RandomAngleDeg = FMath::RandRange(-RandomBounceAngleDegrees, RandomBounceAngleDegrees);
    FVector ExitDir = BlendedDir.RotateAngleAxis(RandomAngleDeg, FVector::YAxisVector).GetSafeNormal();

    // Enforce a minimum exit speed so the ball never dies against a pin
    float ExitSpeed = FMath::Max(IncomingSpeed, MinExitSpeed) * ReflectionForceMultiplier * m_fGlobalReflectionForceMultiplier;

    OtherComp->SetPhysicsLinearVelocity(ExitDir * ExitSpeed);

#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Log, TEXT("Pin: Bounced %s | InSpeed: %.1f | ExitSpeed: %.1f | ExitDir: %s"), *OtherActor->GetName(), IncomingSpeed, ExitSpeed, *ExitDir.ToString());
#endif
}