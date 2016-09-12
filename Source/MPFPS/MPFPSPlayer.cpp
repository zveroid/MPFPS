// Fill out your copyright notice in the Description page of Project Settings.

#include "MPFPS.h"
#include "Net/UnrealNetwork.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "MPFPSWeapon.h"
#include "MPFPSHUD.h"
#include "MPFPSPlayerController.h"
#include "MPFPSPlayer.h"


// Sets default values
AMPFPSPlayer::AMPFPSPlayer() : 
	EquippedWeapon(nullptr)
	, Health(0)
	, CameraPitch(0)
	, BloodEmitterTemplate(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
}

AMPFPSPlayer::AMPFPSPlayer(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
	, EquippedWeapon(nullptr)
	, Health(0)
	, BloodEmitterTemplate(nullptr)
{
	FirstPersonCameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh(), TEXT("FPCameraSocket"));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> BloodPS
		(TEXT("ParticleSystem'/Game/Assets/VFX/P_body_bullet_impact'"));
	BloodEmitterTemplate = BloodPS.Object;
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
	AMPFPSPlayerController* controller = Cast<AMPFPSPlayerController>(GetController());
	if (controller)
	{
		if (!Inventory.MainWeapon)
		{
			Inventory.MainWeapon = GetWorld()->SpawnActor<AMPFPSWeapon>(controller->MainWeaponClass,
				SpawnParams);
		}
		if (!Inventory.SecondaryWeapon)
		{
			Inventory.SecondaryWeapon = GetWorld()->SpawnActor<AMPFPSWeapon>(
				controller->SecondaryWeaponClass,
				SpawnParams);
		}
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

		SwitchWeapon();
		SetManequinColor(controller->CharacterColor);
		OnRespawn();
	}
}

void AMPFPSPlayer::OnDie_Implementation()
{
	AMPFPSPlayerController* controller = Cast<AMPFPSPlayerController>(GetController());
	if (controller != nullptr)
	{
		DisableInput(controller);
		if (controller->GetHUD())
		{
			Cast<AMPFPSHUD>(controller->GetHUD())->SwitchToLeaderboard();
		}
	}
}

void AMPFPSPlayer::OnRespawn_Implementation()
{
	AMPFPSPlayerController* controller = Cast<AMPFPSPlayerController>(GetController());
	if (controller && controller->GetHUD())
	{
		Cast<AMPFPSHUD>(controller->GetHUD())->SwitchToHud();
		EnableInput(controller);
	}
}

// Called every frame
void AMPFPSPlayer::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// TODO: Some dragging can be noted on client. What's a deal  with Client Prediction in UE4?
	float pitch = FMath::ClampAngle(FMath::RInterpTo(FRotator(CameraPitch, 0, 0), GetControlRotation(), DeltaTime, 15).Pitch, -90, 90);
	SetPitch(pitch);

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
	_InputComponent->BindAction("Fire", IE_Pressed, this, &AMPFPSPlayer::FireStart);
	_InputComponent->BindAction("Fire", IE_Released, this, &AMPFPSPlayer::FireEnd);
	_InputComponent->BindAction("NextWeapon", IE_Pressed, this, &AMPFPSPlayer::SwitchWeapon);
	_InputComponent->BindAction("PrevWeapon", IE_Pressed, this, &AMPFPSPlayer::SwitchWeapon);
	_InputComponent->BindAction("Crouch", IE_Pressed, this, &AMPFPSPlayer::StartCrouch);
	_InputComponent->BindAction("Crouch", IE_Released, this, &AMPFPSPlayer::EndCrouch);
	_InputComponent->BindAction("Zoom", IE_Pressed, this, &AMPFPSPlayer::CameraZoomIn);
	_InputComponent->BindAction("Zoom", IE_Released, this, &AMPFPSPlayer::CameraZoomOut);
	_InputComponent->BindAction("Sprint", IE_Pressed, this, &AMPFPSPlayer::SprintStart);
	_InputComponent->BindAction("Sprint", IE_Released, this, &AMPFPSPlayer::SprintStop);
	_InputComponent->BindAction("ShowScore", IE_Pressed, this, &AMPFPSPlayer::ShowScore);
	_InputComponent->BindAction("ShowScore", IE_Released, this, &AMPFPSPlayer::HideScore);
}

