#include "Ollama/OllamaSubsystem.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Ollama/OllamaRequest.h"
#include "Ollama/OllamaRequestQueue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogOllamaSubsystem, Log, All);

void UOllamaSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    RequestQueue = NewObject<UOllamaRequestQueue>(this);
    PerformStartupHealthCheck();
}

void UOllamaSubsystem::Deinitialize()
{
    RequestQueue = nullptr;
    Super::Deinitialize();
}

void UOllamaSubsystem::SendGenerateRequest(const FString& Model, const FString& Prompt, const FOllamaRequestOptions& Options, FOnOllamaComplete Callback)
{
    if (!RequestQueue)
    {
        Callback.ExecuteIfBound(false, TEXT("Ollama request queue is unavailable."), 0.f);
        return;
    }

    UOllamaRequest* Request = NewObject<UOllamaRequest>(this);
    Request->Initialize(Model, Prompt, Options, FOnOllamaRequestComplete::CreateLambda([Callback](bool bSuccess, const FString& ResponseText, float Latency)
    {
        Callback.ExecuteIfBound(bSuccess, ResponseText, Latency);
    }));

    RequestQueue->Enqueue(Request);
    DispatchNextRequest();
}

FString UOllamaSubsystem::BuildUrl(const FString& Path) const
{
    return FString::Printf(TEXT("%s%s"), *OllamaBaseUrl, *Path);
}

void UOllamaSubsystem::PerformStartupHealthCheck()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BuildUrl(TEXT("/api/tags")));
    Request->SetVerb(TEXT("GET"));
    Request->SetTimeout(RequestTimeoutSeconds);
    Request->OnProcessRequestComplete().BindUObject(this, &UOllamaSubsystem::OnHealthResponse);
    Request->ProcessRequest();
}

void UOllamaSubsystem::OnHealthResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
    AvailableModels.Reset();

    if (!bConnectedSuccessfully || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        bOllamaAvailable = false;
        LastHealthMessage = TEXT("Ollama is unavailable. Start Ollama and confirm localhost:11434 is reachable.");
        UE_LOG(LogOllamaSubsystem, Warning, TEXT("%s"), *LastHealthMessage);
        OnOllamaHealthChanged.Broadcast(LastHealthMessage);
        return;
    }

    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        bOllamaAvailable = false;
        LastHealthMessage = TEXT("Ollama health check returned invalid JSON payload.");
        UE_LOG(LogOllamaSubsystem, Error, TEXT("%s"), *LastHealthMessage);
        OnOllamaHealthChanged.Broadcast(LastHealthMessage);
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* ModelsField = nullptr;
    if (Root->TryGetArrayField(TEXT("models"), ModelsField))
    {
        for (const TSharedPtr<FJsonValue>& Value : *ModelsField)
        {
            const TSharedPtr<FJsonObject>* ModelObject = nullptr;
            if (Value.IsValid() && Value->TryGetObject(ModelObject) && ModelObject && ModelObject->IsValid())
            {
                FString Name;
                if ((*ModelObject)->TryGetStringField(TEXT("name"), Name))
                {
                    AvailableModels.Add(Name);
                }
            }
        }
    }

    bOllamaAvailable = true;

    TArray<FString> MissingModels;
    const TArray<FString> RequiredModels = { TEXT("qwen2.5:32b"), TEXT("llama3.1:8b") };
    for (const FString& Required : RequiredModels)
    {
        bool bFound = false;
        for (const FString& Installed : AvailableModels)
        {
            if (Installed.StartsWith(Required))
            {
                bFound = true;
                break;
            }
        }

        if (!bFound)
        {
            MissingModels.Add(Required);
        }
    }

    if (MissingModels.Num() > 0)
    {
        LastHealthMessage = BuildMissingModelsMessage(MissingModels);
        UE_LOG(LogOllamaSubsystem, Warning, TEXT("%s"), *LastHealthMessage);
    }
    else
    {
        LastHealthMessage = TEXT("Ollama health-check passed. Required models are installed.");
        UE_LOG(LogOllamaSubsystem, Log, TEXT("%s"), *LastHealthMessage);
    }

    OnOllamaHealthChanged.Broadcast(LastHealthMessage);
}

