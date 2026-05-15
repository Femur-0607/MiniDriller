// Fill out your copyright notice in the Description page of Project Settings.


#include "DrillerCharacter.h"
#include "Block.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameStatusSubsystem.h"
#include "Components/CapsuleComponent.h"

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
		
		// 시작할 때는 계단을 못 올라가게 0으로 설정합니다.
		GetCharacterMovement()->MaxStepHeight = 0.0f;
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
	
	// 시작할 때의 높이를 최저점으로 초기화합니다. (2D 플랫포머는 보통 Z축이 위아래입니다)
	lowestZ = GetActorLocation().Z;
}

void ADrillerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 승천(Die2) 상태라면 매 프레임 위로(Z축 양수) 이동시킵니다.
	if (bIsFloating)
	{
		// 스프라이트만 위로 부드럽게 둥둥 띄웁니다. (속도는 150.f, 취향껏 조절!)
		GetSprite()->AddRelativeLocation(FVector(0.f, 0.f, 150.f * DeltaTime));
	}
	
	// 1. 현재 Z축 위치를 가져옵니다.
	float currentZ = GetActorLocation().Z;

	// 2. 현재 위치가 '기록된 최저점'보다 42(블록 1칸) 이상 더 아래로 내려갔다면?
	if (currentZ <= lowestZ - 42.0f)
	{
		// 3. 최저점을 한 칸(42) 아래로 갱신합니다.
		lowestZ -= 42.0f; 

		// 4. 서브시스템에 논리적 깊이(5m)를 추가합니다!
		if (UGameStatusSubsystem* status = GetWorld()->GetSubsystem<UGameStatusSubsystem>())
		{
			status->AddDepth(5); 
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
	if (bIsDead || bIsDigging) return;
    
	float moveVector = value.Get<float>();
	if (moveVector == 0.f) return;

	float inputDir = FMath::Sign(moveVector);
    
	// 1. 방향 전환 시 즉시 회전
	if (inputDir != currentFacingDir)
	{
		currentFacingDir = inputDir;
		GetSprite()->SetRelativeRotation(FRotator(0.f, (currentFacingDir < 0.f) ? 0.f : 180.f, 0.f));
		climbEnableTimer = 0.0f; // 방향 바꾸면 타이머 초기화
	}

	// 2. 앞에 벽(블록)이 있는지 체크
	FHitResult wallHit;
	FVector start = GetActorLocation();
	FVector end = start + FVector(inputDir * RayIntersection, 0.0f, 0.0f);
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	bool bHitBlock = GetWorld()->LineTraceSingleByChannel(wallHit, start, end, ECC_Visibility, params) && Cast<ABlock>(wallHit.GetActor());

	if (bHitBlock)
	{
		// 벽을 향해 계속 밀고 있다면 타이머 증가
		climbEnableTimer += GetWorld()->GetDeltaSeconds(); // Move가 매 프레임 호출되므로 누적 가능

		if (climbEnableTimer >= CLIMB_DELAY)
		{
			// 지연 시간이 지났으므로 계단 오르기 능력 개방!
			GetCharacterMovement()->MaxStepHeight = DEFAULT_STEP_HEIGHT;
		}
	}
	else
	{
		// 벽이 없으면 즉시 능력 회수 및 타이머 초기화
		climbEnableTimer = 0.0f;
		GetCharacterMovement()->MaxStepHeight = 0.0f;
	}

	// 이동 입력은 항상 넣어줍니다. (MaxStepHeight가 0이면 알아서 못 가고 비비기만 합니다)
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), moveVector);
}

void ADrillerCharacter::Dig()
{
	if (bIsDead) return;
	
	// 1. 디깅 연타로 인해 애니메이션이 1프레임만에 계속 리셋되는 것을 방지합니다.
	// (이 줄이 들어가야 연속 파기를 할 때 모션이 꼬이지 않습니다!)
	if (bIsDigging) return;
	
	StartDigging();
	
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
		
		// 옆으로 파기 상태 ON
		bIsDiggingSide = true;
	}
	else
	{
		// 아래로 파기 상태 (X축 입력이 없을 때)
		bIsDiggingSide = false;
	}
	
	// 3. 레이캐스트 끝점(시작점 + (바라보는 방향 * 채굴 사거리))을 계산합니다.
	FVector rayEnd = rayStart + (digDirection * RayIntersection); // Z축 아래로 30 유닛
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
    
		// 7. 캐스팅에 성공했다면 해당 블록의 OnInteracted(this) 함수를 호출합니다.
		if (HitBlock != nullptr)
		{
			HitBlock->OnInteracted(this);
			
			// 4. [핵심 2] 아래로 파기에 성공했다면 속도 주입 (스매시)
			if (!bIsDiggingSide)
			{
				// ✅ [핵심 해결책] 엔진에게 강제로 "낙하 모드"로 전환하라고 명령합니다!
				// 이렇게 하면 42만큼 얕게 파더라도 계단 스냅(순간이동)이 발생하지 않고 부드럽게 떨어집니다.
				GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			}
		}
	}
}

bool ADrillerCharacter::GetIsDigging() const
{
	return bIsDigging;
}

bool ADrillerCharacter::GetIsDiggingSide() const
{
	return bIsDiggingSide;
}

void ADrillerCharacter::StartDigging()
{
	if (!bIsDigging)
	{
		bIsDigging = true;
		
		// 현재 속도에서 좌우(X)만 0으로 만들고, 중력(Z)은 그대로 둡니다!
		FVector currentVelocity = GetCharacterMovement()->Velocity;
		currentVelocity.X = 0.0f; 
		GetCharacterMovement()->Velocity = currentVelocity;
	}
}

// ABP에서 디깅 애니메이션 끝나면 호출하게끔 설정해놓음
void ADrillerCharacter::StopDigging()
{
	if (bIsDigging)
	{
		bIsDigging = false;
	}
}

void ADrillerCharacter::HandleDeath()
{
	if (bIsDead) return; 
    
	bIsDead = true; // ABP가 이 값을 보고 즉시 'Die1' 애니메이션을 틉니다.

	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADrillerCharacter::StartGhostFloat()
{
	// 승천 상태 ON! (Tick 함수에서 위로 올라가기 시작하고, ABP가 'Die2' 애니메이션을 틉니다)
	bIsFloating = true;

	// 기획하신 대로 정확히 2초(2.0f) 뒤에 애니메이션을 끝내고 이벤트를 호출합니다.
	GetWorld()->GetTimerManager().SetTimer(deathTimerHandle, this, &ADrillerCharacter::FinalizeDeath, 2.0f, false);
}

void ADrillerCharacter::FinalizeDeath()
{
	// 승천 종료
	bIsFloating = false;
    
	// 시각적으로 완전히 투명하게(사라지게) 만듭니다.
	GetSprite()->SetHiddenInGame(true);

	// 드디어 서브시스템에 찐 사망 소식을 방송(Broadcast)하여 게임 오버 창을 띄우게 합니다!
	if (UWorld* World = GetWorld())
	{
		if (UGameStatusSubsystem* StatusSubsystem = World->GetSubsystem<UGameStatusSubsystem>())
		{
			StatusSubsystem->NotifyPlayerDeath(); 
		}
	}
}

// --- Getter 함수 구현 ---
bool ADrillerCharacter::GetIsDead() const { return bIsDead; }
bool ADrillerCharacter::GetIsFloating() const { return bIsFloating; }
