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

static FString NetModeToString(const UWorld* World)
{
	if (!World) return TEXT("NoWorld");

	switch (World->GetNetMode())
	{
	case NM_Standalone:      return TEXT("Standalone");
	case NM_ListenServer:    return TEXT("ListenServer");
	case NM_DedicatedServer: return TEXT("DedicatedServer");
	case NM_Client:          return TEXT("Client");
	default:                 return TEXT("Unknown");
	}
}

void UWeaponManagerComponent::EquipWeapon(TSubclassOf<AWeaponBase> WeaponClass)
{
	if (!OwnerChar)
	{
		OwnerChar = Cast<AAmaruShooterCharacter>(GetOwner());
		if (!OwnerChar)
		{
			UE_LOG(LogTemp, Warning, TEXT("WeaponManagerComponent: Owner is not AAmaruShooterCharacter!"));
			return;
		}
	}
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

