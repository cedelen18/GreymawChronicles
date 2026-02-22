#include "DungeonMaster/DMBrainSubsystem.h"

#include "DungeonMaster/DMConversationHistory.h"
#include "DungeonMaster/DMPromptBuilder.h"
#include "DungeonMaster/DMResponseParser.h"
#include "Ollama/OllamaSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogDMBrainSubsystem, Log, All);

void UDMBrainSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    PromptBuilder = NewObject<UDMPromptBuilder>(this);
    ResponseParser = NewObject<UDMResponseParser>(this);
    ConversationHistory = NewObject<UDMConversationHistory>(this);
    OllamaSubsystem = GetGameInstance()->GetSubsystem<UOllamaSubsystem>();
}

void UDMBrainSubsystem::ProcessPlayerInput(const FString& PlayerInput)
{
    if (!OllamaSubsystem || !PromptBuilder || !ResponseParser || !ConversationHistory)
    {
        UE_LOG(LogDMBrainSubsystem, Error, TEXT("DM Brain dependencies are missing."));
        return;
    }

    FDMPromptContext Context;
    Context.SceneContext = TEXT("scene_id=tavern_arrival");
    Context.CharacterSheets = TEXT("{}") ;
    Context.AdventureState = TEXT("{}") ;
    Context.ConversationHistory = ConversationHistory->ToPromptString();

    const FString Prompt = PromptBuilder->BuildDMPrompt(PlayerInput, Context);

    FOllamaRequestOptions Options;
    Options.Priority = EOllamaRequestPriority::High;
    Options.Temperature = 0.7f;

    OllamaSubsystem->SendGenerateRequest(TEXT("qwen2.5:32b"), Prompt, Options,
        FOnOllamaComplete::CreateUObject(this, &UDMBrainSubsystem::OnOllamaCompletion, PlayerInput));
}

void UDMBrainSubsystem::OnOllamaCompletion(bool bSuccess, const FString& ResponseText, float LatencySeconds, FString OriginalPlayerInput)
{
    if (!bSuccess)
    {
        UE_LOG(LogDMBrainSubsystem, Warning, TEXT("DM request failed in %.2fs: %s"), LatencySeconds, *ResponseText);
        OnDMNarration.Broadcast(TEXT("The DM falters for a moment. Please try again."));
        return;
    }

    const FDMResponse Parsed = ResponseParser->ParseResponse(ResponseText);
    if (!Parsed.bValid)
    {
        UE_LOG(LogDMBrainSubsystem, Warning, TEXT("DM parse failure: %s"), *Parsed.Error);
        OnDMNarration.Broadcast(TEXT("I couldn't interpret that response cleanly. Try rephrasing your action."));
        return;
    }

    FDMExchange Exchange;
    Exchange.PlayerInput = OriginalPlayerInput;
    Exchange.DMResponseSummary = Parsed.Narration.Left(250);
    Exchange.SceneID = TEXT("tavern_arrival");
    Exchange.TimestampUtc = FDateTime::UtcNow();
    ConversationHistory->AddExchange(Exchange);

    OnDMNarration.Broadcast(Parsed.Narration);
    OnDMActionsReady.Broadcast(Parsed.Actions);
}
