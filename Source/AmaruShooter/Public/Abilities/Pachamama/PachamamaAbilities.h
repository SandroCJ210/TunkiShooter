#pragma once

#include "CoreMinimal.h"
#include "Abilities/AmaruGA_ProjectileFire.h"
#include "Abilities/AmaruGA_SelfBuff.h"
#include "AmaruShooter/AmaruGameplayAbility.h"
#include "Weapons/AmaruProjectile.h"
#include "PachamamaAbilities.generated.h"

class UGameplayEffect;
class APachamamaWall;
class APachamamaDome;

// Núcleo de Arcilla: orbe de tierra a media velocidad que explota en área.
UCLASS()
class AMARUSHOOTER_API APachamamaOrb : public AAmaruProjectile
{
	GENERATED_BODY()
public:
	APachamamaOrb();
};

UCLASS()
class AMARUSHOOTER_API UAmaruGA_PachaPrimary : public UAmaruGA_ProjectileFire
{
	GENERATED_BODY()
public:
	UAmaruGA_PachaPrimary();
};

// Abrazo de la Tierra: burbuja (puntos de Shield) sobre el aliado apuntado o sí misma.
UCLASS()
class AMARUSHOOTER_API UAmaruGA_PachaBubble : public UAmaruGameplayAbility
{
	GENERATED_BODY()
public:
	UAmaruGA_PachaBubble();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bubble")
	float BubbleAmount = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bubble")
	float TargetRange = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Bubble")
	TSubclassOf<UGameplayEffect> BubbleEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

// Muro de la Pachamama: pared destructible que da escudo a aliados que dañan cerca.
UCLASS()
class AMARUSHOOTER_API UAmaruGA_PachaWall : public UAmaruGameplayAbility
{
	GENERATED_BODY()
public:
	UAmaruGA_PachaWall();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wall")
	TSubclassOf<APachamamaWall> WallClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wall")
	float SpawnDistance = 800.f;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

// Santuario del Ande (ultimate): domo destructible centrado en Pachamama.
UCLASS()
class AMARUSHOOTER_API UAmaruGA_PachaDome : public UAmaruGA_SelfBuff
{
	GENERATED_BODY()
public:
	UAmaruGA_PachaDome();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dome")
	TSubclassOf<APachamamaDome> DomeClass;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
