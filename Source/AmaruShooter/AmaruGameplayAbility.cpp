// Fill out your copyright notice in the Description page of Project Settings.


#include "AmaruGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AmaruGameplayTags.h"

UAmaruGameplayAbility::UAmaruGameplayAbility()
{
	ActivationBlockedTags.AddTag(AmaruTags::State_Dead);
}

float UAmaruGameplayAbility::GetCooldownDuration() const
{
	float Duration = CooldownDuration;
	if (bScaleCooldownWithAttackSpeed)
	{
		if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			if (ASC->HasMatchingGameplayTag(AmaruTags::State_Rage) ||
				ASC->HasMatchingGameplayTag(AmaruTags::State_Haste))
			{
				Duration *= AttackSpeedCooldownMultiplier;
			}
		}
	}
	return Duration;
}

const FGameplayTagContainer* UAmaruGameplayAbility::GetCooldownTags() const
{
	if (CooldownTags.IsEmpty())
	{
		return Super::GetCooldownTags();
	}

	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&TempCooldownTags);
	MutableTags->Reset();
	if (const FGameplayTagContainer* ParentTags = Super::GetCooldownTags())
	{
		MutableTags->AppendTags(*ParentTags);
	}
	MutableTags->AppendTags(CooldownTags);
	return MutableTags;
}

void UAmaruGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (!CooldownGE)
	{
		return;
	}

	if (CooldownTags.IsEmpty() || GetCooldownDuration() <= 0.f)
	{
		Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->DynamicGrantedTags.AppendTags(CooldownTags);
		SpecHandle.Data->SetSetByCallerMagnitude(AmaruTags::Data_Cooldown, GetCooldownDuration());
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}
