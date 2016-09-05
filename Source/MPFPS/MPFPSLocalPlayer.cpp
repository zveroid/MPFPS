// Fill out your copyright notice in the Description page of Project Settings.

#include "MPFPS.h"
#include "Net/UnrealNetwork.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "MPFPSWeapon.h"
#include "MPFPSLocalPlayer.h"


// Sets default values
AMPFPSLocalPlayer::AMPFPSLocalPlayer() : 
	MainWeaponClass(nullptr)
	, SecondaryWeaponClass(nullptr)
	, EquippedWeapon(nullptr)
	, Health(100)
	, CameraPitch(0)
{
	PrimaryActorTick.bCanEverTick = true;
}

AMPFPSLocalPlayer::AMPFPSLocalPlayer(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
	, MainWeaponClass(nullptr)
	, SecondaryWeaponClass(nullptr)
	, EquippedWeapon(nullptr)
	, Health(100)
	, RespawnTimer(0.f)
{
	static ConstructorHelpers::FClassFinder<AMPFPSWeapon> MainWeaponBlueprint(TEXT("Blueprint'/Game/Blueprints/BP_Weapon_Shotgun.BP_Weapon_Shotgun_C'"));
	static ConstructorHelpers::FClassFinder<AMPFPSWeapon> SecondaryWeaponBlueprint(TEXT("Blueprint'/Game/Blueprints/BP_Weapon_MP443.BP_Weapon_MP443_C'"));

	if (MainWeaponBlueprint.Succeeded())
	{
		MainWeaponClass = MainWeaponBlueprint.Class;
	}
	if (SecondaryWeaponBlueprint.Succeeded())
	{
		SecondaryWeaponClass = SecondaryWeaponBlueprint.Class;
	}

	FirstPersonCameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh(), TEXT("FPCameraSocket"));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	bReplicates = true;
}

// Called when the game starts or when spawned
void AMPFPSLocalPlayer::BeginPlay()
{
	Super::BeginPlay();

	Health = 100;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = Instigator;
	Inventory.MainWeapon = GetWorld()->SpawnActor<AMPFPSWeapon>(MainWeaponClass, SpawnParams);
	Inventory.SecondaryWeapon = GetWorld()->SpawnActor<AMPFPSWeapon>(SecondaryWeaponClass, SpawnParams);
	if (Inventory.MainWeapon)
	{
		Inventory.MainWeapon->WeaponMesh->SetCastShadow(true);
		Inventory.MainWeapon->WeaponMesh->SetOnlyOwnerSee(false);
		
	}
	if (Inventory.SecondaryWeapon)
	{
		Inventory.SecondaryWeapon->WeaponMesh->SetCastShadow(true);
		Inventory.SecondaryWeapon->WeaponMesh->SetOnlyOwnerSee(false);
	}

	EquipWeapon(Inventory.MainWeapon);
}

// Called every frame
void AMPFPSLocalPlayer::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	if (Health <= 0)
	{
		if (InputEnabled())
		{
			DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			RespawnTimer = 5.f;
		}
		else
		{
			if (GetWorld()->GetAuthGameMode())
			{
				RespawnTimer -= DeltaTime;
				if (RespawnTimer <= 0.f)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("RESPAAAAWN!!"));
					GetWorld()->GetAuthGameMode()->RestartPlayer(GetController());
				}
			}
				
		}
	}
	else
	{
		// TODO: Some dragging can be noted on client. What's a deal  with Client Prediction in UE4?
		SetPitch(FMath::ClampAngle(FMath::RInterpTo(FRotator(CameraPitch, 0, 0), GetControlRotation(), DeltaTime, 15).Pitch, -90, 90));
	}
}

