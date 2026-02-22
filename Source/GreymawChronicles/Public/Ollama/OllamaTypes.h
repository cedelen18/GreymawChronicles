#pragma once

#include "CoreMinimal.h"
#include "OllamaTypes.generated.h"

UENUM(BlueprintType)
enum class EOllamaRequestState : uint8
{
    Pending,
    InProgress,
    Completed,
    Failed,
    TimedOut
};

UENUM(BlueprintType)
enum class EOllamaRequestPriority : uint8
{
    Low,
    Normal,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct FOllamaRequestOptions
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ollama")
    float Temperature = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ollama")
    int32 NumPredict = 512;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ollama")
    int32 Seed = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ollama")
    EOllamaRequestPriority Priority = EOllamaRequestPriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ollama")
    bool bStream = false;
};

DECLARE_DELEGATE_ThreeParams(FOnOllamaComplete, bool /*bSuccess*/, const FString& /*ResponseText*/, float /*LatencySeconds*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOllamaHealthChanged, const FString&, StatusMessage);
