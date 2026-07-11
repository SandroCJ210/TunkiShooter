#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "GameFramework/Actor.h"
#include "CapturePoint.generated.h"

class UBoxComponent;

DECLARE_MULTICAST_DELEGATE(FOnCapturePointCapturedNative);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCapturePointCaptured);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCaptureProgressChanged, float, Progress);

// Punto estático inicial del modo Convoy: los atacantes lo capturan
// permaneciendo dentro sin defensores presentes.
UCLASS()
class AMARUSHOOTER_API ACapturePoint : public AActor
{
	GENERATED_BODY()

public:
	ACapturePoint();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> Zone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Capture")
	float CaptureTime = 15.f;

	UPROPERTY(ReplicatedUsing = OnRep_Progress, BlueprintReadOnly, Category = "Capture")
	float CaptureProgress = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_Captured, BlueprintReadOnly, Category = "Capture")
	bool bCaptured = false;

	UPROPERTY(BlueprintAssignable)
	FOnCapturePointCaptured OnCaptured;

	UPROPERTY(BlueprintAssignable)
	FOnCaptureProgressChanged OnProgressChanged;

	FOnCapturePointCapturedNative OnCapturedNative;

	// Servidor: activa/desactiva la captura para el equipo atacante dado.
	void SetPointActive(int32 InAttackingTeamId);
	void SetPointInactive();
	void ResetPoint();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	void TickCapture();

	UFUNCTION()
	void OnRep_Progress();

	UFUNCTION()
	void OnRep_Captured();

	FTimerHandle CaptureTimerHandle;
	int32 AttackingTeamId = INDEX_NONE;

	static constexpr float TickInterval = 0.1f;
};
