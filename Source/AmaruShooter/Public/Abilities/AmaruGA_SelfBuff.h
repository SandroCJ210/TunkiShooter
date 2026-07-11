#pragma once

#include "CoreMinimal.h"
#include "AmaruShooter/AmaruGameplayAbility.h"
#include "AmaruGA_SelfBuff.generated.h"

class UGameplayEffect;

// Habilidad genérica "aplicar buff a sí mismo": el GE de duración gestiona la
// expiración, la habilidad termina inmediatamente. Con bIsUltimate el coste es
// la carga completa de ultimate (sin GE de coste: CheckCost/ApplyCost directos).
UCLASS()
class AMARUSHOOTER_API UAmaruGA_SelfBuff : public UAmaruGameplayAbility
{
	GENERATED_BODY()

public:
	UAmaruGA_SelfBuff();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buff")
	TSubclassOf<UGameplayEffect> BuffEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buff")
	bool bIsUltimate = false;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
};
