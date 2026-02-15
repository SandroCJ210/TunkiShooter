// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class UGameplayAbility;

USTRUCT(BlueprintType)
struct FWeaponConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<UGameplayAbility*> AbilitiesToGrant;

	UPROPERTY(EditAnywhere)
	FName EquippedSocketName;
};

UCLASS()
class AMARUSHOOTER_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
