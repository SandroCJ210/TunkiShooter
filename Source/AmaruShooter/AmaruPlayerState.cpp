// Fill out your copyright notice in the Description page of Project Settings.


#include "AmaruPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.h"
#include "AmaruAbilitySystemComponent.h"

AAmaruPlayerState::AAmaruPlayerState()
{
	SetNetUpdateFrequency(100.0f);
	SetMinNetUpdateFrequency(66.0f);

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
