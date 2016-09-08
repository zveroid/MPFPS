// Fill out your copyright notice in the Description page of Project Settings.

#include "MPFPS.h"
#include "Net/UnrealNetwork.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "MPFPSWeapon.h"
#include "MPFPSPlayer.h"


// Sets default values
AMPFPSPlayer::AMPFPSPlayer() : 
	MainWeaponClass(nullptr)
	, SecondaryWeaponClass(nullptr)
	, EquippedWeapon(nullptr)
	, Health(0)
	, CameraPitch(0)
{
	PrimaryActorTick.bCanEverTick = true;
}

AMPFPSPlayer::AMPFPSPlayer(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
	, MainWeaponClass(nullptr)
	, SecondaryWeaponClass(nullptr)
	, EquippedWeapon(nullptr)
	, Health(0)
	, RespawnTimer(0.f)
{
	static ConstructorHelpers::FClassFinder<AMPFPSWeapon> MainWeaponBlueprint(TEXT("Blueprint'/Game/Blueprints/Weapons/BP_Weapon_Shotgun.BP_Weapon_Shotgun_C'"));
	static ConstructorHelpers::FClassFinder<AMPFPSWeapon> SecondaryWeaponBlueprint(TEXT("Blueprint'/Game/Blueprints/Weapons/BP_Weapon_MP443.BP_Weapon_MP443_C'"));

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
void AMPFPSPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void AMPFPSPlayer::SetPlayerDefaults()
{
	Health = MaxHealth;
	GetCharacterMovement()->MaxWalkSpeed = DefaultMovementSpeed;

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
void AMPFPSPlayer::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	//TODO: Must be more elegant way
	if (Health <= 0)
	{
		if (InputEnabled())
		{
			DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			RespawnTimer = 3.f;
		}
		else
		{
			if (GetWorld()->GetAuthGameMode())
			{
				RespawnTimer -= DeltaTime;
				if (RespawnTimer <= 0.f)
				{
					GetWorld()->GetAuthGameMode()->RestartPlayer(GetController());
				}
			}
				
		}
	}
	else
	{
		if (!InputEnabled())
			EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		// TODO: Some dragging can be noted on client. What's a deal  with Client Prediction in UE4?
		SetPitch(FMath::ClampAngle(FMath::RInterpTo(FRotator(CameraPitch, 0, 0), GetControlRotation(), DeltaTime, 15).Pitch, -90, 90));
	}
}

// Called to bind functionality to input
void AMPFPSPlayer::SetupPlayerInputComponent(class UInputComponent* _InputComponent)
{
	Super::SetupPlayerInputComponent(_InputComponent);

	_InputComponent->BindAxis("MoveForward", this, &AMPFPSPlayer::MoveForward);
	_InputComponent->BindAxis("StrafeRight", this, &AMPFPSPlayer::StrafeRight);
	_InputComponent->BindAxis("Turn", this, &AMPFPSPlayer::AddControllerYawInput);
	_InputComponent->BindAxis("LookUp", this, &AMPFPSPlayer::AddControllerPitchInput);
	_InputComponent->BindAction("Jump", IE_Pressed, this, &AMPFPSPlayer::StartJump);
	_InputComponent->BindAction("Jump", IE_Released, this, &AMPFPSPlayer::EndJump);
	_InputComponent->BindAction("Fire", IE_Pressed, this, &AMPFPSPlayer::Fire);
	_InputComponent->BindAction("NextWeapon", IE_Pressed, this, &AMPFPSPlayer::SwitchWeapon);
	_InputComponent->BindAction("PrevWeapon", IE_Pressed, this, &AMPFPSPlayer::SwitchWeapon);
	_InputComponent->BindAction("Crouch", IE_Pressed, this, &AMPFPSPlayer::StartCrouch);
	_InputComponent->BindAction("Crouch", IE_Released, this, &AMPFPSPlayer::EndCrouch);
	_InputComponent->BindAction("Zoom", IE_Pressed, this, &AMPFPSPlayer::CameraZoomIn);
	_InputComponent->BindAction("Zoom", IE_Released, this, &AMPFPSPlayer::CameraZoomOut);
	_InputComponent->BindAction("Sprint", IE_Pressed, this, &AMPFPSPlayer::SprintStart);
	_InputComponent->BindAction("Sprint", IE_Released, this, &AMPFPSPlayer::SprintStop);
}

float AMPFPSPlayer::TakeDamage(float Damage, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	ApplyDamage(Damage);
	return Health;
}

void AMPFPSPlayer::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMPFPSPlayer, EquippedWeapon);
	DOREPLIFETIME(AMPFPSPlayer, CameraPitch);
	DOREPLIFETIME(AMPFPSPlayer, Health);
}

void AMPFPSPlayer::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.0f && Health > 0)
	{
		if (Value < 0.f)
			SprintStop();
		FRotator Rotation = Controller->GetControlRotation();
		
		if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
		{
			Rotation.Pitch = 0.0f;
		}
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMPFPSPlayer::StrafeRight(float Value)
{
	if (Controller != nullptr && Value != 0.0f && Health > 0)
	{
		SprintStop();
		FRotator Rotation = Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AMPFPSPlayer::ApplyDamage_Implementation(float Damage)
{
	if (Health > 0)
		Health -= Damage;
}

bool AMPFPSPlayer::ApplyDamage_Validate(float Damage)
{
	return true;
}

void AMPFPSPlayer::SwitchWeapon_Implementation()
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

bool AMPFPSPlayer::SwitchWeapon_Validate()
{
	return true;
}

void AMPFPSPlayer::SetupWeaponMesh_Implementation(AMPFPSWeapon* Weapon)
{
	if (!Weapon || !Weapon->WeaponMesh || !GetMesh())
		return;

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

void AMPFPSPlayer::CameraZoomIn()
{
	FirstPersonCameraComponent->SetFieldOfView(60.f);
}

void AMPFPSPlayer::CameraZoomOut()
{
	FirstPersonCameraComponent->SetFieldOfView(90.f);
}

void AMPFPSPlayer::SprintStart()
{
	if (FVector::DotProduct(GetVelocity(), GetActorForwardVector()) > 0.5f)
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultMovementSpeed * 2;
	}
}

void AMPFPSPlayer::SprintStop()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultMovementSpeed;
}

void AMPFPSPlayer::Fire()
{
	if (!EquippedWeapon || Health <= 0)
		return;
	
	FVector CameraLocation;
	FRotator CameraRotation;
	GetActorEyesViewPoint(CameraLocation, CameraRotation);
	if (EquippedWeapon->CanShoot() && GetCharacterMovement()->MaxWalkSpeed <= DefaultMovementSpeed)
	{
		EquippedWeapon->Shoot(CameraRotation.Vector());
		AddControllerPitchInput(-(EquippedWeapon->WeaponConfig.Recoil));
		if (EquippedWeapon->WeaponConfig.ShootingAnim)
		{
			GetMesh()->GetAnimInstance()->Montage_Play(EquippedWeapon->WeaponConfig.ShootingAnim);
		}
	}
}

void AMPFPSPlayer::EquipWeapon(AMPFPSWeapon* Weapon)
{
	if (Weapon == nullptr)
		return;
	//TODO: REALLY don't like this stuff. Mustn't be implemented this way.
	SetupWeaponMesh(Weapon);
	UnequipWeapon();
	EquippedWeapon = Weapon;
}

void AMPFPSPlayer::UnequipWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon = nullptr;
	}
}
