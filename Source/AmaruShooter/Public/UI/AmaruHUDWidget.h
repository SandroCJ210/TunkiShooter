#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "Engine/TimerHandle.h"
#include "GameplayTagContainer.h"
#include "AmaruShooter/Enums.h"
#include "AmaruGameState.h"
#include "AmaruHUDWidget.generated.h"

class AAmaruPlayerState;
class ACapturePoint;
class AConvoyCart;
class UAbilitySystemComponent;
class UAmaruAttributeSet;
struct FOnAttributeChangeData;

/**
 * Base C++ del HUD: se auto-vincula al ASC del PlayerState local (con reintento
 * hasta que replique) y traduce GAS a eventos tipados. El Blueprint hijo solo
 * implementa los eventos On* con la parte estética. Sustituye al binding por
 * ProcessEvent("SetupAbilitySystemComponent").
 */
UCLASS(Abstract)
class AMARUSHOOTER_API UAmaruHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ---- Eventos para el Blueprint (solo estética) ----

	// Disparado una vez cuando PS/ASC están listos (cargar iconos, loadout, etc.).
	UFUNCTION(BlueprintImplementableEvent, Category = "Amaru|HUD")
	void OnHUDReady();

	UFUNCTION(BlueprintImplementableEvent, Category = "Amaru|HUD")
	void OnHealthChanged(float Current, float Max);

	UFUNCTION(BlueprintImplementableEvent, Category = "Amaru|HUD")
	void OnShieldChanged(float Current, float Max);

	UFUNCTION(BlueprintImplementableEvent, Category = "Amaru|HUD")
	void OnAmmoChanged(float Current, float Max);

	UFUNCTION(BlueprintImplementableEvent, Category = "Amaru|HUD")
	void OnUltimateChargeChanged(float Current, float Max);

	UFUNCTION(BlueprintImplementableEvent, Category = "Amaru|HUD")
	void OnAbilityChargeChanged(EAmaruAbilityInputID InputID, float Current, float Max);

	UFUNCTION(BlueprintImplementableEvent, Category = "Amaru|HUD")
	void OnTeamAssigned(int32 TeamId);

	// ---- Getters puros para bindings de UMG ----

	UFUNCTION(BlueprintPure, Category = "Amaru|HUD")
	AAmaruPlayerState* GetAmaruPlayerState() const;

	// Restante y duración del cooldown activo con ese tag (Cooldown.Atoq.Primary, etc.).
	// false si no hay cooldown activo.
	UFUNCTION(BlueprintPure, Category = "Amaru|HUD")
	bool GetCooldownRemaining(FGameplayTag CooldownTag, float& TimeRemaining, float& Duration) const;

	UFUNCTION(BlueprintPure, Category = "Amaru|Match")
	float GetMatchTimeRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Amaru|Match")
	int32 GetWinningTeam() const;

	UFUNCTION(BlueprintPure, Category = "Amaru|Match")
	APlayerState* GetWinningPlayer() const;

	// ---- Convoy ----

	UFUNCTION(BlueprintPure, Category = "Amaru|Convoy")
	EConvoyPhase GetConvoyPhase() const;

	UFUNCTION(BlueprintPure, Category = "Amaru|Convoy")
	int32 GetCurrentRound() const;

	UFUNCTION(BlueprintPure, Category = "Amaru|Convoy")
	int32 GetAttackingTeamId() const;

	UFUNCTION(BlueprintPure, Category = "Amaru|Convoy")
	bool IsLocalPlayerAttacking() const;

	// Progreso 0..1 de la captura del punto inicial.
	UFUNCTION(BlueprintPure, Category = "Amaru|Convoy")
	float GetCaptureProgress() const;

	// Progreso 0..1 del carro sobre el spline.
	UFUNCTION(BlueprintPure, Category = "Amaru|Convoy")
	float GetConvoyProgress() const;

	UFUNCTION(BlueprintPure, Category = "Amaru|Convoy")
	bool IsConvoyMoving() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void TryBindToAbilitySystem();
	void UnbindFromAbilitySystem();
	void BroadcastInitialValues();
	void HandleAttributeChanged(const FOnAttributeChangeData& Data);

	UFUNCTION()
	void HandleTeamChanged(int32 NewTeamId);

	AAmaruGameState* GetAmaruGameState() const;
	ACapturePoint* GetCapturePointActor() const;
	AConvoyCart* GetConvoyCartActor() const;

	TWeakObjectPtr<AAmaruPlayerState> BoundPS;
	TWeakObjectPtr<UAmaruAttributeSet> BoundAS;
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;

	TArray<TPair<FGameplayAttribute, FDelegateHandle>> AttributeBindings;
	FTimerHandle BindRetryHandle;

	// Cache de actores de objetivo (búsqueda perezosa una sola vez).
	mutable TWeakObjectPtr<ACapturePoint> CachedCapturePoint;
	mutable TWeakObjectPtr<AConvoyCart> CachedConvoyCart;
};

// ---- Scoreboard ----

USTRUCT(BlueprintType)
struct FAmaruScoreRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	int32 TeamId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Assists = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Deaths = 0;

	UPROPERTY(BlueprintReadOnly)
	float Accuracy = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float HealingDone = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float DamageMitigated = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLocalPlayer = false;
};

/**
 * Base del marcador: entrega las filas ya ordenadas (equipo, luego kills);
 * el Blueprint solo las pinta.
 */
UCLASS(Abstract)
class AMARUSHOOTER_API UAmaruScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Amaru|Scoreboard")
	TArray<FAmaruScoreRow> GetScoreRows() const;
};
