// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AmaruPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInkaChanged, int32, PlayerIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamChanged, int32, NewTeamId);

/**
 *
 */

class UAmaruAttributeSet;
class UAmaruAbilitySystemComponent;
class UAttributeSet;
class UInkaDataAsset;

UCLASS()
class AMARUSHOOTER_API AAmaruPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAmaruPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UAmaruAbilitySystemComponent* GetAmaruAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GAS")
	UAmaruAttributeSet* GetAttributeSet() const { return AttributeSet;}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAmaruAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAmaruAttributeSet> AttributeSet;

	UFUNCTION()
	void OnRep_SelectedInka();

	UFUNCTION(Server, Reliable)
	void ServerSetSelectedInka(const TSoftObjectPtr<UInkaDataAsset>& NewInka);

	UPROPERTY(ReplicatedUsing = OnRep_TeamId, BlueprintReadOnly, Category = "Team", meta = (AllowPrivateAccess = "true"))
	int32 TeamId = INDEX_NONE;

	UFUNCTION()
	void OnRep_TeamId();

	// Historial reciente de daño recibido (solo servidor) para kills/assists.
	struct FAmaruDamageRecord
	{
		TWeakObjectPtr<AAmaruPlayerState> Damager;
		double Timestamp = 0.0;
	};
	TArray<FAmaruDamageRecord> RecentDamagers;

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_SelectedInka, BlueprintReadOnly, Category = "Inka")
	TSoftObjectPtr<UInkaDataAsset> SelectedInka;

	UFUNCTION(BlueprintCallable, Category = "Inka")
	void SetSelectedInka(const TSoftObjectPtr<UInkaDataAsset>& NewInka);

	FOnInkaChanged OnInkaChanged;

	// ---- Equipos ----

	UPROPERTY(BlueprintAssignable, Category = "Team")
	FOnTeamChanged OnTeamChanged;

	UFUNCTION(BlueprintPure, Category = "Team")
	int32 GetTeamId() const { return TeamId; }

	void SetTeamId(int32 NewTeamId);

	// ---- Stats de partida (escritura solo servidor) ----

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Kills = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Assists = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 Deaths = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	float DamageMitigated = 0.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	float HealingDone = 0.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 ShotsFired = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	int32 ShotsHit = 0;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddShotFired();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddShotHit();

	void AddDamageMitigated(float Amount);
	void AddHealingDone(float Amount);

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetAccuracy() const { return ShotsFired > 0 ? static_cast<float>(ShotsHit) / ShotsFired : 0.f; }

	// ---- Atribución de kills (solo servidor) ----

	void RecordDamageReceived(AAmaruPlayerState* Source);
	AAmaruPlayerState* GetLastDamager(double Window) const;
	void GetAssisters(double Window, const AAmaruPlayerState* Killer, TArray<AAmaruPlayerState*>& OutAssisters) const;
	void ClearRecentDamagers();
};
