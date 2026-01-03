// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AmaruShooter/Enums.h"
#include "AmaruAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class AMARUSHOOTER_API UAmaruAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	void HandleAbilityLocalInputPressed(EAmaruAbilityInputID AbilityInputID);
	void HandleAbilityLocalInputReleased(EAmaruAbilityInputID AbilityInputID);
};
