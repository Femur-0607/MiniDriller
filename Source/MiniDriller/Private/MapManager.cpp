// 블록 풀링과 라인 재배치를 담당하는 맵 매니저 구현부


#include "MapManager.h"
#include "Block.h"
#include "ObstacleBlock.h"
#include "OxygenBlock.h"
#include "PaperSpriteComponent.h"


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
	
	// 1. 게임 시작 시 3개의 바구니를 블록으로 가득 채웁니다.
	InitializePools();

	// 2. 초기 맵 세팅 (예: 시작할 때 화면에 10줄을 쫙 깔아줍니다)
	for (int32 i = 0; i < 10; ++i)
	{
		SpawnNewRow(i);
	}
}

void AMapManager::InitializePools()
{
	// 예: 각 타입별로 50개씩 미리 생성
	auto PreSpawn = [&](TSubclassOf<ABlock> Class, TArray<ABlock*>& Pool, int32 Count) {
		for (int32 i = 0; i < Count; ++i) {
			ABlock* NewBlock = GetWorld()->SpawnActor<ABlock>(Class, FVector::ZeroVector, FRotator::ZeroRotator);
			if (NewBlock) {
				NewBlock->SetActorHiddenInGame(true);
				NewBlock->SetActorEnableCollision(false);
				NewBlock->onBlockDestroyedDelegate.AddUObject(this, &AMapManager::ReturnBlockToPool);
				Pool.Add(NewBlock);
			}
		}
	};

	PreSpawn(NormalBlockClass, NormalPool, 100); // 일반 블록은 많이
	PreSpawn(OxygenBlockClass, OxygenPool, 20);
	PreSpawn(ObstacleBlockClass, ObstaclePool, 30);
}

ABlock* AMapManager::GetBlockFromPool(EBlockType Type)
{
	TArray<ABlock*>* TargetPool = nullptr;
	TSubclassOf<ABlock> TargetClass;

	// 타입에 따라 알맞은 바구니(Pool)를 선택합니다.
	switch (Type) {
	case EBlockType::Normal:   TargetPool = &NormalPool;   TargetClass = NormalBlockClass; break;
	case EBlockType::Oxygen:   TargetPool = &OxygenPool;   TargetClass = OxygenBlockClass; break;
	case EBlockType::Obstacle: TargetPool = &ObstaclePool; TargetClass = ObstacleBlockClass; break;
	}

	// 바구니에 남은 블록이 있다면 하나 꺼내서 줍니다.
	if (TargetPool && TargetPool->Num() > 0) {
		ABlock* Block = TargetPool->Pop();
		Block->SetActorHiddenInGame(false);
		
		// [투명 인간 방지] 숨겨놨던 스프라이트를 다시 보이게 켜줍니다!
		if (Block->spriteComponent)
		{
			Block->spriteComponent->SetHiddenInGame(false);
		}
		
		Block->SetActorEnableCollision(true);
		return Block;
	}

	// 만약 바구니가 텅텅 비었다면? 임시로 하나 새로 만들어서 줍니다. (안전장치)
	return GetWorld()->SpawnActor<ABlock>(TargetClass, FVector::ZeroVector, FRotator::ZeroRotator);
}

void AMapManager::ReturnBlockToPool(ABlock* returnedBlock)
{
	if (returnedBlock == nullptr) return;

	// IsA() 함수를 사용해 이 블록이 정확히 어떤 클래스인지 검사하여 알맞은 바구니에 넣습니다.
	if (returnedBlock->IsA(OxygenBlockClass)) 
	{
		OxygenPool.Add(returnedBlock);
	} 
	else if (returnedBlock->IsA(ObstacleBlockClass)) 
	{
		ObstaclePool.Add(returnedBlock);
	} 
	else 
	{
		NormalPool.Add(returnedBlock);
	}
}

void AMapManager::SpawnNewRow(int32 RowIndex)
{
	// GridWidth 대신 선언해두신 MaxCols를 사용합니다!
	for (int32 Col = 0; Col < MaxCols; ++Col) { 
		float Rand = FMath::FRand(); 
		EBlockType SelectedType = EBlockType::Normal;

		if (Rand < 0.05f)        SelectedType = EBlockType::Oxygen;   // 5% 확률
		else if (Rand < 0.15f)   SelectedType = EBlockType::Obstacle; // 10% 확률
        
		ABlock* NewBlock = GetBlockFromPool(SelectedType);
		if (NewBlock)
		{
			// [추가된 로직] 일반 블록일 경우에만 색상과 스프라이트를 랜덤으로 부여합니다!
			if (SelectedType == EBlockType::Normal)
			{
				// 1(Blue) ~ 4(Yellow) 사이의 랜덤한 색상 인덱스 추출
				int32 ColorIndex = FMath::RandRange(1, 4);
				EBlockColor RandomColor = static_cast<EBlockColor>(ColorIndex);

				// 에디터에서 할당한 배열(크기 4)에서 꺼내오기 위해 인덱스 - 1을 해줍니다 (0~3)
				UPaperSprite* SelectedSprite = BlockSprites.IsValidIndex(ColorIndex - 1) ? BlockSprites[ColorIndex - 1] : nullptr;
				UPaperFlipbook* SelectedFlipbook = BlockDestructionFlipbooks.IsValidIndex(ColorIndex - 1) ? BlockDestructionFlipbooks[ColorIndex - 1] : nullptr;
				UPaperFlipbook* SelectedEffectFlipbook = BlockEffectFlipbooks.IsValidIndex(ColorIndex - 1) ? BlockEffectFlipbooks[ColorIndex - 1] : nullptr;

				// 블록에 랜덤 색상과 스프라이트 적용
				NewBlock->SetBlockColor(RandomColor, SelectedSprite, SelectedFlipbook, SelectedEffectFlipbook);
			}
			
			// 1. 물리적 위치 설정 (Z축을 밑으로 내림)
			FVector SpawnPos = FVector(Col * TileSize, 0.0f, -RowIndex * TileSize);
			NewBlock->SetActorLocation(SpawnPos);

			// 2. [매우 중요] 매니저 참조를 넘겨줍니다. 그래야 블록이 매니저의 장부에 접근할 수 있습니다.
			NewBlock->mapManagerRef = this;

			// 3. 매니저의 장부에 기록합니다.
			RegisterBlock(Col, RowIndex, NewBlock);
		}
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
				FVector NewTargetPos = FVector(Col * TileSize, 0.0f, -TargetRow * TileSize);
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