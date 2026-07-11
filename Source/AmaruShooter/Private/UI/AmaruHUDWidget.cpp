#include "UI/AmaruHUDWidget.h"

#include "AbilitySystemComponent.h"
#include "AmaruAttributeSet.h"
#include "AmaruShooter/AmaruPlayerState.h"
#include "EngineUtils.h"
#include "GameplayEffectTypes.h"
#include "Objectives/CapturePoint.h"
#include "Objectives/ConvoyCart.h"
#include "Components/SplineComponent.h"
#include "TimerManager.h"

void UAmaruHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	TryBindToAbilitySystem();
}

void UAmaruHUDWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BindRetryHandle);
	}
	UnbindFromAbilitySystem();
	Super::NativeDestruct();
}

void UAmaruHUDWidget::TryBindToAbilitySystem()
{
	AAmaruPlayerState* PS = GetOwningPlayer() ? GetOwningPlayer()->GetPlayerState<AAmaruPlayerState>() : nullptr;
	UAmaruAttributeSet* AS = PS ? PS->GetAttributeSet() : nullptr;
	UAbilitySystemComponent* ASC = PS ? PS->GetAbilitySystemComponent() : nullptr;

	if (!PS || !AS || !ASC)
	{
		// En cliente el PlayerState puede tardar en replicar: reintentar.
		GetWorld()->GetTimerManager().SetTimer(BindRetryHandle, this, &UAmaruHUDWidget::TryBindToAbilitySystem, 0.25f, false);
		return;
	}

	UnbindFromAbilitySystem();
	BoundPS = PS;
	BoundAS = AS;
	BoundASC = ASC;

	const TArray<FGameplayAttribute> Watched = {
		AS->GetHealthAttribute(), AS->GetMaxHealthAttribute(),
		AS->GetShieldAttribute(), AS->GetMaxShieldAttribute(),
		AS->GetAmmoAttribute(), AS->GetMaxAmmoAttribute(),
		AS->GetUltimateChargeAttribute(), AS->GetMaxUltimateChargeAttribute(),
		AS->GetChargeAbility1Attribute(), AS->GetMaxChargeAbility1Attribute(),
		AS->GetChargeAbility2Attribute(), AS->GetMaxChargeAbility2Attribute()
	};
	for (const FGameplayAttribute& Attr : Watched)
	{
		const FDelegateHandle Handle = ASC->GetGameplayAttributeValueChangeDelegate(Attr)
			.AddUObject(this, &UAmaruHUDWidget::HandleAttributeChanged);
		AttributeBindings.Add({ Attr, Handle });
	}

	PS->OnTeamChanged.AddUniqueDynamic(this, &UAmaruHUDWidget::HandleTeamChanged);

	BroadcastInitialValues();
	OnHUDReady();
}

void UAmaruHUDWidget::UnbindFromAbilitySystem()
{
	if (UAbilitySystemComponent* ASC = BoundASC.Get())
	{
		for (TPair<FGameplayAttribute, FDelegateHandle>& Binding : AttributeBindings)
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Binding.Key).Remove(Binding.Value);
		}
	}
	AttributeBindings.Reset();

	if (AAmaruPlayerState* PS = BoundPS.Get())
	{
		PS->OnTeamChanged.RemoveDynamic(this, &UAmaruHUDWidget::HandleTeamChanged);
	}

	BoundPS = nullptr;
	BoundAS = nullptr;
	BoundASC = nullptr;
}

void UAmaruHUDWidget::BroadcastInitialValues()
{
	const UAmaruAttributeSet* AS = BoundAS.Get();
	if (!AS) return;

	OnHealthChanged(AS->GetHealth(), AS->GetMaxHealth());
	OnShieldChanged(AS->GetShield(), AS->GetMaxShield());
	OnAmmoChanged(AS->GetAmmo(), AS->GetMaxAmmo());
	OnUltimateChargeChanged(AS->GetUltimateCharge(), AS->GetMaxUltimateCharge());
	OnAbilityChargeChanged(EAmaruAbilityInputID::Ability1, AS->GetChargeAbility1(), AS->GetMaxChargeAbility1());
	OnAbilityChargeChanged(EAmaruAbilityInputID::Ability2, AS->GetChargeAbility2(), AS->GetMaxChargeAbility2());

	if (const AAmaruPlayerState* PS = BoundPS.Get())
	{
		if (PS->GetTeamId() != INDEX_NONE)
		{
			OnTeamAssigned(PS->GetTeamId());
		}
	}
}

