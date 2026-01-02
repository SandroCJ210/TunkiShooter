// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AmaruGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class AMARUSHOOTER_API UAmaruGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	int32 InputID = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	bool bEndOnInputRelease = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	FString DebugName = "Ability";
};