void UOllamaSubsystem::DispatchNextRequest()
{
    if (!RequestQueue)
    {
        return;
    }

    UOllamaRequest* NextRequest = RequestQueue->DequeueNext();
    if (!NextRequest)
    {
        return;
    }

    SubmitRequest(NextRequest);
}

void UOllamaSubsystem::SubmitRequest(UOllamaRequest* Request)
{
    if (!RequestQueue || !Request)
    {
        return;
    }

    RequestQueue->MarkModelInFlight(Request->GetModel());
    Request->SetState(EOllamaRequestState::InProgress);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(BuildUrl(TEXT("/api/generate")));
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetTimeout(RequestTimeoutSeconds);

    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    Body->SetStringField(TEXT("model"), Request->GetModel());
    Body->SetStringField(TEXT("prompt"), Request->GetPrompt());
    Body->SetBoolField(TEXT("stream"), Request->GetOptions().bStream);

    TSharedRef<FJsonObject> OptionsObject = MakeShared<FJsonObject>();
    OptionsObject->SetNumberField(TEXT("temperature"), Request->GetOptions().Temperature);
    OptionsObject->SetNumberField(TEXT("num_predict"), Request->GetOptions().NumPredict);
    if (Request->GetOptions().Seed >= 0)
    {
        OptionsObject->SetNumberField(TEXT("seed"), Request->GetOptions().Seed);
    }
    Body->SetObjectField(TEXT("options"), OptionsObject);

    FString BodyString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
    FJsonSerializer::Serialize(Body, Writer);
    HttpRequest->SetContentAsString(BodyString);

    const double StartSeconds = FPlatformTime::Seconds();
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UOllamaSubsystem::OnGenerateResponse, Request, StartSeconds);
    HttpRequest->ProcessRequest();
}

void UOllamaSubsystem::OnGenerateResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bConnectedSuccessfully, UOllamaRequest* SourceRequest, double RequestStartSeconds)
{
    const float Latency = static_cast<float>(FPlatformTime::Seconds() - RequestStartSeconds);

    if (!RequestQueue || !SourceRequest)
    {
        return;
    }

    const FString ModelName = SourceRequest->GetModel();
    RequestQueue->MarkModelCompleted(ModelName);

    bool bSuccess = false;
    FString Message;

    if (!bConnectedSuccessfully || !HttpResponse.IsValid())
    {
        Message = TEXT("Ollama request failed to connect.");
    }
    else if (!EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
    {
        Message = FString::Printf(TEXT("Ollama HTTP error %d"), HttpResponse->GetResponseCode());
    }
    else
    {
        TSharedPtr<FJsonObject> Root;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
        if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
        {
            if (Root->TryGetStringField(TEXT("response"), Message))
            {
                bSuccess = true;
            }
            else
            {
                Message = TEXT("Ollama response JSON missing 'response' field.");
            }
        }
        else
        {
            Message = TEXT("Failed to parse Ollama response JSON.");
        }
    }

    if (!bSuccess && SourceRequest->CanRetry())
    {
        SourceRequest->MarkRetry();
        SourceRequest->SetState(EOllamaRequestState::Pending);
        RequestQueue->Enqueue(SourceRequest);
        DispatchNextRequest();
        return;
    }

    SourceRequest->Complete(bSuccess, Message, Latency);
    DispatchNextRequest();
}

FString UOllamaSubsystem::BuildMissingModelsMessage(const TArray<FString>& MissingModels) const
{
    FString Message = TEXT("Ollama is running but required models are missing. Install with:\n");
    for (const FString& Model : MissingModels)
    {
        Message += FString::Printf(TEXT("  ollama pull %s\n"), *Model);
    }

    Message += TEXT("After pulling models, restart Greymaw Chronicles.");
    return Message;
}
