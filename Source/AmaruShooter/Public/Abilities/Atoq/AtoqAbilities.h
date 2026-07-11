#pragma once

#include "CoreMinimal.h"
#include "Abilities/AmaruGA_HitscanFire.h"
#include "Abilities/AmaruGA_SelfBuff.h"
#include "AtoqAbilities.generated.h"

// Atoq (Damage): soldado andino con ballesta hitscan.
// Subclases triviales: solo configuran defaults; el DataAsset las referencia directamente.

// Ballesta de Hierro: disparo hitscan de alta cadencia.
UCLASS()
class AMARUSHOOTER_API UAmaruGA_AtoqPrimary : public UAmaruGA_HitscanFire
{
	GENERATED_BODY()
public:
	UAmaruGA_AtoqPrimary();
};

// Ojo del Centinela: precisión/daño mejorado + hook de mira térmica.
UCLASS()
class AMARUSHOOTER_API UAmaruGA_AtoqFocus : public UAmaruGA_SelfBuff
{
	GENERATED_BODY()
public:
	UAmaruGA_AtoqFocus();
};

// Paso del Atoq: invisibilidad + velocidad.
UCLASS()
class AMARUSHOOTER_API UAmaruGA_AtoqStealth : public UAmaruGA_SelfBuff
{
	GENERATED_BODY()
public:
	UAmaruGA_AtoqStealth();
};

// Furia del Yawar (ultimate): más daño y cadencia; cura al matar (GameMode).
UCLASS()
class AMARUSHOOTER_API UAmaruGA_AtoqRage : public UAmaruGA_SelfBuff
{
	GENERATED_BODY()
public:
	UAmaruGA_AtoqRage();
};
