#include "BrawlerNpcAi.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "UObject/ConstructorHelpers.h"

ABrawlerNpcAi::ABrawlerNpcAi(const FObjectInitializer& ObjectInitializer)
{
    // Get the behavior tree.
    static ConstructorHelpers::FObjectFinder<UBehaviorTree> Obj(TEXT("BehaviorTree'/Game/Blueprints/AI/BrawlerNpcBehavior.BrawlerNpcBehavior'"));
    if (Obj.Succeeded()) {
        BehaviorTree = Obj.Object;
    }

    // Create the behavior tree and blackboard components.
    BehaviorTreeComponent = ObjectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorTreeComponent"));
    Blackboard            = ObjectInitializer.CreateDefaultSubobject<UBlackboardComponent  >(this, TEXT("Blackboard"));
}

void ABrawlerNpcAi::BeginPlay()
{
    Super::BeginPlay();

    // Start the behavior tree.
    if (BehaviorTree && BehaviorTreeComponent)
    {
        RunBehaviorTree(BehaviorTree);
        BehaviorTreeComponent->StartTree(*BehaviorTree);
    }
}

void ABrawlerNpcAi::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Initialize the blackboard.
    if (Blackboard && BehaviorTree)
    {
        Blackboard->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
    }
}

UBlackboardComponent* ABrawlerNpcAi::GetBlackboard() const
{
    return Blackboard;
}