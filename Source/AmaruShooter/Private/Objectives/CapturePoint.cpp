#include "Objectives/CapturePoint.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AmaruGameplayStatics.h"
#include "AmaruGameplayTags.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

ACapturePoint::ACapturePoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	Zone->InitBoxExtent(FVector(400.f, 400.f, 200.f));
	Zone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Zone->SetCollisionResponseToAllChannels(ECR_Ignore);
	Zone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = Zone;
}

void ACapturePoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACapturePoint, CaptureProgress);
	DOREPLIFETIME(ACapturePoint, bCaptured);
}

void ACapturePoint::SetPointActive(int32 InAttackingTeamId)
{
	if (!HasAuthority()) return;
	AttackingTeamId = InAttackingTeamId;
	GetWorldTimerManager().SetTimer(CaptureTimerHandle, this, &ACapturePoint::TickCapture, TickInterval, true);
}

void ACapturePoint::SetPointInactive()
{
	if (!HasAuthority()) return;
	GetWorldTimerManager().ClearTimer(CaptureTimerHandle);
}

void ACapturePoint::ResetPoint()
{
	if (!HasAuthority()) return;
	SetPointInactive();
	CaptureProgress = 0.f;
	bCaptured = false;
	AttackingTeamId = INDEX_NONE;
}

void ACapturePoint::TickCapture()
{
	if (bCaptured) return;

	int32 Attackers = 0;
	int32 Defenders = 0;

	TArray<AActor*> Overlapping;
	Zone->GetOverlappingActors(Overlapping, APawn::StaticClass());
	for (const AActor* Actor : Overlapping)
	{
		const AAmaruPlayerState* PS = UAmaruGameplayStatics::GetAmaruPlayerStateFromActor(Actor);
		if (!PS || PS->GetTeamId() == INDEX_NONE) continue;

		// Los muertos no cuentan.
		if (const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(Actor)))
		{
			if (ASC->HasMatchingGameplayTag(AmaruTags::State_Dead)) continue;
		}

		if (PS->GetTeamId() == AttackingTeamId) ++Attackers;
		else ++Defenders;
	}

	// Avanza solo con atacantes presentes y sin defensores; contested = pausa.
	if (Attackers > 0 && Defenders == 0)
	{
		CaptureProgress = FMath::Min(1.f, CaptureProgress + TickInterval / CaptureTime);
		OnRep_Progress();

		if (CaptureProgress >= 1.f)
		{
			bCaptured = true;
			SetPointInactive();
			OnRep_Captured();
			OnCapturedNative.Broadcast();
		}
	}
}

void ACapturePoint::OnRep_Progress()
{
	OnProgressChanged.Broadcast(CaptureProgress);
}

void ACapturePoint::OnRep_Captured()
{
	if (bCaptured)
	{
		OnCaptured.Broadcast();
	}
}
