// 파괴 및 중력 로직을 포함한 블록 액터 구현부


#include "MiniDriller/Public/Block.h"

#include "DrillerCharacter.h"
#include "PaperSpriteComponent.h"
#include "PaperFlipbookComponent.h"


// Sets default values
ABlock::ABlock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// 1. 형태가 없는 빈 씬 컴포넌트를 생성하고 루트(중심)로 설정합니다.
	USceneComponent* defaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	RootComponent = defaultRoot;

	// 2. 스프라이트를 생성하고 루트 컴포넌트의 자식으로 부착합니다.
	// 스프라이트를 상위로 할 경우 현재 위치한 로컬을 받아오므로 빈 상위 컴포넌트를 만들어야 효과(흔들림)을 구현할 수 있음
	spriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("MySprite"));
	spriteComponent->SetupAttachment(RootComponent);
	
	// 3. 파괴 이펙트(플립북) 컴포넌트 부착 및 초기 세팅
	destructionEffectComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("DestructionEffect"));
	destructionEffectComponent->SetupAttachment(RootComponent);
	destructionEffectComponent->SetLooping(false); // 반복 재생 금지
}

// Called when the game starts or when spawned
void ABlock::BeginPlay()
{
	Super::BeginPlay();
	
	// 시작할 때는 이펙트가 안 보이게 숨겨둡니다.
	if (destructionEffectComponent)
	{
		destructionEffectComponent->SetHiddenInGame(true);
        
		// 애니메이션이 끝났을 때(OnFinishedPlaying) 내 함수(OnDestructionEffectFinished)를 실행하도록 연결합니다.
		//OnFinishedPlaying는 사운드, 비디오, 애니메이션 또는 레벨 시퀀스가 재생을 마치고 끝에 도달했을때 실행되는 콜백 함수나 이벤트
		destructionEffectComponent->OnFinishedPlaying.AddDynamic(this, &ABlock::OnDestructionEffectFinished);
	}
	
}

void ABlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	switch (currentState)
	{
		case EBlockState::Idle:
			// Idle 상태일 때는 Tick이 꺼져 있어야 하므로, 여기 들어올 일은 원칙적으로 없습니다.
			break;
			
		case EBlockState::Anticipating:
			// [흔들림 연출] 스프라이트를 위,아래로 흔듭니다.
			// GetTimeSeconds()는 게임이 시작된 후 흐른 총 시간입니다.
			// 속도(예: 50.f)를 곱해 진동 주기를 결정하고, 진폭(예: 3.f)을 곱해 흔들리는 거리를 결정합니다.
			shakeOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * 50.f) * 3.f;
			spriteComponent->SetRelativeLocation(FVector(0.f, 0.f, shakeOffset));
			break;

		case EBlockState::Falling:
			// 1. 현재 위치 가져오기
			FVector currentLocation = GetActorLocation();

			// 2. 등속 보간(MoveTowards)으로 다음 프레임 위치 계산 (이동 속도는 임의로 300.f 설정)
			FVector nextLocation = FMath::VInterpConstantTo(currentLocation, targetLocation, DeltaTime, 300.f);

			// 3. 액터 위치 업데이트
			SetActorLocation(nextLocation);

			// 4. 도착 판정 (목표 위치와의 거리가 1.0f 미만이면 도착한 것으로 간주)
			if (FVector::Dist(currentLocation, targetLocation) < 1.0f)
			{
				currentState = EBlockState::Idle;
				SetActorTickEnabled(false);
				
				// 내 발밑(-1.f)에 액터가 없다면(nullptr) 계속 떨어져라!
				if (GetActorInDirection(FVector(0.f, 0.f, -1.f)) == nullptr)
				{
					CheckAndFall();
				}
			}
			break;
	}
}

// --- 상호작용 및 파괴 시스템 ---
#pragma region Interaction System
// 플레이어가 블록 파괴시 실행 되는 함수
void ABlock::OnInteracted(class ADrillerCharacter* Player)
{
	// 0. 흔들리는 애니메이션 초기화
	GetWorld()->GetTimerManager().ClearTimer(anticipationTimerHandle);
	
	// 1. 기존 스프라이트 숨기기 및 물리 충돌 끄기 (SetActorHiddenInGame은 자식까지 다 숨기므로 금지!)
	spriteComponent->SetHiddenInGame(true);
	SetActorEnableCollision(false);
	
	// 2. 내 위쪽에 액터가 있는지 확인하고, 그게 ABlock 타입이라면 깨워라!
	AActor* hitActor = GetActorInDirection(FVector(0.f, 0.f, 1.f));
	if (hitActor != nullptr)
	{
		ABlock* upperBlock = Cast<ABlock>(hitActor);
		if (upperBlock != nullptr)
		{
			upperBlock->CheckAndFall();
		}
	}
	
	// 3. 숨겨뒀던 파괴 이펙트를 보이게 하고 재생 시작!
	if (destructionEffectComponent)
	{
		destructionEffectComponent->SetHiddenInGame(false);
		destructionEffectComponent->PlayFromStart(); // 처음부터 재생
	}
}

