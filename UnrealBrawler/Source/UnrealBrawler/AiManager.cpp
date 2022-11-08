#include "AiManager.h"

#include "BrawlerNpcAi.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BlackboardComponent.h"

AAiManager::AAiManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAiManager::BeginPlay()
{
	Super::BeginPlay();

	// Find all enemies spawned by default in the level.
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABrawlerNpcAi::StaticClass(), FoundEnemies);
	for (AActor* Actor : FoundEnemies)
		AiEnemies.Push(Cast<ABrawlerNpcAi>(Actor));
}

void AAiManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (AttackCooldown <= 0)
	{
		AttackCooldown = AttackInterval;
		
		// TODO: This could use a refacor.
		// Find a random enemy that is in range of the player and not attacking.
		int RandEnemyIdx = FMath::RandRange(0, AiEnemies.Num()-1);
		for (int j = 0; j < AiEnemies.Num() && AiEnemies[RandEnemyIdx]->GetBlackboard()->GetValueAsBool("Attacking") && AiEnemies[RandEnemyIdx]->GetBlackboard()->GetValueAsBool("TooCloseToPlayer"); j++)
			RandEnemyIdx = (RandEnemyIdx + 1) % AiEnemies.Num();

		// Let the enemy start an attack.
		if (!AiEnemies[RandEnemyIdx]->GetBlackboard()->GetValueAsBool("Attacking") && !AiEnemies[RandEnemyIdx]->GetBlackboard()->GetValueAsBool("TooCloseToPlayer"))
			AiEnemies[RandEnemyIdx]->GetBlackboard()->SetValueAsBool("Attacking", true);
	}
	else
	{
		AttackCooldown -= DeltaTime;
	}
}

