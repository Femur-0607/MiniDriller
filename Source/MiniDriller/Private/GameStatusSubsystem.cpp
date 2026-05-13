// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStatusSubsystem.h"

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
	return (CurrentOxygen <= 0.f) || bIsPlayerDead; 
}

void UGameStatusSubsystem::AddScore(int32 Amount) { TotalScore += Amount; }
void UGameStatusSubsystem::ConsumeOxygen(float Amount) { CurrentOxygen -= Amount; }
void UGameStatusSubsystem::CheckLevelUp() { /* 깊이 체크 및 레벨업 로직 추가 예정 */ }