// Called to bind functionality to input
void AMPFPSLocalPlayer::SetupPlayerInputComponent(class UInputComponent* _InputComponent)
{
	Super::SetupPlayerInputComponent(_InputComponent);

	_InputComponent->BindAxis("MoveForward", this, &AMPFPSLocalPlayer::MoveForward);
	_InputComponent->BindAxis("StrafeRight", this, &AMPFPSLocalPlayer::StrafeRight);
	_InputComponent->BindAxis("Turn", this, &AMPFPSLocalPlayer::AddControllerYawInput);
	_InputComponent->BindAxis("LookUp", this, &AMPFPSLocalPlayer::AddControllerPitchInput);
	_InputComponent->BindAction("Jump", IE_Pressed, this, &AMPFPSLocalPlayer::StartJump);
	_InputComponent->BindAction("Jump", IE_Released, this, &AMPFPSLocalPlayer::EndJump);
	_InputComponent->BindAction("Fire", IE_Pressed, this, &AMPFPSLocalPlayer::Fire);
	_InputComponent->BindAction("NextWeapon", IE_Pressed, this, &AMPFPSLocalPlayer::SwitchWeapon);
	_InputComponent->BindAction("PrevWeapon", IE_Pressed, this, &AMPFPSLocalPlayer::SwitchWeapon);
	_InputComponent->BindAction("Crouch", IE_Pressed, this, &AMPFPSLocalPlayer::StartCrouch);
	_InputComponent->BindAction("Crouch", IE_Released, this, &AMPFPSLocalPlayer::EndCrouch);
}

float AMPFPSLocalPlayer::TakeDamage(float Damage, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Ouch!"));
	ApplyDamage(Damage);
	return Health;
}

void AMPFPSLocalPlayer::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMPFPSLocalPlayer, EquippedWeapon);
	DOREPLIFETIME(AMPFPSLocalPlayer, CameraPitch);
	DOREPLIFETIME(AMPFPSLocalPlayer, Health);
}

void AMPFPSLocalPlayer::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.0f && Health > 0)
	{
		FRotator Rotation = Controller->GetControlRotation();
		
		if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
		{
			Rotation.Pitch = 0.0f;
		}
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMPFPSLocalPlayer::ApplyDamage_Implementation(float Damage)
{
	if (Health > 0)
		Health -= Damage;
}

bool AMPFPSLocalPlayer::ApplyDamage_Validate(float Damage)
{
	return true;
}

void AMPFPSLocalPlayer::SwitchWeapon_Implementation()
{
	if (Health <= 0)
		return;

	if (EquippedWeapon == Inventory.MainWeapon)
	{
		EquipWeapon(Inventory.SecondaryWeapon);
	}
	else
	{
		EquipWeapon(Inventory.MainWeapon);
	}
}

bool AMPFPSLocalPlayer::SwitchWeapon_Validate()
{
	return true;
}

void AMPFPSLocalPlayer::SetupWeaponMesh_Implementation(AMPFPSWeapon* Weapon)
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, false);
		EquippedWeapon->DetachAllSceneComponents(GetMesh(), DetachRules);
		EquippedWeapon->WeaponMesh->SetVisibility(false);
	}
	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
	Weapon->AttachToComponent(GetMesh(), AttachRules, TEXT("WeaponSocket"));
	Weapon->WeaponMesh->SetVisibility(true);
	if (Weapon->WeaponConfig.AnimationSet)
	{
		GetMesh()->SetAnimInstanceClass(Weapon->WeaponConfig.AnimationSet);
	}
}

void AMPFPSLocalPlayer::StrafeRight(float Value)
{
	if (Controller != nullptr && Value != 0.0f && Health > 0)
	{
		FRotator Rotation = Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AMPFPSLocalPlayer::Fire()
{
	if (!EquippedWeapon || Health <= 0)
		return;
	
	FVector CameraLocation;
	FRotator CameraRotation;
	GetActorEyesViewPoint(CameraLocation, CameraRotation);
	if (EquippedWeapon->CanShoot())
	{
		EquippedWeapon->Shoot(CameraRotation.Vector());
		if (EquippedWeapon->WeaponConfig.ShootingAnim)
		{
			GetMesh()->GetAnimInstance()->Montage_Play(EquippedWeapon->WeaponConfig.ShootingAnim);
		}
	}
}

void AMPFPSLocalPlayer::EquipWeapon(AMPFPSWeapon* Weapon)
{
	check(Weapon != nullptr);
	SetupWeaponMesh(Weapon);
	UnequipWeapon();
	EquippedWeapon = Weapon;
}

void AMPFPSLocalPlayer::UnequipWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon = nullptr;
	}
}
