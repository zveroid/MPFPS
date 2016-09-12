// Fill out your copyright notice in the Description page of Project Settings.

#include "MPFPS.h"
#include "Net/UnrealNetwork.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "MPFPSProjectile.h"
#include "MPFPSPlayer.h"
#include "MPFPSWeapon.h"

static FORCEINLINE bool Trace(
							const UWorld* World,
							const FVector & From, 
							const FVector & To, 
							FHitResult& OutHit, 
							AActor* IgnoredActor = nullptr,
							ECollisionChannel CollisionChannel = WEAPON_TRACE_CHANNEL,
							bool ReturnPhysMat = false)
{
	FCollisionQueryParams TraceParams(FName(TEXT("WeaponTrace")), true, IgnoredActor);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = ReturnPhysMat;
	TraceParams.AddIgnoredActor(IgnoredActor);

	OutHit = FHitResult(ForceInit);
	
	World->LineTraceSingleByChannel(
		OutHit,
		From,
		To,
		CollisionChannel,
		TraceParams
		);

	return (OutHit.GetActor() != NULL);
}

// Sets default values
AMPFPSWeapon::AMPFPSWeapon() :
	Ammo(0)
	, Cooldown(0.f)
	, Reloading(0.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AMPFPSWeapon::AMPFPSWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Ammo(0)
	, Cooldown(0.f)
	, Reloading(0.f)
{
	WeaponMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMPFPSWeapon::BeginPlay()
{
	Super::BeginPlay();

	Ammo = WeaponConfig.ClipSize;
	Cooldown = 0.f;

	switch (ProjectileType)
	{
	case EWeaponProjectile::EPT_Bullet:
		ShootFunc = std::bind(&AMPFPSWeapon::InstantShoot, this, std::placeholders::_1, std::placeholders::_2);
		break;
	case EWeaponProjectile::EPT_Spread:
		ShootFunc = std::bind(&AMPFPSWeapon::SpreadShoot, this, std::placeholders::_1, std::placeholders::_2);
		break;
	case EWeaponProjectile::EPT_Projectile:
		ShootFunc = std::bind(&AMPFPSWeapon::ProjectileShoot, this, std::placeholders::_1, std::placeholders::_2);
		break;
	default:
		checkNoEntry();
		break;
	}
}

// Called every frame
void AMPFPSWeapon::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	if (Cooldown > 0.f)
	{
		Cooldown -= DeltaTime;
		if (Cooldown <= 0.f && RapidShooting)
		{
			Shoot(FVector(), FVector());
		}
	}
	if (Reloading >= 0.f)
	{
		Reloading -= DeltaTime;
		if (Reloading <= 0.f)
			Ammo = WeaponConfig.ClipSize;
	}
}

void AMPFPSWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMPFPSWeapon, Ammo);
	DOREPLIFETIME(AMPFPSWeapon, Cooldown);
	DOREPLIFETIME(AMPFPSWeapon, Reloading);
}

void AMPFPSWeapon::Shoot_Implementation(const FVector& ShootStartLocation, const FVector & ShootDirection)
{
	if (Cooldown <= 0.f)
	{
		if (Ammo > 0)
		{
			AMPFPSPlayer* owner = Cast<AMPFPSPlayer>(Instigator);
			if (owner)
			{
				ShootEffect();
				//TODO: Shoot from the muzzle looks better right now. Prob'ly make it shooting from camera center later
				FVector CameraLocation;
				FRotator CameraRotation;
				Instigator->GetActorEyesViewPoint(CameraLocation, CameraRotation);
				ShootFunc(WeaponMesh->GetSocketLocation(TEXT("Muzzle")), CameraRotation.Vector());
				Ammo--;
				owner->AddControllerPitchInput(-(WeaponConfig.Recoil));
				//TODO: Make this stuff more elegant. Play some animation etc...
				if (Ammo == 0)
				{
					StopShooting();
					Reloading = WeaponConfig.ReloadingTime;
				}
				Cooldown = WeaponConfig.Cooldown;
			}
		}
	}
}

bool AMPFPSWeapon::Shoot_Validate(const FVector& ShootStartLocation, const FVector & ShootDirection)
{
	return true;
}

void AMPFPSWeapon::ShootEffect_Implementation()
{
	if (WeaponConfig.MuzzleFlash)
	{
		//TODO: Must be some way to simply replay same component, not to spawn it again every time
		UParticleSystemComponent*	MuzzleFlashComponent = UGameplayStatics::SpawnEmitterAttached(WeaponConfig.MuzzleFlash, WeaponMesh, TEXT("Muzzle"));
		if (MuzzleFlashComponent)
		{
			MuzzleFlashComponent->ActivateSystem();
		}
	}

	if (WeaponConfig.ShootingAnim)
	{
		Cast<AMPFPSPlayer>(Instigator)->GetMesh()->GetAnimInstance()->Montage_Play(WeaponConfig.ShootingAnim);
	}
}

void AMPFPSWeapon::StartShooting_Implementation()
{
	RapidShooting = WeaponConfig.RapidFire;
	Shoot(FVector(), FVector());
}

void AMPFPSWeapon::StopShooting_Implementation()
{
	RapidShooting = false;
}

void AMPFPSWeapon::InstantShoot(const FVector& ShootStartLocation, const FVector& ShootDirection)
{
	const FVector Start = ShootStartLocation;
	const FVector End = Start + 
		ShootDirection.RotateAngleAxis(FMath::FRandRange(0.0F, WeaponConfig.ShootingRejection), FMath::VRand()) * 
		WeaponConfig.ShootingRange;
	FHitResult	hit;

	if (Trace(GetWorld(), Start, End, hit, Instigator))
	{
		//DrawDebugLine(GetWorld(), Start, hit.ImpactPoint, FColor::Black, true);
		TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());

		UGameplayStatics::ApplyPointDamage(hit.GetActor(), WeaponConfig.Damage, hit.TraceEnd - hit.TraceStart, hit, GetOwner()->GetInstigatorController(), GetOwner(), ValidDamageTypeClass);
	}
	else
	{
		//DrawDebugLine(GetWorld(), Start, End, FColor::Black, true);
	}
}

void AMPFPSWeapon::SpreadShoot(const FVector & ShootStartLocation, const FVector & ShootDirection)
{
	for (int i = 0; i < 10; ++i)
	{
		InstantShoot(ShootStartLocation, ShootDirection);
	}
}

void AMPFPSWeapon::ProjectileShoot(const FVector& ShootStartLocation, const FVector& ShootDirection)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Instigator;
	AMPFPSProjectile* const Projectile = GetWorld()->SpawnActor<AMPFPSProjectile>(WeaponConfig.ProjectileClass, WeaponMesh->GetSocketLocation(TEXT("Muzzle")), WeaponMesh->GetSocketRotation(TEXT("Muzzle")), SpawnParams);
	if (Projectile)
	{
		Projectile->Throw(ShootDirection);
	}
}

void AMPFPSWeapon::ProcessHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDirection)
{

}
