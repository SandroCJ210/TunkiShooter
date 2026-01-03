// Fill out your copyright notice in the Description page of Project Settings.


#include "AmaruAbilitySystemComponent.h"

void UAmaruAbilitySystemComponent::HandleAbilityLocalInputPressed(EAmaruAbilityInputID AbilityInputID)
{
	AbilityLocalInputPressed(static_cast<uint8>(AbilityInputID));
}

void UAmaruAbilitySystemComponent::HandleAbilityLocalInputReleased(EAmaruAbilityInputID AbilityInputID)
{
	AbilityLocalInputReleased(static_cast<uint8>(AbilityInputID));
}
