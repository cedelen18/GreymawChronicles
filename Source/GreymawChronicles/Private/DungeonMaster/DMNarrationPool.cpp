#include "DungeonMaster/DMNarrationPool.h"

void UDMNarrationPool::RegisterSlot(const FString& SlotKey, const TArray<FString>& Lines)
{
    if (Lines.Num() > 0)
    {
        Pool.Add(SlotKey, Lines);
    }
}

FString UDMNarrationPool::PickRandom(const FString& SlotKey) const
{
    const TArray<FString>* Lines = Pool.Find(SlotKey);
    if (!Lines || Lines->Num() == 0)
    {
        return FString();
    }

    if (Lines->Num() == 1)
    {
        return (*Lines)[0];
    }

    const int32 Last = LastUsedIndex.Contains(SlotKey) ? LastUsedIndex[SlotKey] : INDEX_NONE;
    int32 Pick;
    do
    {
        Pick = FMath::RandRange(0, Lines->Num() - 1);
    } while (Pick == Last);

    LastUsedIndex.Add(SlotKey, Pick);
    return (*Lines)[Pick];
}

void UDMNarrationPool::PopulateTavernDefaults()
{
    // --- Look around ---
    RegisterSlot(TEXT("look_around"), {
        TEXT("Warm lamplight spills across the Thornhaven taproom. Marta polishes a mug behind the bar, Kael leans against a post watching the room, and old Durgan mutters into his ale as rain taps the windows. A hearth crackles in the corner and a faded quest board hangs by the door."),
        TEXT("The taproom is alive with quiet murmur. Marta wipes down the bar while Kael watches the door. Durgan hunches over his drink in the far corner. Rain streaks the windows and the hearth pops with sparks. A notice board catches your eye near the entrance."),
        TEXT("You take in the scene: smoke curls from the hearth, patrons nurse their drinks, and the rain outside drums a steady rhythm. Marta tends the bar, Kael keeps his back to the wall, and Durgan stares into nothing. A weathered quest board hangs beside the door.")
    });

    // --- Talk to marta ---
    RegisterSlot(TEXT("talk_marta"), {
        TEXT("Marta lowers her voice. 'Three villagers vanished near the Greymaw trail last week. The guard won't go, says it's bad luck. If you're heading that way, I'll pay fifty gold for news of what happened to them.'"),
        TEXT("Marta glances around before leaning in. 'People are disappearing on the Greymaw trail. Three gone in a week. The watch says it's wolves, but wolves don't take whole families. Fifty gold if you find answers.'"),
        TEXT("Marta sets down her cloth and fixes you with a steady look. 'You seem capable. The Greymaw trail has swallowed three souls this week alone. Fifty gold to anyone who brings back word of what happened. Interested?'")
    });

    // --- Walk to bar ---
    RegisterSlot(TEXT("move_bar"), {
        TEXT("You cross the taproom floor toward the bar. Floorboards creak underfoot and conversations pause as the regulars size you up."),
        TEXT("Your boots thud against old wood as you make your way to the bar. A few heads turn, measuring you with quiet suspicion."),
        TEXT("You weave between tables toward the scarred bar top. The smell of stale ale and woodsmoke thickens with each step.")
    });

    // --- Arm wrestle setup ---
    RegisterSlot(TEXT("challenge_kael_setup"), {
        TEXT("Kael grins and slams his elbow on the table. 'Let's see what you've got, adventurer.' The taproom cheers as you grip hands."),
        TEXT("Kael rolls his sleeves. 'Haven't had a decent match in weeks. Don't embarrass yourself.' He plants his elbow and the crowd gathers."),
        TEXT("Kael cracks his knuckles. 'Alright. One round, no magic. Winner drinks free.' He clasps your hand and the taproom goes quiet.")
    });

    // --- Arm wrestle success ---
    RegisterSlot(TEXT("challenge_kael_win"), {
        TEXT("With a surge of strength, you slam Kael's hand to the table! The room erupts. Kael laughs and claps you on the shoulder. 'Stronger than you look!'"),
        TEXT("You push through the burn and force Kael's arm down with a crack against the table. He whistles, impressed. 'Well played. Drinks are on me.'"),
        TEXT("Inch by inch, you overpower Kael until his knuckles hit wood. The crowd roars. Kael shakes his hand out, grinning. 'I'll remember that grip.'")
    });

    // --- Arm wrestle failure ---
    RegisterSlot(TEXT("challenge_kael_lose"), {
        TEXT("Kael overpowers you with a grin and pins your arm. 'Good effort! Next round's on me.' He slides a mug your way."),
        TEXT("Your arm buckles and Kael drives it to the table with ease. 'Don't feel bad. I've been doing this since I could lift a mug.' He pushes a drink your way."),
        TEXT("Kael's arm barely trembles as he pins yours flat. 'Better luck next time, friend.' He buys you an ale to soften the sting.")
    });

    // --- Talk to kael ---
    RegisterSlot(TEXT("talk_kael"), {
        TEXT("Kael straightens up. 'I've been keeping an eye on the trail north. Something doesn't feel right -- too quiet. If you're going, I'm coming with you.'"),
        TEXT("Kael folds his arms. 'The Greymaw trail used to see traders every day. Now nothing. I'm no coward, and I don't like mysteries. Count me in if you're headed that way.'"),
        TEXT("Kael meets your eyes. 'I lost a friend on that trail two winters back. Never found the body. If you're investigating, I want in. I owe it to him.'")
    });

    // --- Durgan ---
    RegisterSlot(TEXT("talk_durgan"), {
        TEXT("Old Durgan peers at you with rheumy eyes. 'The Greymaw... I saw it once. Decades ago. A mouth in the earth that swallowed the screams.' He turns back to his ale, hands shaking."),
        TEXT("Durgan's voice drops to a rasp. 'Don't go looking for the Greymaw at night. It hears you thinking. I barely made it back, and I was young then.' His tankard trembles in his grip."),
        TEXT("Durgan barely looks up. 'The earth opened and swallowed them. All of them. You think iron and courage will save you? The Greymaw doesn't care about brave fools.' He drains his ale in one pull.")
    });

    // --- Quest board ---
    RegisterSlot(TEXT("inspect_board"), {
        TEXT("The quest board holds a single weathered parchment: 'MISSING -- Three villagers last seen on the Greymaw trail. 50 gold reward for information. -- Marta, Thornhaven Taproom.'"),
        TEXT("A single notice dominates the board, pinned with a rusted nail: 'WANTED -- Information on three missing persons. Last seen heading north on the Greymaw trail. Reward: 50 gold. See Marta.'"),
        TEXT("The board is mostly bare save for one urgent notice scrawled in Marta's hand: 'Elin, Thom, and young Sara -- gone. Greymaw trail. 50 gold for answers. Please help. -- M.'")
    });

    // --- Catch-all / unknown ---
    RegisterSlot(TEXT("fallback"), {
        TEXT("You pause, considering your next move. The taproom hums around you -- Marta at the bar, Kael by the post, Durgan in his corner, and a quest board by the door."),
        TEXT("The moment stretches. Around you, the tavern carries on -- murmured conversations, the clink of mugs, rain against glass. Marta, Kael, and Durgan are nearby, and the quest board waits."),
        TEXT("Nothing immediately comes of your action, but the tavern offers plenty of options. Marta tends the bar, Kael watches the room, Durgan broods, and the quest board by the door might be worth a look.")
    });

    // --- Examine hearth ---
    RegisterSlot(TEXT("inspect_hearth"), {
        TEXT("The hearth crackles with a low, steady fire. The warmth is welcome after the rain. Someone has carved initials into the mantel -- decades of visitors leaving their mark."),
        TEXT("Embers pulse in the hearth like a breathing thing. The stonework is old, smoke-blackened, with scratched tallies along one edge. A poker leans against the wall."),
        TEXT("The fire throws shifting shadows across the taproom floor. Above the mantel, a faded painting shows Thornhaven in better days -- more buildings, more people.")
    });

    // --- Buy / drink ale ---
    RegisterSlot(TEXT("use_drink"), {
        TEXT("Marta slides a foaming mug your way without being asked. The ale is dark and bitter, brewed local. It takes the edge off the road dust."),
        TEXT("You signal for a drink. Marta pours a dark ale and pushes it across the bar. 'First one's a copper. After that, we'll see.' It's not great, but it's warm."),
        TEXT("The ale hits your throat with a malty burn. It's rough stuff -- Thornhaven doesn't brew for flavor -- but your shoulders loosen and the room feels a shade friendlier.")
    });

    // --- Help durgan ---
    RegisterSlot(TEXT("help_durgan"), {
        TEXT("You steady Durgan's trembling hand around his tankard. He looks up, startled. 'Kind of you. Most folk pretend I'm not here.' He musters a thin smile."),
        TEXT("Durgan flinches when you approach, then relaxes. 'Haven't had someone check on me in weeks. I'm fine. Just... memories.' He pats your arm with a shaking hand."),
        TEXT("You offer to fetch Durgan another ale. He nods gratefully. 'You remind me of someone. Someone who came through here before the trail went bad.' His eyes go distant.")
    });
}
