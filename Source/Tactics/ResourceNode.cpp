// Copyright Epic Games, Inc. All Rights Reserved.

#include "ResourceNode.h"
#include "ResourceManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AResourceNode::AResourceNode()
{
    PrimaryActorTick.bCanEverTick = true;

    // 루트 컴포넌트 생성
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    // 메시 컴포넌트
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

    // 상호작용 범위
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // 기본값 설정
    ResourceType = EResourceType::Wood;
    GatherAmount = 1;
    GatherTime = 5.0f;
    MaxGatherCount = 0; // 무제한
    RespawnTime = 0.0f; // 리스폰 안 함
    InteractionRange = 200.0f;
    InteractionPrompt = FText::FromString(TEXT("E: 채집"));

    GatherState = EGatherState::Idle;
    CurrentGatherTime = 0.0f;
    CurrentGatherer = nullptr;
    RemainingGatherCount = MaxGatherCount;
}

void AResourceNode::BeginPlay()
{
    Super::BeginPlay();
    
    // 상호작용 범위 설정
    InteractionSphere->SetSphereRadius(InteractionRange);

    // 무제한 채집이면 RemainingGatherCount를 -1로 설정
    if (MaxGatherCount == 0)
    {
        RemainingGatherCount = -1;
    }
    else
    {
        RemainingGatherCount = MaxGatherCount;
    }

    UpdateVisual(GatherState);
}

void AResourceNode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 채집 중일 때만 처리
    if (GatherState == EGatherState::Gathering)
    {
        // 채집자가 범위를 벗어났는지 확인
        if (!IsGathererInRange())
        {
            UE_LOG(LogTemp, Warning, TEXT("Gatherer moved out of range, stopping gather"));
            StopGathering();
            return;
        }

        // 진행 시간 증가
        CurrentGatherTime += DeltaTime;
        float Progress = FMath::Clamp(CurrentGatherTime / GatherTime, 0.0f, 1.0f);
        float RemainingTime = GatherTime - CurrentGatherTime;

        // 진행도 이벤트
        OnGatherProgress.Broadcast(Progress, RemainingTime);

        // 완료 체크
        if (CurrentGatherTime >= GatherTime)
        {
            CompleteGathering();
        }
    }
}

bool AResourceNode::CanInteract() const
{
    // Idle 상태이고 채집 가능 횟수가 남았을 때만 상호작용 가능
    return GatherState == EGatherState::Idle && 
           (RemainingGatherCount > 0 || RemainingGatherCount == -1);
}

bool AResourceNode::StartGathering(AActor* Gatherer)
{
    if (!CanInteract() || !Gatherer)
    {
        return false;
    }

    // 범위 체크
    float Distance = FVector::Dist(GetActorLocation(), Gatherer->GetActorLocation());
    if (Distance > InteractionRange)
    {
        UE_LOG(LogTemp, Warning, TEXT("Gatherer too far: %.1f > %.1f"), Distance, InteractionRange);
        return false;
    }

    CurrentGatherer = Gatherer;
    GatherState = EGatherState::Gathering;
    CurrentGatherTime = 0.0f;

    ShowProgressBar(true);
    UpdateVisual(GatherState);

    UE_LOG(LogTemp, Log, TEXT("Gathering started: Type=%d, Time=%.1fs"),
        (int32)ResourceType, GatherTime);

    return true;
}

void AResourceNode::StopGathering()
{
    if (GatherState != EGatherState::Gathering)
    {
        return;
    }

    GatherState = EGatherState::Idle;
    CurrentGatherTime = 0.0f;
    CurrentGatherer = nullptr;

    ShowProgressBar(false);
    UpdateVisual(GatherState);

    UE_LOG(LogTemp, Log, TEXT("Gathering stopped"));
}

void AResourceNode::CompleteGathering()
{
    // 자원량 결정 (고기는 10~20 랜덤)
    int32 ActualAmount = GatherAmount;
    if (ResourceType == EResourceType::Meat)
    {
        ActualAmount = FMath::RandRange(10, 20);
    }

    // ResourceManager에 자원 추가
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        UResourceManager* ResourceMgr = GameInstance->GetSubsystem<UResourceManager>();
        if (ResourceMgr)
        {
            int32 AddedAmount = ResourceMgr->AddResource(ResourceType, ActualAmount);
            
            // AP 계산 (ResourceManager가 자동으로 처리하지만 이벤트용으로 다시 계산)
            int32 APGained = AddedAmount; // 자원 1개당 AP 1
            
            OnGatherComplete.Broadcast(ResourceType, AddedAmount, APGained);
            
            UE_LOG(LogTemp, Log, TEXT("Gathering completed: Type=%d, Amount=%d, AP=%d"),
                (int32)ResourceType, AddedAmount, APGained);
        }
    }

    // 채집 횟수 감소
    if (RemainingGatherCount > 0)
    {
        RemainingGatherCount--;
    }

    // 상태 전환
    CurrentGatherer = nullptr;
    ShowProgressBar(false);

    if (RemainingGatherCount == 0)
    {
        // 고갈
        GatherState = EGatherState::Depleted;
        UpdateVisual(GatherState);

        // 리스폰 예약
        if (RespawnTime > 0.0f)
        {
            GetWorldTimerManager().SetTimer(
                RespawnTimerHandle,
                this,
                &AResourceNode::Respawn,
                RespawnTime,
                false
            );
            UE_LOG(LogTemp, Log, TEXT("Node depleted, respawning in %.1fs"), RespawnTime);
        }
    }
    else
    {
        // 다시 채집 가능
        GatherState = EGatherState::Idle;
        UpdateVisual(GatherState);
    }
}

void AResourceNode::Respawn()
{
    // 채집 횟수 복원
    if (MaxGatherCount == 0)
    {
        RemainingGatherCount = -1; // 무제한
    }
    else
    {
        RemainingGatherCount = MaxGatherCount;
    }

    GatherState = EGatherState::Idle;
    UpdateVisual(GatherState);

    UE_LOG(LogTemp, Log, TEXT("Node respawned"));
}

bool AResourceNode::IsGathererInRange() const
{
    if (!CurrentGatherer)
    {
        return false;
    }

    float Distance = FVector::Dist(GetActorLocation(), CurrentGatherer->GetActorLocation());
    return Distance <= InteractionRange;
}

float AResourceNode::GetGatherProgress() const
{
    if (GatherState != EGatherState::Gathering || GatherTime <= 0.0f)
    {
        return 0.0f;
    }

    return FMath::Clamp(CurrentGatherTime / GatherTime, 0.0f, 1.0f);
}

int32 AResourceNode::GetRemainingGatherCount() const
{
    return RemainingGatherCount == -1 ? 999 : RemainingGatherCount; // UI 표시용
}