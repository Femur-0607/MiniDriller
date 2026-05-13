// 맵(월드)의 전반적인 상태(점수, 산소, 게임오버 등)를 관리하는 서브시스템
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameStatusSubsystem.generated.h"

// 블루프린트에서도 쓸 수 있는 다이내믹 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDiedDelegate);

UCLASS()
class MINIDRILLER_API UGameStatusSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
#pragma region Events
	// 다른 곳(UI 등)에서 구독할 이벤트 변수
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDiedDelegate OnPlayerDied;
#pragma endregion

#pragma region Game Data
	UPROPERTY(BlueprintReadWrite, Category = "Game Data")
	float CurrentOxygen = 100.f; // 초기 산소량

	UPROPERTY(BlueprintReadWrite, Category = "Game Data")
	int32 CurrentDepth = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Game Data")
	int32 CurrentLevel = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Game Data")
	int32 TotalScore = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Game Data")
	bool bIsPlayerDead = false;
#pragma endregion

#pragma region Game Logic
	// 캐릭터가 죽었을 때 호출할 함수 (델리게이트 방송용)
	UFUNCTION(BlueprintCallable, Category = "Game Logic")
	void NotifyPlayerDeath();

	// 현재 게임 오버 상태인지 확인하는 함수
	UFUNCTION(BlueprintPure, Category = "Game Logic")
	bool IsGameOver() const;

	UFUNCTION(BlueprintCallable, Category = "Game Logic")
	void AddScore(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Game Logic")
	void ConsumeOxygen(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Game Logic")
	void CheckLevelUp();
#pragma endregion
};