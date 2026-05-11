// 사용자 입력 처리 및 드릴 채굴 로직을 담당하는 플레이어 캐릭터 클래스

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "DrillerCharacter.generated.h"

// 전방 선언: 헤더 포함을 최소화하여 컴파일 속도를 높입니다.
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class MINIDRILLER_API ADrillerCharacter : public APaperCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADrillerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// --- 카메라 컴포넌트 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* springArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* cameraComponent;

	// --- Enhanced Input ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* defaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* moveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* digAction;
	
	// --- 입력 처리 함수 ---
	void Move(const FInputActionValue& value);

public:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void SetupCamera(); // 카메라 초기화
	void Dig(); // 채굴 명령
	void HandleDeath(); // 산소 고갈 시 사망 연출 처리
};
