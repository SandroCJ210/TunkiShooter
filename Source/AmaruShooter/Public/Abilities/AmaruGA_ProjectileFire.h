#pragma once

#include "CoreMinimal.h"
#include "AmaruShooter/AmaruGameplayAbility.h"
#include "AmaruGA_ProjectileFire.generated.h"

class AAmaruProjectile;

// Disparo de proyectil (Núcleo de Arcilla, Bastón Ritual). Coste: 1 munición.
UCLASS()
class AMARUSHOOTER_API UAmaruGA_ProjectileFire : public UAmaruGameplayAbility
{
	GENERATED_BODY()

public:
	UAmaruGA_ProjectileFire();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AAmaruProjectile> ProjectileClass;

	// Distancia del trace de puntería para converger el proyectil con la mira.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float AimTraceRange = 10000.f;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
