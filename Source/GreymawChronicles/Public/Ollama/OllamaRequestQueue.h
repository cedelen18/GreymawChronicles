#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Ollama/OllamaTypes.h"
#include "OllamaRequestQueue.generated.h"

class UOllamaRequest;

UCLASS()
class GREYMAWCHRONICLES_API UOllamaRequestQueue : public UObject
{
    GENERATED_BODY()

public:
    void Enqueue(UOllamaRequest* Request);
    UOllamaRequest* DequeueNext();

    bool HasPending() const;

    bool CanDispatchForModel(const FString& Model) const;
    void MarkModelInFlight(const FString& Model);
    void MarkModelCompleted(const FString& Model);

private:
    TArray<TObjectPtr<UOllamaRequest>> CriticalQueue;
    TArray<TObjectPtr<UOllamaRequest>> HighQueue;
    TArray<TObjectPtr<UOllamaRequest>> NormalQueue;
    TArray<TObjectPtr<UOllamaRequest>> LowQueue;

    UPROPERTY()
    TSet<FString> ModelsInFlight;

    UOllamaRequest* PopFirstDispatchable(TArray<TObjectPtr<UOllamaRequest>>& Queue);
};
