// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class AMARUSHOOTER_API UAmaruAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UAmaruAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAmaruAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAmaruAttributeSet, MaxHealth)
		
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Shield", ReplicatedUsing = OnRep_Shield)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UAmaruAttributeSet, Shield)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Shield", ReplicatedUsing = OnRep_MaxShield)
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UAmaruAttributeSet, MaxShield)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Movement", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UAmaruAttributeSet, MoveSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Damage", ReplicatedUsing = OnRep_DamageMultiplier)
	FGameplayAttributeData DamageMultiplier;
	ATTRIBUTE_ACCESSORS(UAmaruAttributeSet, DamageMultiplier)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Cost", ReplicatedUsing = OnRep_ChargeAbility1)
	FGameplayAttributeData ChargeAbility1;
	ATTRIBUTE_ACCESSORS(UAmaruAttributeSet, ChargeAbility1)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Cost", ReplicatedUsing = OnRep_MaxChargeAbility1)
	FGameplayAttributeData MaxChargeAbility1;
	ATTRIBUTE_ACCESSORS(UAmaruAttributeSet, MaxChargeAbility1)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Cost", ReplicatedUsing = OnRep_ChargeAbility2)
	FGameplayAttributeData ChargeAbility2;
	ATTRIBUTE_ACCESSORS(UAmaruAttributeSet, ChargeAbility2)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Cost", ReplicatedUsing = OnRep_MaxChargeAbility2)
	FGameplayAttributeData MaxChargeAbility2;
	ATTRIBUTE_ACCESSORS(UAmaruAttributeSet, MaxChargeAbility2)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Weapon", ReplicatedUsing = OnRep_Ammo)
	FGameplayAttributeData Ammo;
	ATTRIBUTE_ACCESSORS(UAmaruAttributeSet, Ammo)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Weapon", ReplicatedUsing = OnRep_MaxAmmo)
	FGameplayAttributeData MaxAmmo;
	ATTRIBUTE_ACCESSORS(UAmaruAttributeSet, MaxAmmo)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DamageMultiplier(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ChargeAbility1(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxChargeAbility1(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ChargeAbility2(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxChargeAbility2(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Ammo(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxAmmo(const FGameplayAttributeData& OldValue);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
