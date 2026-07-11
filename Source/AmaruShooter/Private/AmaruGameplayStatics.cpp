#include "AmaruGameplayStatics.h"

#include "AmaruShooter/AmaruPlayerState.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

AAmaruPlayerState* UAmaruGameplayStatics::GetAmaruPlayerStateFromActor(const AActor* Actor)
{
	if (!Actor) return nullptr;

	if (AAmaruPlayerState* PS = const_cast<AAmaruPlayerState*>(Cast<AAmaruPlayerState>(Actor)))
	{
		return PS;
	}
	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		return Pawn->GetPlayerState<AAmaruPlayerState>();
	}
	if (const AController* Controller = Cast<AController>(Actor))
	{
		return Controller->GetPlayerState<AAmaruPlayerState>();
	}
	return nullptr;
}

bool UAmaruGameplayStatics::AreSameTeam(const AActor* A, const AActor* B)
{
	const AAmaruPlayerState* PSA = GetAmaruPlayerStateFromActor(A);
	const AAmaruPlayerState* PSB = GetAmaruPlayerStateFromActor(B);
	if (!PSA || !PSB) return false;
	if (PSA->GetTeamId() == INDEX_NONE || PSB->GetTeamId() == INDEX_NONE) return false;
	return PSA->GetTeamId() == PSB->GetTeamId();
}
