// Fill out your copyright notice in the Description page of Project Settings.


#include "AmaruPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.h"
#include "AmaruAbilitySystemComponent.h"
#include "InkaDataAsset.h"
#include "Net/UnrealNetwork.h"

AAmaruPlayerState::AAmaruPlayerState()
{
	SetNetUpdateFrequency(100.0f);
	SetMinNetUpdateFrequency(66.0f);
	bReplicates = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAmaruAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<UAmaruAttributeSet>(TEXT("AttributeSet"));
}

void AAmaruPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AAmaruPlayerState* PS = Cast<AAmaruPlayerState>(PlayerState))
	{
		PS->SelectedInka = SelectedInka;
	}
}

UAbilitySystemComponent* AAmaruPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAmaruAbilitySystemComponent* AAmaruPlayerState::GetAmaruAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAmaruPlayerState::OnRep_SelectedInka()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("OnRep_SelectedInka called!"));
	UE_LOG(LogTemp, Warning, TEXT("[%s] SelectedInka = %s"),
		HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
		*GetNameSafe(SelectedInka.Get()));
	OnInkaChanged.Broadcast(GetPlayerId());
}

void AAmaruPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAmaruPlayerState, SelectedInka);
}

void AAmaruPlayerState::SetSelectedInka(const TSoftObjectPtr<UInkaDataAsset>& NewInka)
{
	if (!HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SetSelectedInka called on client!"));
		return;
	}
	if (SelectedInka != NewInka)
	{
		SelectedInka = NewInka;
		OnRep_SelectedInka();
	}
}
