// Fill out your copyright notice in the Description page of Project Settings.


#include "DrillerCharacter.h"

#include "Block.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ADrillerCharacter::ADrillerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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
		// 왼쪽(< 0)일 때는 원본 그대로 0도, 오른쪽일 때 180도 회전
		float targetYaw = (moveVector < 0.f) ? 0.f : 180.f;
		GetSprite()->SetRelativeRotation(FRotator(0.f, targetYaw, 0.f));
	}
}

void ADrillerCharacter::Dig()
{
	// 1. 레이캐스트 시작점(캐릭터의 현재 위치)을 가져옵니다.
	FVector rayStart = GetActorLocation();
	// 2. 기본 채굴 방향은 아래쪽(Z: -1)으로 설정합니다.
	FVector digDirection = FVector(0.f, 0.f, -1.f);
	// 2-1. 캐릭터에 입력된 마지막 이동 벡터를 가져옵니다.
	FVector lastInput = GetLastMovementInputVector();
	// 2-2. X축 입력이 존재한다면 (좌/우 이동 키를 누르고 있다면) 방향을 덮어씁니다.
	if (!FMath::IsNearlyZero(lastInput.X))
	{
		// 입력값의 부호(+1 혹은 -1)에 따라 파는 방향을 결정합니다.
		float dirX = FMath::Sign(lastInput.X);
		digDirection = FVector(dirX, 0.f, 0.f);
	}
	// 3. 레이캐스트 끝점(시작점 + (바라보는 방향 * 채굴 사거리))을 계산합니다.
	FVector rayEnd = rayStart + (digDirection * 42.f); // Z축 아래로 42 유닛
	// 4. FHitResult 구조체를 선언하여 충돌 결과를 담을 바구니를 준비합니다.
	FHitResult hit;
	// 5. GetWorld()->LineTraceSingleByChannel(...) 함수를 호출하여 레이를 발사합니다.
	FCollisionQueryParams params;
	params.AddIgnoredActor(this); // 자기 자신을 레이캐스트 대상에서 제외합니다.
	bool bHit = GetWorld()->LineTraceSingleByChannel(hit, rayStart, rayEnd, ECC_Visibility, params);
	
	// 디버그 라인 그리기 (월드, 시작점, 끝점, 색상, 지속여부, 수명, 깊이 우선, 굵기) 전처리기 사용
#if WITH_EDITOR
	DrawDebugLine(GetWorld(), rayStart, rayEnd, FColor::Red, false, 2.0f, 0, 2.0f);
#endif
	
	// 6. 충돌한 액터가 있다면 HitResult에서 가져와 ABlock 클래스로 Cast<ABlock>을 시도합니다.
	if (bHit)
	{
		// 부딪힌 액터가 ABlock 타입인지 확인하고 안전하게 캐스팅합니다.
		ABlock* HitBlock = Cast<ABlock>(hit.GetActor());
    
		// 6. 캐스팅에 성공했다면 해당 블록의 OnInteracted(this) 함수를 호출합니다.
		if (HitBlock != nullptr)
		{
			HitBlock->OnInteracted(this);
		}
	}
}

void ADrillerCharacter::HandleDeath()
{
	// 사망 애니메이션 처리
}
