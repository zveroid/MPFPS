// Fill out your copyright notice in the Description page of Project Settings.

#include "MPFPS.h"
#include "MPFPSHUD.h"
#include "MPFPSPlayerController.h"
#include "MPFPSPlayer.h"
#include "MPFPSGameMode.h"


AMPFPSGameMode::AMPFPSGameMode(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnObject(TEXT("Pawn'/Game/Blueprints/BP_Player.BP_Player_C'"));
	if (PlayerPawnObject.Succeeded())
	{
		DefaultPawnClass = PlayerPawnObject.Class;
	}

	PlayerControllerClass = AMPFPSPlayerController::StaticClass();
	HUDClass = AMPFPSHUD::StaticClass();

	MinRespawnDelay = 5.f;
	bStartPlayersAsSpectators = true;
}

void AMPFPSGameMode::StartPlay()
{
	Super::StartPlay();
	StartMatch();
}