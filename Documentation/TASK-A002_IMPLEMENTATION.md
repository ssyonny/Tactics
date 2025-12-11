# TASK-A002: 플레이어 이동 및 기본 공격 구현

> 시작일: 2025-11-15
> 예상 기간: 3일
> 담당자: ssyonny

---

## 📋 현재 상태 분석

### ✅ 이미 구현된 것
- Enhanced Input System 활성화됨
- TacticsCharacter 클래스 (TopDown 카메라 설정 완료)
- TacticsPlayerController 클래스 (마우스 클릭 이동)
- SpringArm + Camera 컴포넌트 구성
- Input Mapping Context 구조

### ❌ 추가 필요한 것
- WASD 8방향 이동
- 마우스 방향으로 캐릭터 회전
- 기본 공격 시스템
- 공격 애니메이션
- 데미지 계산 시스템

---

## 🎯 Day 1: Enhanced Input + 이동 시스템 (2025-11-15)

### 목표
- WASD 이동 Input Action 생성
- 캐릭터 이동 로직 구현
- 마우스 커서 방향으로 캐릭터 회전

### 구현 계획

#### 1. Input Actions 추가
파일 위치: `Content/TopDown/Input/`
- `IA_Move` (Vector2D): WASD 키 매핑
- `IA_Attack` (Button): 마우스 왼쪽 클릭

#### 2. TacticsPlayerController 확장
```cpp
// 새로운 Input Actions
UPROPERTY(EditAnywhere, Category="Input")
UInputAction* MoveAction;

UPROPERTY(EditAnywhere, Category="Input")
UInputAction* AttackAction;

// 새로운 함수
void OnMoveTriggered(const FInputActionValue& Value);
void OnAttackTriggered();
```

#### 3. TacticsCharacter 확장
```cpp
// 마우스 방향으로 회전
void RotateTowardsCursor();

// 공격 함수
void PerformAttack();

// 속성
UPROPERTY(EditAnywhere, Category="Combat")
float BaseDamage = 20.0f;

UPROPERTY(EditAnywhere, Category="Combat")
float AttackRange = 200.0f;

UPROPERTY(EditAnywhere, Category="Combat")
float AttackCooldown = 0.5f;
```

### 수용 기준
- [x] WASD로 8방향 이동 가능
- [x] 캐릭터가 마우스 커서 방향을 바라봄 (C++ 구현 완료)
- [ ] 입력 지연 < 100ms 체감 (테스트 필요)
- [ ] 애니메이션 전환 부드러움 (애니메이션 설정 필요)

### 코드 정리 완료 (2025-12-11)
- [x] 중복 include 제거
- [x] 테스트 함수 정리 (TestPerformAttack, EmergencyAttackTest)
- [x] Warning 로그를 Log/Verbose로 변경
- [x] ForceRotateToDirection 함수 정리

---

## 🎯 Day 2: 마우스 조준 + 카메라 (예정)

### 목표
- 마우스 월드 포지션 계산
- 캐릭터 자동 회전
- 카메라 설정 최적화

---

## 🎯 Day 3: 기본 공격 시스템 (예정)

### 목표
- 공격 입력 처리
- Box/Sphere Trace로 타격 감지
- 데미지 공식 적용
- 공격 이펙트

---

## 📝 작업 로그

### 2025-11-15 14:30
- 프로젝트 구조 분석 완료
- 기존 TacticsCharacter, TacticsPlayerController 확인
- Enhanced Input System 이미 구현되어 있음 확인
- Day 1 작업 시작

