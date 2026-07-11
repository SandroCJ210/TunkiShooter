// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponManagerComponent.h"

#include "AmaruAbilitySystemComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayAbilitySpec.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/WeaponBase.h"
#include "../AmaruShooterCharacter.h"

// Sets default values for this component's properties
UWeaponManagerComponent::UWeaponManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();


	OwnerChar = Cast<AAmaruShooterCharacter>(GetOwner());

}

void UWeaponManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// El arma es un actor aparte con Owner = personaje; si el personaje muere
	// sin esto, el arma queda huérfana en el mundo.
	if (CanModifyWeaponState())
	{
		UnequipWeapon();
	}
	Super::EndPlay(EndPlayReason);
}

void UWeaponManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponManagerComponent, CurrentWeapon);
}

void UWeaponManagerComponent::OnRep_CurrentWeapon()
{
	// En clientes remotos el equip solo ocurre en el servidor; aquí aplicamos
	// la parte cosmética (AnimInstance del mesh 3P) cuando llega la réplica.
	if (!OwnerChar)
	{
		OwnerChar = Cast<AAmaruShooterCharacter>(GetOwner());
	}
	if (!OwnerChar || !OwnerChar->GetMesh3P())
	{
		return;
	}

	if (CurrentWeapon)
	{
		OwnerChar->GetMesh3P()->SetAnimInstanceClass(CurrentWeapon->WeaponConfig.AnimationClass);
	}
	else
	{
		OwnerChar->GetMesh3P()->SetAnimInstanceClass(DefaultAnimClass);
	}
}

void UWeaponManagerComponent::EquipWeapon(TSubclassOf<AWeaponBase> WeaponClass)
{
	if (!OwnerChar)
	{
		OwnerChar = Cast<AAmaruShooterCharacter>(GetOwner());
		if (!OwnerChar)
		{
			return;
		}
	}

	if (!CanModifyWeaponState())
	{
		return;
	}

	if (CurrentWeapon && CurrentWeapon->GetClass() == WeaponClass)
	{
		return;
	}

	UnequipWeapon();

	if (OwnerChar && WeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerChar;
		SpawnParams.Instigator = Cast<APawn>(GetOwner());
		CurrentWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, SpawnParams);
		if (CurrentWeapon)
		{
			FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
			CurrentWeapon->AttachToComponent(OwnerChar->GetMesh3P(), AttachmentRules, CurrentWeapon->WeaponConfig.EquippedSocketName);
			OwnerChar->GetMesh3P()->SetAnimInstanceClass(CurrentWeapon->WeaponConfig.AnimationClass);
		}
	}

	GrantAbilities();
}

void UWeaponManagerComponent::UnequipWeapon()
{
	if (!CanModifyWeaponState())
	{
		return;
	}

	ClearGrantedAbilities();

	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}
	if (OwnerChar)
	{
		OwnerChar->GetMesh3P()->SetAnimInstanceClass(DefaultAnimClass);
	}
}

FVector UWeaponManagerComponent::GetSpawnBulletSocket() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->Mesh->GetSocketLocation(TEXT("Muzzle"));
	}
	return FVector::ZeroVector;
}

void UWeaponManagerComponent::GrantAbilities()
{
	ClearGrantedAbilities();

	if (!CurrentWeapon || !OwnerChar)
	{
		return;
	}

	const auto ASC = OwnerChar->GetAmaruAbilitySystemComponent();
	for (const auto& AbilityClass : CurrentWeapon->WeaponConfig.AbilitiesToGrant)
	{
		if (ASC && AbilityClass)
		{
			const FGameplayAbilitySpecHandle Handle =
				ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, static_cast<int32>(EAmaruAbilityInputID::PrimaryFire)));
			GrantedWeaponAbilityHandles.Add(Handle);
		}
	}
}

void UWeaponManagerComponent::ClearGrantedAbilities()
{
	if (!OwnerChar)
	{
		return;
	}

	const auto ASC = OwnerChar->GetAmaruAbilitySystemComponent();
	if (!ASC)
	{
		GrantedWeaponAbilityHandles.Reset();
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : GrantedWeaponAbilityHandles)
	{
		if (Handle.IsValid())
		{
			ASC->ClearAbility(Handle);
		}
	}

	GrantedWeaponAbilityHandles.Reset();
}

bool UWeaponManagerComponent::CanModifyWeaponState() const
{
	return OwnerChar && OwnerChar->HasAuthority();
}

