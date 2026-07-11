#include "Abilities/Atoq/AtoqAbilities.h"

#include "AmaruGameplayTags.h"
#include "Effects/AmaruGameplayEffects.h"

UAmaruGA_AtoqPrimary::UAmaruGA_AtoqPrimary()
{
	DebugName = TEXT("Atoq Primary");
	BaseDamage = 20.f;
	Range = 12000.f;
	CooldownDuration = 0.35f;
	CooldownTags.AddTag(AmaruTags::Cooldown_Atoq_Primary);
}

UAmaruGA_AtoqFocus::UAmaruGA_AtoqFocus()
{
	DebugName = TEXT("Ojo del Centinela");
	BuffEffect = UAmaruGE_Focus::StaticClass();
	CooldownDuration = 12.f;
	CooldownTags.AddTag(AmaruTags::Cooldown_Atoq_Ability1);
}

UAmaruGA_AtoqStealth::UAmaruGA_AtoqStealth()
{
	DebugName = TEXT("Paso del Atoq");
	BuffEffect = UAmaruGE_Stealth::StaticClass();
	CooldownDuration = 15.f;
	CooldownTags.AddTag(AmaruTags::Cooldown_Atoq_Ability2);
}

UAmaruGA_AtoqRage::UAmaruGA_AtoqRage()
{
	DebugName = TEXT("Furia del Yawar");
	BuffEffect = UAmaruGE_Rage::StaticClass();
	bIsUltimate = true;
	CooldownDuration = 2.f;
	CooldownTags.AddTag(AmaruTags::Cooldown_Ultimate);
}
