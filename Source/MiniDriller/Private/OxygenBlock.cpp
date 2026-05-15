// Fill out your copyright notice in the Description page of Project Settings.


#include "OxygenBlock.h"

#include "DrillerCharacter.h"
#include "GameStatusSubsystem.h"
#include "MapManager.h"
#include "PaperSpriteComponent.h"


// Sets default values
AOxygenBlock::AOxygenBlock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AOxygenBlock::BeginPlay()
{
	Super::BeginPlay();
}

void AOxygenBlock::OnInteracted(class ADrillerCharacter* Player)
{
	// [중요] 부모의 연쇄 파괴 로직을 막기 위해 Super::OnInteracted를 절대 부르지 않습니다!
    
	// 몸으로 닿았을 때와 똑같이 산소를 주고 파괴되도록 처리합니다.
	if (UGameStatusSubsystem* statusSubsystem = GetWorld()->GetSubsystem<UGameStatusSubsystem>())
	{
		statusSubsystem->AddOxygen(20.0f);
	}
	Pop();
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