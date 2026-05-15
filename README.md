# ⛏️ 미니 드릴러 (Mini Driller)

## 📖 프로젝트 개요

**미니 드릴러**는 2주 안에 Core Loop 완성을 목표로 개발된 Mr.Driller Clone **2D 타임어택 무한 채굴 아케이드 게임**입니다.

메모리 최적화(Object Pooling), 다형성, 그리고 언리얼 서브시스템(Subsystem)을 활용한 견고한 C++ 아키텍처 설계를 증명하기 위한 포트폴리오 프로젝트입니다.

* **장르:** 2D 타임어택 무한 채굴 아케이드
* **개발 엔진:** Unreal Engine 5.6.1
* **핵심 메커니즘:** 블록 파괴 ➡️ 캐릭터 및 블록 하강 ➡️ 산소 및 난이도 관리 ➡️ 깊이에 따른 랜덤 풀링 재배치

## 🛠 기술 스택 및 플러그인

* **Core:** C++, Unreal Engine 5.6.1
* **2D Rendering \& Animation:** Paper2D, PaperZD (2D 애니메이션 상태 머신 제어), Paper2D+
* **VFX:** Niagara (채굴 및 파괴 파티클 이펙트)

## 🏛 핵심 클래스 아키텍처

유니티의 `MonoBehaviour` 의존적인 구조에서 벗어나, 언리얼 엔진의 철학에 맞춘 클래스 설계를 적용했습니다.

* **`UGameStatusSubsystem` (전역 상태 매니저):** 엔진 수명 주기에 맞춰 난이도, 산소량, 점수를 관리합니다. 델리게이트를 통해 플레이어 사망 시 시스템 간의 결합도(Coupling)를 낮췄습니다.
* **`AMapManager` (오브젝트 풀링 \& 맵 생성기):** 유니티의 코루틴/Instantiate 방식 대신 `TArray::Reserve`로 메모리를 사전 할당하고 화면 밖의 블록을 재사용(Recycle)하여 가비지 컬렉션(GC) 스파이크를 방지합니다.
* **`ABlock` (다형성 기반 부모 클래스):** 모든 채굴 객체의 부모입니다. `virtual void OnInteracted()`를 통해 일반 블록, 장애물, 아이템 등 다양한 하위 클래스의 행동을 깔끔하게 확장합니다.
* **`ADrillerCharacter` (플레이어 캐릭터):** `APaperCharacter`를 상속받으며, PaperZD Notify를 활용해 코드와 애니메이션 상태를 동기화합니다.

## 🎮 조작 방법

* **`A` / `D` (방향키 좌우):** 캐릭터 이동
* **`Spacebar`:** 바라보는 방향(또는 아래) 채굴

## 🚀 개발 진행 상황 (2주 스프린트)

* \[x] **Phase 1:** 기초 셋업 및 코어 아키텍처 (GameStatusSubsystem, MapManager 뼈대 구축)
* \[x] **Phase 2:** 코어 메카닉 기초 (Enhanced Input 적용, 캐릭터 이동 및 채굴 모션 연동)
* \[x] **Phase 3:** 채굴 및 파괴 (Object Pooling 반환 로직 및 레이캐스트 기반 타격)
* \[x] **Phase 4:** 중력 및 낙하 시스템 (Raycast 기반 빈 공간 감지, 플레이어 압사 판정)
* \[x] **Phase 5:** 매치 4 퍼즐 시스템 (Flood Fill 알고리즘을 활용한 동일 색상 연쇄 파괴)
* \[ ] **Phase 6:** 코어 게임 루프(Core Game Loop) 구축 및 서브시스템 도입

## 📌 References \& Credits

* **Game Reference:** Mr. Driller 시리즈

