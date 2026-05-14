// 채굴 대상 및 상호작용 오브젝트의 최상위 부모 클래스

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Block.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBlockDestroyed, class ABlock*);

// 블록의 색상을 정의하는 열거형
UENUM(BlueprintType)
enum class EBlockColor : uint8
{
	None,
	Blue,
	Green,
	Red,
	Yellow
};

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
	// 블록 파괴 시 호출되는 델리게이트 변수
	FOnBlockDestroyed onBlockDestroyedDelegate;
	
	// 플레이어와 충돌 및 상호작용 시 실행되는 함수
	virtual void OnInteracted(class ADrillerCharacter* Player);

	// 파괴 이펙트를 담당할 플립북 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UPaperFlipbookComponent* destructionEffectComponent;
	
	// 2. 애니메이션 재생이 끝났을 때 엔진이 호출해 줄 함수
	UFUNCTION()
	void OnDestructionEffectFinished();
	
protected:
	// 유니티의 OnTriggerEnter 역할 (언리얼 AActor에 기본 내장된 함수를 덮어쓰기)
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
private:
	// 전달받은 액터가 플레이어라면 압사시키는 도우미 함수
	void TryKillPlayer(AActor* TargetActor);
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
	float shakeOffset;
	void StartFalling();
#pragma endregion
	
	// 방향과 거리를 넣으면 액터를 반환하는 헬퍼 함수 (거리를 안 넣으면 자동으로 42.f가 됨!)
	AActor* GetActorInDirection(FVector Direction, float Distance = 42.f);
	
	// --- 블럭 색상 및 매칭 시스템 --- 
#pragma region Color and Matching System
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
	EBlockColor blockColor = EBlockColor::None;
	
	// 블록의 색상과 스프라이트(파괴 애니메이션 포함)를 동적으로 변경하는 함수
	void SetBlockColor(EBlockColor NewColor, class UPaperSprite* NewSprite, class UPaperFlipbook* NewFlipbook);
#pragma endregion
};
