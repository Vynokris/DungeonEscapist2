#include "EquipmentActor.h"

#include "BrawlerCharacter.h"
#include "DebugUtils.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

AEquipmentActor::AEquipmentActor()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* EquipmentRootComponent = CreateDefaultSubobject<USceneComponent>("EquipmentRoot");
	RootComponent  = EquipmentRootComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("EquipmentMesh");
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	MeshComponent->SetGenerateOverlapEvents(true);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>("KnifeBox");
	BoxComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	BoxComponent->SetGenerateOverlapEvents(true);

	Tags.Add("Equipment");
}

void AEquipmentActor::BeginPlay()
{
	Debug("Spawning EquipmentActor at = %s", *this->GetActorLocation().ToString());
	
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	MeshComponent->SetGenerateOverlapEvents(true);
	switch (EquipmentType)
	{
	case Weapon:
		Tags.Add("Weapon");
		break;
	case Shield:
		Tags.Add("Shield");
		break;
	default:
		break;
	}
	
	Super::BeginPlay();
}

void AEquipmentActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!WasPickedUp()) {
		SetActorLocation(GetActorLocation() + FVector (0,0, sin(UGameplayStatics::GetRealTimeSeconds(GetWorld())*2)/4));
		SetActorRotation(GetActorRotation() + FRotator(0, 1, 0));
	}
}

void AEquipmentActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	// Get picked up only by the player.
	if (!WasPickedUp())
	{
		if (OtherActor->Tags.Contains("Player"))
			GetPickedUp(Cast<ABrawlerCharacter>(OtherActor));
		return;
	}

	if (Cast<AActor>(ParentCharacter) != OtherActor)
	{
		if (EquipmentType == Weapon && ParentCharacter->IsAttacking())
		{
			// Let player and enemies damage each other.
			if ((ParentCharacter->IsEnemy()  && OtherActor->Tags.Contains("Player")) ||
				(ParentCharacter->IsPlayer() && OtherActor->Tags.Contains("Enemy")))
			{
				ABrawlerCharacter* OtherCharacter = Cast<ABrawlerCharacter>(OtherActor);
				if (!OtherCharacter->IsDefending())
					OtherCharacter->TakeDamageEvent(ParentCharacter->GetAttackDamage());
			}
		}
	}
}

void AEquipmentActor::GetPickedUp(ABrawlerCharacter* NewParentCharacter)
{
	FName SocketName = "";
	switch (EquipmentType)
	{
	case Weapon:
		if (NewParentCharacter->HasEquipment(Weapon)) return;
		SocketName = "WeaponSocket";
		break;
	case Shield:
		if (NewParentCharacter->HasEquipment(Shield)) return;
		SocketName = "ShieldSocket";
		break;
	default:
		break;
	}
	
	AttachToComponent(Cast<USceneComponent>(NewParentCharacter->GetMesh()),
					  FAttachmentTransformRules::SnapToTargetNotIncludingScale,
					  SocketName);
	SetActorRelativeLocation({0, 0, 0});
	SetActorRelativeRotation({0, 0, 0});
	ParentCharacter = NewParentCharacter;
	NewParentCharacter->AddEquipment(this);
}

void AEquipmentActor::GetDropped(const ABrawlerCharacter* CurParentCharacter)
{
	DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
	SetActorRelativeLocation(CurParentCharacter->GetActorLocation());
	SetActorRelativeRotation({0, 0, 0});
	SetActorScale3D({1,1,1});
	ParentCharacter = nullptr;
}

bool AEquipmentActor::WasPickedUp() const
{
	return ParentCharacter != nullptr;
}

EEquipmentType AEquipmentActor::GetType() const
{
	return EquipmentType;
}
