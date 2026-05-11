// 화면에 보이는 만큼의 블록만 메모리에 유지하고, 카메라가 내려가면 화면 밖 상단 블록을 맨 아래로 순환시킵니다.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapManager.generated.h"

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

public:
	// 어떤 종류의 블록을 스폰할지 에디터에서 선택할 수 있게 합니다.
	UPROPERTY(EditAnywhere, Category = "Map Settings")
	TSubclassOf<class ABlock> blockClass;
	UPROPERTY(EditAnywhere)
	TArray<class ABlock*> blockPool; // 재활용 블록 배열 (화면 높이 + 여유분 약 20줄)
	UPROPERTY(EditAnywhere)
	float tileSize; // 타일의 기준 크기
	
	void InitializeMap(); // Reserve를 통한 초기 여유분 블록 풀 스폰 및 배치
	void RecycleTopLine(); // 플레이어가 특정 깊이 이상 내려갈 때마다 호출되어,
	//최상단 라인의 블록들을 맨 아래 라인으로 이동시킵니다. 이동 시 확률에 따라 아이템이나 장애물로 속성을 변환합니다.
	void OnLevelUpExplosion(); // 100칸 도달 시 화면 내 모든 일반 블록을 파괴하고 나이아가라 효과 재생.
	
};
