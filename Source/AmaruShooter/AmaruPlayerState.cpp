// Fill out your copyright notice in the Description page of Project Settings.


#include "AmaruPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.h"

AAmaruPlayerState::AAmaruPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAmaruAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AAmaruPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}