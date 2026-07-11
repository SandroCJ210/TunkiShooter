// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AmaruGameplayAbility.generated.h"

/**
 *
 */
UCLASS()
class AMARUSHOOTER_API UAmaruGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UAmaruGameplayAbility();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	int32 InputID = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	bool bEndOnInputRelease = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	FString DebugName = "Ability";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	UTexture2D* AbilityIcon;

	// Cooldown genérico por SetByCaller: las habilidades C++ comparten
	// UAmaruGE_Cooldown y solo configuran tags + duración. Las habilidades BP
	// que asignan su propio CooldownGameplayEffectClass no se ven afectadas
	// (CooldownTags vacío -> comportamiento estándar).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	FGameplayTagContainer CooldownTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	float CooldownDuration = 0.f;

	// Armas: con State.Rage o State.Haste el cooldown (cadencia) se acorta.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	bool bScaleCooldownWithAttackSpeed = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	float AttackSpeedCooldownMultiplier = 0.6f;

	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	virtual float GetCooldownDuration() const;

	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;
};
