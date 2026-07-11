#include "Abilities/AmaruGA_ProjectileFire.h"

#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "Effects/AmaruGameplayEffects.h"
#include "Weapons/AmaruProjectile.h"
#include "Weapons/WeaponManagerComponent.h"

UAmaruGA_ProjectileFire::UAmaruGA_ProjectileFire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	CostGameplayEffectClass = UAmaruGE_AmmoCost::StaticClass();
	CooldownGameplayEffectClass = UAmaruGE_Cooldown::StaticClass();
	bScaleCooldownWithAttackSpeed = true;
	CooldownDuration = 0.7f;
}

void UAmaruGA_ProjectileFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (HasAuthority(&ActivationInfo) && ProjectileClass)
	{
		APawn* Avatar = Cast<APawn>(GetAvatarActorFromActorInfo());
		AAmaruPlayerState* PS = Cast<AAmaruPlayerState>(GetOwningActorFromActorInfo());
		if (Avatar && PS)
		{
			PS->AddShotFired();

			FVector ViewLoc = Avatar->GetActorLocation();
			FRotator ViewRot = Avatar->GetActorRotation();
			if (APlayerController* PC = ActorInfo->PlayerController.Get())
			{
				PC->GetPlayerViewPoint(ViewLoc, ViewRot);
			}

			// Punto de mira: donde apunta la cámara.
			FHitResult AimHit;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(AmaruProjectileAim), true, Avatar);
			const FVector AimEnd = ViewLoc + ViewRot.Vector() * AimTraceRange;
			GetWorld()->LineTraceSingleByChannel(AimHit, ViewLoc, AimEnd, ECC_Visibility, Params);
			const FVector TargetPoint = AimHit.bBlockingHit ? AimHit.ImpactPoint : AimEnd;

			// Origen: boca del arma si existe, si no la vista.
			FVector MuzzleLoc = ViewLoc + ViewRot.Vector() * 50.f;
			if (const UWeaponManagerComponent* WeaponManager = Avatar->FindComponentByClass<UWeaponManagerComponent>())
			{
				const FVector SocketLoc = WeaponManager->GetSpawnBulletSocket();
				if (!SocketLoc.IsNearlyZero())
				{
					MuzzleLoc = SocketLoc;
				}
			}

			const FRotator SpawnRot = (TargetPoint - MuzzleLoc).Rotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Avatar;
			SpawnParams.Instigator = Avatar;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (AAmaruProjectile* Projectile = GetWorld()->SpawnActor<AAmaruProjectile>(ProjectileClass, MuzzleLoc, SpawnRot, SpawnParams))
			{
				float DamageMult = 1.f;
				if (const UAmaruAttributeSet* AS = PS->GetAttributeSet())
				{
					DamageMult = AS->GetDamageMultiplier() > 0.f ? AS->GetDamageMultiplier() : 1.f;
				}
				Projectile->InitProjectile(GetAbilitySystemComponentFromActorInfo(), DamageMult);
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
