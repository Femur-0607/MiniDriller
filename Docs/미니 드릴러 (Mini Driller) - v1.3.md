## 1. 프로젝트 개요 (Overview)

- **장르** 2D 타임어택 무한 채굴 아케이드.
- **개발 엔진** Unreal Engine 5.6.1.
- **핵심 목표** 2주 내 Core Loop 완성 및 C++ 핵심 역량(메모리 풀링, 다형성, 서브시스템) 증명.
- **Core Loop** 블록 파괴 ➡️ 캐릭터 하강 ➡️ 산소 및 난이도 관리 ➡️ 깊이에 따른 랜덤 풀링 재배치.

## 2. 참고 자료 (References)

- **게임 플레이 레퍼런스**
    - https://kr.game-game.com/192620/
    - https://canarigames.itch.io/picodriller
    - https://youtube.com/shorts/34uwbWnwLio?si=1xp-6Mj6w1mJ3CLN
    - [미스터 드릴러 나무위키](https://namu.wiki/w/%EB%AF%B8%EC%8A%A4%ED%84%B0%20%EB%93%9C%EB%A6%B4%EB%9F%AC%20%EC%8B%9C%EB%A6%AC%EC%A6%88)

## 3. 에셋 및 플러그인 (Assets & Plugins)

- **필수 플러그인**
    - `Paper2D` 2D 스프라이트 및 타일맵 렌더링.
    - `PaperZD` 2D 애니메이션 상태 머신(FSM) 및 노티파이 제어.
    - `Niagara` 파티클 시스템 구현용 내장 플러그인.
- **에셋 수급처**
    - https://kenney.nl/ (UI, 기본 블록 타일).
    - https://itch.io/ (2D Pixel 팩 - 캐릭터 애니메이션).
    - https://www.spriters-resource.com/ (미스터 드릴러 레퍼런스).
        - Gemini 이미지 생성, 음악 생성
        - 에픽게임즈 - 팹 (vfx, 효과음 등)

## 4. 핵심 클래스 설계 (Class Architecture)

### ① `UGameStatusSubsystem` (전역 상태 매니저)

- **역할** 엔진에 의해 수명이 관리되며, 게임 오버 전까지 유지되는 데이터 및 난이도를 관리합니다.
- **주요 변수**
    - `float CurrentOxygen` 현재 산소량.
    - `int32 CurrentDepth` 현재 도달한 깊이.
    - `int32 CurrentLevel` 현재 레벨 (100칸 단위로 증가).
    - `int32 TotalScore` 획득 점수.
- **주요 함수**
    - `void AddScore(int32 Amount)` 점수 추가.
    - `void ConsumeOxygen(float Amount)` 산소 소모 (레벨에 따라 소모량 증가).
    - `bool IsGameOver() const` 산소가 0 이하로 떨어지면 true 반환.
    - `void CheckLevelUp()` 깊이가 100 증가할 때마다 레벨업 및 맵 매니저에 폭발 이벤트 트리거.

### ② `AMapManager` (오브젝트 풀링 & 라인 재배치 매니저)

**역할** 화면에 보이는 만큼의 블록만 메모리에 유지하고, 카메라가 내려가면 화면 밖 상단 블록을 맨 아래로 순환시킵니다.

- **주요 변수**
    - `TArray<ABlock*> BlockPool` 재활용 블록 배열 (화면 높이 + 여유분 약 20줄).
    - `float TileSize` 타일의 기준 크기.
- **주요 함수**
    - `void InitializeMap()` Reserve를 통한 초기 여유분 블록 풀 스폰 및 배치.
    - `void RecycleTopLine()` 플레이어가 특정 깊이 이상 내려갈 때마다 호출되어, 최상단 라인의 블록들을 맨 아래 라인으로 이동시킵니다. 이동 시 확률에 따라 아이템이나 장애물로 속성을 변환합니다.
    - `void OnLevelUpExplosion()` 100칸 도달 시 화면 내 모든 일반 블록을 파괴하고 나이아가라 효과 재생.

### ③ `ABlock` (부모 클래스 - 다형성 적용)

- **역할** 모든 채굴 대상 및 상호작용 오브젝트의 기본형이며, 자식 클래스로 확장합니다.
- **자식 클래스 구성**
    - `ANormalBlock` 1회 타격 시 파괴.
    - `AObstacleBlock` 5회 타격 시 파괴되며, 파괴 시 산소량 10% 감소 페널티 적용.
    - `AItemBlock` 플레이어와 상호작용 시 파괴되며 산소와 점수 상승.
- **주요 함수**
    - `virtual void OnInteracted(ADrillerCharacter* Player)` 드릴과 충돌 및 상호작용 시 실행될 다형성 함수.
    - `void ResetBlock(EBlockType NewType)` 풀링 재배치 시 레벨업 확률에 따라 속성 덮어쓰기.

### ④ `ADrillerCharacter` (플레이어 캐릭터)

- **역할** 카메라를 컴포넌트로 가지며 사용자 입력 처리 및 애니메이션 갱신을 담당합니다.
- **조작계**
    - 스페이스바 (Spacebar) 채굴 명령.
    - 방향키 (Down, Left, Right) 이동 및 카메라 하강 유도.
- **주요 함수**
    - `void Dig()` 채굴 명령.
    - `void HandleDeath()` 산소 고갈 시 사망 연출 처리.

## 5. 핵심 메커니즘 (Core Mechanics)

- **카메라 하강 및 순환 풀링** 캐릭터가 블록을 파고 아래로 내려가면 카메라가 따라 내려갑니다. 화면 위로 벗어난 블록은 맵 매니저에 의해 다시 제일 아래쪽으로 위치가 이동(`SetActorLocation`)되어 무한한 깊이를 연출합니다.
- **확률형 타일 생성** 상단에서 하단으로 블록이 재배치될 때, 난수 생성(`FMath::RandRange`)을 통해 블록이 일반 블록, 장애물, 또는 산소 아이템으로 변환됩니다.
- **레벨업 및 난이도 스케일링** 100칸을 파고 내려갈 때마다 레벨이 오릅니다. 레벨업 시 장애물 출현 확률 증가, 산소통 아이템 출현 확률 감소, 기본 산소 감소 속도 증가가 적용됩니다.
- **게임 오버 조건** 산소통(타이머)이 0에 도달하면 캐릭터가 사망하며 게임이 종료됩니다.

## 6. UI 및 연출 (UI & Polish)

- **메인 메뉴** 사운드 음소거 토글 버튼 및 개발자(본인) 이름 표시(크레딧) 기능 구현.
- **사운드** 깊이가 깊어질수록 리버브(Reverb) 효과 추가 및 배경음 속도 증가.
- **시각 피드백**
    - 채굴 및 레벨업 폭발 시 나이아가라(Niagara) 기반 파티클 연출.
    - 채굴 성공 시 카메라 진동(Camera Shake) 추가.
- **추가 목표 (시간 여유 시)**
    - 플레이어의 달성률을 보여주는 업적창 UI 구현.
    - 블록별로 색상을 나눠서 인접한 색상블록 파괴.
    - 세이브 데이터 구현.