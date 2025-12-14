// Copyright Epic Games, Inc. All Rights Reserved.
// Tactics - Resource Management System

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ResourceManager.generated.h"

/**
 * 자원 타입 열거형
 */
UENUM(BlueprintType)
enum class EResourceType : uint8
{
    Wood    UMETA(DisplayName = "나무"),
    Ore     UMETA(DisplayName = "광석"),
    Berry   UMETA(DisplayName = "열매"),
    Meat    UMETA(DisplayName = "고기")
};

/**
 * 자원 데이터 구조체
 */
USTRUCT(BlueprintType)
struct FResourceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EResourceType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Amount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 MaxAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float GatherTime; // 채집/수렵 소요 시간 (초)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 APPerUnit; // 자원 1개당 획득 AP

    FResourceData()
        : Type(EResourceType::Wood)
        , Amount(0)
        , MaxAmount(100)
        , GatherTime(5.0f)
        , APPerUnit(1)
    {}
};

/**
 * 생산 레시피 구조체
 */
USTRUCT(BlueprintType)
struct FCraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TMap<EResourceType, int32> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTime;

    FCraftingRecipe()
        : CraftingTime(5.0f)
    {}
};

/**
 * 자원 관리 이벤트 델리게이트
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnResourceChanged, EResourceType, ResourceType, int32, NewAmount, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAPChanged, int32, NewAP, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemCrafted, FName, ItemID, bool, bSuccess);

/**
 * UResourceManager
 * 
 * 본거지의 모든 자원을 관리하는 GameInstance Subsystem
 * - 4종 자원 (나무, 광석, 열매, 고기) 관리
 * - AP(행동포인트) 시스템
 * - 생산(크래프팅) 시스템
 */
UCLASS()
class TACTICS_API UResourceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UResourceManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ==================== 자원 관리 ====================

    /**
     * 자원 추가
     * @param Type 자원 타입
     * @param Amount 추가할 양
     * @return 실제로 추가된 양 (상한 제한)
     */
    UFUNCTION(BlueprintCallable, Category = "Resource")
    int32 AddResource(EResourceType Type, int32 Amount);

    /**
     * 자원 소비
     * @param Type 자원 타입
     * @param Amount 소비할 양
     * @return 소비 성공 여부
     */
    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool ConsumeResource(EResourceType Type, int32 Amount);

    /**
     * 현재 자원량 조회
     */
    UFUNCTION(BlueprintPure, Category = "Resource")
    int32 GetResourceAmount(EResourceType Type) const;

    /**
     * 자원 상한 조회
     */
    UFUNCTION(BlueprintPure, Category = "Resource")
    int32 GetResourceMax(EResourceType Type) const;

    /**
     * 자원 상한 업그레이드
     */
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void UpgradeResourceMax(EResourceType Type, int32 AdditionalMax);

    /**
     * 모든 자원 데이터 조회
     */
    UFUNCTION(BlueprintPure, Category = "Resource")
    TMap<EResourceType, FResourceData> GetAllResources() const { return Resources; }

    // ==================== AP 시스템 ====================

    /**
     * AP 추가 (자원 획득 시 자동 호출)
     */
    UFUNCTION(BlueprintCallable, Category = "AP")
    void AddAP(int32 Amount);

    /**
     * AP 소비
     */
    UFUNCTION(BlueprintCallable, Category = "AP")
    bool ConsumeAP(int32 Amount);

    /**
     * 현재 AP 조회
     */
    UFUNCTION(BlueprintPure, Category = "AP")
    int32 GetCurrentAP() const { return CurrentAP; }

    /**
     * AP에 따른 이벤트 간격 계산
     * 공식: max(60s, 300s - (AP - 100) * 5s)
     */
    UFUNCTION(BlueprintPure, Category = "AP")
    float CalculateEventInterval() const;

    // ==================== 생산 시스템 ====================

    /**
     * 레시피 등록
     */
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void RegisterRecipe(const FCraftingRecipe& Recipe);

    /**
     * 아이템 제작 가능 여부 확인
     */
    UFUNCTION(BlueprintPure, Category = "Crafting")
    bool CanCraftItem(FName ItemID) const;

    /**
     * 아이템 제작 시작
     */
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool StartCrafting(FName ItemID);

    /**
     * 등록된 모든 레시피 조회
     */
    UFUNCTION(BlueprintPure, Category = "Crafting")
    TArray<FCraftingRecipe> GetAllRecipes() const;

    // ==================== 세이브/로드 ====================

    /**
     * 자원 상태를 JSON으로 직렬화
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    FString SerializeToJSON() const;

    /**
     * JSON에서 자원 상태 복원
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    bool DeserializeFromJSON(const FString& JSONString);

    // ==================== 이벤트 ====================

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnResourceChanged OnResourceChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAPChanged OnAPChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnItemCrafted OnItemCrafted;

private:
    // 자원 데이터 (4종)
    UPROPERTY()
    TMap<EResourceType, FResourceData> Resources;

    // 현재 AP
    UPROPERTY()
    int32 CurrentAP;

    // 생산 레시피
    UPROPERTY()
    TMap<FName, FCraftingRecipe> CraftingRecipes;

    // 현재 제작 중인 아이템 (비동기 제작용)
    UPROPERTY()
    TArray<FName> CraftingQueue;

    // 초기 자원 데이터 설정
    void InitializeResources();

    // 제작 완료 타이머 콜백
    void OnCraftingComplete(FName ItemID);

    // 타이머 핸들 (제작용)
    TMap<FName, FTimerHandle> CraftingTimers;
};