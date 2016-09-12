// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>
#include "GameFramework/Actor.h"
#include "MPFPSWeapon.generated.h"

#define WEAPON_TRACE_CHANNEL ECC_GameTraceChannel1

USTRUCT()
struct FWeaponConfig
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Config)
	uint32	Damage;

	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	uint32	ClipSize;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	float	Cooldown;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	float	ReloadingTime;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	float	ShootingRange;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	float	ImpactImpulse;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	float	ShootingRejection;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	float	Recoil;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	bool	RapidFire;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	TSubclassOf<class AMPFPSProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	TSubclassOf<class UAnimInstance> AnimationSet;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage *ShootingAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage *EquipAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage *UnequipAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage *ReloadingAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UParticleSystem *MuzzleFlash;
};

UENUM(BlueprintType)
namespace EWeaponProjectile
{
	enum ProjectileType
	{
		EPT_Bullet UMETA(DisplayName = "Bullet"),
		EPT_Spread UMETA(DisplayName = "Spread"),
		EPT_Projectile UMETA(DisplayName = "Projectile")
	};
}

UCLASS()
class MPFPS_API AMPFPSWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMPFPSWeapon();
	AMPFPSWeapon(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	/** Network Functions **/
	UFUNCTION(Reliable, Server, WithValidation)
	void Shoot(const FVector& ShootStartLocation, const FVector& ShootDirection);
	void Shoot_Implementation(const FVector& ShootStartLocation, const FVector& ShootDirection);
	bool Shoot_Validate(const FVector& ShootStartLocation, const FVector& ShootDirection);

	UFUNCTION(Reliable, Server, WithValidation)
	void StartShooting();
	void StartShooting_Implementation();
	bool StartShooting_Validate() { return true; }

	UFUNCTION(Reliable, Server, WithValidation)
	void StopShooting();
	void StopShooting_Implementation();
	bool StopShooting_Validate() { return true; }

	UFUNCTION(Reliable, NetMulticast )
	void ShootEffect();
	void ShootEffect_Implementation();

	bool CanShoot() const { return (Cooldown <= 0.f && Ammo > 0); }

	const int GetAmmo() const { return Ammo; }

	// Main weapon config
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FWeaponConfig	WeaponConfig;
	
	// Ammo type for current weapon
	UPROPERTY(EditDefaultsOnly, Category = Config)
	TEnumAsByte<EWeaponProjectile::ProjectileType>		ProjectileType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh)
	UStaticMeshComponent*	WeaponMesh;

	UPROPERTY(Replicated, BlueprintReadOnly, Category=Ammo)
	int				Ammo;

	UPROPERTY(Replicated)
	float			Cooldown;

	UPROPERTY(Replicated)
	float			Reloading;

protected:
	void InstantShoot(const FVector& ShootStartLocation, const FVector& ShootDirection);
	void SpreadShoot(const FVector& ShootStartLocation, const FVector& ShootDirection);
	void ProjectileShoot(const FVector& ShootStartLocation, const FVector& ShootDirection);
	void ProcessHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDirection);

	std::function<void(const FVector&, const FVector&)>	ShootFunc;
	bool			RapidShooting;
};
