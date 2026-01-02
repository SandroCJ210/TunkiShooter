// Fill out your copyright notice in the Description page of Project Settings.


#include "AmaruAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UAmaruAttributeSet::UAmaruAttributeSet()
{
}

void UAmaruAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, MaxShield, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, DamageMultiplier, COND_None, REPNOTIFY_Always);
}

void UAmaruAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxShield());
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	else if (Attribute == GetDamageMultiplierAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UAmaruAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	const FGameplayAttribute& Attr = Data.EvaluatedData.Attribute;

	if (Attr == GetHealthAttribute())
	{
		const float ClampedHealth = FMath::Clamp(GetHealth(), 0.f, GetMaxHealth());
		SetHealth(ClampedHealth);
	}
	else if (Attr == GetShieldAttribute())
	{
		const float ClampedShield = FMath::Clamp(GetShield(), 0.f, GetMaxShield());
		SetShield(ClampedShield);
	}
	else if (Attr == GetMoveSpeedAttribute())
	{
		SetMoveSpeed(FMath::Max(GetMoveSpeed(), 0.f));
	}
	else if (Attr == GetDamageMultiplierAttribute())
	{
		SetDamageMultiplier(FMath::Max(GetDamageMultiplier(), 0.f));
	}
}

void UAmaruAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, Health, OldValue);
}

void UAmaruAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, MaxHealth, OldValue);
}

void UAmaruAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, Shield, OldValue);
}

void UAmaruAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, MaxShield, OldValue);
}

void UAmaruAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, MoveSpeed, OldValue);
}

void UAmaruAttributeSet::OnRep_DamageMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, DamageMultiplier, OldValue);
}