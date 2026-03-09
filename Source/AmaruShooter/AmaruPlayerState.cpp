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
	const FString InkaPath = SelectedInka.ToSoftObjectPath().ToString();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("[%s] PS=%s PlayerId=%d SelectedInka = %s"), 
		HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"), 
		*GetName(), GetPlayerId(), *InkaPath));

	OnInkaChanged.Broadcast(GetPlayerId());
}

void AAmaruPlayerState::ServerSetSelectedInka_Implementation(const TSoftObjectPtr<UInkaDataAsset>& NewInka)
{
	const FString InkaPath = NewInka.ToSoftObjectPath().ToString();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("ServerSetSelectedInka: PS=%s PlayerId=%d NewInka=%s"),
		*GetName(), GetPlayerId(), *InkaPath));

	SetSelectedInka(NewInka);
}

void AAmaruPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAmaruPlayerState, SelectedInka);
}

void AAmaruPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AAmaruPlayerState* AmaruPlayerState = Cast<AAmaruPlayerState>(PlayerState))
	{
		AmaruPlayerState->SelectedInka = SelectedInka;
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, FString::Printf(TEXT("CopyProperties: Persisting Inka %s to new PlayerState"), *SelectedInka.ToSoftObjectPath().ToString()));
	}
}

void AAmaruPlayerState::SetSelectedInka(const TSoftObjectPtr<UInkaDataAsset>& NewInka)
{
	if (!HasAuthority())
	{
		ServerSetSelectedInka(NewInka);
		return;
	}

	SelectedInka = NewInka;
	OnRep_SelectedInka();
}
