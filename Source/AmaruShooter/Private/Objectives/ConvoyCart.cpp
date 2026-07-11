#include "Objectives/ConvoyCart.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AmaruGameplayStatics.h"
#include "AmaruGameplayTags.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

AConvoyCart::AConvoyCart()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetNetUpdateFrequency(20.f);

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Track = CreateDefaultSubobject<USplineComponent>(TEXT("Track"));
	Track->SetupAttachment(Root);

	CartMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CartMesh"));
	CartMesh->SetupAttachment(Root);
	CartMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	EscortZone = CreateDefaultSubobject<USphereComponent>(TEXT("EscortZone"));
	EscortZone->SetupAttachment(CartMesh);
	EscortZone->InitSphereRadius(400.f);
	EscortZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EscortZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	EscortZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AConvoyCart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AConvoyCart, bLocked);
	DOREPLIFETIME(AConvoyCart, DistanceAlongSpline);
	DOREPLIFETIME(AConvoyCart, CheckpointsReached);
	DOREPLIFETIME(AConvoyCart, bMoving);
}

void AConvoyCart::BeginPlay()
{
	Super::BeginPlay();

	// Checkpoints por defecto: los puntos intermedios del spline.
	if (CheckpointDistances.Num() == 0 && Track->GetNumberOfSplinePoints() > 2)
	{
		for (int32 i = 1; i < Track->GetNumberOfSplinePoints() - 1; ++i)
		{
			CheckpointDistances.Add(Track->GetDistanceAlongSplineAtSplinePoint(i));
		}
	}
	CheckpointDistances.Sort();

	MoveCartTo(0.f);
}

void AConvoyCart::UnlockCart(int32 InAttackingTeamId)
{
	if (!HasAuthority()) return;
	AttackingTeamId = InAttackingTeamId;
	bLocked = false;
}

void AConvoyCart::NetMulticast_ResetCart_Implementation()
{
	bLocked = true;
	bMoving = false;
	DistanceAlongSpline = 0.f;
	CheckpointsReached = 0;
	ClientCurrentDistance = 0.f;
	AttackingTeamId = INDEX_NONE;
	MoveCartTo(0.f);
}

void AConvoyCart::MoveCartTo(float Distance)
{
	const FTransform CartTransform = Track->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	CartMesh->SetWorldLocationAndRotation(CartTransform.GetLocation(), CartTransform.GetRotation());
}

void AConvoyCart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		if (bLocked) return;

		// Avanza con >=1 atacante vivo escoltando y 0 defensores en la zona.
		int32 Attackers = 0;
		int32 Defenders = 0;
		TArray<AActor*> Overlapping;
		EscortZone->GetOverlappingActors(Overlapping, APawn::StaticClass());
		for (const AActor* Actor : Overlapping)
		{
			const AAmaruPlayerState* PS = UAmaruGameplayStatics::GetAmaruPlayerStateFromActor(Actor);
			if (!PS || PS->GetTeamId() == INDEX_NONE) continue;

			if (const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(Actor)))
			{
				if (ASC->HasMatchingGameplayTag(AmaruTags::State_Dead)) continue;
			}

			if (PS->GetTeamId() == AttackingTeamId) ++Attackers;
			else ++Defenders;
		}

		bMoving = Attackers > 0 && Defenders == 0;
		if (!bMoving) return;

		const float SplineLength = Track->GetSplineLength();
		DistanceAlongSpline = FMath::Min(SplineLength, DistanceAlongSpline + Speed * DeltaTime);
		MoveCartTo(DistanceAlongSpline);

		while (CheckpointDistances.IsValidIndex(CheckpointsReached) &&
			DistanceAlongSpline >= CheckpointDistances[CheckpointsReached])
		{
			++CheckpointsReached;
			OnCheckpointReached.Broadcast(CheckpointsReached);
			OnCheckpointReachedNative.Broadcast(CheckpointsReached);
		}

		if (DistanceAlongSpline >= SplineLength)
		{
			bLocked = true;
			bMoving = false;
			OnReachedEndNative.Broadcast();
		}
	}
	else
	{
		// Cliente: interpolar suavemente hacia la distancia replicada.
		ClientCurrentDistance = FMath::FInterpConstantTo(ClientCurrentDistance, DistanceAlongSpline, DeltaTime, Speed * 1.5f);
		MoveCartTo(ClientCurrentDistance);
	}
}
