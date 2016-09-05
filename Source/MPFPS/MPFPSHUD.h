// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "MPFPSHUD.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class MPFPS_API AMPFPSHUD : public AHUD
{
	GENERATED_BODY()
	
	AMPFPSHUD(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	UPROPERTY()
	UTextBlock* HealthTextBlock;

	UPROPERTY()
	UTextBlock* AmmoTextBlock;

	class UClass * HudWidgetClass;
	class UUserWidget * HudWidget;
};
