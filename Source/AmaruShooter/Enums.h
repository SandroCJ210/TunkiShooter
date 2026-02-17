#pragma once
#include "CoreMinimal.h"
#include "Enums.generated.h"

class UGameplayAbility;

UENUM(BlueprintType)
enum class EAmaruAbilityInputID : uint8
{
	None      UMETA(DisplayName = "None"),
	Ability1  UMETA(DisplayName = "Ability1"),
	Ability2  UMETA(DisplayName = "Ability2"),
	Ultimate  UMETA(DisplayName = "Ultimate"),
};

USTRUCT(BlueprintType)
struct FWeaponConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray <TSubclassOf<UGameplayAbility>> AbilitiesToGrant;

	UPROPERTY(EditAnywhere)
	FName EquippedSocketName;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAnimInstance> AnimationClass;
};