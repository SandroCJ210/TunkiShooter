#include "Effects/AmaruGameplayEffects.h"

#include "AmaruAttributeSet.h"
#include "AmaruGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

namespace
{
	FGameplayModifierInfo MakeSetByCallerMod(const FGameplayAttribute& Attribute, const FGameplayTag& DataTag)
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = Attribute;
		Mod.ModifierOp = EGameplayModOp::Additive;
		FSetByCallerFloat SetByCaller;
		SetByCaller.DataTag = DataTag;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
		return Mod;
	}

	FGameplayModifierInfo MakeFlatMod(const FGameplayAttribute& Attribute, float Magnitude)
	{
		FGameplayModifierInfo Mod;
		Mod.Attribute = Attribute;
		Mod.ModifierOp = EGameplayModOp::Additive;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Magnitude));
		return Mod;
	}

	void AddGrantedTag(UGameplayEffect& Effect, const FGameplayTag& Tag)
	{
		UTargetTagsGameplayEffectComponent& TagsComp = Effect.FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
		FInheritedTagContainer TagChanges = TagsComp.GetConfiguredTargetTagChanges();
		TagChanges.Added.AddTag(Tag);
		TagsComp.SetAndApplyTargetTagChanges(TagChanges);
	}
}

UAmaruGE_Cooldown::UAmaruGE_Cooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = AmaruTags::Data_Cooldown;
	DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
}

UAmaruGE_Damage::UAmaruGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	Modifiers.Add(MakeSetByCallerMod(UAmaruAttributeSet::GetIncomingDamageAttribute(), AmaruTags::Data_Damage));
}

UAmaruGE_Heal::UAmaruGE_Heal()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	Modifiers.Add(MakeSetByCallerMod(UAmaruAttributeSet::GetIncomingHealingAttribute(), AmaruTags::Data_Heal));
}

UAmaruGE_AmmoCost::UAmaruGE_AmmoCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	Modifiers.Add(MakeFlatMod(UAmaruAttributeSet::GetAmmoAttribute(), -1.f));
}

UAmaruGE_Focus::UAmaruGE_Focus()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(6.f));
	Modifiers.Add(MakeFlatMod(UAmaruAttributeSet::GetDamageMultiplierAttribute(), 0.25f));
}

void UAmaruGE_Focus::PostInitProperties()
{
	Super::PostInitProperties();
	AddGrantedTag(*this, AmaruTags::State_Focus);
}

UAmaruGE_Stealth::UAmaruGE_Stealth()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(5.f));
	Modifiers.Add(MakeFlatMod(UAmaruAttributeSet::GetMoveSpeedAttribute(), 150.f));
}

void UAmaruGE_Stealth::PostInitProperties()
{
	Super::PostInitProperties();
	AddGrantedTag(*this, AmaruTags::State_Stealth);
}

UAmaruGE_Rage::UAmaruGE_Rage()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(8.f));
	Modifiers.Add(MakeFlatMod(UAmaruAttributeSet::GetDamageMultiplierAttribute(), 0.5f));
}

void UAmaruGE_Rage::PostInitProperties()
{
	Super::PostInitProperties();
	AddGrantedTag(*this, AmaruTags::State_Rage);
}

UAmaruGE_HealOnKill::UAmaruGE_HealOnKill()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	Modifiers.Add(MakeFlatMod(UAmaruAttributeSet::GetIncomingHealingAttribute(), 40.f));
}

UAmaruGE_Bubble::UAmaruGE_Bubble()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	Modifiers.Add(MakeSetByCallerMod(UAmaruAttributeSet::GetShieldAttribute(), AmaruTags::Data_Shield));

	const FGameplayTag BubbleCue = FGameplayTag::RequestGameplayTag(FName("GameplayCue.ShieldBubble"), /*ErrorIfNotFound=*/false);
	if (BubbleCue.IsValid())
	{
		GameplayCues.Add(FGameplayEffectCue(BubbleCue, 0.f, 0.f));
	}
}

UAmaruGE_ShieldOnDamage::UAmaruGE_ShieldOnDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	Modifiers.Add(MakeFlatMod(UAmaruAttributeSet::GetShieldAttribute(), 15.f));
}

UAmaruGE_Protected::UAmaruGE_Protected()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.5f));
}

void UAmaruGE_Protected::PostInitProperties()
{
	Super::PostInitProperties();
	AddGrantedTag(*this, AmaruTags::State_Protected);
}

UAmaruGE_DomeProtect::UAmaruGE_DomeProtect()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
}

void UAmaruGE_DomeProtect::PostInitProperties()
{
	Super::PostInitProperties();
	AddGrantedTag(*this, AmaruTags::State_Protected);
}

UAmaruGE_DomeSlow::UAmaruGE_DomeSlow()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Modifiers.Add(MakeFlatMod(UAmaruAttributeSet::GetMoveSpeedAttribute(), -150.f));
}

void UAmaruGE_DomeSlow::PostInitProperties()
{
	Super::PostInitProperties();
	AddGrantedTag(*this, AmaruTags::State_Slowed);
}

UAmaruGE_Haste::UAmaruGE_Haste()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(6.f));
	Modifiers.Add(MakeFlatMod(UAmaruAttributeSet::GetMoveSpeedAttribute(), 100.f));
}

void UAmaruGE_Haste::PostInitProperties()
{
	Super::PostInitProperties();
	AddGrantedTag(*this, AmaruTags::State_Haste);
}
