// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponManagerComponent.h"
#include "Weapons/WeaponBase.h"
#include "../AmaruShooterCharacter.h"

// Sets default values for this component's properties
UWeaponManagerComponent::UWeaponManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();


	OwnerChar = Cast<AAmaruShooterCharacter>(GetOwner());
	
}


void UWeaponManagerComponent::EquipWeapon(TSubclassOf<AWeaponBase> WeaponClass)
{
	if (OwnerChar)
	{
		if (CurrentWeapon)
		{
			if (CurrentWeapon->GetClass() == WeaponClass)
			{
				UnequipWeapon();
				return;
			}
		}
		if (WeaponClass)
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
	}
}

void UWeaponManagerComponent::UnequipWeapon()
{
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

// Called every frame
void UWeaponManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

