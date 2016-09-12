// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "MPFPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MPFPS_API AMPFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	//There's bug in UE: while playing in the editor, host players are not spawned as spectators even when bStartPlayersAsSpectators set to True.
	//So we override CanRestartPlayer method, so it returns false if controller's state is NAME_Spectating
	//More details here:
	//https://answers.unrealengine.com/questions/228287/476-play-in-editor-ignore-bstartplayersasspectator.html
	bool CanRestartPlayer() override
	{
		return Super::CanRestartPlayer() && (GetStateName() != NAME_Spectating);
	}

	void BeginPlay() override;

	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category = PlayingState)
	void RespawnAsPlayer();
	void RespawnAsPlayer_Implementation();
	bool RespawnAsPlayer_Validate()	{ return true; }

	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category = Weapon)
	void SetMainWeaponClass(TSubclassOf<class AMPFPSWeapon>	Val);
	void SetMainWeaponClass_Implementation(TSubclassOf<class AMPFPSWeapon>	Val)
	{ 
		MainWeaponClass = Val; 
	}
	bool SetMainWeaponClass_Validate(TSubclassOf<class AMPFPSWeapon>	Val) { return true; }

	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category = Weapon)
	void SetSecondaryWeaponClass(TSubclassOf<class AMPFPSWeapon>	Val);
	void SetSecondaryWeaponClass_Implementation(TSubclassOf<class AMPFPSWeapon>	Val) 
	{ 
		SecondaryWeaponClass = Val; 
	}
	bool SetSecondaryWeaponClass_Validate(TSubclassOf<class AMPFPSWeapon>	Val) { return true; }

	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable, Category = Weapon)
	void SetCharacterColor(FLinearColor	Val);
	void SetCharacterColor_Implementation(FLinearColor	Val)
	{
		CharacterColor = Val;
	}
	bool SetCharacterColor_Validate(FLinearColor	Val) { return true; }

	void StartRespawnTimer();

	TSubclassOf<class AMPFPSWeapon>	MainWeaponClass;
	TSubclassOf<class AMPFPSWeapon>	SecondaryWeaponClass;
	FLinearColor	CharacterColor;

	FTimerHandle	RespawnTimerHandle;
};