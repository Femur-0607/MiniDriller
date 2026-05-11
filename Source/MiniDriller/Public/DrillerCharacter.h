// 사용자 입력 처리 및 드릴 채굴 로직을 담당하는 플레이어 캐릭터 클래스

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "DrillerCharacter.generated.h"

// 전방 선언: 헤더 포함을 최소화하여 컴파일 속도를 높입니다.
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS(Blueprintable) // 외부 플러그인 때문에 한번 더 블루프린트 사용한다고 명시함
class MINIDRILLER_API ADrillerCharacter : public APaperZDCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADrillerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// --- Enhanced Input ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* defaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* moveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* digAction;
	
	

public:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* playerInputComponent) override;
	
	void Move(const FInputActionValue& value); // --- 입력 처리 함수 ---
	void Dig(); // 채굴 명령
	void HandleDeath(); // 산소 고갈 시 사망 연출 처리
	
	// PaperZD 애니메이션 인스턴스를 가져오는 헬퍼 함수 (필요 시)
	class UPaperZDAnimInstance* GetZDAnimInstance() const;
};
