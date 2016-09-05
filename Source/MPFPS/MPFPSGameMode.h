// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "MPFPSGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MPFPS_API AMPFPSGameMode : public AGameMode
{
	GENERATED_BODY()

	AMPFPSGameMode(const FObjectInitializer& ObjectInitializer);
	void StartPlay() override;
	
};
