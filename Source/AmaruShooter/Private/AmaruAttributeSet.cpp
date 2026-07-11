// Fill out your copyright notice in the Description page of Project Settings.


#include "AmaruAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AmaruGameplayTags.h"
#include "AmaruGameplayStatics.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "GameModes/AmaruGameMode.h"

FOnAmaruDamageDealt UAmaruAttributeSet::OnDamageDealt;

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

	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, ChargeAbility1, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, MaxChargeAbility1, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, ChargeAbility2, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, MaxChargeAbility2, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, Ammo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, MaxAmmo, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, UltimateCharge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmaruAttributeSet, MaxUltimateCharge, COND_None, REPNOTIFY_Always);
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
	else if (Attribute == GetChargeAbility1Attribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxChargeAbility1());
	}
	else if (Attribute == GetChargeAbility2Attribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxChargeAbility2());
	}
	else if (Attribute == GetAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxAmmo());
	}
	else if (Attribute == GetMaxAmmoAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
		SetAmmo(FMath::Clamp(GetAmmo(), 0.f, NewValue));
	}
	else if (Attribute == GetMaxShieldAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
		SetShield(FMath::Clamp(GetShield(), 0.f, NewValue));
	}
	else if (Attribute == GetUltimateChargeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxUltimateCharge());
	}
	else if (Attribute == GetMaxUltimateChargeAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UAmaruAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	const FGameplayAttribute& Attr = Data.EvaluatedData.Attribute;

	if (Attr == GetIncomingDamageAttribute())
	{
		float Damage = GetIncomingDamage();
		SetIncomingDamage(0.f);
		if (Damage <= 0.f)
		{
			return;
		}

		AAmaruPlayerState* VictimPS = Cast<AAmaruPlayerState>(GetOwningActor());
		AAmaruPlayerState* SourcePS = UAmaruGameplayStatics::GetAmaruPlayerStateFromActor(
			Data.EffectSpec.GetContext().GetOriginalInstigator());

		// Sin friendly fire: mismo equipo y jugadores distintos.
		if (SourcePS && VictimPS && SourcePS != VictimPS &&
			SourcePS->GetTeamId() != INDEX_NONE && SourcePS->GetTeamId() == VictimPS->GetTeamId())
		{
			return;
		}

		// Reducción de daño (domo de Pachamama, dash de Wayra).
		UAbilitySystemComponent* OwnASC = GetOwningAbilitySystemComponent();
		if (OwnASC && OwnASC->HasMatchingGameplayTag(AmaruTags::State_Protected))
		{
			Damage *= ProtectedDamageMultiplier;
		}

		const float ShieldAbsorbed = FMath::Min(GetShield(), Damage);
		SetShield(GetShield() - ShieldAbsorbed);
		const float HealthLost = FMath::Min(GetHealth(), Damage - ShieldAbsorbed);
		SetHealth(GetHealth() - HealthLost);

		const float RealDamage = ShieldAbsorbed + HealthLost;
		if (VictimPS)
		{
			VictimPS->AddDamageMitigated(ShieldAbsorbed);
		}
		if (SourcePS && SourcePS != VictimPS && RealDamage > 0.f)
		{
			if (VictimPS)
			{
				VictimPS->RecordDamageReceived(SourcePS);
			}
			if (UAmaruAttributeSet* SourceAS = SourcePS->GetAttributeSet())
			{
				SourceAS->SetUltimateCharge(SourceAS->GetUltimateCharge() + RealDamage * SourceAS->UltChargePerDamageDealt);
			}
			OnDamageDealt.Broadcast(SourcePS, RealDamage);
		}
		CheckForDeath(Data);
	}
	else if (Attr == GetIncomingHealingAttribute())
	{
		const float Healing = GetIncomingHealing();
		SetIncomingHealing(0.f);
		if (Healing <= 0.f || GetHealth() <= 0.f)
		{
			return;
		}

		const float Applied = FMath::Min(Healing, GetMaxHealth() - GetHealth());
		SetHealth(GetHealth() + Applied);

		if (Applied > 0.f)
		{
			if (AAmaruPlayerState* SourcePS = UAmaruGameplayStatics::GetAmaruPlayerStateFromActor(
					Data.EffectSpec.GetContext().GetOriginalInstigator()))
			{
				SourcePS->AddHealingDone(Applied);
				if (UAmaruAttributeSet* SourceAS = SourcePS->GetAttributeSet())
				{
					SourceAS->SetUltimateCharge(SourceAS->GetUltimateCharge() + Applied * SourceAS->UltChargePerHealingDone);
				}
			}
		}
	}
	else if (Attr == GetHealthAttribute())
	{
		const float ClampedHealth = FMath::Clamp(GetHealth(), 0.f, GetMaxHealth());
		SetHealth(ClampedHealth);
		CheckForDeath(Data);
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
	else if (Attr == GetAmmoAttribute())
	{
		SetAmmo(FMath::Clamp(GetAmmo(), 0.f, GetMaxAmmo()));
	}
	else if (Attr == GetMaxAmmoAttribute())
	{
		SetMaxAmmo(FMath::Max(GetMaxAmmo(), 0.f));
		SetAmmo(FMath::Clamp(GetAmmo(), 0.f, GetMaxAmmo()));
	}
}

void UAmaruAttributeSet::CheckForDeath(const FGameplayEffectModCallbackData& Data)
{
	if (GetHealth() > 0.f)
	{
		bOutOfHealth = false;
		return;
	}

	if (bOutOfHealth)
	{
		return;
	}
	bOutOfHealth = true;

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	AActor* OwningActor = GetOwningActor();
	if (!ASC || !OwningActor || !OwningActor->HasAuthority())
	{
		return;
	}

	// Aviso único de muerte: los Blueprints escuchan Event.Death (kill feed,
	// animación) y el GameMode gestiona stats + respawn.
	FGameplayEventData Payload;
	Payload.EventTag = AmaruTags::Event_Death;
	Payload.Target = ASC->GetAvatarActor();
	Payload.Instigator = Data.EffectSpec.GetContext().GetOriginalInstigator();
	Payload.ContextHandle = Data.EffectSpec.GetContext();
	ASC->HandleGameplayEvent(AmaruTags::Event_Death, &Payload);

	if (AAmaruGameMode* GM = GetWorld()->GetAuthGameMode<AAmaruGameMode>())
	{
		GM->NotifyPlayerDied(Cast<AAmaruPlayerState>(GetOwningActor()),
			Data.EffectSpec.GetContext().GetOriginalInstigator());
	}
}

void UAmaruAttributeSet::ResetVitals()
{
	SetHealth(GetMaxHealth());
	SetShield(GetMaxShield());
	SetAmmo(GetMaxAmmo());
	bOutOfHealth = false;
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

void UAmaruAttributeSet::OnRep_ChargeAbility1(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, ChargeAbility1, OldValue);
}

void UAmaruAttributeSet::OnRep_MaxChargeAbility1(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, MaxChargeAbility1, OldValue);
}

void UAmaruAttributeSet::OnRep_ChargeAbility2(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, ChargeAbility2, OldValue);
}

void UAmaruAttributeSet::OnRep_MaxChargeAbility2(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, MaxChargeAbility2, OldValue);
}

void UAmaruAttributeSet::OnRep_Ammo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, Ammo, OldValue);
}

void UAmaruAttributeSet::OnRep_MaxAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, MaxAmmo, OldValue);
}

void UAmaruAttributeSet::OnRep_UltimateCharge(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, UltimateCharge, OldValue);
}

void UAmaruAttributeSet::OnRep_MaxUltimateCharge(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmaruAttributeSet, MaxUltimateCharge, OldValue);
}
