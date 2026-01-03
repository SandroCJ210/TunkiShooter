// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AmaruPlayerState.generated.h"

/**
 * 
 */

class UAmaruAttributeSet;
class UAmaruAbilitySystemComponent;
class UAttributeSet;

UCLASS()
class AMARUSHOOTER_API AAmaruPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAmaruPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UAmaruAbilitySystemComponent* GetAmaruAbilitySystemComponent() const;

	UAmaruAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAmaruAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAmaruAttributeSet> AttributeSet;
};
