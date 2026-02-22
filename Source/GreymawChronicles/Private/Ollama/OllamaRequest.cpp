#include "Ollama/OllamaRequest.h"

void UOllamaRequest::Initialize(const FString& InModel, const FString& InPrompt, const FOllamaRequestOptions& InOptions, FOnOllamaRequestComplete InCompletion)
{
    Model = InModel;
    Prompt = InPrompt;
    Options = InOptions;
    Completion = InCompletion;
    CreatedAtUtc = FDateTime::UtcNow();
    State = EOllamaRequestState::Pending;
    RetryCount = 0;
    ResponseText.Reset();
}

void UOllamaRequest::Complete(bool bSuccess, const FString& InResponseText, float LatencySeconds)
{
    ResponseText = InResponseText;
    State = bSuccess ? EOllamaRequestState::Completed : EOllamaRequestState::Failed;

    if (Completion.IsBound())
    {
        Completion.Execute(bSuccess, ResponseText, LatencySeconds);
    }
}
