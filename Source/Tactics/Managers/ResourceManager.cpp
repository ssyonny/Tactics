// Copyright Epic Games, Inc. All Rights Reserved.

#include "ResourceManager.h"
#include "TimerManager.h"

UResourceManager::UResourceManager()
    : CurrentAP(0)
{
}

void UResourceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeResources();
    
    UE_LOG(LogTemp, Log, TEXT("ResourceManager Initialized"));
}

void UResourceManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        for (auto& Pair : CraftingTimers)
        {
            World->GetTimerManager().ClearTimer(Pair.Value);
        }
    }
    
    CraftingTimers.Empty();
    
    Super::Deinitialize();
}

void UResourceManager::InitializeResources()
{
    FResourceData WoodData;
    WoodData.Type = EResourceType::Wood;
    WoodData.Amount = 0;
    WoodData.MaxAmount = 100;
    WoodData.GatherTime = 5.0f;
    WoodData.APPerUnit = 1;
    Resources.Add(EResourceType::Wood, WoodData);

    FResourceData OreData;
    OreData.Type = EResourceType::Ore;
    OreData.Amount = 0;
    OreData.MaxAmount = 100;
    OreData.GatherTime = 10.0f;
    OreData.APPerUnit = 1;
    Resources.Add(EResourceType::Ore, OreData);

    FResourceData BerryData;
    BerryData.Type = EResourceType::Berry;
    BerryData.Amount = 0;
    BerryData.MaxAmount = 100;
    BerryData.GatherTime = 3.0f;
    BerryData.APPerUnit = 1;
    Resources.Add(EResourceType::Berry, BerryData);

    FResourceData MeatData;
    MeatData.Type = EResourceType::Meat;
    MeatData.Amount = 0;
    MeatData.MaxAmount = 100;
    MeatData.GatherTime = 30.0f;
    MeatData.APPerUnit = 1;
    Resources.Add(EResourceType::Meat, MeatData);

    CurrentAP = 0;
}

int32 UResourceManager::AddResource(EResourceType Type, int32 Amount)
{
    if (!Resources.Contains(Type) || Amount <= 0)
    {
        return 0;
    }

    FResourceData& Data = Resources[Type];
    int32 SpaceAvailable = Data.MaxAmount - Data.Amount;
    int32 ActualAdded = FMath::Min(Amount, SpaceAvailable);

    Data.Amount += ActualAdded;

    int32 APGain = ActualAdded * Data.APPerUnit;
    AddAP(APGain);

    OnResourceChanged.Broadcast(Type, Data.Amount, ActualAdded);

    UE_LOG(LogTemp, Log, TEXT("Resource Added: Type=%d, Amount=%d/%d (+%d), AP Gain=%d"),
        (int32)Type, Data.Amount, Data.MaxAmount, ActualAdded, APGain);

    return ActualAdded;
}

bool UResourceManager::ConsumeResource(EResourceType Type, int32 Amount)
{
    if (!Resources.Contains(Type) || Amount <= 0)
    {
        return false;
    }

    FResourceData& Data = Resources[Type];
    
    if (Data.Amount < Amount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough resource: Type=%d, Need=%d, Have=%d"),
            (int32)Type, Amount, Data.Amount);
        return false;
    }

    Data.Amount -= Amount;
    OnResourceChanged.Broadcast(Type, Data.Amount, -Amount);

    UE_LOG(LogTemp, Log, TEXT("Resource Consumed: Type=%d, Amount=%d/%d (-%d)"),
        (int32)Type, Data.Amount, Data.MaxAmount, Amount);

    return true;
}

int32 UResourceManager::GetResourceAmount(EResourceType Type) const
{
    if (Resources.Contains(Type))
    {
        return Resources[Type].Amount;
    }
    return 0;
}

int32 UResourceManager::GetResourceMax(EResourceType Type) const
{
    if (Resources.Contains(Type))
    {
        return Resources[Type].MaxAmount;
    }
    return 0;
}

void UResourceManager::UpgradeResourceMax(EResourceType Type, int32 AdditionalMax)
{
    if (Resources.Contains(Type) && AdditionalMax > 0)
    {
        FResourceData& Data = Resources[Type];
        Data.MaxAmount += AdditionalMax;
        
        UE_LOG(LogTemp, Log, TEXT("Resource Max Upgraded: Type=%d, NewMax=%d"),
            (int32)Type, Data.MaxAmount);
    }
}

