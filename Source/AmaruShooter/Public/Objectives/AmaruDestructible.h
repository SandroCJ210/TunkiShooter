#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmaruDestructible.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDestructibleDestroyed);

// Actor destructible por armas (muro/domo de Pachamama). No usa ASC:
// hitscan y proyectiles llaman ReceiveObjectDamage directamente.
UCLASS()
class AMARUSHOOTER_API AAmaruDestructible : public AActor
{
	GENERATED_BODY()

public:
	AAmaruDestructible();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destructible")
	float MaxHealth = 500.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Destructible")
	float Health = 500.f;

	// Equipo del invocador: los aliados no lo dañan.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Destructible")
	int32 TeamId = INDEX_NONE;

	UPROPERTY(BlueprintAssignable)
	FOnDestructibleDestroyed OnDestructibleDestroyed;

	// Solo servidor. Source = actor atacante (para filtro de equipo).
	void ReceiveObjectDamage(float Amount, const AActor* Source);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
};
