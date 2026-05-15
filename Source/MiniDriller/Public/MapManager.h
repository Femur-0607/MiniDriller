// 화면에 보이는 만큼의 블록만 메모리에 유지하고, 카메라가 내려가면 화면 밖 상단 블록을 맨 아래로 순환시킵니다.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapManager.generated.h"

// 블록 타입을 관리하기 위한 열거형
UENUM(BlueprintType)
enum class EBlockType : uint8
{
	Normal,
	Oxygen,
	Obstacle
};

UCLASS()
class MINIDRILLER_API AMapManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMapManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma region pool System
public:
	// 어떤 종류의 블록을 스폰할지 에디터에서 선택할 수 있게 합니다.
	UPROPERTY(EditAnywhere, Category = "Map Settings")
	TSubclassOf<class ABlock> BlockClass;
	UPROPERTY(EditAnywhere)
	TArray<class ABlock*> BlockPool; // 재활용 블록 배열 (화면 높이 + 여유분 약 20줄)
	UPROPERTY(EditAnywhere)
	float TileSize; // 타일의 기준 크기
	// 맵의 가로 세로 최대 크기 변수
	UPROPERTY(EditAnywhere, Category = "Map Settings")
	int32 MaxRows = 20; 
	UPROPERTY(EditAnywhere, Category = "Map Settings")
	int32 MaxCols = 10;
	
	// 기존의 줄 생성 로직에 확률을 더합니다.
	void SpawnNewRow(int32 RowIndex);
	// 블록이 Pop 되었을 때 다시 풀로 돌려받는 함수
	UFUNCTION()
	void ReturnBlockToPool(ABlock* returnedBlock);

	// 1. 에디터에서 할당할 클래스 종류들
	UPROPERTY(EditAnywhere, Category = "Pool | Classes")
	TSubclassOf<class ABlock> NormalBlockClass;
	UPROPERTY(EditAnywhere, Category = "Pool | Classes")
	TSubclassOf<class AOxygenBlock> OxygenBlockClass;
	UPROPERTY(EditAnywhere, Category = "Pool | Classes")
	TSubclassOf<class AObstacleBlock> ObstacleBlockClass;
	// 2. 비활성화된 블록들을 담아둘 바구니 (Pools)
	TArray<ABlock*> NormalPool;
	TArray<ABlock*> OxygenPool;
	TArray<ABlock*> ObstaclePool;

	// 3. 풀 초기화 함수
	void InitializePools();
	// 4. 풀에서 블록을 꺼내오는 핵심 함수
	ABlock* GetBlockFromPool(EBlockType Type);
#pragma endregion
	
	// --- 블럭 색상 및 매칭 시스템 --- 
#pragma region Color and Matching System
	// 에디터에서 [0]:파랑, [1]:초록, [2]:빨강, [3]:노랑 스프라이트를 직접 할당할 배열
	UPROPERTY(EditAnywhere, Category = "Map Settings")
	TArray<class UPaperSprite*> BlockSprites;
	// 에디터에서 [0]:파랑, [1]:초록, [2]:빨강, [3]:노랑 플립북(파괴 이펙트)를 직접 할당할 배열
	UPROPERTY(EditAnywhere, Category = "Map Settings")
	TArray<class UPaperFlipbook*> BlockDestructionFlipbooks;
	// 에디터에서 [0]:파랑, [1]:초록, [2]:빨강, [3]:노랑 플립북(파괴효과 이펙트)를 직접 할당할 배열
	UPROPERTY(EditAnywhere, Category = "Map Settings")
	TArray<class UPaperFlipbook*> BlockEffectFlipbooks;
	// [Model] 사장님의 논리적 바둑판 장부
	UPROPERTY()
	TMap<FIntPoint, class ABlock*> GridMap;
	
	void RegisterBlock(int32 Col, int32 Row, class ABlock* Block);  // 블록들을 장부에 기록함
	void RemoveBlockFromGrid(int32 Col, int32 Row); // 장부에서 좌표를 지우는 함수
	class ABlock* GetBlockAtGrid(int32 Col, int32 Row); // 장부를 읽는 함수
	void ProcessFalling(int32 Col, int32 StartRow); // 장부를 보고 블록 하락 시스템을 지시하는 역활
	bool IsGridEmpty(int32 Col, int32 Row);
#pragma endregion
};

// void RecycleTopLine(); // 플레이어가 특정 깊이 이상 내려갈 때마다 호출되어,
//최상단 라인의 블록들을 맨 아래 라인으로 이동시킵니다. 이동 시 확률에 따라 아이템이나 장애물로 속성을 변환합니다.
// void OnLevelUpExplosion(); // 100칸 도달 시 화면 내 모든 일반 블록을 파괴하고 나이아가라 효과 재생.
