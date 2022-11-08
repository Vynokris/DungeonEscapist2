

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "KnifeActor.generated.h"

UCLASS()
class UNREALBRAWLER_API AKnifeActor : public AActor
{
	GENERATED_BODY()

private:
	USceneComponent* KnifeRoot;
	UShapeComponent* KnifeBox;

public:
	UStaticMeshComponent* KnifeMesh;
	
	UPROPERTY(EditAnywhere) float moveSpeed = 2.0f;
	
private:
	void OnHit(UPrimitiveComponent* _HitComp, AActor* _OtherActor, UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex, bool _FromSweep, const FHitResult& _Hit);
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float _DeltaTime) override;
	
public:	
	AKnifeActor();
};
