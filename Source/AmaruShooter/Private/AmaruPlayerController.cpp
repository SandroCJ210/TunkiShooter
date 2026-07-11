// Fill out your copyright notice in the Description page of Project Settings.


#include "AmaruPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "UI/AmaruHUDWidget.h"

void AAmaruPlayerController::CreateHUDInka()
{
	if (HUDInka) return;
    if (IsLocalPlayerController())
    {
        if (HUDInkaClass)
        {
            HUDInka = CreateWidget(this, HUDInkaClass);
            if (HUDInka)
            {
                HUDInka->AddToViewport();
            }
        }
    }
}

void AAmaruPlayerController::Server_SetSelectedInka_Implementation(const TSoftObjectPtr<UInkaDataAsset>& NewInka)
{
	if (AAmaruPlayerState* APS = GetPlayerState<AAmaruPlayerState>())
	{
		APS->SetSelectedInka(NewInka);
	}
}

void AAmaruPlayerController::TryBindHUDToASC()
{
    if (!IsLocalPlayerController()) return;
    if (!HUDInka) CreateHUDInka();
    if (!HUDInka) return;

    // Los HUD basados en UAmaruHUDWidget se vinculan solos al ASC en C++.
    if (HUDInka->IsA<UAmaruHUDWidget>()) return;

    AAmaruPlayerState* APS = GetPlayerState<AAmaruPlayerState>();
    if (!APS) return;

    UAbilitySystemComponent* ASC = APS->GetAbilitySystemComponent();
    if (!ASC) return;

    struct FParam { UAbilitySystemComponent* ASC; };
    FParam Params{ ASC };
    CallBPFunction(HUDInka, FName("SetupAbilitySystemComponent"), &Params);
}

void AAmaruPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalPlayerController()) return;

    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
        {
            Subsystem->ClearAllMappings();
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
    CreateHUDInka();
    TryBindHUDToASC();
}

void AAmaruPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
    if (!IsLocalPlayerController()) return;
	CreateHUDInka();
	TryBindHUDToASC();
}

void AAmaruPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
    CreateHUDInka();
	TryBindHUDToASC();
}
