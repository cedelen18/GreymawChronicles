#include "DungeonMaster/DMIntentClassifier.h"

TArray<UDMIntentClassifier::FIntentRule> UDMIntentClassifier::BuildRules()
{
    TArray<FIntentRule> Rules;

    // Look / Observe
    Rules.Add({ EDMIntent::Look, {
        TEXT("look"), TEXT("observe"), TEXT("survey"), TEXT("scan"), TEXT("glance"),
        TEXT("where am i"), TEXT("surroundings"), TEXT("see")
    }, 0.9f });

    // Move / Go
    Rules.Add({ EDMIntent::Move, {
        TEXT("walk"), TEXT("move"), TEXT("go"), TEXT("approach"), TEXT("head"),
        TEXT("travel"), TEXT("step"), TEXT("cross"), TEXT("enter"), TEXT("leave"),
        TEXT("sit"), TEXT("stand")
    }, 0.9f });

    // Talk / Speak / Ask
    Rules.Add({ EDMIntent::Talk, {
        TEXT("talk"), TEXT("speak"), TEXT("ask"), TEXT("tell"), TEXT("chat"),
        TEXT("say"), TEXT("greet"), TEXT("hail"), TEXT("call"), TEXT("question"),
        TEXT("inquire"), TEXT("converse")
    }, 0.9f });

    // Inspect / Examine / Read
    Rules.Add({ EDMIntent::Inspect, {
        TEXT("inspect"), TEXT("examine"), TEXT("read"), TEXT("study"), TEXT("check"),
        TEXT("investigate"), TEXT("search"), TEXT("peer"), TEXT("scrutinize")
    }, 0.9f });

    // Challenge / Fight / Contest
    Rules.Add({ EDMIntent::Challenge, {
        TEXT("challenge"), TEXT("fight"), TEXT("wrestle"), TEXT("arm wrestle"),
        TEXT("attack"), TEXT("duel"), TEXT("contest"), TEXT("spar"), TEXT("punch"),
        TEXT("grapple"), TEXT("shove"), TEXT("intimidate"), TEXT("threaten")
    }, 0.95f });

    // Help / Assist
    Rules.Add({ EDMIntent::Help, {
        TEXT("help"), TEXT("assist"), TEXT("heal"), TEXT("aid"), TEXT("comfort"),
        TEXT("tend"), TEXT("bandage"), TEXT("support"), TEXT("protect")
    }, 0.85f });

    // Use / Interact / Open
    Rules.Add({ EDMIntent::Use, {
        TEXT("use"), TEXT("open"), TEXT("close"), TEXT("grab"), TEXT("take"),
        TEXT("pick up"), TEXT("drink"), TEXT("eat"), TEXT("light"), TEXT("touch"),
        TEXT("pull"), TEXT("push"), TEXT("interact")
    }, 0.85f });

    // Sprint I: Order / Buy / Purchase
    Rules.Add({ EDMIntent::Order, {
        TEXT("order"), TEXT("buy"), TEXT("purchase"), TEXT("request"), TEXT("serve"),
        TEXT("another round")
    }, 0.90f });

    // Sprint I: Steal / Pickpocket
    Rules.Add({ EDMIntent::Steal, {
        TEXT("steal"), TEXT("pickpocket"), TEXT("pilfer"), TEXT("swipe"),
        TEXT("snatch"), TEXT("pocket"), TEXT("filch"), TEXT("lift")
    }, 0.95f });

    // Sprint I: Listen / Eavesdrop
    Rules.Add({ EDMIntent::Listen, {
        TEXT("listen"), TEXT("eavesdrop"), TEXT("overhear"), TEXT("spy")
    }, 0.90f });

    // Sprint I: Persuade / Convince / Charm
    Rules.Add({ EDMIntent::Persuade, {
        TEXT("persuade"), TEXT("convince"), TEXT("bribe"), TEXT("coax"),
        TEXT("entreat"), TEXT("charm"), TEXT("flatter"), TEXT("flirt")
    }, 0.90f });

    // Sprint I: Rest / Relax / Meditate
    Rules.Add({ EDMIntent::Rest, {
        TEXT("rest"), TEXT("sleep"), TEXT("meditate"), TEXT("relax"),
        TEXT("nap"), TEXT("take a break")
    }, 0.85f });

    // Sprint I: Gamble / Bet / Wager
    Rules.Add({ EDMIntent::Gamble, {
        TEXT("gamble"), TEXT("bet"), TEXT("wager"), TEXT("play dice"),
        TEXT("play cards")
    }, 0.90f });

    // Sprint K: Accept / Agree / Volunteer
    Rules.Add({ EDMIntent::Accept, {
        TEXT("accept"), TEXT("agree"), TEXT("take the job"), TEXT("sign up"),
        TEXT("volunteer"), TEXT("take on"), TEXT("take it")
    }, 0.92f });

    // Sprint K: Report / Inform / Share findings
    Rules.Add({ EDMIntent::Report, {
        TEXT("report"), TEXT("share findings"), TEXT("inform"),
        TEXT("reveal"), TEXT("tell what i found"), TEXT("share what i know")
    }, 0.92f });

    return Rules;
}

