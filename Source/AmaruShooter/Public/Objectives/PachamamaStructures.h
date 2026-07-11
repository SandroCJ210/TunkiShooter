#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Objectives/AmaruDestructible.h"
#include "PachamamaStructures.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class USphereComponent;
class AAmaruPlayerState;

// Muro de la Pachamama: pared destructible; mientras vive, los aliados cercanos
// ganan escudo cada vez que infligen daño.
UCLASS()
class AMARUSHOOTER_API APachamamaWall : public AAmaruDestructible
{
	GENERATED_BODY()

public:
	APachamamaWall();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall")
	float BuffRadius = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall")
	float LifeTime = 15.f;

	UPROPERTY(EditAnywhere, Category = "Wall")
	TSubclassOf<UGameplayEffect> ShieldOnDamageEffect;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void HandleDamageDealt(AAmaruPlayerState* Source, float Damage);

	FDelegateHandle DamageDealtHandle;
};

// Santuario del Ande: domo destructible; aliados dentro reciben reducción de
// daño (State.Protected), enemigos ralentización (State.Slowed).
UCLASS()
class AMARUSHOOTER_API APachamamaDome : public AAmaruDestructible
{
	GENERATED_BODY()

public:
	APachamamaDome();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Zone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dome")
	float LifeTime = 12.f;

	UPROPERTY(EditAnywhere, Category = "Dome")
	TSubclassOf<UGameplayEffect> AllyEffect;

	UPROPERTY(EditAnywhere, Category = "Dome")
	TSubclassOf<UGameplayEffect> EnemyEffect;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	TMap<TWeakObjectPtr<UAbilitySystemComponent>, FActiveGameplayEffectHandle> AppliedEffects;
};