// 블록 파괴 애니메이션이 끝나면 호출되는 함수(블럭을 풀로 되돌림)
void ABlock::OnDestructionEffectFinished()
{
	// 애니메이션이 완전히 끝났으므로, 이제 MapManager에게 나를 회수(Pool)하라고 알립니다.
	if (onBlockDestroyedDelegate.IsBound())
	{
		onBlockDestroyedDelegate.Broadcast(this);
	}

	// 풀에 들어갔다가 다시 나올 때를 대비해 상태를 초기화합니다.
	destructionEffectComponent->SetHiddenInGame(true);
}

void ABlock::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	// 1. 부딪힌 대상이 존재하고, 현재 블록이 '낙하 중(Falling)'일 때만 검사합니다.
	if (OtherActor != nullptr && currentState == EBlockState::Falling)
	{
		TryKillPlayer(OtherActor);
	}
}

void ABlock::TryKillPlayer(AActor* TargetActor)
{
	// 1. 방어 코드 (null 체크)
	if (TargetActor == nullptr) return;

	// 2. 플레이어인지 확인하고 맞다면 처형!
	if (ADrillerCharacter* Player = Cast<ADrillerCharacter>(TargetActor))
	{
		Player->HandleDeath();
	}
}
#pragma endregion

// --- 중력 및 낙하 시스템 ---
#pragma region Falling System
//  블록 상태 변환 떨어질 준비하면서 효과 발생
void ABlock::CheckAndFall()
{
	if (currentState != EBlockState::Idle) return;
	
	currentState = EBlockState::Anticipating;
	SetActorTickEnabled(true);
	targetLocation = GetActorLocation() + FVector(0.f, 0.f, -42.f);
	
	// [핵심] 언리얼의 타이머 매니저: 1초 뒤에 StartFalling 함수를 1회(false) 실행해라!
	GetWorld()->GetTimerManager().SetTimer(anticipationTimerHandle, this, &ABlock::StartFalling, 1.f, false);
	
	// 내 위쪽(1.f)에 액터가 있는지 확인하고, 그게 ABlock 타입이라면 깨워라!
	AActor* hitActor = GetActorInDirection(FVector(0.f, 0.f, 1.f));
	if (hitActor != nullptr)
	{
		ABlock* upperBlock = Cast<ABlock>(hitActor);
		if (upperBlock != nullptr)
		{
			upperBlock->CheckAndFall();
		}
	}
}

// 실제로 떨어지는 로직
void ABlock::StartFalling()
{
	// 타이머에 의해 0.2초 뒤 호출됨
	spriteComponent->SetRelativeLocation(FVector::ZeroVector); // 위치 원상복구
	currentState = EBlockState::Falling; // 상태를 Falling으로 넘김 (이제 Tick에서 부드럽게 떨어지기 시작함)
	
	// --- [버그 픽스: 낙하 시작 시점에 이미 겹쳐있는 플레이어 압사 처리] ---
	// 불필요해서 수정함
	// --- [최적화된 버그 픽스: 배열과 반복문 없이 단일 플레이어만 직접 검사] ---
	// 1. 현재 월드에 존재하는 첫 번째 플레이어(0번 플레이어)를 가져옵니다.
	if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
	{
		// 2. 이 블록(this)이 방금 찾은 플레이어와 겹쳐 있는지(Overlap) 가볍게 확인합니다.
		if (IsOverlappingActor(PlayerPawn))
		{
			// 3. 겹쳐 있다면 우리가 아까 만든 깔끔한 처형 함수를 호출합니다!
			TryKillPlayer(PlayerPawn);
		}
	}
}
#pragma endregion

AActor* ABlock::GetActorInDirection(FVector Direction, float Distance)
{
	FVector rayStart = GetActorLocation();
	FVector rayEnd = rayStart + (Direction * Distance); // 매개변수로 받은 방향과 거리만큼 쏨
    
	FHitResult hit;
	// 자기자신은 레이 무시
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
    
	bool bHit = GetWorld()->LineTraceSingleByChannel(hit, rayStart, rayEnd, ECC_Visibility, params);
    
	// 전처리기로 에디터 내에서만 레이 보이게끔
#if WITH_EDITOR
	DrawDebugLine(GetWorld(), rayStart, rayEnd, FColor::Blue, false, 2.0f, 0, 2.0f);
#endif

	// 부딪힌 게 있으면 그 액터를 반환하고, 없으면 nullptr(빈 공간) 반환
	return bHit ? hit.GetActor() : nullptr; 
}