void UAmaruHUDWidget::HandleAttributeChanged(const FOnAttributeChangeData& Data)
{
	const UAmaruAttributeSet* AS = BoundAS.Get();
	if (!AS) return;

	const FGameplayAttribute& Attr = Data.Attribute;

	if (Attr == AS->GetHealthAttribute() || Attr == AS->GetMaxHealthAttribute())
	{
		OnHealthChanged(AS->GetHealth(), AS->GetMaxHealth());
	}
	else if (Attr == AS->GetShieldAttribute() || Attr == AS->GetMaxShieldAttribute())
	{
		OnShieldChanged(AS->GetShield(), AS->GetMaxShield());
	}
	else if (Attr == AS->GetAmmoAttribute() || Attr == AS->GetMaxAmmoAttribute())
	{
		OnAmmoChanged(AS->GetAmmo(), AS->GetMaxAmmo());
	}
	else if (Attr == AS->GetUltimateChargeAttribute() || Attr == AS->GetMaxUltimateChargeAttribute())
	{
		OnUltimateChargeChanged(AS->GetUltimateCharge(), AS->GetMaxUltimateCharge());
	}
	else if (Attr == AS->GetChargeAbility1Attribute() || Attr == AS->GetMaxChargeAbility1Attribute())
	{
		OnAbilityChargeChanged(EAmaruAbilityInputID::Ability1, AS->GetChargeAbility1(), AS->GetMaxChargeAbility1());
	}
	else if (Attr == AS->GetChargeAbility2Attribute() || Attr == AS->GetMaxChargeAbility2Attribute())
	{
		OnAbilityChargeChanged(EAmaruAbilityInputID::Ability2, AS->GetChargeAbility2(), AS->GetMaxChargeAbility2());
	}
}

void UAmaruHUDWidget::HandleTeamChanged(int32 NewTeamId)
{
	OnTeamAssigned(NewTeamId);
}

AAmaruPlayerState* UAmaruHUDWidget::GetAmaruPlayerState() const
{
	if (AAmaruPlayerState* PS = BoundPS.Get())
	{
		return PS;
	}
	return GetOwningPlayer() ? GetOwningPlayer()->GetPlayerState<AAmaruPlayerState>() : nullptr;
}

bool UAmaruHUDWidget::GetCooldownRemaining(FGameplayTag CooldownTag, float& TimeRemaining, float& Duration) const
{
	TimeRemaining = 0.f;
	Duration = 0.f;

	UAbilitySystemComponent* ASC = BoundASC.Get();
	if (!ASC || !CooldownTag.IsValid()) return false;

	const FGameplayEffectQuery Query =
		FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(CooldownTag));
	const TArray<TPair<float, float>> Durations = ASC->GetActiveEffectsTimeRemainingAndDuration(Query);

	for (const TPair<float, float>& Pair : Durations)
	{
		if (Pair.Key > TimeRemaining)
		{
			TimeRemaining = Pair.Key;
			Duration = Pair.Value;
		}
	}
	return TimeRemaining > 0.f;
}

AAmaruGameState* UAmaruHUDWidget::GetAmaruGameState() const
{
	return GetWorld() ? GetWorld()->GetGameState<AAmaruGameState>() : nullptr;
}

float UAmaruHUDWidget::GetMatchTimeRemaining() const
{
	const AAmaruGameState* GS = GetAmaruGameState();
	return GS ? GS->GetMatchTimeRemaining() : 0.f;
}

int32 UAmaruHUDWidget::GetWinningTeam() const
{
	const AAmaruGameState* GS = GetAmaruGameState();
	return GS ? GS->WinningTeam : INDEX_NONE;
}

