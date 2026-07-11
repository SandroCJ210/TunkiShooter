#include "Abilities/Pachamama/PachamamaAbilities.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AmaruGameplayStatics.h"
#include "AmaruGameplayTags.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "Effects/AmaruGameplayEffects.h"
#include "Objectives/PachamamaStructures.h"
#include "GameFramework/Pawn.h"

APachamamaOrb::APachamamaOrb()
{
	Speed = 1200.f;
	MaxDistance = 6000.f;
	BaseDamage = 45.f;
	ExplosionRadius = 250.f;
	GravityScale = 0.3f;
}

UAmaruGA_PachaPrimary::UAmaruGA_PachaPrimary()
{
	DebugName = TEXT("Pachamama Primary");
	ProjectileClass = APachamamaOrb::StaticClass();
	CooldownDuration = 0.9f;
	CooldownTags.AddTag(AmaruTags::Cooldown_Pachamama_Primary);
}

UAmaruGA_PachaBubble::UAmaruGA_PachaBubble()
{
	DebugName = TEXT("Abrazo de la Tierra");
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	CooldownGameplayEffectClass = UAmaruGE_Cooldown::StaticClass();
	BubbleEffect = UAmaruGE_Bubble::StaticClass();
	CooldownDuration = 10.f;
	CooldownTags.AddTag(AmaruTags::Cooldown_Pachamama_Ability1);
}

void UAmaruGA_PachaBubble::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (HasAuthority(&ActivationInfo) && BubbleEffect)
	{
		const AActor* Avatar = GetAvatarActorFromActorInfo();
		UAbilitySystemComponent* OwnASC = GetAbilitySystemComponentFromActorInfo();
		UAbilitySystemComponent* TargetASC = OwnASC;

		// Aliado apuntado dentro de rango, si no: a sí misma.
		FVector ViewLoc = Avatar ? Avatar->GetActorLocation() : FVector::ZeroVector;
		FRotator ViewRot = Avatar ? Avatar->GetActorRotation() : FRotator::ZeroRotator;
		if (APlayerController* PC = ActorInfo->PlayerController.Get())
		{
			PC->GetPlayerViewPoint(ViewLoc, ViewRot);
		}

		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(PachaBubble), true, Avatar);
		GetWorld()->LineTraceSingleByChannel(Hit, ViewLoc, ViewLoc + ViewRot.Vector() * TargetRange, ECC_Visibility, Params);

		if (const APawn* HitPawn = Cast<APawn>(Hit.GetActor()))
		{
			if (UAmaruGameplayStatics::AreSameTeam(GetOwningActorFromActorInfo(), HitPawn))
			{
				if (UAbilitySystemComponent* AllyASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<APawn*>(HitPawn)))
				{
					TargetASC = AllyASC;
				}
			}
		}

		if (OwnASC && TargetASC)
		{
			FGameplayEffectContextHandle Ctx = OwnASC->MakeEffectContext();
			Ctx.AddSourceObject(this);
			FGameplayEffectSpecHandle Spec = OwnASC->MakeOutgoingSpec(BubbleEffect, GetAbilityLevel(), Ctx);
			if (Spec.IsValid())
			{
				Spec.Data->SetSetByCallerMagnitude(AmaruTags::Data_Shield, BubbleAmount);
				OwnASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UAmaruGA_PachaWall::UAmaruGA_PachaWall()
{
	DebugName = TEXT("Muro de la Pachamama");
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	CooldownGameplayEffectClass = UAmaruGE_Cooldown::StaticClass();
	WallClass = APachamamaWall::StaticClass();
	CooldownDuration = 18.f;
	CooldownTags.AddTag(AmaruTags::Cooldown_Pachamama_Ability2);
}

void UAmaruGA_PachaWall::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (HasAuthority(&ActivationInfo) && WallClass)
	{
		APawn* Avatar = Cast<APawn>(GetAvatarActorFromActorInfo());
		const AAmaruPlayerState* PS = Cast<AAmaruPlayerState>(GetOwningActorFromActorInfo());
		if (Avatar && PS)
		{
			// Punto frente al jugador, proyectado al suelo.
			const FRotator YawRot(0.f, Avatar->GetActorRotation().Yaw, 0.f);
			const FVector Forward = YawRot.Vector();
			FVector SpawnLoc = Avatar->GetActorLocation() + Forward * SpawnDistance;

			FHitResult GroundHit;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(PachaWallGround), false, Avatar);
			if (GetWorld()->LineTraceSingleByChannel(GroundHit, SpawnLoc + FVector(0, 0, 500.f), SpawnLoc - FVector(0, 0, 1000.f), ECC_WorldStatic, Params))
			{
				SpawnLoc = GroundHit.ImpactPoint;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Avatar;
			SpawnParams.Instigator = Avatar;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			if (APachamamaWall* Wall = GetWorld()->SpawnActor<APachamamaWall>(WallClass, SpawnLoc, YawRot, SpawnParams))
			{
				Wall->TeamId = PS->GetTeamId();
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UAmaruGA_PachaDome::UAmaruGA_PachaDome()
{
	DebugName = TEXT("Santuario del Ande");
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	DomeClass = APachamamaDome::StaticClass();
	BuffEffect = nullptr;
	bIsUltimate = true;
	CooldownDuration = 2.f;
	CooldownTags.AddTag(AmaruTags::Cooldown_Ultimate);
}

void UAmaruGA_PachaDome::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (HasAuthority(&ActivationInfo) && DomeClass)
	{
		APawn* Avatar = Cast<APawn>(GetAvatarActorFromActorInfo());
		const AAmaruPlayerState* PS = Cast<AAmaruPlayerState>(GetOwningActorFromActorInfo());
		if (Avatar && PS)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Avatar;
			SpawnParams.Instigator = Avatar;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (APachamamaDome* Dome = GetWorld()->SpawnActor<APachamamaDome>(DomeClass, Avatar->GetActorLocation(), FRotator::ZeroRotator, SpawnParams))
			{
				Dome->TeamId = PS->GetTeamId();
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
