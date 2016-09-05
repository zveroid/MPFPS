// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MPFPSProjectile.generated.h"

UCLASS()
class MPFPS_API AMPFPSProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMPFPSProjectile();
	AMPFPSProjectile(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	void Throw(const FVector& Direction);

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComp;
	
	UPROPERTY(EditDefaultsOnly, Category = Movement)
	UProjectileMovementComponent* ProjectileMovement;
};
