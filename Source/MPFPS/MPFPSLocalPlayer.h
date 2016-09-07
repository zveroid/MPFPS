// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "MPFPSLocalPlayer.generated.h"

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
class MPFPS_API AMPFPSLocalPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMPFPSLocalPlayer();
	AMPFPSLocalPlayer(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void SetPlayerDefaults() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* _InputComponent) override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	//UFUNCTION(Reliable, Server, WithValidation)
	UFUNCTION()
	void MoveForward(float Value);

	/** Networking functions **/
	UFUNCTION(Reliable, Server, WithValidation)
	void ApplyDamage(float Damage);
	void ApplyDamage_Implementation(float Damage);
	bool ApplyDamage_Validate(float Damage);

	UFUNCTION(Reliable, Server, WithValidation )
	void SetIsCrouched(bool Val);
	void SetIsCrouched_Implementation(bool Val) { bIsCrouched = Val; }
	bool SetIsCrouched_Validate(bool Val) { return true; }

	UFUNCTION(Unreliable, Server, WithValidation)
	void SetPitch(float Pitch);
	void SetPitch_Implementation(float Pitch) { CameraPitch = Pitch; }
	bool SetPitch_Validate(float Pitch) { return true; }

	UFUNCTION(Reliable, Server, WithValidation)
	void SwitchWeapon();
	void SwitchWeapon_Implementation();
	bool SwitchWeapon_Validate();

	UFUNCTION(Reliable, NetMulticast)
	void SetupWeaponMesh(AMPFPSWeapon* Weapon);
	void SetupWeaponMesh_Implementation(AMPFPSWeapon* Weapon);

	UFUNCTION()
	void StrafeRight(float Value);

	UFUNCTION()
	void StartJump() { bPressedJump = true; }

	UFUNCTION()
	void EndJump() { bPressedJump = false; }

	UFUNCTION()
	void StartCrouch() { SetIsCrouched(true); }

	UFUNCTION()
	void EndCrouch() { SetIsCrouched(false); }

	UFUNCTION()
	void CameraZoomIn();

	UFUNCTION()
	void CameraZoomOut();

	UFUNCTION()
	void SprintStart();

	UFUNCTION()
	void SprintStop();

	UFUNCTION()
	void Fire();

	const AMPFPSWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	/* FPS Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	TSubclassOf<class AMPFPSWeapon>	MainWeaponClass;
	UPROPERTY(VisibleAnywhere, Category = Weapon)
	TSubclassOf<class AMPFPSWeapon>	SecondaryWeaponClass;

	UPROPERTY(VisibleAnywhere, Category = Inventory)
	FPlayerInventory	Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	float CameraPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int		Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float	DefaultMovementSpeed;

protected:
	UPROPERTY(VisibleAnywhere, Category = Weapon, replicated )
	AMPFPSWeapon*		EquippedWeapon;

private:
	void EquipWeapon(AMPFPSWeapon* Weapon);
	void UnequipWeapon();

	float	RespawnTimer;
};
