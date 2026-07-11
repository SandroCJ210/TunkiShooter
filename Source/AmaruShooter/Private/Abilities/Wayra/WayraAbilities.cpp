#include "Abilities/Wayra/WayraAbilities.h"

#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.h"
#include "AmaruGameplayTags.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "Effects/AmaruGameplayEffects.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"

// ---------------- Orbe ----------------

AWayraOrb::AWayraOrb()
{
	Speed = 1000.f;
	MaxDistance = 8000.f;
	BaseDamage = 25.f;
	ExplosionRadius = 0.f;
}

void AWayraOrb::OnEnemyHit(UAbilitySystemComponent* TargetASC)
{
	// CDR de Sanación del Viento al acertar: retrasar el inicio del cooldown
	// activo hacia atrás acorta su tiempo restante.
	if (UAbilitySystemComponent* ASC = SourceASC.Get())
	{
		const FGameplayTagContainer CooldownTag(AmaruTags::Cooldown_Wayra_Ability1);
		const TArray<FActiveGameplayEffectHandle> Handles = ASC->GetActiveEffectsWithAllTags(CooldownTag);
		for (const FActiveGameplayEffectHandle& Handle : Handles)
		{
			ASC->ModifyActiveEffectStartTime(Handle, -HealCooldownReduction);
		}
	}
}

UAmaruGA_WayraPrimary::UAmaruGA_WayraPrimary()
{
	DebugName = TEXT("Wayra Primary");
	ProjectileClass = AWayraOrb::StaticClass();
	CooldownDuration = 0.6f;
	CooldownTags.AddTag(AmaruTags::Cooldown_Wayra_Primary);
}

// ---------------- Sanación del Viento ----------------

UAmaruGA_WayraHeal::UAmaruGA_WayraHeal()
{
	DebugName = TEXT("Sanación del Viento");
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	CooldownGameplayEffectClass = UAmaruGE_Cooldown::StaticClass();
	HealEffect = UAmaruGE_Heal::StaticClass();
	CooldownDuration = 8.f;
	CooldownTags.AddTag(AmaruTags::Cooldown_Wayra_Ability1);
}

