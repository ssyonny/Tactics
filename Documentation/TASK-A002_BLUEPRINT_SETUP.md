# TASK-A002 Day 2: Blueprint 설정 가이드

> 날짜: 2025-11-15
> 작업: Unreal Editor에서 Input Actions 및 Blueprint 설정

---

## 📋 작업 체크리스트

### ✅ 완료
- [x] C++ 코드 구현 (Day 1)
- [x] Git 커밋 & 푸시

### 🔄 진행 중 (Day 2)
- [ ] Input Action 생성 (IA_Move)
- [ ] Input Action 생성 (IA_Attack)
- [ ] Input Mapping Context 업데이트
- [ ] Blueprint 설정
- [ ] 에디터 테스트

---

## 🎯 Step 1: IA_Move 생성

### 위치
`Content/TopDown/Input/Actions/`

### 생성 방법
1. Content Browser에서 우클릭
2. Input → Input Action 선택
3. 이름: `IA_Move`

### 설정
- **Value Type**: `Axis2D (Vector2D)`
- **Description**: "WASD movement input"

---

## 🎯 Step 2: IA_Attack 생성

### 위치
`Content/TopDown/Input/Actions/`

### 생성 방법
1. Content Browser에서 우클릭
2. Input → Input Action 선택
3. 이름: `IA_Attack`

### 설정
- **Value Type**: `Digital (bool)`
- **Description**: "Attack input"

---

## 🎯 Step 3: Input Mapping Context 업데이트

### 파일 열기
`Content/TopDown/Input/IMC_Default.uasset`

### 추가할 매핑

#### IA_Move 매핑
| Key | Modifiers | Value |
|-----|-----------|-------|
| **W** | - | X: 0.0, Y: 1.0 |
| **S** | - | X: 0.0, Y: -1.0 |
| **A** | - | X: -1.0, Y: 0.0 |
| **D** | - | X: 1.0, Y: 0.0 |

#### IA_Attack 매핑
| Key | Modifiers | Value |
|-----|-----------|-------|
| **Left Mouse Button** | - | (default) |
| **Gamepad Face Button Bottom** | - | (optional) |

---

## 🎯 Step 4: Blueprint 설정

### 파일 찾기
`Content/TopDown/Blueprints/BP_TopDownPlayerController` 또는
`Content/TopDown/Blueprints/BP_TacticsPlayerController`

### 설정할 속성
Blueprint를 열고 **Default Values**에서:

#### Input Actions 레퍼런스
- **Move Action**: `IA_Move` 선택
- **Attack Action**: `IA_Attack` 선택

#### 기존 설정 유지
- **Default Mapping Context**: `IMC_Default` (이미 있음)
- **Set Destination Click Action**: 기존 유지
- **Set Destination Touch Action**: 기존 유지

---

## 🎯 Step 5: Character Blueprint 설정 (선택)

### 파일 찾기
`Content/TopDown/Blueprints/BP_TopDownCharacter` 또는
`Content/TopDown/Blueprints/BP_TacticsCharacter`

### Combat 속성 조정
- **Base Damage**: 20.0 (기본값)
- **Attack Range**: 200.0 (기본값)
- **Attack Cooldown**: 0.5 (기본값)

### 테스트용으로 값 조정 권장
- **Base Damage**: 50.0 (더 쉽게 확인)
- **Attack Range**: 300.0 (더 넓은 범위)
- **Attack Cooldown**: 0.3 (더 빠른 공격)

---

## 🎯 Step 6: 테스트

### Play In Editor (PIE)
1. Toolbar에서 **Play** 버튼 클릭 (또는 Alt+P)
2. 테스트 항목:

#### WASD 이동
- [ ] W: 앞으로 이동
- [ ] S: 뒤로 이동
- [ ] A: 왼쪽으로 이동
- [ ] D: 오른쪽으로 이동
- [ ] WA, WD, SA, SD: 대각선 이동

#### 마우스 회전
- [ ] 마우스 커서를 움직이면 캐릭터가 그 방향을 바라봄
- [ ] 이동 중에도 회전 동작

#### 공격
- [ ] 마우스 왼쪽 클릭 시 로그 출력
- [ ] Output Log에 "Attack performed!" 메시지 확인
- [ ] 쿨다운 동작 (0.5초 내에 다시 클릭해도 한 번만 공격)

### 디버깅 팁
- **Output Log**: Window → Developer Tools → Output Log
- **Blueprint Debugger**: Blueprint 에디터에서 F5로 디버그 모드
- **Visual Logger**: Tools → Debug → Visual Logger

---

## 🐛 문제 해결

### "MoveAction is None" 에러
→ PlayerController Blueprint에서 Move Action 레퍼런스 설정 안 됨
→ BP_TopDownPlayerController 열고 Move Action에 IA_Move 할당

### WASD 입력이 안 됨
→ IMC_Default에 IA_Move 매핑 추가 안 됨
→ Input Mapping Context에서 WASD 키 매핑 확인

### 캐릭터가 회전 안 됨
→ Character Movement 설정 확인
→ "Use Controller Rotation Yaw" = False 확인
→ "Orient Rotation to Movement" = True 확인

### 공격 로그가 안 나옴
→ Output Log 창이 안 열려있음
→ Window → Developer Tools → Output Log 열기
→ "LogTactics" 카테고리 확인

---

## 📊 예상 결과

### 성공 시 동작
```
- WASD로 부드럽게 8방향 이동
- 마우스 커서 방향으로 캐릭터 즉시 회전
- 클릭 시 Output Log에 공격 메시지
- 쿨다운 동작 확인 (0.5초)
```

### Output Log 예시
```
LogTactics: Attack performed! Cooldown: 0.500000
LogTactics: Hit Floor with 20.000000 damage
LogTactics: Attack performed! Cooldown: 0.500000
```

---

## 🚀 다음 단계 (Day 3)

테스트가 성공하면:
1. 공격 애니메이션 Montage 추가
2. 타격 이펙트 (Niagara System)
3. 공격 사운드 추가
4. 적 캐릭터 테스트

---

## 📝 작업 노트

### 현재 상황
- C++ 코드 구현 완료
- Unreal Editor 실행 중
- Blueprint 설정 대기 중

### 참고 파일
- `Source/Tactics/TacticsPlayerController.h`
- `Source/Tactics/TacticsPlayerController.cpp`
- `Source/Tactics/TacticsCharacter.h`
- `Source/Tactics/TacticsCharacter.cpp`

