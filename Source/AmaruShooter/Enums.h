#pragma once
#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAmaruAbilityInputID : uint8
{
	None      UMETA(DisplayName = "None"),
	Ability1  UMETA(DisplayName = "Ability1"),
	Ability2  UMETA(DisplayName = "Ability2"),
	Ultimate  UMETA(DisplayName = "Ultimate"),
};
