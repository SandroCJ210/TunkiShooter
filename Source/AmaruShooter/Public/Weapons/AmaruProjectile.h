#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmaruProjectile.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class UProjectileMovementComponent;
class USphereComponent;

// Proyectil servidor-autoritativo. ExplosionRadius > 0 = daño AoE al impactar
// o al agotar MaxDistance (Núcleo de Arcilla); 0 = daño directo (Bastón Ritual).
UCLASS()
class AMARUSHOOTER_API AAmaruProjectile : public AActor
{
	GENERATED_BODY()

public:
	AAmaruProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> Movement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float Speed = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float MaxDistance = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float BaseDamage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float ExplosionRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float GravityScale = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// Llamado por la habilidad al spawnearlo (solo servidor).
	void InitProjectile(UAbilitySystemComponent* InSourceASC, float InDamageMultiplier);

protected:
	virtual void BeginPlay() override;
	virtual void LifeSpanExpired() override;

	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void HandleImpact(AActor* OtherActor, const FVector& ImpactLocation);
	void Explode(const FVector& Location);
	// true si aplicó daño a un enemigo con ASC.
	bool ApplyDamageToActor(AActor* Target, const FVector& ImpactLocation);

	// Hook por héroe (CDR de Wayra al acertar).
	virtual void OnEnemyHit(UAbilitySystemComponent* TargetASC) {}

	void ReportHitOnce();

	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;
	float DamageMultiplier = 1.f;
	bool bReportedHit = false;
	bool bImpactHandled = false;
};
