#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "AmaruGameplayEffects.generated.h"

// Efectos compartidos definidos en C++: los DataAssets y habilidades los
// referencian directamente sin necesidad de assets Blueprint.
// ponytail: números balance hardcodeados en ctor; crear subclase BP si diseño necesita tunear sin recompilar.

// Cooldown genérico: duración SetByCaller Data.Cooldown, tags por DynamicGrantedTags.
UCLASS()
class AMARUSHOOTER_API UAmaruGE_Cooldown : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_Cooldown();
};

// Daño: IncomingDamage += SetByCaller Data.Damage.
UCLASS()
class AMARUSHOOTER_API UAmaruGE_Damage : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_Damage();
};

// Curación: IncomingHealing += SetByCaller Data.Heal.
UCLASS()
class AMARUSHOOTER_API UAmaruGE_Heal : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_Heal();
};

// Coste de munición: Ammo -1 fijo (SetByCaller en costes rompe CheckCost).
UCLASS()
class AMARUSHOOTER_API UAmaruGE_AmmoCost : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_AmmoCost();
};

// Ojo del Centinela: +25% daño, State.Focus, 6s.
UCLASS()
class AMARUSHOOTER_API UAmaruGE_Focus : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_Focus();
	virtual void PostInitProperties() override;
};

// Paso del Atoq: invisibilidad + velocidad, State.Stealth, 5s.
UCLASS()
class AMARUSHOOTER_API UAmaruGE_Stealth : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_Stealth();
	virtual void PostInitProperties() override;
};

// Furia del Yawar: +50% daño y cadencia, State.Rage, 8s.
UCLASS()
class AMARUSHOOTER_API UAmaruGE_Rage : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_Rage();
	virtual void PostInitProperties() override;
};

// Cura al matar en rage.
UCLASS()
class AMARUSHOOTER_API UAmaruGE_HealOnKill : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_HealOnKill();
};

// Abrazo de la Tierra: Shield += SetByCaller Data.Shield.
UCLASS()
class AMARUSHOOTER_API UAmaruGE_Bubble : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_Bubble();
};

// Buff del Muro: escudo pequeño al infligir daño cerca del muro.
UCLASS()
class AMARUSHOOTER_API UAmaruGE_ShieldOnDamage : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_ShieldOnDamage();
};

// Reducción de daño breve (dash de Wayra), State.Protected, 0.5s.
UCLASS()
class AMARUSHOOTER_API UAmaruGE_Protected : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_Protected();
	virtual void PostInitProperties() override;
};

// Reducción de daño mientras se está dentro del domo (removido por handle).
UCLASS()
class AMARUSHOOTER_API UAmaruGE_DomeProtect : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_DomeProtect();
	virtual void PostInitProperties() override;
};

// Ralentización de enemigos dentro del domo (removido por handle).
UCLASS()
class AMARUSHOOTER_API UAmaruGE_DomeSlow : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_DomeSlow();
	virtual void PostInitProperties() override;
};

// Danza de los Apus: velocidad de ataque y movimiento, State.Haste, 6s.
UCLASS()
class AMARUSHOOTER_API UAmaruGE_Haste : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UAmaruGE_Haste();
	virtual void PostInitProperties() override;
};
