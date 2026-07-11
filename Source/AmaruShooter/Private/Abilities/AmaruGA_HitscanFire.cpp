#include "Abilities/AmaruGA_HitscanFire.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.h"
#include "AmaruGameplayStatics.h"
#include "AmaruGameplayTags.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "Effects/AmaruGameplayEffects.h"
#include "Objectives/AmaruDestructible.h"

UAmaruGA_HitscanFire::UAmaruGA_HitscanFire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	CostGameplayEffectClass = UAmaruGE_AmmoCost::StaticClass();
	CooldownGameplayEffectClass = UAmaruGE_Cooldown::StaticClass();
	DamageEffect = UAmaruGE_Damage::StaticClass();
	bScaleCooldownWithAttackSpeed = true;
	CooldownDuration = 0.5f;
}

void UAmaruGA_HitscanFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (HasAuthority(&ActivationInfo))
	{
		AAmaruPlayerState* PS = Cast<AAmaruPlayerState>(GetOwningActorFromActorInfo());
		if (PS)
		{
			PS->AddShotFired();
		}

		FVector ViewLoc = FVector::ZeroVector;
		FRotator ViewRot = FRotator::ZeroRotator;
		if (APlayerController* PC = ActorInfo->PlayerController.Get())
		{
			PC->GetPlayerViewPoint(ViewLoc, ViewRot);
		}
		else if (const AActor* Avatar = GetAvatarActorFromActorInfo())
		{
			ViewLoc = Avatar->GetActorLocation();
			ViewRot = Avatar->GetActorRotation();
		}

		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(AmaruHitscan), /*bTraceComplex=*/true, GetAvatarActorFromActorInfo());
		GetWorld()->LineTraceSingleByChannel(Hit, ViewLoc, ViewLoc + ViewRot.Vector() * Range, ECC_Visibility, Params);

		if (AActor* HitActor = Hit.GetActor())
		{
			float DamageMult = 1.f;
			if (PS)
			{
				if (const UAmaruAttributeSet* AS = PS->GetAttributeSet())
				{
					DamageMult = AS->GetDamageMultiplier() > 0.f ? AS->GetDamageMultiplier() : 1.f;
				}
			}
			const float Damage = BaseDamage * DamageMult;

			if (AAmaruDestructible* Destructible = Cast<AAmaruDestructible>(HitActor))
			{
				Destructible->ReceiveObjectDamage(Damage, GetAvatarActorFromActorInfo());
			}
			else if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
			{
				if (DamageEffect && !UAmaruGameplayStatics::AreSameTeam(GetOwningActorFromActorInfo(), HitActor))
				{
					if (PS)
					{
						PS->AddShotHit();
					}
					FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel());
					if (Spec.IsValid())
					{
						Spec.Data->GetContext().AddHitResult(Hit);
						Spec.Data->SetSetByCallerMagnitude(AmaruTags::Data_Damage, Damage);
						GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
					}
				}
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
