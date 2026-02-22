#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "Ollama/OllamaTypes.h"
#include "OllamaSubsystem.generated.h"

class UOllamaRequest;
class UOllamaRequestQueue;

UCLASS()
class GREYMAWCHRONICLES_API UOllamaSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Ollama")
    bool IsOllamaAvailable() const { return bOllamaAvailable; }

    UFUNCTION(BlueprintCallable, Category = "Ollama")
    const TArray<FString>& GetAvailableModels() const { return AvailableModels; }

    UFUNCTION(BlueprintCallable, Category = "Ollama")
    FString GetLastHealthMessage() const { return LastHealthMessage; }

    void SendGenerateRequest(const FString& Model, const FString& Prompt, const FOllamaRequestOptions& Options, FOnOllamaComplete Callback);

    UPROPERTY(BlueprintAssignable, Category = "Ollama")
    FOnOllamaHealthChanged OnOllamaHealthChanged;

private:
    FString BuildUrl(const FString& Path) const;
    void PerformStartupHealthCheck();
    void OnHealthResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

    void DispatchNextRequest();
    void SubmitRequest(UOllamaRequest* Request);
    void OnGenerateResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bConnectedSuccessfully, UOllamaRequest* SourceRequest, double RequestStartSeconds);

    FString BuildMissingModelsMessage(const TArray<FString>& MissingModels) const;

private:
    UPROPERTY(EditAnywhere, Category = "Ollama")
    FString OllamaBaseUrl = TEXT("http://localhost:11434");

    UPROPERTY(EditAnywhere, Category = "Ollama")
    int32 RequestTimeoutSeconds = 15;

    UPROPERTY()
    bool bOllamaAvailable = false;

    UPROPERTY()
    TArray<FString> AvailableModels;

    UPROPERTY()
    FString LastHealthMessage;

    UPROPERTY()
    TObjectPtr<UOllamaRequestQueue> RequestQueue;
};
