#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AmaruGameplayStatics.generated.h"

class AAmaruPlayerState;

UCLASS()
class AMARUSHOOTER_API UAmaruGameplayStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Resuelve el PlayerState desde un PS, Pawn o Controller.
	// Necesario porque GetOriginalInstigator() del contexto GE devuelve el
	// PlayerState (dueño del ASC), no el pawn.
	UFUNCTION(BlueprintPure, Category = "Amaru")
	static AAmaruPlayerState* GetAmaruPlayerStateFromActor(const AActor* Actor);

	// false si alguno no tiene equipo asignado (INDEX_NONE).
	UFUNCTION(BlueprintPure, Category = "Amaru|Team")
	static bool AreSameTeam(const AActor* A, const AActor* B);
};
