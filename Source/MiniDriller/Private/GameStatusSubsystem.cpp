// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStatusSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UGameStatusSubsystem::NotifyPlayerDeath()
{
	bIsPlayerDead = true;
	
	// 구독자가 있다면 사망 소식을 방송합니다!
	if (OnPlayerDied.IsBound())
	{
		OnPlayerDied.Broadcast();
	}
}

bool UGameStatusSubsystem::IsGameOver() const
{
	// 산소가 0 이하로 떨어졌거나(OR) 플레이어가 사망했다면 게임 오버!
	return (currentOxygen <= 0.f) || bIsPlayerDead; 
}

void UGameStatusSubsystem::AddScore(int32 Amount) { totalScore += Amount; }
void UGameStatusSubsystem::ConsumeOxygen(float Amount) { currentOxygen -= Amount; }
void UGameStatusSubsystem::AddOxygen(float Amount)
{
	currentOxygen += Amount;
	// 산소가 100.0f를 넘지 않도록 Clamp (Simplicity First!)
	currentOxygen = FMath::Clamp(currentOxygen, 0.0f, 100.0f); 
}

void UGameStatusSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	/// 1초마다 DecreaseOxygenTick 함수를 반복 실행하도록 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(
		oxygenTimerHandle,                           // 조종할 핸들 변수 (lowerCamelCase 적용)
		this,                                        // 함수를 보유한 객체 (서브시스템 자신)
		&UGameStatusSubsystem::DecreaseOxygenTick,   // 실행할 함수의 메모리 주소(포인터)
		1.0f,                                        // 실행 주기 (초)
		true                                         // 반복 여부 (true)
	);
}

void UGameStatusSubsystem::Deinitialize()
{
	// 메모리 누수 방지를 위해 타이머 해제
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(oxygenTimerHandle);
	}
	
	Super::Deinitialize();
}


void UGameStatusSubsystem::DecreaseOxygenTick()
{
	// 산소를 초당 1씩 감소
	ConsumeOxygen(1.0f);
	
	if (IsGameOver())
	{
		GetWorld()->GetTimerManager().ClearTimer(oxygenTimerHandle);
		NotifyPlayerDeath();
	}
}

void UGameStatusSubsystem::AddDepth(int32 Amount)
{
	// 1. currentDepth에 Amount를 더합니다.
    currentDepth += Amount;
	// 2. 깊이가 증가했으니 레벨업 조건에 도달했는지 검사합니다.
	CheckLevelUp();
}

void UGameStatusSubsystem::CheckLevelUp()
{
	// [Simplicity First] 가장 단순한 수학적 레벨업 공식입니다.
	// 현재 깊이가 (현재 레벨 * 100) 보다 크거나 같다면 레벨을 올립니다.
	if (currentDepth >= currentLevel * 100)
	{
		currentLevel++;
        
		// TODO: 나중에 여기에 난이도 증가(산소 소모량 증가, 매니저에 폭발 지시) 로직을 추가할 예정입니다.
		UE_LOG(LogTemp, Warning, TEXT("Level Up! Current Level: %d"), currentLevel);
	}
}