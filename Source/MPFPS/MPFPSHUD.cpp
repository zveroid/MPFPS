// Fill out your copyright notice in the Description page of Project Settings.

#include "MPFPS.h"

#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

#include "MPFPSPlayer.h"
#include "MPFPSWeapon.h"
#include "MPFPSHUD.h"

AMPFPSHUD::AMPFPSHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, HudWidgetClass(nullptr)
	, InventoryWidgetClass(nullptr)
	, LeaderboardWidgetClass(nullptr)
	, CurrentWidget(nullptr)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> HudWidgetObj(TEXT("/Game/Blueprints/Widgets/BP_HUD"));
	static ConstructorHelpers::FClassFinder<UUserWidget> InventoryWidgetObj(TEXT("/Game/Blueprints/Widgets/BP_Inventory"));
	static ConstructorHelpers::FClassFinder<UUserWidget> LeaderboardWidgetObj(TEXT("/Game/Blueprints/Widgets/BP_Leaderboard"));
	HudWidgetClass = HudWidgetObj.Class;
	InventoryWidgetClass = InventoryWidgetObj.Class;
	LeaderboardWidgetClass = LeaderboardWidgetObj.Class;

	/*this->bReplicates = false;
	this->bNetLoadOnClient = true;
	this->bOnlyRelevantToOwner = true;*/
}

void AMPFPSHUD::BeginPlay()
{
	Super::BeginPlay();
}

void AMPFPSHUD::DrawHUD()
{
	Super::DrawHUD();
}

void AMPFPSHUD::SwitchToHud()
{
	if (CurrentWidget)
	{
		if (CurrentWidget->IsA(HudWidgetClass))
			return;

		CurrentWidget->RemoveFromParent();
	}

	CurrentWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), HudWidgetClass);
	CurrentWidget->AddToViewport();
	if (PlayerOwner)
	{
		PlayerOwner->SetInputMode(FInputModeGameOnly());
		PlayerOwner->bShowMouseCursor = false;
	}
}

void AMPFPSHUD::SwitchToInventory()
{
	if (CurrentWidget)
	{
		if (CurrentWidget->IsA(InventoryWidgetClass))
			return;

		CurrentWidget->RemoveFromParent();
	}

	CurrentWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), InventoryWidgetClass);
	CurrentWidget->AddToViewport();
	if (PlayerOwner)
	{
		PlayerOwner->SetInputMode(FInputModeUIOnly());
		PlayerOwner->bShowMouseCursor = true;
	}
}

void AMPFPSHUD::SwitchToLeaderboard()
{
	if (CurrentWidget)
	{
		if (CurrentWidget->IsA(LeaderboardWidgetClass))
			return;

		CurrentWidget->RemoveFromParent();
	}

	CurrentWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), LeaderboardWidgetClass);
	CurrentWidget->AddToViewport();
}