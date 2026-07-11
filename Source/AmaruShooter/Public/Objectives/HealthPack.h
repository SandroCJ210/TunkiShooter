#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "GameFramework/Actor.h"
#include "HealthPack.generated.h"

class UGameplayEffect;
class USphereComponent;

// Botiquín de mapa: cura al pasar por encima, se desactiva y reaparece.
UCLASS()
class AMARUSHOOTER_API AHealthPack : public AActor
{
	GENERATED_BODY()

public:
	AHealthPack();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Pickup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HealthPack")
	float HealAmount = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HealthPack")
	float RespawnCooldown = 15.f;

	UPROPERTY(EditAnywhere, Category = "HealthPack")
	TSubclassOf<UGameplayEffect> HealEffect;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_IsActive)
	bool bIsActive = true;

	UFUNCTION()
	void OnRep_IsActive();

	UFUNCTION()
	void OnPickupBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Reactivate();

	FTimerHandle RespawnTimerHandle;
};
