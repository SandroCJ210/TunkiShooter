// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponManagerComponent.generated.h"

class AAmaruShooterCharacter;
class AWeaponBase;
struct FGameplayAbilitySpecHandle;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AMARUSHOOTER_API UWeaponManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	AWeaponBase* CurrentWeapon;

	UFUNCTION()
	void OnRep_CurrentWeapon();

	UPROPERTY()
	AAmaruShooterCharacter* OwnerChar;

	UPROPERTY(Transient)
	TArray<FGameplayAbilitySpecHandle> GrantedWeaponAbilityHandles;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAnimInstance> DefaultAnimClass;

	UFUNCTION()
	void GrantAbilities();
	void ClearGrantedAbilities();
	bool CanModifyWeaponState() const;

public:
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(TSubclassOf<AWeaponBase> WeaponClass);

	UFUNCTION(BlueprintCallable)
	void UnequipWeapon();

	UFUNCTION(BlueprintCallable)
	FVector GetSpawnBulletSocket() const;
		
};
