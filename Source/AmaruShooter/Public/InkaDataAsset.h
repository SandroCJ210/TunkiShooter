// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InkaDataAsset.generated.h"

class UAmaruGameplayAbility;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FAmaruGrantedAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAmaruGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 InputID = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Cooldown = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxCharges = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Level = 1;
};

UCLASS()
class AMARUSHOOTER_API UInkaDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<FAmaruGrantedAbility> Abilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;
};
