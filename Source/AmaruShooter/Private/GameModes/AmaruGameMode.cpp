// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/AmaruGameMode.h"

#include "AbilitySystemComponent.h"
#include "AmaruGameplayStatics.h"
#include "AmaruGameplayTags.h"
#include "AmaruGameState.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "AmaruShooter/AmaruShooterCharacter.h"
#include "AmaruAttributeSet.h"
#include "Effects/AmaruGameplayEffects.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "TimerManager.h"

AAmaruGameMode::AAmaruGameMode()
	: Super()
{
	GameStateClass = AAmaruGameState::StaticClass();
	RageHealOnKillEffect = UAmaruGE_HealOnKill::StaticClass();
}

void AAmaruGameMode::GenericPlayerInitialization(AController* C)
{
	Super::GenericPlayerInitialization(C);

	if (AAmaruPlayerState* PS = C ? C->GetPlayerState<AAmaruPlayerState>() : nullptr)
	{
		if (PS->GetTeamId() == INDEX_NONE)
		{
			AssignTeam(PS);
		}
	}
}

void AAmaruGameMode::AssignTeam(AAmaruPlayerState* PS)
{
	if (!PS) return;

	// Balance simple: el equipo con menos miembros.
	TArray<int32> Counts;
	Counts.SetNumZeroed(FMath::Max(NumTeams, 1));
	for (APlayerState* Other : GameState->PlayerArray)
	{
		const AAmaruPlayerState* OtherPS = Cast<AAmaruPlayerState>(Other);
		if (OtherPS && OtherPS != PS && Counts.IsValidIndex(OtherPS->GetTeamId()))
		{
			++Counts[OtherPS->GetTeamId()];
		}
	}

	int32 BestTeam = 0;
	for (int32 i = 1; i < Counts.Num(); ++i)
	{
		if (Counts[i] < Counts[BestTeam])
		{
			BestTeam = i;
		}
	}
	PS->SetTeamId(BestTeam);
}

FName AAmaruGameMode::GetPlayerStartTagForPlayer(const AAmaruPlayerState* PS) const
{
	return PS ? FName(*FString::Printf(TEXT("Team%d"), PS->GetTeamId())) : NAME_None;
}

AActor* AAmaruGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	const AAmaruPlayerState* PS = Player ? Player->GetPlayerState<AAmaruPlayerState>() : nullptr;
	const FName WantedTag = GetPlayerStartTagForPlayer(PS);

	if (!WantedTag.IsNone())
	{
		TArray<APlayerStart*> Matching;
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			if (It->PlayerStartTag == WantedTag)
			{
				Matching.Add(*It);
			}
		}
		if (Matching.Num() > 0)
		{
			return Matching[FMath::RandRange(0, Matching.Num() - 1)];
		}
	}
	return Super::ChoosePlayerStart_Implementation(Player);
}

void AAmaruGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (AAmaruGameState* GS = GetGameState<AAmaruGameState>())
	{
		GS->TeamScores.SetNumZeroed(FMath::Max(NumTeams, 1));
		GS->MatchEndServerTime = GS->GetServerWorldTimeSeconds() + MatchDuration;
	}
	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &AAmaruGameMode::OnMatchTimeExpired, MatchDuration);
}

void AAmaruGameMode::OnMatchTimeExpired()
{
	EndMatch();
}

void AAmaruGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	if (NewPlayer && NewPlayer->GetPawn())
	{
		ApplyModeRulesToPawn(NewPlayer->GetPawn());
	}
}

void AAmaruGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (!NewPlayer || !NewPlayer->GetPawn()) return;

	ApplyModeRulesToPawn(NewPlayer->GetPawn());

	if (AAmaruPlayerState* PS = NewPlayer->GetPlayerState<AAmaruPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			FGameplayTagContainer TagContainer;
			TagContainer.AddTag(AmaruTags::State_Dead);
			UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(ASC->GetOwner(), TagContainer, true);
		}
		if (UAmaruAttributeSet* AS = PS->GetAttributeSet())
		{
			AS->ResetVitals();
		}
	}
}

