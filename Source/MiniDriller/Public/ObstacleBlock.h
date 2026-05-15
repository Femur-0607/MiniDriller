// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Block.h"
#include "ObstacleBlock.generated.h"

UCLASS()
class MINIDRILLER_API AObstacleBlock : public ABlock
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AObstacleBlock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	int32 currentHp = 5; // 장애물블록의 체력 (5번 공격 시 파괴)

public:
	virtual void OnInteracted(class ADrillerCharacter* Player) override;
};
