// 블록 풀링과 라인 재배치를 담당하는 맵 매니저 구현부


#include "MapManager.h"
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
	int32 totalBlocks = 200;
	
	
	// 2. 성능 최적화를 위해 TArray의 메모리를 미리 할당
	blockPool.Reserve(totalBlocks);

	// 3. 2차원 그리드 형태로 스폰하기 위한 이중 반복문 (가로 / 20, 세로 / 10)
	for (int32 Row = 0; Row < totalBlocks / 10; ++Row)
	{
		for (int32 Col = 0; Col < totalBlocks / 20; ++Col)
		{
			int32 randomIndex = FMath::RandRange(0, 3); // 0번(Blue)부터 3번(Yellow) 사이의 난수 추출
			EBlockColor randomColor = (EBlockColor)randomIndex;
			
			// 4. 스폰할 위치 계산 (TileSize 변수 활용)
			// X축은 가로(Col), Z축은 세로(Row, 아래로 내려가야 하므로 마이너스 값 적용)
			FVector spawnLocation = FVector(Col * tileSize, 0.0f, -Row * tileSize);
			FRotator spawnRotation = FRotator::ZeroRotator;

			// 5. 월드에 블록 스폰 (지금은 ABlock 기본 클래스를 스폰)
			ABlock* newBlock = GetWorld()->SpawnActor<ABlock>(blockClass, spawnLocation, FRotator::ZeroRotator);
			if (newBlock)
			{
				// 오너 설정, 색상 세팅, 델리게이트 연결 등
				newBlock->SetOwner(this); 
				newBlock->mapManagerRef = this;// ⭐ [의존성 주입] 블록이 태어나자마자 사장님(나 자신)의 연락처를 쥐어줍니다!
				newBlock->SetBlockColor(randomColor, blockSprites[randomIndex], blockDestructionFlipbooks[randomIndex]);
				newBlock->onBlockDestroyedDelegate.AddUObject(this, &AMapManager::ReturnBlockToPool);
    
				// ⭐ [신규 추가] 스폰된 블록을 사장님의 논리 장부에 즉시 등록!
				RegisterBlock(Col, Row, newBlock);
    
				blockPool.Add(newBlock);
			}
		}
	}
}

void AMapManager::ReturnBlockToPool(ABlock* ReturnedBlock)
{
	if (ReturnedBlock != nullptr)
	{
		// 1. 비활성화된 블록을 풀 배열에 다시 집어넣어 재활용 대기 상태로 만듭니다.
		blockPool.Add(ReturnedBlock);
	}
}

void AMapManager::RegisterBlock(int32 Col, int32 Row, ABlock* Block)
{
	if (Block != nullptr)
	{
		// 1. 매니저의 장부(GridMap)에 기록합니다.
		GridMap.Add(FIntPoint(Col, Row), Block);
        
		// 2. 직원의 사원증(내부 변수)에 자기 자리를 각인시킵니다.
		Block->GridX = Col; 
		Block->GridY = Row;
	}
}

void AMapManager::RemoveBlockFromGrid(int32 Col, int32 Row)
{
	GridMap.Remove(FIntPoint(Col, Row));
}

ABlock* AMapManager::GetBlockAtGrid(int32 Col, int32 Row)
{
	FIntPoint Pos(Col, Row);
	// 장부에 해당 좌표가 기록되어 있다면 그 블록을 반환하고, 아니면 빈 공간(nullptr)을 반환합니다.
	if (GridMap.Contains(Pos))
	{
		return GridMap[Pos];
	}
	return nullptr;
}

void AMapManager::ProcessFalling(int32 Col, int32 StartRow)
{
	// 터진 블록의 바로 윗칸(StartRow - 1)부터 맨 꼭대기(0)까지 역순으로 훑어 올라갑니다.
	for (int32 CurrentRow = StartRow - 1; CurrentRow >= 0; --CurrentRow)
	{
		// 현재 칸에 블록이 있는가?
		ABlock* BlockToFall = GetBlockAtGrid(Col, CurrentRow);
		if (BlockToFall != nullptr)
		{
			int32 TargetRow = CurrentRow;
            
			// [핵심 로직] 내 밑칸(TargetRow + 1)이 비어있고, 바닥(MaxRows)을 뚫지 않는 한 끝까지 탐색!
			while (IsGridEmpty(Col, TargetRow + 1) && (TargetRow + 1 < MaxRows))
			{
				TargetRow++; // 빈칸 수만큼 목표 좌표를 아래로 낮춤
			}

			// 밑으로 한 칸이라도 내려갈 공간이 있다면 이동 지시!
			if (TargetRow != CurrentRow)
			{
				// [데이터 선점] 화면에서 움직이기 전에 장부부터 고침 (매우 중요!)
				RemoveBlockFromGrid(Col, CurrentRow); // 옛날 자리 지우고
				RegisterBlock(Col, TargetRow, BlockToFall); // 새 자리 예약

				// [연출 예약] 블록에게 "너 나중에 여기까지 내려와야 해"라고 목적지 전달
				FVector NewTargetPos = FVector(Col * tileSize, 0.0f, -TargetRow * tileSize);
				BlockToFall->PrepareToFall(NewTargetPos);
			}
		}
	}
}

bool AMapManager::IsGridEmpty(int32 Col, int32 Row)
{
	// 1. 맵의 양옆 벽이나 바닥(MaxRows)을 뚫고 내려가려 하면 "빈 공간이 아니다(false)"라고 막음
	if (Col < 0 || Col >= MaxCols || Row < 0 || Row >= MaxRows)
	{
		return false;
	}
    
	// 2. 장부(GridMap)에 해당 좌표(Col, Row)가 없으면(true) 빈 공간임!
	return !GridMap.Contains(FIntPoint(Col, Row));
}