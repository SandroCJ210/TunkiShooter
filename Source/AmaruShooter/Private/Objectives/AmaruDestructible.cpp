#include "Objectives/AmaruDestructible.h"

#include "AmaruGameplayStatics.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

AAmaruDestructible::AAmaruDestructible()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
}

void AAmaruDestructible::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
}

void AAmaruDestructible::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAmaruDestructible, Health);
	DOREPLIFETIME(AAmaruDestructible, TeamId);
}

void AAmaruDestructible::ReceiveObjectDamage(float Amount, const AActor* Source)
{
	if (!HasAuthority() || Amount <= 0.f || Health <= 0.f) return;

	if (const AAmaruPlayerState* SourcePS = UAmaruGameplayStatics::GetAmaruPlayerStateFromActor(Source))
	{
		if (TeamId != INDEX_NONE && SourcePS->GetTeamId() == TeamId)
		{
			return;
		}
	}

	Health = FMath::Max(0.f, Health - Amount);
	if (Health <= 0.f)
	{
		OnDestructibleDestroyed.Broadcast();
		Destroy();
	}
}
