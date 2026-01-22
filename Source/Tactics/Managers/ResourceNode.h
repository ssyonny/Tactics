// Copyright Epic Games, Inc. All Rights Reserved.
// Tactics - Resource Node Actor

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceManager.h"
#include "ResourceNode.generated.h"

/**
 * 채집 상태 열거형
 */
UENUM(BlueprintType)
enum class EGatherState : uint8
{
    Idle        UMETA(DisplayName = "대기"),
    Gathering   UMETA(DisplayName = "채집중"),
    Depleted    UMETA(DisplayName = "고갈됨"),
    Respawning  UMETA(DisplayName = "리스폰중")
};

/**
 * 채집 진행 델리게이트
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGatherProgress, float, Progress, float, RemainingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGatherComplete, EResourceType, Type, int32, Amount, int32, APGained);

/**
 * AResourceNode
 * 
 * 본거지에 배치되는 채집/수렵 가능한 자원 노드
 * - 상호작용(E 키)으로 채집 시작
 * - 채집 시간 동안 프로그레스 바 표시
 * - 완료 시 자원 자동 추가
 * - 고갈 후 리스폰 시스템
 */
UCLASS()
class TACTICS_API AResourceNode : public AActor
{
    GENERATED_BODY()
    
public:    
    AResourceNode();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    // ==================== 설정 ====================

    /** 자원 타입 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EResourceType ResourceType;

    /** 1회 채집 시 획득량 (고기는 10~20 랜덤) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 GatherAmount;

    /** 채집 소요 시간 (초) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float GatherTime;

    /** 최대 채집 가능 횟수 (0 = 무제한) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 MaxGatherCount;

    /** 리스폰 시간 (초, 0 = 리스폰 안 함) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnTime;

    /** 상호작용 거리 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float InteractionRange;

    /** 상호작용 프롬프트 텍스트 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FText InteractionPrompt;

    // ==================== 컴포넌트 ====================

    /** 메시 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    /** 상호작용 범위 표시 (디버그용) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    // ==================== 상호작용 ====================

    /**
     * 플레이어가 상호작용 가능한지 확인
     */
    UFUNCTION(BlueprintPure, Category = "Resource")
    bool CanInteract() const;

    /**
     * 상호작용 시작 (채집 시작)
     * @param Gatherer 채집하는 액터 (플레이어)
     */
    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool StartGathering(AActor* Gatherer);

    /**
     * 채집 중단
     */
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void StopGathering();

    /**
     * 현재 상태 조회
     */
    UFUNCTION(BlueprintPure, Category = "Resource")
    EGatherState GetGatherState() const { return GatherState; }

    /**
     * 채집 진행도 조회 (0.0 ~ 1.0)
     */
    UFUNCTION(BlueprintPure, Category = "Resource")
    float GetGatherProgress() const;

    /**
     * 남은 채집 가능 횟수
     */
    UFUNCTION(BlueprintPure, Category = "Resource")
    int32 GetRemainingGatherCount() const;

    // ==================== 이벤트 ====================

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnGatherProgress OnGatherProgress;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnGatherComplete OnGatherComplete;

    // ==================== 비주얼 ====================

    /**
     * 상태에 따라 메시 외형 변경 (Blueprint에서 구현)
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Visual")
    void UpdateVisual(EGatherState State);

    /**
     * 프로그레스 바 표시 (Blueprint에서 구현)
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Visual")
    void ShowProgressBar(bool bShow);

private:
    // 현재 상태
    UPROPERTY()
    EGatherState GatherState;

    // 현재 채집 진행 시간
    UPROPERTY()
    float CurrentGatherTime;

    // 현재 채집자
    UPROPERTY()
    AActor* CurrentGatherer;

    // 남은 채집 횟수
    UPROPERTY()
    int32 RemainingGatherCount;

    // 리스폰 타이머 핸들
    FTimerHandle RespawnTimerHandle;

    // 채집 완료 처리
    void CompleteGathering();

    // 리스폰 처리
    void Respawn();

    // 채집자가 범위 내에 있는지 확인
    bool IsGathererInRange() const;
};