float AMPFPSPlayer::TakeDamage(float Damage, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	if (Health > 0)
	{
		if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
		{
			FPointDamageEvent* PointDamage = (FPointDamageEvent*)&DamageEvent;
			SpawnBloodEffect(PointDamage->HitInfo.ImpactPoint, PointDamage->HitInfo.ImpactNormal.Rotation());
		}
		Health -= Damage;
		if (Health <= 0)
		{
			AMPFPSPlayerController* controller = Cast<AMPFPSPlayerController>(GetController());
			if (controller != nullptr)
			{
				UnequipWeapon();
				OnDie();
				controller->StartRespawnTimer();
			}
			if (EventInstigator && EventInstigator->PlayerState)
				EventInstigator->PlayerState->Score++;
		}
	}
	return Health;
}

void AMPFPSPlayer::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMPFPSPlayer, EquippedWeapon);
	DOREPLIFETIME(AMPFPSPlayer, CameraPitch);
	DOREPLIFETIME(AMPFPSPlayer, Health);
	DOREPLIFETIME(AMPFPSPlayer, IsSprinting);
	DOREPLIFETIME(AMPFPSPlayer, IsCrouching);
}

void AMPFPSPlayer::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.0f)
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
	if (Controller != nullptr && Value != 0.0f)
	{
		SprintStop();
		FRotator Rotation = Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
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
	SetSprinting(true);
}

void AMPFPSPlayer::SprintStop()
{
	SetSprinting(false);
}

void AMPFPSPlayer::SetSprinting_Implementation(bool Val)
{
	if (Val)
	{
		if (FVector::DotProduct(GetVelocity(), GetActorForwardVector()) > 0.5f)
		{
			GetCharacterMovement()->MaxWalkSpeed = DefaultMovementSpeed * 2;
		}
	}
	else
		GetCharacterMovement()->MaxWalkSpeed = DefaultMovementSpeed;

	IsSprinting = Val;
}

void AMPFPSPlayer::FireStart()
{
	if (!EquippedWeapon || Health <= 0)
		return;
	
	FVector CameraLocation;
	FRotator CameraRotation;
	FVector MuzzleLocation;
	GetActorEyesViewPoint(CameraLocation, CameraRotation);
	MuzzleLocation = CameraLocation + FTransform(CameraRotation).TransformVector(FVector(80.0f, 0.f, -5.f));
	if (EquippedWeapon->CanShoot() && GetCharacterMovement()->MaxWalkSpeed <= DefaultMovementSpeed)
	{
		EquippedWeapon->StartShooting();
		/*AddControllerPitchInput(-(EquippedWeapon->WeaponConfig.Recoil));
		if (EquippedWeapon->WeaponConfig.ShootingAnim)
		{
			GetMesh()->GetAnimInstance()->Montage_Play(EquippedWeapon->WeaponConfig.ShootingAnim);
		}*/
	}
}

void AMPFPSPlayer::FireEnd()
{
	if (!EquippedWeapon)
		return;

	EquippedWeapon->StopShooting();
}

void AMPFPSPlayer::ShowScore()
{
	AMPFPSPlayerController* controller = Cast<AMPFPSPlayerController>(GetController());
	if (controller != nullptr && controller->GetHUD())
	{
		Cast<AMPFPSHUD>(controller->GetHUD())->SwitchToLeaderboard();
	}
}

void AMPFPSPlayer::HideScore()
{
	AMPFPSPlayerController* controller = Cast<AMPFPSPlayerController>(GetController());
	if (controller != nullptr && controller->GetHUD())
	{
		Cast<AMPFPSHUD>(controller->GetHUD())->SwitchToHud();
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

void AMPFPSPlayer::SetManequinColor_Implementation(const FLinearColor& color)
{
	if (GetMesh())
	{
		for (auto i = 0; i < GetMesh()->GetNumMaterials(); ++i)
		{
			UMaterialInstanceDynamic* MatInstance = GetMesh()->CreateAndSetMaterialInstanceDynamic(i);
			if (MatInstance)
			{
				MatInstance->SetVectorParameterValue(TEXT("BaseColor"), color);
			}
		}
	}
}

void AMPFPSPlayer::SpawnBloodEffect_Implementation(const FVector& Position, const FRotator& Rotation)
{
	UParticleSystemComponent*	BloodComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodEmitterTemplate, FTransform(Rotation, Position));
	if (BloodComponent)
	{
		BloodComponent->ActivateSystem();
	}
}