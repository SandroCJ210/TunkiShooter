// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AmaruPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInkaChanged, int32, PlayerIndex);

/**
 * 
 */

class UAmaruAttributeSet;
class UAmaruAbilitySystemComponent;
class UAttributeSet;
class UInkaDataAsset;

UCLASS()
class AMARUSHOOTER_API AAmaruPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAmaruPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UAmaruAbilitySystemComponent* GetAmaruAbilitySystemComponent() const;
	
	UFUNCTION(BlueprintCallable, Category = "GAS")
	UAmaruAttributeSet* GetAttributeSet() const { return AttributeSet;}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAmaruAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAmaruAttributeSet> AttributeSet;

	UFUNCTION()
	void OnRep_SelectedInka();

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_SelectedInka, BlueprintReadOnly, Category = "Inka")
	TSoftObjectPtr<UInkaDataAsset> SelectedInka;

	UFUNCTION(BlueprintCallable, Category = "Inka")
	void SetSelectedInka(const TSoftObjectPtr<UInkaDataAsset>& NewInka);

	FOnInkaChanged OnInkaChanged;
};
