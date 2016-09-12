// Fill out your copyright notice in the Description page of Project Settings.

#include "MPFPS.h"
#include "MPFPSHUD.h"
#include "MPFPSPlayerController.h"


void AMPFPSPlayerController::BeginPlay()
{
	if (GetHUD())
		Cast<AMPFPSHUD>(GetHUD())->SwitchToInventory();
}

void AMPFPSPlayerController::RespawnAsPlayer_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
	PlayerState->bIsSpectator = false;
	ChangeState(NAME_Playing);
	GetWorld()->GetAuthGameMode()->RestartPlayer(this);
}

void AMPFPSPlayerController::StartRespawnTimer()
{
	if (!GetWorld()->GetAuthGameMode())
		return;
	GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AMPFPSPlayerController::RespawnAsPlayer, GetWorld()->GetAuthGameMode()->MinRespawnDelay);
}
