#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EquipmentActor.generated.h"
class ABrawlerCharacter;

UENUM()
enum EEquipmentType
{
	Weapon UMETA(DisplayName = "Weapon"),
	Shield UMETA(DisplayName = "Shield"),
};

UCLASS()
class UNREALBRAWLER_API AEquipmentActor : public AActor
{
	GENERATED_BODY()

private:
	ABrawlerCharacter* ParentCharacter = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Properties", meta = (AllowPrivateAccess = "true")) UShapeComponent*            BoxComponent  = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Properties", meta = (AllowPrivateAccess = "true")) UStaticMeshComponent*       MeshComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Properties", meta = (AllowPrivateAccess = "true")) TEnumAsByte<EEquipmentType> EquipmentType = Weapon;
	
private:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
public:	
	AEquipmentActor();

	void GetPickedUp(ABrawlerCharacter* NewParentCharacter);
	void GetDropped (const ABrawlerCharacter* CurParentCharacter);
	
	bool WasPickedUp() const;
	EEquipmentType GetType() const;
};