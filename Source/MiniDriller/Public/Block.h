// 채굴 대상 및 상호작용 오브젝트의 최상위 부모 클래스

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Block.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBlockDestroyed, class ABlock*);

// 블록의 현재 상태를 정의하는 열거형
UENUM(BlueprintType)
enum class EBlockState : uint8
{
	Idle,        // 평상시 (Tick 꺼짐)
	Anticipating,// 덜덜 흔들리는 대기 상태 (Tick 켜짐)
	Falling      // 아래로 떨어지는 상태 (Tick 켜짐)
};

UCLASS()
class MINIDRILLER_API ABlock : public AActor
{
	GENERATED_BODY()
	
public:
	ABlock();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UPaperSpriteComponent* spriteComponent;	// UPaperSpriteComponent 언리얼에서 스프라이트 렌더링 및 충돌 처리를 담당하는 클래스
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
// --- 상호작용 및 파괴 시스템 ---
#pragma region Interaction System
public:
	// 플레이어와 충돌 및 상호작용 시 실행되는 함수
	virtual void OnInteracted(class ADrillerCharacter* Player);
	// 블록 파괴 시 호출되는 델리게이트 변수
	FOnBlockDestroyed onBlockDestroyedDelegate;
#pragma endregion

	// --- 중력 및 낙하 시스템 ---
#pragma region Falling System
public:
	EBlockState currentState = EBlockState::Idle; // 블록의 현재 상태
	FVector targetLocation; // 목표 낙하 위치 (도착해야 할 Z 좌표)
	void CheckAndFall(); // 아래 블록이 파괴되었을 때 위 블록이 호출받을 함수

private:
	// 흔들림 효과 타이머 핸들러 (유니티의 Coroutine 대체)
	struct FTimerHandle anticipationTimerHandle;
	void StartFalling();
#pragma endregion
};
