#include "Ollama/OllamaRequestQueue.h"
#include "Ollama/OllamaRequest.h"

void UOllamaRequestQueue::Enqueue(UOllamaRequest* Request)
{
    if (!IsValid(Request))
    {
        return;
    }

    switch (Request->GetOptions().Priority)
    {
        case EOllamaRequestPriority::Critical:
            CriticalQueue.Add(Request);
            break;
        case EOllamaRequestPriority::High:
            HighQueue.Add(Request);
            break;
        case EOllamaRequestPriority::Normal:
            NormalQueue.Add(Request);
            break;
        default:
            LowQueue.Add(Request);
            break;
    }
}

UOllamaRequest* UOllamaRequestQueue::DequeueNext()
{
    if (UOllamaRequest* Critical = PopFirstDispatchable(CriticalQueue))
    {
        return Critical;
    }

    if (UOllamaRequest* High = PopFirstDispatchable(HighQueue))
    {
        return High;
    }

    if (UOllamaRequest* Normal = PopFirstDispatchable(NormalQueue))
    {
        return Normal;
    }

    return PopFirstDispatchable(LowQueue);
}

bool UOllamaRequestQueue::HasPending() const
{
    return CriticalQueue.Num() > 0 || HighQueue.Num() > 0 || NormalQueue.Num() > 0 || LowQueue.Num() > 0;
}

bool UOllamaRequestQueue::CanDispatchForModel(const FString& Model) const
{
    return !ModelsInFlight.Contains(Model);
}

void UOllamaRequestQueue::MarkModelInFlight(const FString& Model)
{
    ModelsInFlight.Add(Model);
}

void UOllamaRequestQueue::MarkModelCompleted(const FString& Model)
{
    ModelsInFlight.Remove(Model);
}

UOllamaRequest* UOllamaRequestQueue::PopFirstDispatchable(TArray<TObjectPtr<UOllamaRequest>>& Queue)
{
    for (int32 Index = 0; Index < Queue.Num(); ++Index)
    {
        UOllamaRequest* Candidate = Queue[Index];
        if (IsValid(Candidate) && CanDispatchForModel(Candidate->GetModel()))
        {
            Queue.RemoveAt(Index);
            return Candidate;
        }
    }

    return nullptr;
}
