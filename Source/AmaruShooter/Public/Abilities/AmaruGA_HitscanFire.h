#pragma once

#include "CoreMinimal.h"
#include "AmaruShooter/AmaruGameplayAbility.h"
#include "AmaruGA_HitscanFire.generated.h"

class UGameplayEffect;

// Disparo hitscan (Ballesta de Hierro de Atoq). Coste: 1 munición.
// Cooldown = cadencia, acortado por Rage/Haste.
UCLASS()
class AMARUSHOOTER_API UAmaruGA_HitscanFire : public UAmaruGameplayAbility
{
	GENERATED_BODY()

public:
	UAmaruGA_HitscanFire();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float Range = 10000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UGameplayEffect> DamageEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
