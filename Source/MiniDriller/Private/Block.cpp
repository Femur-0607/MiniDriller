// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniDriller/Public/Block.h"
#include "PaperSpriteComponent.h"


// Sets default values
ABlock::ABlock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	// 1. 스프라이트 컴포넌트를 메모리에 생성합니다. ("MySprite"는 에디터에 표시될 이름입니다.)
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("MySprite"));

	// 2. 이 컴포넌트를 액터의 중심(Root)으로 설정합니다.
	RootComponent = SpriteComponent;
}

// Called when the game starts or when spawned
void ABlock::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlock::OnInteracted(class ADrillerCharacter* Player)
{
	UE_LOG(LogTemp, Log, TEXT("Block Interacted!"));
}

