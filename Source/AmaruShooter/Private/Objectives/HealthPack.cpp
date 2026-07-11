#include "Objectives/HealthPack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.h"
#include "AmaruGameplayStatics.h"
#include "AmaruGameplayTags.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Effects/AmaruGameplayEffects.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AHealthPack::AHealthPack()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Pickup = CreateDefaultSubobject<USphereComponent>(TEXT("Pickup"));
	Pickup->InitSphereRadius(80.f);
	Pickup->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Pickup->SetCollisionResponseToAllChannels(ECR_Ignore);
	Pickup->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Pickup->OnComponentBeginOverlap.AddDynamic(this, &AHealthPack::OnPickupBeginOverlap);
	RootComponent = Pickup;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Pickup);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HealEffect = UAmaruGE_Heal::StaticClass();
}

void AHealthPack::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHealthPack, bIsActive);
}

void AHealthPack::OnPickupBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !bIsActive || !HealEffect) return;

	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	const AAmaruPlayerState* PS = UAmaruGameplayStatics::GetAmaruPlayerStateFromActor(Pawn);
	const UAmaruAttributeSet* AS = PS ? PS->GetAttributeSet() : nullptr;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!AS || !ASC) return;

	// Solo si le falta vida y está vivo.
	if (AS->GetHealth() <= 0.f || AS->GetHealth() >= AS->GetMaxHealth()) return;

	FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
	Ctx.AddSourceObject(this);
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(HealEffect, 1.f, Ctx);
	if (!Spec.IsValid()) return;

	Spec.Data->SetSetByCallerMagnitude(AmaruTags::Data_Heal, HealAmount);
	ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	bIsActive = false;
	OnRep_IsActive();
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AHealthPack::Reactivate, RespawnCooldown, false);
}

void AHealthPack::Reactivate()
{
	bIsActive = true;
	OnRep_IsActive();
}

void AHealthPack::OnRep_IsActive()
{
	Mesh->SetVisibility(bIsActive, true);
	if (HasAuthority())
	{
		Pickup->SetCollisionEnabled(bIsActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}
