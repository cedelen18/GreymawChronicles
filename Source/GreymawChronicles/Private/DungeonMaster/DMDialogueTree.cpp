#include "DungeonMaster/DMDialogueTree.h"
#include "DungeonMaster/DMWorldStateSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogDialogueTree, Log, All);

void UDMDialogueTree::RegisterNode(const FGCDialogueNode& Node)
{
    Nodes.Add(Node.NodeId, Node);
}

FGCDialogueNode UDMDialogueTree::GetNode(const FString& NodeId) const
{
    if (const FGCDialogueNode* Found = Nodes.Find(NodeId))
    {
        return *Found;
    }
    return FGCDialogueNode();
}

bool UDMDialogueTree::HasNode(const FString& NodeId) const
{
    return Nodes.Contains(NodeId);
}

TArray<FGCDialogueOption> UDMDialogueTree::GetAvailableOptions(const FString& NodeId, const UDMWorldStateSubsystem* WorldState) const
{
    TArray<FGCDialogueOption> Available;

    const FGCDialogueNode* Node = Nodes.Find(NodeId);
    if (!Node) return Available;

    for (const FGCDialogueOption& Option : Node->Options)
    {
        // Check state requirement
        if (!Option.RequiredStateCategory.IsEmpty())
        {
            if (!WorldState)
            {
                continue; // Requirement exists but no WorldState — skip
            }

            const FString ActualValue = WorldState->GetState(Option.RequiredStateCategory, Option.RequiredStateKey);
            if (ActualValue != Option.RequiredStateValue)
            {
                continue; // Requirement not met — skip
            }
        }

        Available.Add(Option);
    }

    return Available;
}

void UDMDialogueTree::PopulateMartaDialogue()
{
    // Root node
    FGCDialogueNode MartaRoot;
    MartaRoot.NodeId = TEXT("marta_root");
    MartaRoot.NPCGreeting = TEXT("Marta wipes a mug and looks you over. 'What would you know, stranger?'");

    // Option 1: Ask about missing villagers (always available)
    FGCDialogueOption AskVillagers;
    AskVillagers.OptionText = TEXT("Tell me about the missing villagers.");
    AskVillagers.ResponseNarration = TEXT("Marta's expression darkens. 'Three folk vanished in the past fortnight. All last seen heading toward the Greymaw trail. The constable says it's wolves, but wolves don't take people whole.'");
    AskVillagers.NextNodeId = TEXT("marta_root");
    MartaRoot.Options.Add(AskVillagers);

    // Option 2: Order food (always available)
    FGCDialogueOption OrderFood;
    OrderFood.OptionText = TEXT("What's good to eat?");
    OrderFood.ResponseNarration = TEXT("'Mutton stew, if you've coin. It'll warm your bones.' She slides a bowl toward you with practiced ease.");

    FDMWorldChange FoodChange;
    FoodChange.Type = TEXT("npc_interaction_state");
    FoodChange.Key = TEXT("marta");
    FoodChange.Value = TEXT("helpful");
    OrderFood.WorldChanges.Add(FoodChange);
    OrderFood.NextNodeId = TEXT("");
    MartaRoot.Options.Add(OrderFood);

    // Option 3: Report findings (requires accepted task)
    FGCDialogueOption ReportFindings;
    ReportFindings.OptionText = TEXT("I've been investigating. Here's what I found.");
    ReportFindings.RequiredStateCategory = TEXT("task");
    ReportFindings.RequiredStateKey = TEXT("tavern_investigation");
    ReportFindings.RequiredStateValue = TEXT("accepted");
    ReportFindings.ResponseNarration = TEXT("Marta listens intently as you share what you've learned. Her eyes widen at the details.");
    ReportFindings.NextNodeId = TEXT("");
    MartaRoot.Options.Add(ReportFindings);

    // Option 4: Never mind (always available)
    FGCDialogueOption NeverMind;
    NeverMind.OptionText = TEXT("Never mind.");
    NeverMind.ResponseNarration = TEXT("Marta shrugs and returns to polishing the bar. 'Suit yourself.'");
    NeverMind.NextNodeId = TEXT("");
    MartaRoot.Options.Add(NeverMind);

    RegisterNode(MartaRoot);

    UE_LOG(LogDialogueTree, Log, TEXT("Populated Marta dialogue tree."));
}

void UDMDialogueTree::PopulateDurganDialogue()
{
    FGCDialogueNode DurganRoot;
    DurganRoot.NodeId = TEXT("durgan_root");
    DurganRoot.NPCGreeting = TEXT("Durgan peers at you with rheumy eyes. 'You look like someone seeking answers. Or trouble. Often the same thing.'");

    // Option 1: Ask about Greymaw
    FGCDialogueOption AskGreymaw;
    AskGreymaw.OptionText = TEXT("What do you know about the Greymaw?");
    AskGreymaw.ResponseNarration = TEXT("'The Greymaw was sealed decades ago after the mine collapse. But the seal... it's weakening. I've heard things at night. The earth remembers what was buried there.'");
    AskGreymaw.NextNodeId = TEXT("durgan_root");
    DurganRoot.Options.Add(AskGreymaw);

    // Option 2: Ask about lore (requires helpful state)
    FGCDialogueOption AskLore;
    AskLore.OptionText = TEXT("Tell me the old stories about what's beneath.");
    AskLore.RequiredStateCategory = TEXT("npc_interaction_state");
    AskLore.RequiredStateKey = TEXT("durgan");
    AskLore.RequiredStateValue = TEXT("helpful");
    AskLore.ResponseNarration = TEXT("Durgan leans close. 'Before the mine, it was a temple. Devoted to something old — older than the gods the priests name. The miners broke through into its chambers. What they found... changed them.'");

    FDMWorldChange LoreClue;
    LoreClue.Type = TEXT("set_state");
    LoreClue.Key = TEXT("task_clue:durgan_lore");
    LoreClue.Value = TEXT("true");
    AskLore.WorldChanges.Add(LoreClue);
    AskLore.NextNodeId = TEXT("durgan_root");
    DurganRoot.Options.Add(AskLore);

    // Option 3: Leave
    FGCDialogueOption Leave;
    Leave.OptionText = TEXT("I'll let you rest.");
    Leave.ResponseNarration = TEXT("Durgan nods slowly and turns back to his ale. 'Be careful out there.'");
    Leave.NextNodeId = TEXT("");
    DurganRoot.Options.Add(Leave);

    RegisterNode(DurganRoot);

    UE_LOG(LogDialogueTree, Log, TEXT("Populated Durgan dialogue tree."));
}
