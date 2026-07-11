#include "Abilities/AmaruGA_SelfBuff.h"

#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "Effects/AmaruGameplayEffects.h"

UAmaruGA_SelfBuff::UAmaruGA_SelfBuff()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	CooldownGameplayEffectClass = UAmaruGE_Cooldown::StaticClass();
}

void UAmaruGA_SelfBuff::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (BuffEffect)
	{
		FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(BuffEffect, GetAbilityLevel());
		if (Spec.IsValid())
		{
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UAmaruGA_SelfBuff::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!bIsUltimate)
	{
		return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
	}

	const AAmaruPlayerState* PS = ActorInfo ? Cast<AAmaruPlayerState>(ActorInfo->OwnerActor.Get()) : nullptr;
	const UAmaruAttributeSet* AS = PS ? PS->GetAttributeSet() : nullptr;
	return AS && AS->GetMaxUltimateCharge() > 0.f && AS->GetUltimateCharge() >= AS->GetMaxUltimateCharge();
}

void UAmaruGA_SelfBuff::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!bIsUltimate)
	{
		Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
		return;
	}

	if (ActorInfo && ActorInfo->IsNetAuthority())
	{
		if (const AAmaruPlayerState* PS = Cast<AAmaruPlayerState>(ActorInfo->OwnerActor.Get()))
		{
			if (UAmaruAttributeSet* AS = PS->GetAttributeSet())
			{
				AS->SetUltimateCharge(0.f);
			}
		}
	}
}
