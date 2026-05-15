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
	
	// 매번 Cast하지 않고 태어날 때 저장해 둘 사장님 다이렉트 연락처
	UPROPERTY()
	class AMapManager* mapManagerRef;

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
	// 파괴 이펙트를 담당할 플립북 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UPaperFlipbookComponent* destructionEffectComponent;
	
	// 애니메이션 재생이 끝났을 때 엔진이 호출해 줄 함수
	UFUNCTION()
	void OnDestructionEffectFinished();
	
	// 플레이어와 충돌 및 상호작용 시 실행되는 함수
	virtual void OnInteracted(class ADrillerCharacter* Player);
	// 매치 검사 및 파괴 로직(바닥 도착 시 4개 이상일 경우 파괴(연쇄 파괴))
	void CheckMatch();
	// 블럭 파괴 로직
	virtual void Pop();
	
protected:
	// 유니티의 OnTriggerEnter 역할 (언리얼 AActor에 기본 내장된 함수를 덮어쓰기)
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
private:
	// 전달받은 액터가 플레이어라면 압사시키는 도우미 함수
	void TryKillPlayer(AActor* TargetActor);
#pragma endregion

	// --- 낙하 시스템 ---
#pragma region Falling System
public:
	EBlockState currentState = EBlockState::Idle; // 블록의 현재 상태 기본값 Idle
	FVector targetLocation; // 목표 낙하 위치 (도착해야 할 Z 좌표)
	// [Model] 블록이 자신이 바둑판의 어디에 위치해 있는지 기억할 사원증(좌표)
	// 초기값은 -1로 설정하여, 스폰 전이나 풀(Pool)에 있을 때는 '장부 밖'임을 명시합니다.
	int32 GridX = -1;
	int32 GridY = -1;
	
	void PrepareToFall(FVector NewTargetLocation); // 블록에게 "곧 떨어질 준비해!"라고 명령하는 함수
	void ActuallyStartFalling(); // 타이머가 끝나면 실제로 물리적 이동을 시작하는 함수

private:
	// 흔들림 효과 타이머 핸들러 (유니티의 Coroutine 대체)
	struct FTimerHandle anticipationTimerHandle;
	float shakeOffset;
#pragma endregion
	
	// --- 블럭 색상 시스템 --- 
#pragma region Color
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
	EBlockColor blockColor = EBlockColor::None;
	
	// 블록의 색상과 스프라이트(파괴 애니메이션 포함)를 동적으로 변경하는 함수
	void SetBlockColor(EBlockColor NewColor, class UPaperSprite* NewSprite, class UPaperFlipbook* NewFlipbook);
	
	// 1. TargetColor: 기준이 되는 색상(비교), 2. CurrentBlock: 현재 탐색을 진행중인 블록 자신, OutMatchedBlocks:탐색한 블록들을 담아두고 중복을 검사할 컨테이너
	void ExecuteFloodFill(EBlockColor TargetColor, ABlock* CurrentBlock, TSet<ABlock*>& OutMatchedBlocks);
#pragma endregion
};
