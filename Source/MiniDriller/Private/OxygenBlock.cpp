// Fill out your copyright notice in the Description page of Project Settings.


#include "OxygenBlock.h"

#include "DrillerCharacter.h"
#include "GameStatusSubsystem.h"
#include "MapManager.h"


// Sets default values
AOxygenBlock::AOxygenBlock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AOxygenBlock::BeginPlay()
{
	Super::BeginPlay();
}


void AOxygenBlock::NotifyActorBeginOverlap(AActor* OtherActor)
{
	// 1. 나랑 겹친 액터가 '플레이어(ADrillerCharacter)'인지 확인합니다. (Cast 연산 활용)
	if (class ADrillerCharacter* Player = Cast<ADrillerCharacter>(OtherActor))
	{
		// 3. 닿은 액터가 플레이어가 맞다면 서브시스템을 불러와 산소를 회복시킵니다.
		if (UGameStatusSubsystem* statusSubsystem = GetWorld()->GetSubsystem<UGameStatusSubsystem>())
		{
			statusSubsystem->AddOxygen(20.0f); // 산소 20 회복
		}

		// 4. 플레이어에게 먹혔으므로 블록 파괴 (연쇄 파괴 없이 혼자 팝!)
		Pop();
	}
}

void AOxygenBlock::Pop()
{
		if (mapManagerRef)
	{
		mapManagerRef->RemoveBlockFromGrid(GridX, GridY);
        mapManagerRef->ProcessFalling(GridX, GridY);
	}

	// 1. [View] 시각적으로 즉시 숨깁니다. (부모처럼 애니메이션을 기다리지 않음)
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	// 3. [System] 델리게이트를 통해 매니저에게 "나 다 썼으니 풀로 데려가세요"라고 알립니다.
	// 이 호출이 발생하면 MapManager가 ReturnBlockToPool을 실행하게 됩니다.
	if (onBlockDestroyedDelegate.IsBound())
	{
		onBlockDestroyedDelegate.Broadcast(this);
	}
}