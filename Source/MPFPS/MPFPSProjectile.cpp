// Fill out your copyright notice in the Description page of Project Settings.

#include "MPFPS.h"
#include "MPFPSProjectile.h"


// Sets default values
AMPFPSProjectile::AMPFPSProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
}

AMPFPSProjectile::AMPFPSProjectile(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer)
{
	// Init collision component (sphere)
	CollisionComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f);
	RootComponent = CollisionComp;

	// Init basic projectile movement properties
	ProjectileMovement = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.3f;
}
// Called when the game starts or when spawned
void AMPFPSProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMPFPSProjectile::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void AMPFPSProjectile::Throw(const FVector& Direction)
{
	check(ProjectileMovement != nullptr);
	//FVector loc = this->GetActorLocation();
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%f %f %f"), loc.X, loc.Y, loc.Z));
	float len;
	FVector dir;
	Instigator->GetVelocity().ToDirectionAndLength(dir, len);
	ProjectileMovement->Velocity = Direction * (ProjectileMovement->InitialSpeed + len);
}