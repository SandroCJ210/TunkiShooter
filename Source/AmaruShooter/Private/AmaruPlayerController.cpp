// Fill out your copyright notice in the Description page of Project Settings.


#include "AmaruPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "AmaruShooter/AmaruPlayerState.h"

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
    UE_LOG(LogTemp, Warning, TEXT("PC %s Local=%d DefaultMappingContext=%s"),
        *GetName(), IsLocalPlayerController(), *GetNameSafe(DefaultMappingContext));
    CreateHUDInka();

}

void AAmaruPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
    if (!IsLocalPlayerController()) return;
	CreateHUDInka();
}

void AAmaruPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
    CreateHUDInka();
    if (!IsLocalPlayerController() || !HUDInka) return;

    if (AAmaruPlayerState* APS = GetPlayerState<AAmaruPlayerState>())
    {
        if (UAbilitySystemComponent* ASC = APS->GetAbilitySystemComponent())
        {
            struct FParam { UAbilitySystemComponent* ASC; };
			FParam Params{ ASC };
            CallBPFunction(HUDInka, FName("SetupAbilitySystemComponent"), &Params);
        }
    }
}
