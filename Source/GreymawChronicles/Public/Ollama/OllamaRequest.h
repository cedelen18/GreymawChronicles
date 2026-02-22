#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Ollama/OllamaTypes.h"
#include "OllamaRequest.generated.h"

DECLARE_DELEGATE_ThreeParams(FOnOllamaRequestComplete, bool /*bSuccess*/, const FString& /*ResponseText*/, float /*LatencySeconds*/);

UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UOllamaRequest : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(const FString& InModel, const FString& InPrompt, const FOllamaRequestOptions& InOptions, FOnOllamaRequestComplete InCompletion);

    bool CanRetry() const { return RetryCount < MaxRetries; }
    void MarkRetry() { ++RetryCount; }

    void SetState(EOllamaRequestState InState) { State = InState; }
    EOllamaRequestState GetState() const { return State; }

    const FString& GetModel() const { return Model; }
    const FString& GetPrompt() const { return Prompt; }
    const FOllamaRequestOptions& GetOptions() const { return Options; }

    void Complete(bool bSuccess, const FString& InResponseText, float LatencySeconds);

private:
    UPROPERTY()
    FString Model;

    UPROPERTY()
    FString Prompt;

    UPROPERTY()
    FOllamaRequestOptions Options;

    UPROPERTY()
    EOllamaRequestState State = EOllamaRequestState::Pending;

    UPROPERTY()
    FString ResponseText;

    UPROPERTY()
    int32 RetryCount = 0;

    UPROPERTY()
    int32 MaxRetries = 2;

    UPROPERTY()
    FDateTime CreatedAtUtc;

    FOnOllamaRequestComplete Completion;
};
