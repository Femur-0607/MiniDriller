// 채굴 대상 및 상호작용 오브젝트의 최상위 부모 클래스

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Block.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBlockDestroyed, class ABlock*);

UCLASS()
class MINIDRILLER_API ABlock : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABlock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UPaperSpriteComponent* spriteComponent; // UPaperSpriteComponent 언리얼에서 스프라이트 렌더링 및 충돌 처리를 담당하는 클래스
	// 플레이어와 충돌 및 상호작용 시 실행되는 함수
	virtual void OnInteracted(class ADrillerCharacter* Player);
	FOnBlockDestroyed onBlockDestroyedDelegate;
};
