// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/AmaruGameMode.h"
#include "AmaruGM_Deathmatch.generated.h"

/**
 * Todos contra todos, solo armas. Gana el que más kills tenga al acabar el tiempo.
 */
UCLASS()
class AMARUSHOOTER_API AAmaruGM_Deathmatch : public AAmaruGameMode
{
	GENERATED_BODY()
public:
	AAmaruGM_Deathmatch();

	// 0 = sin límite; con N > 0 la partida acaba al llegar alguien a N kills.
	UPROPERTY(EditDefaultsOnly, Category = "Mode Rules")
	int32 KillLimit = 0;

protected:
	virtual void AssignTeam(AAmaruPlayerState* PS) override;
	virtual void OnMatchTimeExpired() override;
	virtual void OnPlayerKilled(AAmaruPlayerState* Killer, AAmaruPlayerState* Victim) override;
	virtual FName GetPlayerStartTagForPlayer(const AAmaruPlayerState* PS) const override;

	void DecideWinnerAndEnd();

	int32 NextFFATeamId = 0;
};
