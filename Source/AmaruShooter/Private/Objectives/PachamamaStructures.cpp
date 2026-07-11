#include "Objectives/PachamamaStructures.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.h"
#include "AmaruGameplayStatics.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Effects/AmaruGameplayEffects.h"
#include "GameFramework/Pawn.h"

// ---------------- Muro ----------------

APachamamaWall::APachamamaWall()
{
	MaxHealth = 600.f;
	ShieldOnDamageEffect = UAmaruGE_ShieldOnDamage::StaticClass();
}

void APachamamaWall::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeTime);

	if (HasAuthority())
	{
		DamageDealtHandle = UAmaruAttributeSet::OnDamageDealt.AddUObject(this, &APachamamaWall::HandleDamageDealt);
	}
}

void APachamamaWall::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UAmaruAttributeSet::OnDamageDealt.Remove(DamageDealtHandle);
	Super::EndPlay(EndPlayReason);
}

void APachamamaWall::HandleDamageDealt(AAmaruPlayerState* Source, float Damage)
{
	if (!HasAuthority() || !Source || !ShieldOnDamageEffect) return;
	if (TeamId == INDEX_NONE || Source->GetTeamId() != TeamId) return;

	const APawn* SourcePawn = Source->GetPawn();
	if (!SourcePawn || FVector::DistSquared(SourcePawn->GetActorLocation(), GetActorLocation()) > FMath::Square(BuffRadius))
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = Source->GetAbilitySystemComponent())
	{
		FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
		Ctx.AddSourceObject(this);
		ASC->ApplyGameplayEffectToSelf(ShieldOnDamageEffect->GetDefaultObject<UGameplayEffect>(), 1.f, Ctx);
	}
}

// ---------------- Domo ----------------

APachamamaDome::APachamamaDome()
{
	MaxHealth = 800.f;
	AllyEffect = UAmaruGE_DomeProtect::StaticClass();
	EnemyEffect = UAmaruGE_DomeSlow::StaticClass();

	Zone = CreateDefaultSubobject<USphereComponent>(TEXT("Zone"));
	Zone->SetupAttachment(RootComponent);
	Zone->InitSphereRadius(600.f);
	Zone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Zone->SetCollisionResponseToAllChannels(ECR_Ignore);
	Zone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Zone->OnComponentBeginOverlap.AddDynamic(this, &APachamamaDome::OnZoneBeginOverlap);
	Zone->OnComponentEndOverlap.AddDynamic(this, &APachamamaDome::OnZoneEndOverlap);
}

void APachamamaDome::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeTime);
}

void APachamamaDome::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Al destruirse (por daño o expiración) se retiran todos los efectos.
	for (TPair<TWeakObjectPtr<UAbilitySystemComponent>, FActiveGameplayEffectHandle>& Pair : AppliedEffects)
	{
		if (UAbilitySystemComponent* ASC = Pair.Key.Get())
		{
			ASC->RemoveActiveGameplayEffect(Pair.Value);
		}
	}
	AppliedEffects.Reset();
	Super::EndPlay(EndPlayReason);
}

void APachamamaDome::OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	const AAmaruPlayerState* PS = UAmaruGameplayStatics::GetAmaruPlayerStateFromActor(Pawn);
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!PS || !ASC || AppliedEffects.Contains(ASC)) return;

	const bool bAlly = TeamId != INDEX_NONE && PS->GetTeamId() == TeamId;
	const TSubclassOf<UGameplayEffect> EffectClass = bAlly ? AllyEffect : EnemyEffect;
	if (!EffectClass) return;

	FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
	Ctx.AddSourceObject(this);
	const FActiveGameplayEffectHandle Handle =
		ASC->ApplyGameplayEffectToSelf(EffectClass->GetDefaultObject<UGameplayEffect>(), 1.f, Ctx);
	if (Handle.IsValid())
	{
		AppliedEffects.Add(ASC, Handle);
	}
}

void APachamamaDome::OnZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!ASC) return;

	if (FActiveGameplayEffectHandle* Handle = AppliedEffects.Find(ASC))
	{
		ASC->RemoveActiveGameplayEffect(*Handle);
		AppliedEffects.Remove(ASC);
	}
}