void AAmaruGameMode::ApplyModeRulesToPawn(APawn* Pawn)
{
	if (AAmaruShooterCharacter* Char = Cast<AAmaruShooterCharacter>(Pawn))
	{
		Char->Server_EnableAbilitiesForMode();
	}
}

void AAmaruGameMode::NotifyPlayerDied(AAmaruPlayerState* VictimPS, AActor* ContextInstigator)
{
	if (!VictimPS || !IsMatchInProgress()) return;

	VictimPS->Deaths++;

	AAmaruPlayerState* Killer = VictimPS->GetLastDamager(AssistWindow);
	if (!Killer)
	{
		Killer = UAmaruGameplayStatics::GetAmaruPlayerStateFromActor(ContextInstigator);
	}
	if (Killer == VictimPS)
	{
		Killer = nullptr;
	}

	if (Killer)
	{
		Killer->Kills++;
		OnPlayerKilled(Killer, VictimPS);
	}

	TArray<AAmaruPlayerState*> Assisters;
	VictimPS->GetAssisters(AssistWindow, Killer, Assisters);
	for (AAmaruPlayerState* Assister : Assisters)
	{
		Assister->Assists++;
	}
	VictimPS->ClearRecentDamagers();

	// Marcar muerto y desmontar el pawn.
	if (UAbilitySystemComponent* ASC = VictimPS->GetAbilitySystemComponent())
	{
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(AmaruTags::State_Dead);
		UAbilitySystemBlueprintLibrary::AddLooseGameplayTags(ASC->GetOwner(), TagContainer, true);
		ASC->CancelAllAbilities();
	}

	AController* Controller = nullptr;
	if (APawn* Pawn = VictimPS->GetPawn())
	{
		Controller = Pawn->GetController();
		if (AAmaruShooterCharacter* Char = Cast<AAmaruShooterCharacter>(Pawn))
		{
			Char->Multicast_OnDeath();
		}
		Pawn->DetachFromControllerPendingDestroy();
		Pawn->SetLifeSpan(RespawnDelay);
	}

	if (Controller)
	{
		ScheduleRespawn(Controller);
	}
}

void AAmaruGameMode::OnPlayerKilled(AAmaruPlayerState* Killer, AAmaruPlayerState* Victim)
{
	UAbilitySystemComponent* KillerASC = Killer ? Killer->GetAbilitySystemComponent() : nullptr;
	if (!KillerASC) return;

	// Furia del Yawar: curación al confirmar una kill en rage.
	if (RageHealOnKillEffect && KillerASC->HasMatchingGameplayTag(AmaruTags::State_Rage))
	{
		FGameplayEffectContextHandle Ctx = KillerASC->MakeEffectContext();
		Ctx.AddSourceObject(Killer);
		KillerASC->ApplyGameplayEffectToSelf(RageHealOnKillEffect->GetDefaultObject<UGameplayEffect>(), 1.f, Ctx);
	}

	FGameplayEventData Payload;
	Payload.EventTag = AmaruTags::Event_Kill;
	Payload.Instigator = Killer;
	Payload.Target = Victim;
	KillerASC->HandleGameplayEvent(AmaruTags::Event_Kill, &Payload);
}

void AAmaruGameMode::ScheduleRespawn(AController* Controller)
{
	if (!Controller) return;

	FTimerHandle& Handle = PendingRespawns.FindOrAdd(Controller);
	TWeakObjectPtr<AController> WeakController = Controller;
	GetWorldTimerManager().SetTimer(Handle, [this, WeakController]()
	{
		AController* C = WeakController.Get();
		if (C && IsMatchInProgress() && !C->GetPawn())
		{
			RestartPlayer(C);
		}
		if (C)
		{
			PendingRespawns.Remove(C);
		}
	}, RespawnDelay, false);
}

void AAmaruGameMode::CancelPendingRespawns()
{
	for (TPair<TWeakObjectPtr<AController>, FTimerHandle>& Pair : PendingRespawns)
	{
		GetWorldTimerManager().ClearTimer(Pair.Value);
	}
	PendingRespawns.Reset();
}
