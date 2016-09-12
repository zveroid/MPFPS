// Fill out your copyright notice in the Description page of Project Settings.

#include "MPFPS.h"
#include "MPFPSPlayer.h"
#include "MPFPSProjectile.h"


// Sets default values
AMPFPSProjectile::AMPFPSProjectile()
{
}

AMPFPSProjectile::AMPFPSProjectile(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer)
{
	// Init collision component (sphere)
	CollisionSphere = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	CollisionSphere->InitSphereRadius(15.f);
	RootComponent = CollisionSphere;

	//RootComponent = Mesh;
	// Init basic projectile movement properties
	ProjectileMovement = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.3f;

	PrimaryActorTick.bCanEverTick = true;
}
// Called when the game starts or when spawned
void AMPFPSProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMPFPSProjectile::BeginDestroy()
{
	Super::BeginDestroy();
	
}

// Called every frame
void AMPFPSProjectile::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	if (HasAuthority() && GetLifeSpan() < 1.f)
	{
		Explode(RootComponent->GetComponentLocation());
	}
}

void AMPFPSProjectile::NotifyHit(class UPrimitiveComponent* MyComp, 
							AActor* Other, 
							class UPrimitiveComponent* OtherComp,
							bool bSelfMoved, 
							FVector HitLocation,
							FVector HitNormal, 
							FVector NormalImpulse, 
							const FHitResult& Hit)
{
	if (Cast<AMPFPSPlayer>(Other) != nullptr)
	{
		Explode(HitLocation);
	}
}

void AMPFPSProjectile::Throw(const FVector& Direction)
{
	check(ProjectileMovement != nullptr);
	//FVector loc = this->GetActorLocation();
	FVector velocity = Direction * (ProjectileMovement->InitialSpeed + Instigator->GetVelocity().Size());
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Direction: %f %f %f"), Direction.X, Direction.Y, Direction.Z));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Velocity: %f %f %f"), velocity.X, velocity.Y, velocity.Z));
	ProjectileMovement->Velocity = velocity;
}

void AMPFPSProjectile::Explode(const FVector& Location)
{
	ExplosionEffect(Location);
	TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
	UGameplayStatics::ApplyRadialDamageWithFalloff(this, 
		ProjectileConfig.EpicenterDamage, ProjectileConfig.MinimumDamage, Location, 
		ProjectileConfig.ExplosionInnerRadius, ProjectileConfig.ExplosionOuterRadius, ProjectileConfig.Falloff, 
		ValidDamageTypeClass, TArray<AActor*>(), Instigator);

	Destroy();
}

void AMPFPSProjectile::ExplosionEffect_Implementation(const FVector& Location)
{
	if (ProjectileConfig.ExplosionEffect)
	{
		UParticleSystemComponent*	ExplosionComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ProjectileConfig.ExplosionEffect, FTransform(Location));
		if (ExplosionComponent)
		{
			ExplosionComponent->ActivateSystem();
		}
	}
}