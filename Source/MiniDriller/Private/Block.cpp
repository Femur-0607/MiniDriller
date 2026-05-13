// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniDriller/Public/Block.h"
#include "PaperSpriteComponent.h"


// Sets default values
ABlock::ABlock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// 1. 형태가 없는 빈 씬 컴포넌트를 생성하고 루트(중심)로 설정합니다.
	USceneComponent* defaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	RootComponent = defaultRoot;

	// 2. 스프라이트를 생성하고 루트 컴포넌트의 자식으로 부착합니다.
	spriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("MySprite"));
	spriteComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABlock::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	switch (currentState)
	{
		case EBlockState::Idle:
			// Idle 상태일 때는 Tick이 꺼져 있어야 하므로, 여기 들어올 일은 원칙적으로 없습니다.
			break;
			
		case EBlockState::Anticipating:
			// [흔들림 연출] 스프라이트를 좌우로 미세하게 흔듭니다.
			// 0.1초 뒤에 떨어지도록 타이머를 설정하는 로직이 필요하지만, 지금은 바로 Falling으로 넘기겠습니다.
			spriteComponent->SetRelativeLocation(FVector(0.f , 0.f, FMath::RandRange(-2.f, 2.f)));
	            
			// TODO: 실제로는 일정 시간(예: 0.2초) 대기 후 Falling으로 넘어가야 합니다.
			// 임시로 바로 상태를 넘깁니다.
			spriteComponent->SetRelativeLocation(FVector::ZeroVector); // 위치 원상복구
			currentState = EBlockState::Falling;
			break;

		case EBlockState::Falling:
			// 1. 현재 위치 가져오기
			FVector currentLocation = GetActorLocation();

			// 2. 등속 보간(MoveTowards)으로 다음 프레임 위치 계산 (이동 속도는 임의로 300.f 설정)
			FVector nextLocation = FMath::VInterpConstantTo(currentLocation, targetLocation, DeltaTime, 300.f);

			// 3. 액터 위치 업데이트
			SetActorLocation(nextLocation);

			// 4. 도착 판정 (목표 위치와의 거리가 1.0f 미만이면 도착한 것으로 간주)
			if (FVector::Dist(currentLocation, targetLocation) < 1.0f)
			{
				currentState = EBlockState::Idle;
				SetActorTickEnabled(false);
			}
			break;
	}
}

void ABlock::OnInteracted(class ADrillerCharacter* Player)
{
	// 1. 시각적, 물리적 비활성화
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	// 2. 레이캐스트 쏴서 위에 블록 있는지 검사
	FVector rayStart = GetActorLocation();
	FVector fallDirection = FVector(0.f, 0.f, 1.f);
	FVector rayEnd = rayStart + (fallDirection * 42.f); // Z축 위로 42 유닛
	
	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this); // 자기 자신을 레이캐스트 대상에서 제외합니다.
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(hit, rayStart, rayEnd, ECC_Visibility, params);
	
	// 디버그 라인 그리기 (월드, 시작점, 끝점, 색상, 지속여부, 수명, 깊이 우선, 굵기) 전처리기 사용
#if WITH_EDITOR
	DrawDebugLine(GetWorld(), rayStart, rayEnd, FColor::Blue, false, 2.0f, 0, 2.0f);
#endif
	
	// 3. 충돌한 대상이 있다면 ABlock인지 확인하고 명령 내리기
	if (bHit)
	{
		ABlock* upperBlock = Cast<ABlock>(hit.GetActor());
		if (upperBlock != nullptr)
		{
			// 위에있는 블록한테 떨어질 준비하라고 알림
			upperBlock->CheckAndFall(); 
		}
	}
	
	// 4. 구독자(MapManager)들에게 내가 파괴되었음을 알립니다.
	if (onBlockDestroyedDelegate.IsBound())
	{
		onBlockDestroyedDelegate.Broadcast(this);
	}
}

void ABlock::CheckAndFall()
{
	currentState = EBlockState::Anticipating;
	SetActorTickEnabled(true);
	targetLocation = GetActorLocation() + FVector(0.f, 0.f, -42.f);
}

