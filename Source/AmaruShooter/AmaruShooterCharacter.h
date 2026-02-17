// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h"
#include "Enums.h"
#include "GameplayAbilitySpecHandle.h"
#include "AmaruShooterCharacter.generated.h"

class UAmaruGameplayAbility;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilityLoadoutChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChargeAbilityChanged, EAmaruAbilityInputID, AbilityInput, float, NewChargeValue);

struct FActiveGameplayEffectHandle;
struct FGameplayAbilitySpecHandle;
class UInkaDataAsset;
class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class UAmaruAbilitySystemComponent;
class UAmaruAttributeSet;
class AAmaruPlayerState;
class USpringArmComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AAmaruShooterCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh3P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* ThirdPersonCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Ability1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Ability2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* UltimateAction;

public:
	AAmaruShooterCharacter();

	virtual void BeginPlay() override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void Server_EnableAbilitiesForMode();
	void Server_DisableAbilitiesForMode();

protected:

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	virtual void OnRep_Controller() override;

	void InitAbilityActorInfo();

public:

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnAbilityLoadoutChanged OnAbilityLoadoutChanged;

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnChargeAbilityChanged OnChargeAbilityChanged;

protected:

	UPROPERTY(Transient, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAmaruAbilitySystemComponent> CachedASC = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AAmaruPlayerState> CachedPS = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inka", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInkaDataAsset> InkaDefinition;

	UPROPERTY(Transient)
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

	UPROPERTY(Transient)
	TArray<FActiveGameplayEffectHandle> StartupEffectHandles;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAmaruGameplayAbility> EquipWeaponAbility;

	void ApplyStartupEffectsFromDefinition();
	void GiveAbilitiesFromDefinition();
	void ClearGrantedAbilities();

	UPROPERTY(Transient)
	bool bStartupApplied = false;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void OnJumpStarted(const FInputActionValue& Value);
	void OnJumpCompleted(const FInputActionValue& Value);

	void Shoot(const FInputActionValue& Value);
	void StopShooting(const FInputActionValue& Value);

	void Ability1Pressed(const FInputActionValue& Value);
	void Ability1Released(const FInputActionValue& Value);
	void Ability1Canceled(const FInputActionValue& Value);

	void Ability2Pressed(const FInputActionValue& Value);
	void Ability2Released(const FInputActionValue& Value);
	void Ability2Canceled(const FInputActionValue& Value);

	void UltimatePressed(const FInputActionValue& Value);
	void UltimateReleased(const FInputActionValue& Value);
	void UltimateCanceled(const FInputActionValue& Value);


	FDelegateHandle MoveSpeedChangedHandle;
	FDelegateHandle ChargeAbility1ChangedHandle;
	FDelegateHandle ChargeAbility2ChangedHandle;
	TWeakObjectPtr<UAmaruAttributeSet> BoundAS;
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	FGameplayAbilitySpecHandle EquipWeaponHandle;

public:
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }

	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UFUNCTION(BlueprintCallable)
	UInkaDataAsset* GetInkaDataAsset() const { return InkaDefinition; }

	UFUNCTION(BlueprintPure, Category = "GAS")
	AAmaruPlayerState* GetAmaruPlayerState() const { return CachedPS; }

	UFUNCTION(BlueprintPure, Category = "GAS")
	UAmaruAbilitySystemComponent* GetAmaruAbilitySystemComponent() const { return CachedASC; }

	UFUNCTION(BlueprintPure, Category = "GAS")
	UAmaruAttributeSet* GetAmaruAttributeSet() const;

};

