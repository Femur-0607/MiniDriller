// Fill out your copyright notice in the Description page of Project Settings.


#include "ObstacleBlock.h"

#include "GameStatusSubsystem.h"
#include "MapManager.h"


// Sets default values
AObstacleBlock::AObstacleBlock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AObstacleBlock::BeginPlay()
{
	Super::BeginPlay();
	
}

void AObstacleBlock::OnInteracted(class ADrillerCharacter* Player)
{
	currentHp--;
	
	if (currentHp > 0)
	{
		return;
		// TODO: 타격 횟수마다 스프라이트 변경 로직 필요
	}
	else
	{
		// 1. 산소 패널티 부여 (안전한 널 체크 적용)
		if (UGameStatusSubsystem* statusSubsystem = GetWorld()->GetSubsystem<UGameStatusSubsystem>())
		{
			statusSubsystem->ConsumeOxygen(20.0f);
		}
       
		// 2. [중요] 체력이 0이 되었으므로 파괴!
		Pop();
	}
}