APlayerState* UAmaruHUDWidget::GetWinningPlayer() const
{
	const AAmaruGameState* GS = GetAmaruGameState();
	return GS ? GS->WinningPlayer.Get() : nullptr;
}

EConvoyPhase UAmaruHUDWidget::GetConvoyPhase() const
{
	const AAmaruGameState* GS = GetAmaruGameState();
	return GS ? GS->ConvoyPhase : EConvoyPhase::None;
}

int32 UAmaruHUDWidget::GetCurrentRound() const
{
	const AAmaruGameState* GS = GetAmaruGameState();
	return GS ? GS->CurrentRound : 0;
}

int32 UAmaruHUDWidget::GetAttackingTeamId() const
{
	const AAmaruGameState* GS = GetAmaruGameState();
	return GS ? GS->AttackingTeamId : INDEX_NONE;
}

bool UAmaruHUDWidget::IsLocalPlayerAttacking() const
{
	const AAmaruPlayerState* PS = GetAmaruPlayerState();
	return PS && PS->GetTeamId() != INDEX_NONE && PS->GetTeamId() == GetAttackingTeamId();
}

ACapturePoint* UAmaruHUDWidget::GetCapturePointActor() const
{
	if (!CachedCapturePoint.IsValid() && GetWorld())
	{
		for (TActorIterator<ACapturePoint> It(GetWorld()); It; ++It)
		{
			CachedCapturePoint = *It;
			break;
		}
	}
	return CachedCapturePoint.Get();
}

AConvoyCart* UAmaruHUDWidget::GetConvoyCartActor() const
{
	if (!CachedConvoyCart.IsValid() && GetWorld())
	{
		for (TActorIterator<AConvoyCart> It(GetWorld()); It; ++It)
		{
			CachedConvoyCart = *It;
			break;
		}
	}
	return CachedConvoyCart.Get();
}

float UAmaruHUDWidget::GetCaptureProgress() const
{
	const ACapturePoint* Point = GetCapturePointActor();
	return Point ? Point->CaptureProgress : 0.f;
}

float UAmaruHUDWidget::GetConvoyProgress() const
{
	const AConvoyCart* Cart = GetConvoyCartActor();
	if (!Cart || !Cart->Track) return 0.f;

	const float Length = Cart->Track->GetSplineLength();
	return Length > 0.f ? Cart->DistanceAlongSpline / Length : 0.f;
}

bool UAmaruHUDWidget::IsConvoyMoving() const
{
	const AConvoyCart* Cart = GetConvoyCartActor();
	return Cart && Cart->bMoving;
}

// ---- Scoreboard ----

TArray<FAmaruScoreRow> UAmaruScoreboardWidget::GetScoreRows() const
{
	TArray<FAmaruScoreRow> Rows;

	const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (!GS) return Rows;

	const APlayerState* LocalPS = GetOwningPlayer() ? GetOwningPlayer()->PlayerState : nullptr;

	for (APlayerState* PS : GS->PlayerArray)
	{
		const AAmaruPlayerState* APS = Cast<AAmaruPlayerState>(PS);
		if (!APS) continue;

		FAmaruScoreRow Row;
		Row.PlayerName = APS->GetPlayerName();
		Row.TeamId = APS->GetTeamId();
		Row.Kills = APS->Kills;
		Row.Assists = APS->Assists;
		Row.Deaths = APS->Deaths;
		Row.Accuracy = APS->GetAccuracy();
		Row.HealingDone = APS->HealingDone;
		Row.DamageMitigated = APS->DamageMitigated;
		Row.bIsLocalPlayer = APS == LocalPS;
		Rows.Add(Row);
	}

	Rows.Sort([](const FAmaruScoreRow& A, const FAmaruScoreRow& B)
	{
		if (A.TeamId != B.TeamId) return A.TeamId < B.TeamId;
		if (A.Kills != B.Kills) return A.Kills > B.Kills;
		return A.Deaths < B.Deaths;
	});

	return Rows;
}