void UAmaruGA_WayraHeal::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (HasAuthority(&ActivationInfo) && HealEffect)
	{
		const AActor* Avatar = GetAvatarActorFromActorInfo();
		AAmaruPlayerState* OwnPS = Cast<AAmaruPlayerState>(GetOwningActorFromActorInfo());
		UAbilitySystemComponent* OwnASC = GetAbilitySystemComponentFromActorInfo();

		if (Avatar && OwnPS && OwnASC)
		{
			// Aliado vivo con menor ratio de vida dentro del rango; fallback: ella misma.
			AAmaruPlayerState* BestTarget = OwnPS;
			float BestRatio = 1.f;
			if (const UAmaruAttributeSet* OwnAS = OwnPS->GetAttributeSet())
			{
				BestRatio = OwnAS->GetMaxHealth() > 0.f ? OwnAS->GetHealth() / OwnAS->GetMaxHealth() : 1.f;
			}

			if (const AGameStateBase* GS = GetWorld()->GetGameState())
			{
				for (APlayerState* PS : GS->PlayerArray)
				{
					AAmaruPlayerState* AllyPS = Cast<AAmaruPlayerState>(PS);
					if (!AllyPS || AllyPS == OwnPS) continue;
					if (AllyPS->GetTeamId() == INDEX_NONE || AllyPS->GetTeamId() != OwnPS->GetTeamId()) continue;

					const APawn* AllyPawn = AllyPS->GetPawn();
					const UAmaruAttributeSet* AllyAS = AllyPS->GetAttributeSet();
					if (!AllyPawn || !AllyAS || AllyAS->GetHealth() <= 0.f || AllyAS->GetMaxHealth() <= 0.f) continue;
					if (FVector::DistSquared(AllyPawn->GetActorLocation(), Avatar->GetActorLocation()) > FMath::Square(HealRange)) continue;

					const float Ratio = AllyAS->GetHealth() / AllyAS->GetMaxHealth();
					if (Ratio < BestRatio)
					{
						BestRatio = Ratio;
						BestTarget = AllyPS;
					}
				}
			}

			if (UAbilitySystemComponent* TargetASC = BestTarget->GetAbilitySystemComponent())
			{
				FGameplayEffectContextHandle Ctx = OwnASC->MakeEffectContext();
				Ctx.AddSourceObject(this);
				FGameplayEffectSpecHandle Spec = OwnASC->MakeOutgoingSpec(HealEffect, GetAbilityLevel(), Ctx);
				if (Spec.IsValid())
				{
					Spec.Data->SetSetByCallerMagnitude(AmaruTags::Data_Heal, HealAmount);
					OwnASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
				}
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// ---------------- Paso del Viento ----------------

UAmaruGA_WayraDash::UAmaruGA_WayraDash()
{
	DebugName = TEXT("Paso del Viento");
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	CooldownGameplayEffectClass = UAmaruGE_Cooldown::StaticClass();
	ProtectionEffect = UAmaruGE_Protected::StaticClass();
	CooldownDuration = 6.f;
	CooldownTags.AddTag(AmaruTags::Cooldown_Wayra_Ability2);
}

void UAmaruGA_WayraDash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		// Dirección del movimiento actual; si está quieta, hacia delante.
		FVector Direction = Character->GetVelocity();
		Direction.Z = 0.f;
		if (!Direction.Normalize())
		{
			Direction = Character->GetActorForwardVector();
		}
		Character->LaunchCharacter(Direction * DashImpulse, true, false);
	}

	if (ProtectionEffect)
	{
		FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(ProtectionEffect, GetAbilityLevel());
		if (Spec.IsValid())
		{
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// ---------------- Danza de los Apus ----------------

UAmaruGA_WayraUlt::UAmaruGA_WayraUlt()
{
	DebugName = TEXT("Danza de los Apus");
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	HealEffect = UAmaruGE_Heal::StaticClass();
	HasteEffect = UAmaruGE_Haste::StaticClass();
	BuffEffect = nullptr;
	bIsUltimate = true;
	CooldownDuration = 2.f;
	CooldownTags.AddTag(AmaruTags::Cooldown_Ultimate);
}

void UAmaruGA_WayraUlt::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (HasAuthority(&ActivationInfo))
	{
		const AAmaruPlayerState* OwnPS = Cast<AAmaruPlayerState>(GetOwningActorFromActorInfo());
		UAbilitySystemComponent* OwnASC = GetAbilitySystemComponentFromActorInfo();

		if (OwnPS && OwnASC)
		{
			if (const AGameStateBase* GS = GetWorld()->GetGameState())
			{
				for (APlayerState* PS : GS->PlayerArray)
				{
					const AAmaruPlayerState* AllyPS = Cast<AAmaruPlayerState>(PS);
					if (!AllyPS) continue;
					const bool bAlly = AllyPS == OwnPS ||
						(AllyPS->GetTeamId() != INDEX_NONE && AllyPS->GetTeamId() == OwnPS->GetTeamId());
					if (!bAlly) continue;

					const UAmaruAttributeSet* AllyAS = AllyPS->GetAttributeSet();
					if (!AllyAS || AllyAS->GetHealth() <= 0.f) continue;

					UAbilitySystemComponent* AllyASC = AllyPS->GetAbilitySystemComponent();
					if (!AllyASC) continue;

					FGameplayEffectContextHandle Ctx = OwnASC->MakeEffectContext();
					Ctx.AddSourceObject(this);

					if (HealEffect)
					{
						FGameplayEffectSpecHandle Spec = OwnASC->MakeOutgoingSpec(HealEffect, GetAbilityLevel(), Ctx);
						if (Spec.IsValid())
						{
							Spec.Data->SetSetByCallerMagnitude(AmaruTags::Data_Heal, HealAmount);
							OwnASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), AllyASC);
						}
					}
					if (HasteEffect)
					{
						FGameplayEffectSpecHandle Spec = OwnASC->MakeOutgoingSpec(HasteEffect, GetAbilityLevel(), Ctx);
						if (Spec.IsValid())
						{
							OwnASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), AllyASC);
						}
					}
				}
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