void UResourceManager::AddAP(int32 Amount)
{
    if (Amount <= 0) return;

    int32 OldAP = CurrentAP;
    CurrentAP += Amount;

    OnAPChanged.Broadcast(CurrentAP, Amount);

    UE_LOG(LogTemp, Log, TEXT("AP Added: %d -> %d (+%d)"), OldAP, CurrentAP, Amount);
}

bool UResourceManager::ConsumeAP(int32 Amount)
{
    if (Amount <= 0 || CurrentAP < Amount)
    {
        return false;
    }

    int32 OldAP = CurrentAP;
    CurrentAP -= Amount;

    OnAPChanged.Broadcast(CurrentAP, -Amount);

    UE_LOG(LogTemp, Log, TEXT("AP Consumed: %d -> %d (-%d)"), OldAP, CurrentAP, Amount);

    return true;
}

float UResourceManager::CalculateEventInterval() const
{
    const float BaseInterval = 300.0f;
    const float MinInterval = 60.0f;
    const int32 APThreshold = 100;
    const float IntervalReduction = 5.0f;

    if (CurrentAP <= APThreshold)
    {
        return BaseInterval;
    }

    float Interval = BaseInterval - ((CurrentAP - APThreshold) * IntervalReduction);
    return FMath::Max(Interval, MinInterval);
}

void UResourceManager::RegisterRecipe(const FCraftingRecipe& Recipe)
{
    if (Recipe.ItemID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register recipe with empty ItemID"));
        return;
    }

    CraftingRecipes.Add(Recipe.ItemID, Recipe);
    
    UE_LOG(LogTemp, Log, TEXT("Recipe Registered: %s"), *Recipe.ItemID.ToString());
}

bool UResourceManager::CanCraftItem(FName ItemID) const
{
    if (!CraftingRecipes.Contains(ItemID))
    {
        return false;
    }

    const FCraftingRecipe& Recipe = CraftingRecipes[ItemID];

    for (const auto& Pair : Recipe.RequiredResources)
    {
        EResourceType Type = Pair.Key;
        int32 Required = Pair.Value;

        if (GetResourceAmount(Type) < Required)
        {
            return false;
        }
    }

    return true;
}

bool UResourceManager::StartCrafting(FName ItemID)
{
    if (!CanCraftItem(ItemID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot craft item: %s"), *ItemID.ToString());
        OnItemCrafted.Broadcast(ItemID, false);
        return false;
    }

    const FCraftingRecipe& Recipe = CraftingRecipes[ItemID];

    for (const auto& Pair : Recipe.RequiredResources)
    {
        ConsumeResource(Pair.Key, Pair.Value);
    }

    CraftingQueue.Add(ItemID);

    UWorld* World = GetWorld();
    if (World)
    {
        FTimerHandle TimerHandle;
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindUObject(this, &UResourceManager::OnCraftingComplete, ItemID);
        
        World->GetTimerManager().SetTimer(
            TimerHandle,
            TimerDelegate,
            Recipe.CraftingTime,
            false
        );

        CraftingTimers.Add(ItemID, TimerHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("Crafting Started: %s (Time: %.1fs)"),
        *ItemID.ToString(), Recipe.CraftingTime);

    return true;
}

void UResourceManager::OnCraftingComplete(FName ItemID)
{
    CraftingQueue.Remove(ItemID);
    CraftingTimers.Remove(ItemID);

    OnItemCrafted.Broadcast(ItemID, true);

    UE_LOG(LogTemp, Log, TEXT("Crafting Complete: %s"), *ItemID.ToString());
}

TArray<FCraftingRecipe> UResourceManager::GetAllRecipes() const
{
    TArray<FCraftingRecipe> RecipeArray;
    CraftingRecipes.GenerateValueArray(RecipeArray);
    return RecipeArray;
}

FString UResourceManager::SerializeToJSON() const
{
    UE_LOG(LogTemp, Warning, TEXT("SerializeToJSON: Not implemented yet"));
    
    FString Result = FString::Printf(TEXT("AP=%d,Wood=%d,Ore=%d,Berry=%d,Meat=%d"),
        CurrentAP,
        GetResourceAmount(EResourceType::Wood),
        GetResourceAmount(EResourceType::Ore),
        GetResourceAmount(EResourceType::Berry),
        GetResourceAmount(EResourceType::Meat)
    );
    
    return Result;
}

bool UResourceManager::DeserializeFromJSON(const FString& JSONString)
{
    UE_LOG(LogTemp, Warning, TEXT("DeserializeFromJSON: Not implemented yet"));
    UE_LOG(LogTemp, Log, TEXT("Input: %s"), *JSONString);
    
    return true;
}