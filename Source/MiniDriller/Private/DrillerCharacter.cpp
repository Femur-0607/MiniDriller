// Fill out your copyright notice in the Description page of Project Settings.


#include "DrillerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
ADrillerCharacter::ADrillerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	SetupCamera();
}

void ADrillerCharacter::SetupCamera()
{
	springArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	springArmComponent->SetupAttachment(RootComponent);
    
	// 2. 2D 뷰를 위한 스프링 암 설정
	springArmComponent->TargetArmLength = 500.f; // 카메라와 캐릭터 사이의 거리
	springArmComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f)); // 캐릭터의 옆면을 바라보도록 회전
	springArmComponent->bDoCollisionTest = false; // 2D 게임이므로 장애물 충돌 검사 제외
	springArmComponent->bInheritPitch = false;
	springArmComponent->bInheritYaw = false;
	springArmComponent->bInheritRoll = false;

	// 3. 카메라 생성 및 부착
	cameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	cameraComponent->SetupAttachment(springArmComponent);

	// 4. 직교(Orthographic) 투영 설정
	cameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	cameraComponent->OrthoWidth = 500.f; // 화면에 보여질 가로 범위 (해상도에 맞춰 조절)

	// 5. 9:16 종횡비 고정 설정
	cameraComponent->AspectRatio = 9.f / 16.f;
	cameraComponent->bConstrainAspectRatio = true; // 화면 비율을 강제로 고정하여 레터박스 생성
	
	// 카메라 래그 설정을 명시적으로 끕니다. (기본값이 false이나 가독성을 위해 작성합니다.)
	springArmComponent->bEnableCameraLag = false;
	springArmComponent->bEnableCameraRotationLag = false;

	// 2D 게임에서는 캐릭터가 회전해도 카메라가 같이 돌지 않게 방지하는 것이 중요합니다.
	springArmComponent->SetUsingAbsoluteRotation(true);
}

// Called when the game starts or when spawned
void ADrillerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called to bind functionality to input
void ADrillerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

