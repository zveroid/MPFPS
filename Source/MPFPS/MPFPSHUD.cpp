// Fill out your copyright notice in the Description page of Project Settings.

#include "MPFPS.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "MPFPSLocalPlayer.h"
#include "MPFPSWeapon.h"
#include "MPFPSHUD.h"

AMPFPSHUD::AMPFPSHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, HudWidgetClass(nullptr)
	, HudWidget(nullptr)
	, HealthTextBlock(nullptr)
	, AmmoTextBlock(nullptr)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> HudWidgetObj(TEXT("/Game/Blueprints/Widgets/BP_HUD"));
	check(HudWidgetObj.Succeeded());
	HudWidgetClass = HudWidgetObj.Class;
}

void AMPFPSHUD::BeginPlay()
{
	check(HudWidgetClass);
	if (HudWidgetClass)
	{
		HudWidget = CreateWidget<UUserWidget>(this->GetOwningPlayerController(), this->HudWidgetClass);
		HudWidget->AddToViewport();

		HealthTextBlock = Cast<UTextBlock>(HudWidget->GetWidgetFromName(TEXT("HealthText")));
		AmmoTextBlock = Cast<UTextBlock>(HudWidget->GetWidgetFromName(TEXT("AmmoText")));
	}
}

void AMPFPSHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!HudWidget)
		return;

	AMPFPSLocalPlayer* OwnPlayer = Cast<AMPFPSLocalPlayer>(GetOwningPawn());

	check(HealthTextBlock != nullptr && AmmoTextBlock != nullptr);
	
	if (OwnPlayer)
	{
		check(OwnPlayer->GetEquippedWeapon() != nullptr);
		HealthTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%d"), OwnPlayer->Health)));
		AmmoTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%u"), OwnPlayer->GetEquippedWeapon()->GetAmmo())));
	}
}