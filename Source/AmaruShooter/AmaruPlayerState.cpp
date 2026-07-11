// Fill out your copyright notice in the Description page of Project Settings.


#include "AmaruPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.h"
#include "AmaruAbilitySystemComponent.h"
#include "InkaDataAsset.h"
#include "Net/UnrealNetwork.h"

AAmaruPlayerState::AAmaruPlayerState()
{
	SetNetUpdateFrequency(100.0f);
	SetMinNetUpdateFrequency(66.0f);
	bReplicates = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAmaruAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<UAmaruAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AAmaruPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAmaruAbilitySystemComponent* AAmaruPlayerState::GetAmaruAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAmaruPlayerState::OnRep_SelectedInka()
{
	OnInkaChanged.Broadcast(GetPlayerId());
}

void AAmaruPlayerState::ServerSetSelectedInka_Implementation(const TSoftObjectPtr<UInkaDataAsset>& NewInka)
{
	SetSelectedInka(NewInka);
}

void AAmaruPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAmaruPlayerState, SelectedInka);
	DOREPLIFETIME(AAmaruPlayerState, TeamId);
	DOREPLIFETIME(AAmaruPlayerState, Kills);
	DOREPLIFETIME(AAmaruPlayerState, Assists);
	DOREPLIFETIME(AAmaruPlayerState, Deaths);
	DOREPLIFETIME(AAmaruPlayerState, DamageMitigated);
	DOREPLIFETIME(AAmaruPlayerState, HealingDone);
	DOREPLIFETIME(AAmaruPlayerState, ShotsFired);
	DOREPLIFETIME(AAmaruPlayerState, ShotsHit);
}

void AAmaruPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AAmaruPlayerState* AmaruPlayerState = Cast<AAmaruPlayerState>(PlayerState))
	{
		AmaruPlayerState->SelectedInka = SelectedInka;
		AmaruPlayerState->TeamId = TeamId;
		// Las stats no se copian: partida nueva, marcador limpio.
	}
}

void AAmaruPlayerState::OnRep_TeamId()
{
	OnTeamChanged.Broadcast(TeamId);
}

void AAmaruPlayerState::SetTeamId(int32 NewTeamId)
{
	if (!HasAuthority() || TeamId == NewTeamId) return;
	TeamId = NewTeamId;
	OnRep_TeamId();
}

void AAmaruPlayerState::AddShotFired()
{
	if (HasAuthority()) ++ShotsFired;
}

void AAmaruPlayerState::AddShotHit()
{
	if (HasAuthority()) ++ShotsHit;
}

void AAmaruPlayerState::AddDamageMitigated(float Amount)
{
	if (HasAuthority() && Amount > 0.f) DamageMitigated += Amount;
}

void AAmaruPlayerState::AddHealingDone(float Amount)
{
	if (HasAuthority() && Amount > 0.f) HealingDone += Amount;
}

void AAmaruPlayerState::RecordDamageReceived(AAmaruPlayerState* Source)
{
	if (!HasAuthority() || !Source || Source == this) return;

	const double Now = GetWorld()->GetTimeSeconds();
	for (FAmaruDamageRecord& Record : RecentDamagers)
	{
		if (Record.Damager == Source)
		{
			Record.Timestamp = Now;
			return;
		}
	}
	RecentDamagers.Add({ Source, Now });
}

AAmaruPlayerState* AAmaruPlayerState::GetLastDamager(double Window) const
{
	const double Now = GetWorld()->GetTimeSeconds();
	AAmaruPlayerState* Last = nullptr;
	double LastTime = -1.0;
	for (const FAmaruDamageRecord& Record : RecentDamagers)
	{
		if (Record.Damager.IsValid() && Now - Record.Timestamp <= Window && Record.Timestamp > LastTime)
		{
			Last = Record.Damager.Get();
			LastTime = Record.Timestamp;
		}
	}
	return Last;
}

void AAmaruPlayerState::GetAssisters(double Window, const AAmaruPlayerState* Killer, TArray<AAmaruPlayerState*>& OutAssisters) const
{
	const double Now = GetWorld()->GetTimeSeconds();
	for (const FAmaruDamageRecord& Record : RecentDamagers)
	{
		if (Record.Damager.IsValid() && Record.Damager != Killer && Now - Record.Timestamp <= Window)
		{
			OutAssisters.Add(Record.Damager.Get());
		}
	}
}

void AAmaruPlayerState::ClearRecentDamagers()
{
	RecentDamagers.Reset();
}

void AAmaruPlayerState::SetSelectedInka(const TSoftObjectPtr<UInkaDataAsset>& NewInka)
{
	if (!HasAuthority())
	{
		ServerSetSelectedInka(NewInka);
		return;
	}

	SelectedInka = NewInka;
	OnRep_SelectedInka();
}
