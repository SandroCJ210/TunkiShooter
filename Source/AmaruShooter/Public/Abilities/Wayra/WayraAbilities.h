#pragma once

#include "CoreMinimal.h"
#include "Abilities/AmaruGA_ProjectileFire.h"
#include "Abilities/AmaruGA_SelfBuff.h"
#include "AmaruShooter/AmaruGameplayAbility.h"
#include "Weapons/AmaruProjectile.h"
#include "WayraAbilities.generated.h"

class UGameplayEffect;

// Bastón Ritual: orbe lento; al acertar a un enemigo reduce el cooldown de Sanación del Viento.
UCLASS()
class AMARUSHOOTER_API AWayraOrb : public AAmaruProjectile
{
	GENERATED_BODY()
public:
	AWayraOrb();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float HealCooldownReduction = 1.f;

protected:
	virtual void OnEnemyHit(UAbilitySystemComponent* TargetASC) override;
};

UCLASS()
class AMARUSHOOTER_API UAmaruGA_WayraPrimary : public UAmaruGA_ProjectileFire
{
	GENERATED_BODY()
public:
	UAmaruGA_WayraPrimary();
};

// Sanación del Viento: cura al aliado vivo con menos vida en rango (o a sí misma).
UCLASS()
class AMARUSHOOTER_API UAmaruGA_WayraHeal : public UAmaruGameplayAbility
{
	GENERATED_BODY()
public:
	UAmaruGA_WayraHeal();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heal")
	float HealAmount = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heal")
	float HealRange = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Heal")
	TSubclassOf<UGameplayEffect> HealEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

// Paso del Viento: dash en la dirección de movimiento + reducción de daño breve.
UCLASS()
class AMARUSHOOTER_API UAmaruGA_WayraDash : public UAmaruGameplayAbility
{
	GENERATED_BODY()
public:
	UAmaruGA_WayraDash();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash")
	float DashImpulse = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	TSubclassOf<UGameplayEffect> ProtectionEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

// Danza de los Apus (ultimate): cura a todos los aliados y les da haste.
UCLASS()
class AMARUSHOOTER_API UAmaruGA_WayraUlt : public UAmaruGA_SelfBuff
{
	GENERATED_BODY()
public:
	UAmaruGA_WayraUlt();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate")
	float HealAmount = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ultimate")
	TSubclassOf<UGameplayEffect> HealEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Ultimate")
	TSubclassOf<UGameplayEffect> HasteEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
