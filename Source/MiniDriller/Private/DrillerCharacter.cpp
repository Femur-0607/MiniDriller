// Fill out your copyright notice in the Description page of Project Settings.


#include "DrillerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "PaperFlipbookComponent.h"
#include "PaperZDAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ADrillerCharacter::ADrillerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// 캐릭터 자체가 컨트롤러나 이동 방향에 따라 도는 것을 방지
	bUseControllerRotationYaw = false;
	
	if (GetCharacterMovement())
	{
		// PaperZD가 애니메이션 상태(Velocity)를 기반으로 작동하게 하되, 
		// 캐릭터 자체가 회전하는 것은 막습니다.
		GetCharacterMovement()->bOrientRotationToMovement = false;
        
		// Y축 이동 고정
		GetCharacterMovement()->bConstrainToPlane = true;
		GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, 1.0f, 0.0f));
	}
}

// Called when the game starts or when spawned
void ADrillerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 1. Mapping Context 등록
	if (APlayerController* pc = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer()))
		{
			subsystem->AddMappingContext(defaultMappingContext, 0);
		}
	}
	
}

// Called to bind functionality to input
void ADrillerCharacter::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{
	Super::SetupPlayerInputComponent(playerInputComponent);
	
	// 2. Action 바인딩
	if (UEnhancedInputComponent* ei = CastChecked<UEnhancedInputComponent>(playerInputComponent))
	{
		ei->BindAction(moveAction, ETriggerEvent::Triggered, this, &ADrillerCharacter::Move);
		ei->BindAction(digAction, ETriggerEvent::Started, this, &ADrillerCharacter::Dig);
	}
}

void ADrillerCharacter::Move(const FInputActionValue& value)
{
	// 입력 값 가져오기 (float)
	float moveVector = value.Get<float>();

	if (Controller != nullptr && moveVector != 0.f)
	{
		// 1. 이동 입력 전달
		AddMovementInput(FVector(1.0f, 0.0f, 0.0f), moveVector);

		// 스프라이트 방향 제어 (PaperZD와 연동 시 가장 깔끔한 방식)
		// 캐릭터 몸체는 가만히 두고 Sprite 컴포넌트만 Yaw 값을 조절해 반전시킵니다.
		float targetYaw = (moveVector < 0.f) ? 180.f : 0.f;
		GetSprite()->SetRelativeRotation(FRotator(0.f, targetYaw, 0.f));
	}
}

void ADrillerCharacter::Dig()
{
	UE_LOG(LogTemp, Warning, TEXT("Dig Action Triggered!"));
}

void ADrillerCharacter::HandleDeath()
{
	// 사망 애니메이션 처리
}
