#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ConvoyCart.generated.h"

class USplineComponent;
class USphereComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCheckpointReachedNative, int32 /*CheckpointIndex*/);
DECLARE_MULTICAST_DELEGATE(FOnConvoyReachedEndNative);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCheckpointReached, int32, CheckpointIndex);

// Carro del convoy: el actor no se mueve; el CartMesh recorre el spline
// (data de nivel idéntica en server y clientes; solo se replica la distancia).
UCLASS()
class AMARUSHOOTER_API AConvoyCart : public AActor
{
	GENERATED_BODY()

public:
	AConvoyCart();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USplineComponent> Track;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> CartMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> EscortZone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Convoy")
	float Speed = 150.f;

	// Distancias de checkpoint sobre el spline; vacío = autogenerar de los
	// puntos intermedios del spline en BeginPlay.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Convoy")
	TArray<float> CheckpointDistances;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Convoy")
	bool bLocked = true;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Convoy")
	float DistanceAlongSpline = 0.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Convoy")
	int32 CheckpointsReached = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Convoy")
	bool bMoving = false;

	UPROPERTY(BlueprintAssignable)
	FOnCheckpointReached OnCheckpointReached;

	FOnCheckpointReachedNative OnCheckpointReachedNative;
	FOnConvoyReachedEndNative OnReachedEndNative;

	// Servidor: activa el escort tras capturar el punto.
	void UnlockCart(int32 InAttackingTeamId);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_ResetCart();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	void MoveCartTo(float Distance);

	int32 AttackingTeamId = INDEX_NONE;

	// Distancia visual interpolada en clientes (la replicada es la meta).
	float ClientCurrentDistance = 0.f;
};
