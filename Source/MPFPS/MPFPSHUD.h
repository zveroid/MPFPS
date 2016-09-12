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

public:
	void SwitchToInventory();
	void SwitchToLeaderboard();
	void SwitchToHud();

	AMPFPSHUD(const FObjectInitializer& ObjectInitializer);

	void BeginPlay() override;
	void DrawHUD() override;

private:
	class UClass * HudWidgetClass;
	class UClass * InventoryWidgetClass;
	class UClass * LeaderboardWidgetClass;
	class UUserWidget * CurrentWidget;
};
