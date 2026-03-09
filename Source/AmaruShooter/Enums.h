#pragma once
#include "CoreMinimal.h"
#include "Enums.generated.h"

class UGameplayAbility;

UENUM(BlueprintType)
enum class EAmaruAbilityInputID : uint8
{
	None = 0      UMETA(DisplayName = "None"),
	Ability1 = 1  UMETA(DisplayName = "Ability1"),
	Ability2 = 2  UMETA(DisplayName = "Ability2"),
	Ultimate = 3  UMETA(DisplayName = "Ultimate"),
	PrimaryFire = 4 UMETA(DisplayName = "PrimaryFire"),
	Reload = 5 UMETA(DisplayName = "Reload"),
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