FDMIntentResult UDMIntentClassifier::Classify(const FString& PlayerInput) const
{
    FDMIntentResult Best;
    Best.Intent = EDMIntent::Unknown;
    Best.Confidence = 0.0f;

    const FString Lower = PlayerInput.ToLower();
    const TArray<FIntentRule> Rules = BuildRules();

    TArray<FString> MatchedKeywords;

    for (const FIntentRule& Rule : Rules)
    {
        for (const FString& Keyword : Rule.Keywords)
        {
            if (Lower.Contains(Keyword))
            {
                // Longer keyword matches are more specific, boost confidence slightly
                const float LengthBonus = FMath::Clamp(static_cast<float>(Keyword.Len()) / 12.0f, 0.0f, 0.1f);
                const float Score = Rule.BaseConfidence + LengthBonus;

                if (Score > Best.Confidence)
                {
                    Best.Intent = Rule.Intent;
                    Best.Confidence = Score;
                    MatchedKeywords.Reset();
                    MatchedKeywords.Add(Keyword);
                }
            }
        }
    }

    Best.Subject = ExtractSubject(Lower, MatchedKeywords);
    return Best;
}

FString UDMIntentClassifier::ExtractSubject(const FString& Input, const TArray<FString>& ConsumedKeywords) const
{
    // Known subjects in the tavern scene
    static const TArray<FString> KnownSubjects = {
        TEXT("marta"), TEXT("kael"), TEXT("durgan"), TEXT("old man"),
        TEXT("quest board"), TEXT("notice board"), TEXT("board"),
        TEXT("bar"), TEXT("hearth"), TEXT("fire"), TEXT("fireplace"),
        TEXT("door"), TEXT("ale"), TEXT("mug"), TEXT("drink"),
        TEXT("table"), TEXT("chair"), TEXT("post"), TEXT("window"),
        // Sprint I additions
        TEXT("coin"), TEXT("pouch"), TEXT("purse"),
        TEXT("corner"), TEXT("booth"), TEXT("shadows"),
        TEXT("food"), TEXT("stew"), TEXT("bread"), TEXT("wine"),
        TEXT("dice"), TEXT("cards"), TEXT("game")
    };

    // Check for longest known subject first
    FString BestMatch;
    for (const FString& Subject : KnownSubjects)
    {
        if (Input.Contains(Subject) && Subject.Len() > BestMatch.Len())
        {
            BestMatch = Subject;
        }
    }

    if (!BestMatch.IsEmpty())
    {
        return BestMatch;
    }

    // Fallback: strip consumed keywords and prepositions, return remainder
    FString Remainder = Input;
    for (const FString& Kw : ConsumedKeywords)
    {
        Remainder = Remainder.Replace(*Kw, TEXT(""));
    }

    // Strip common prepositions
    static const TArray<FString> StopWords = {
        TEXT("to"), TEXT("the"), TEXT("at"), TEXT("a"), TEXT("an"),
        TEXT("with"), TEXT("around"), TEXT("over"), TEXT("in"), TEXT("on")
    };

    TArray<FString> Words;
    Remainder.ParseIntoArray(Words, TEXT(" "), true);
    Words.RemoveAll([&](const FString& W) { return StopWords.Contains(W) || W.IsEmpty(); });

    return Words.Num() > 0 ? FString::Join(Words, TEXT(" ")) : FString();
}
