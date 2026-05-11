// 블록 풀링과 라인 재배치를 담당하는 맵 매니저 구현부


#include "MiniDriller/Public/MapManager.h"
#include "Block.h"


// Sets default values
AMapManager::AMapManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AMapManager::BeginPlay()
{
	Super::BeginPlay();
	InitializeMap();
	
}

void AMapManager::InitializeMap()
{
	// 1. 필요한 총 블록 개수 계산 (예: 가로 10칸, 세로 20줄이면 총 200개)
	int32 TotalBlocks = 200;

	// 2. 성능 최적화를 위해 TArray의 메모리를 미리 할당
	BlockPool.Reserve(TotalBlocks);

	// 3. 2차원 그리드 형태로 스폰하기 위한 이중 반복문 (가로 10, 세로 20)
	for (int32 Row = 0; Row < 20; ++Row)
	{
		for (int32 Col = 0; Col < 10; ++Col)
		{
			// 4. 스폰할 위치 계산 (TileSize 변수 활용)
			// X축은 가로(Col), Z축은 세로(Row, 아래로 내려가야 하므로 마이너스 값 적용)
			FVector SpawnLocation = FVector(Col * TileSize, 0.0f, -Row * TileSize);
			FRotator SpawnRotation = FRotator::ZeroRotator;

			// 5. 월드에 블록 스폰 (지금은 ABlock 기본 클래스를 스폰)
			ABlock* NewBlock = GetWorld()->SpawnActor<ABlock>(ABlock::StaticClass(), SpawnLocation, SpawnRotation);

			// 6. 스폰된 블록을 BlockPool 배열에 추가
			if (NewBlock != nullptr)
			{
				BlockPool.Add(NewBlock);
			}
		}
	}
}

