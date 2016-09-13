// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/Animation/AnimMontage.h"
#include "MPFPSHUD.h"
#include "MPFPSPlayer.generated.h"

class AMPFPSWeapon;

USTRUCT()
struct FPlayerInventory
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, Category = Inventory)
	AMPFPSWeapon*	MainWeapon;

	UPROPERTY(VisibleAnywhere, Category = Inventory)
	AMPFPSWeapon*	SecondaryWeapon;
};

UCLASS()
class MPFPS_API AMPFPSPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMPFPSPlayer();
	AMPFPSPlayer(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void SetPlayerDefaults() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* _InputComponent) override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	void MoveForward(float Value);

	/** Networking functions **/
	UFUNCTION(Reliable, Server, WithValidation )
	void SetIsCrouching(bool Val);
	void SetIsCrouching_Implementation(bool Val) { IsCrouching = Val; }
	bool SetIsCrouching_Validate(bool Val) { return true; }

	UFUNCTION(Reliable, Server, WithValidation)
	void SetSprinting(bool Val);
	void SetSprinting_Implementation(bool Val);
	bool SetSprinting_Validate(bool Val) { return true; }

	UFUNCTION(Unreliable, Server, WithValidation)
	void SetPitch(float Pitch);
	void SetPitch_Implementation(float Pitch) { CameraPitch = Pitch; }
	bool SetPitch_Validate(float Pitch) { return true; }

	UFUNCTION(Reliable, Server, WithValidation)
	void SwitchWeapon();
	void SwitchWeapon_Implementation();
	bool SwitchWeapon_Validate();

	UFUNCTION(Reliable, Client)
	void OnDie();
	void OnDie_Implementation();

	UFUNCTION(Reliable, Client)
	void OnRespawn();
	void OnRespawn_Implementation();

	UFUNCTION(Reliable, NetMulticast)
	void SetupWeaponMesh(AMPFPSWeapon* Weapon);
	void SetupWeaponMesh_Implementation(AMPFPSWeapon* Weapon);

	UFUNCTION(Reliable, NetMulticast)
	void SetManequinColor(const FLinearColor& color);
	void SetManequinColor_Implementation(const FLinearColor& color);

	UFUNCTION(Reliable, NetMulticast)
	void SpawnBloodEffect(const FVector& Position, const FRotator& Rotation);
	void SpawnBloodEffect_Implementation(const FVector& Position, const FRotator& Rotation);

	void StrafeRight(float Value);

	void StartJump() { bPressedJump = true; }
	void EndJump() { bPressedJump = false; }

	void StartCrouch() { SetIsCrouching(true); }
	void EndCrouch() { SetIsCrouching(false); }

	void CameraZoomIn();
	void CameraZoomOut();

	void SprintStart();
	void SprintStop();

	void FireStart();
	void FireEnd();

	void ShowScore();
	void HideScore();

	const AMPFPSWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	void OnReloadMontageEnd(UAnimMontage* Montage, bool bInterrupted);

	/* FPS Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, Category = Inventory)
	FPlayerInventory	Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	float CameraPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int		Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool	IsSprinting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool	IsCrouching;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float	DefaultMovementSpeed;

	UPROPERTY(EditAnywhere)
	int32	MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, replicated )
	AMPFPSWeapon*		EquippedWeapon;

private:
	void EquipWeapon(AMPFPSWeapon* Weapon);
	void UnequipWeapon();

	UParticleSystem*	BloodEmitterTemplate;
};
