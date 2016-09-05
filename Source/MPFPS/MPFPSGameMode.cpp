// Fill out your copyright notice in the Description page of Project Settings.

#include "MPFPS.h"
#include "MPFPSHUD.h"
#include "MPFPSGameMode.h"


AMPFPSGameMode::AMPFPSGameMode(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnObject(TEXT("Pawn'/Game/Blueprints/BP_LocalPlayer.BP_LocalPlayer_C'"));
	if (PlayerPawnObject.Succeeded())
	{
		DefaultPawnClass = PlayerPawnObject.Class;
	}

	HUDClass = AMPFPSHUD::StaticClass();
}

void AMPFPSGameMode::StartPlay()
{
	Super::StartPlay();
	
	StartMatch();
}