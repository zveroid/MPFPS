// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MPFPSProjectile.generated.h"

USTRUCT()
struct FProjectileConfig
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Explosion)
	uint32	EpicenterDamage;

	UPROPERTY(EditDefaultsOnly, Category = Explosion)
	uint32	MinimumDamage;

	UPROPERTY(EditDefaultsOnly, Category = Explosion)
	float	Falloff;

	UPROPERTY(EditDefaultsOnly, Category = Explosion)
	float	ExplosionInnerRadius;

	UPROPERTY(EditDefaultsOnly, Category = Explosion)
	float	ExplosionOuterRadius;

	UPROPERTY(EditDefaultsOnly, Category = Explosion)
	uint32	ExplosionImpulse;

	UPROPERTY(EditDefaultsOnly, Category = Explosion)
	UParticleSystem*	ExplosionEffect;
};

UCLASS()
class MPFPS_API AMPFPSProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMPFPSProjectile();
	AMPFPSProjectile(const FObjectInitializer& ObjectInitializer);

	void BeginPlay() override;
	void BeginDestroy() override;
	
	void Tick( float DeltaSeconds ) override;

	void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	void Throw(const FVector& Direction);

	void Explode(const FVector& Location);

	UFUNCTION(Reliable, NetMulticast)
	void ExplosionEffect(const FVector& Location);
	void ExplosionEffect_Implementation(const FVector& Location);

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UMeshComponent* Mesh;
	
	UPROPERTY(EditDefaultsOnly, Category = Movement)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	FProjectileConfig ProjectileConfig;
};
