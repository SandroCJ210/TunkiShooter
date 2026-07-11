#include "Weapons/AmaruProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AmaruGameplayStatics.h"
#include "AmaruGameplayTags.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Effects/AmaruGameplayEffects.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Objectives/AmaruDestructible.h"

AAmaruProjectile::AAmaruProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(10.f);
	Collision->SetCollisionProfileName(TEXT("Projectile"));
	Collision->SetGenerateOverlapEvents(true);
	Collision->OnComponentHit.AddDynamic(this, &AAmaruProjectile::OnProjectileHit);
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AAmaruProjectile::OnProjectileOverlap);
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->bRotationFollowsVelocity = true;
	Movement->bShouldBounce = false;

	DamageEffect = UAmaruGE_Damage::StaticClass();
}

void AAmaruProjectile::BeginPlay()
{
	Super::BeginPlay();

	Movement->InitialSpeed = Speed;
	Movement->MaxSpeed = Speed;
	Movement->ProjectileGravityScale = GravityScale;
	Movement->Velocity = GetActorForwardVector() * Speed;

	if (Speed > 0.f)
	{
		SetLifeSpan(MaxDistance / Speed);
	}

	// No chocar con quien lo dispara.
	if (AActor* MyInstigator = GetInstigator())
	{
		Collision->IgnoreActorWhenMoving(MyInstigator, true);
	}
}

void AAmaruProjectile::InitProjectile(UAbilitySystemComponent* InSourceASC, float InDamageMultiplier)
{
	SourceASC = InSourceASC;
	DamageMultiplier = InDamageMultiplier > 0.f ? InDamageMultiplier : 1.f;
}

void AAmaruProjectile::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	HandleImpact(OtherActor, Hit.ImpactPoint);
}

void AAmaruProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	HandleImpact(OtherActor, GetActorLocation());
}

void AAmaruProjectile::HandleImpact(AActor* OtherActor, const FVector& ImpactLocation)
{
	if (!HasAuthority() || bImpactHandled) return;
	if (OtherActor == this || OtherActor == GetInstigator() || OtherActor == GetOwner()) return;
	if (Cast<AAmaruProjectile>(OtherActor)) return;

	bImpactHandled = true;

	if (ExplosionRadius > 0.f)
	{
		Explode(ImpactLocation);
	}
	else
	{
		ApplyDamageToActor(OtherActor, ImpactLocation);
	}

	Destroy();
}

void AAmaruProjectile::LifeSpanExpired()
{
	// Distancia máxima alcanzada: el Núcleo de Arcilla explota igualmente.
	if (HasAuthority() && !bImpactHandled && ExplosionRadius > 0.f)
	{
		bImpactHandled = true;
		Explode(GetActorLocation());
	}
	Super::LifeSpanExpired();
}

void AAmaruProjectile::Explode(const FVector& Location)
{
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	GetWorld()->OverlapMultiByObjectType(Overlaps, Location, FQuat::Identity, ObjParams, FCollisionShape::MakeSphere(ExplosionRadius));

	TSet<AActor*> Damaged;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (Target && Target != GetInstigator() && !Damaged.Contains(Target))
		{
			Damaged.Add(Target);
			ApplyDamageToActor(Target, Location);
		}
	}
}

bool AAmaruProjectile::ApplyDamageToActor(AActor* Target, const FVector& ImpactLocation)
{
	if (!Target) return false;

	if (AAmaruDestructible* Destructible = Cast<AAmaruDestructible>(Target))
	{
		Destructible->ReceiveObjectDamage(BaseDamage * DamageMultiplier, GetInstigator());
		return false;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC || !SourceASC.IsValid() || !DamageEffect) return false;

	if (UAmaruGameplayStatics::AreSameTeam(GetInstigator(), Target)) return false;

	FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
	Ctx.AddSourceObject(this);
	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffect, 1.f, Ctx);
	if (!Spec.IsValid()) return false;

	Spec.Data->SetSetByCallerMagnitude(AmaruTags::Data_Damage, BaseDamage * DamageMultiplier);
	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);

	ReportHitOnce();
	OnEnemyHit(TargetASC);
	return true;
}

void AAmaruProjectile::ReportHitOnce()
{
	if (bReportedHit) return;
	bReportedHit = true;

	if (SourceASC.IsValid())
	{
		if (AAmaruPlayerState* PS = UAmaruGameplayStatics::GetAmaruPlayerStateFromActor(SourceASC->GetOwnerActor()))
		{
			PS->AddShotHit();
		}
	}
}
