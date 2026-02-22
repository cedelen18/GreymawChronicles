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

    // ===================================================================
    // Sprint I: New narration pool slots (16 slots x 3 variants = 48 lines)
    // ===================================================================

    // --- Order food ---
    RegisterSlot(TEXT("order_food"), {
        TEXT("Marta nods and disappears behind the bar. She returns with a steaming bowl of stew and a heel of dark bread. 'Eat up. You'll need your strength.'"),
        TEXT("'Coming right up.' Marta ladles thick stew into a clay bowl and slides it your way with a chunk of bread. The warmth seeps into your hands."),
        TEXT("Marta sets down a plate of bread, cheese, and a bowl of peppery stew. 'Best I've got tonight. The supply carts stopped coming last week.'")
    });

    // --- Order refused (Marta suspicious) ---
    RegisterSlot(TEXT("order_refused"), {
        TEXT("Marta's eyes narrow. 'I don't serve thieves. You can drink rainwater for all I care.' She turns her back and busies herself with mugs."),
        TEXT("Marta folds her arms. 'After what you pulled? You're lucky I haven't called the watch. No food, no ale. Move along.'"),
        TEXT("'Not a chance.' Marta slaps the bar with her cloth. 'You want service, earn back my trust first. Until then, the bar is closed to you.'")
    });

    // --- Steal setup ---
    RegisterSlot(TEXT("steal_setup"), {
        TEXT("You edge closer, eyes fixed on an unguarded pouch. Your fingers twitch with practiced intent as you time your move."),
        TEXT("The crowd shifts and you see your opening. A coin pouch dangles loosely, its owner distracted by drink and conversation."),
        TEXT("You slip through the press of bodies, angling toward your mark. One clean grab -- that's all it takes. If you're fast enough.")
    });

    // --- Steal success ---
    RegisterSlot(TEXT("steal_success"), {
        TEXT("Your fingers close around the pouch in one smooth motion. Nobody saw a thing. You pocket the coins and melt back into the crowd."),
        TEXT("Quick as a shadow, the pouch is yours. A satisfying weight of coin settles in your pocket. The mark keeps drinking, oblivious."),
        TEXT("A deft hand and good timing -- the pouch slides free without a sound. You feel the familiar thrill of a clean lift.")
    });

    // --- Steal fail ---
    RegisterSlot(TEXT("steal_fail"), {
        TEXT("Your hand brushes the pouch but the mark whips around. 'Hey! What do you think you're doing?' Angry eyes bore into you."),
        TEXT("Too slow. The owner catches your wrist mid-grab and yanks your hand away. 'Thief!' The word cuts through the tavern noise."),
        TEXT("Your fingers fumble and the pouch slips. The mark grabs your arm with surprising strength. 'I saw that. Don't try it again.'")
    });

    // --- Listen success ---
    RegisterSlot(TEXT("listen_success"), {
        TEXT("Through the din you catch a whispered exchange: '...the old mine shaft. That's where they went in. None came back.' The speakers fall silent when a barmaid passes."),
        TEXT("You overhear two merchants in hushed tones: 'The Greymaw path glows at night now. Blue light, like foxfire but deeper.' One shudders visibly."),
        TEXT("A snippet of conversation drifts your way: '...Durgan knows more than he lets on. Ask him about the ward-stones. He helped place them decades ago.'")
    });

    // --- Listen fail ---
    RegisterSlot(TEXT("listen_fail"), {
        TEXT("The rain drums too loudly against the windows. All you catch are fragments -- laughter, the clink of mugs, nothing useful."),
        TEXT("You strain to hear but the hearth crackles and a group at the next table bursts into song. The moment passes."),
        TEXT("Too much noise. Between the rain, the fire, and the crowd, every whisper dissolves into the general hum of the taproom.")
    });

    // --- Persuade Marta success ---
    RegisterSlot(TEXT("persuade_marta_success"), {
        TEXT("Marta's guarded expression softens. 'Alright. I believe you mean well.' She leans closer. 'The last group that went to Greymaw -- they had a map. I kept a copy.'"),
        TEXT("Something in your words reaches her. Marta sighs. 'Fine. I'll trust you. There's a back trail the villagers used -- safer than the main road. I'll mark it for you.'"),
        TEXT("Marta studies your face, then nods slowly. 'You've got honest eyes, I'll give you that. Here --' she slides a small iron key across the bar. 'Supply cache by the north gate.'")
    });

    // --- Persuade Durgan success ---
    RegisterSlot(TEXT("persuade_durgan_success"), {
        TEXT("Durgan's resistance crumbles. 'Fine. You want the truth? The Greymaw isn't a place -- it's alive. I watched it swallow a man whole.' His voice breaks."),
        TEXT("The old man lowers his guard. 'There are ward-stones along the trail. I helped place them forty years ago. If they've cracked...' He trails off, fear in his eyes."),
        TEXT("Durgan grips your arm. 'Listen carefully. The Greymaw sleeps in cycles. When it wakes, the ground hums. If you feel that hum -- run. Don't look back.'")
    });

    // --- Rest in tavern ---
    RegisterSlot(TEXT("rest_tavern"), {
        TEXT("You find a quiet corner and close your eyes. The warmth of the hearth and the murmur of voices lull you into a brief, restorative rest."),
        TEXT("You lean back against the wall and let the tension drain from your shoulders. The taproom noise fades to a gentle hum as you catch your breath."),
        TEXT("You settle into a chair by the fire and let yourself drift. Minutes pass in comfortable stillness. When you open your eyes, you feel steadier.")
    });

    // --- Gamble setup ---
    RegisterSlot(TEXT("gamble_setup"), {
        TEXT("A rough-looking patron catches your eye and rattles a cup of bone dice. 'Fancy a game, stranger? Five coppers a throw.' He grins with too few teeth."),
        TEXT("You spot a dice game in the corner. The players look up as you approach. 'Room for one more,' says a wiry woman, pushing dice your way."),
        TEXT("A scarred man deals cards from a worn deck. 'Stakes are simple -- copper a hand, silver if you're bold.' He shuffles with practiced ease.")
    });

    // --- Gamble win ---
    RegisterSlot(TEXT("gamble_win"), {
        TEXT("The dice fall in your favor! Your opponent stares, then pushes a small pile of coins your way. 'Lucky throw. Best not push it.'"),
        TEXT("A winning hand! The table groans as you rake in the coppers. 'Beginner's luck,' someone mutters, but the coins are real enough."),
        TEXT("The last throw seals it -- you win cleanly. A few coins richer and a reputation for luck established, you lean back with satisfaction.")
    });

    // --- Gamble lose ---
    RegisterSlot(TEXT("gamble_lose"), {
        TEXT("The dice betray you. Your opponent scoops the coppers with a grin. 'Better luck next time, friend.' Your pouch feels lighter."),
        TEXT("A losing hand. The wiry woman collects your coins with a sympathetic shrug. 'The dice are fickle tonight.' You cut your losses."),
        TEXT("The cards don't cooperate. You slide your coppers across the table and the scarred dealer pockets them. 'House always wins eventually.'")
    });

    // --- Talk Marta (friendly disposition) ---
    RegisterSlot(TEXT("talk_marta_friendly"), {
        TEXT("Marta's face brightens when she sees you. 'Ah, my favorite customer.' She slides an extra bread roll your way. 'The trail news is grim, but I trust you.'"),
        TEXT("'Back again? Good.' Marta leans in with a warmer tone than before. 'I've been asking around. The missing folk -- they all took the north fork.'"),
        TEXT("Marta greets you with a rare smile. 'You've been kind to an old barkeep. I won't forget it.' She tops off your mug unbidden.")
    });

    // --- Talk Marta (suspicious disposition) ---
    RegisterSlot(TEXT("talk_marta_suspicious"), {
        TEXT("Marta's eyes are cold. 'What do you want now? Keep your hands where I can see them.' She grips a heavy mug like a weapon."),
        TEXT("'You.' Marta's voice is flat. 'I've got nothing to say to you. Try that again and I'll have Kael throw you out.' She turns away."),
        TEXT("Marta barely glances at you. 'Still here? I'd watch my back if I were you. Word gets around in a small village.' Her tone is ice.")
    });

    // --- Talk Durgan (open disposition) ---
    RegisterSlot(TEXT("talk_durgan_open"), {
        TEXT("Durgan gestures you closer. 'Since you asked proper-like, I'll tell you more. The ward-stones mark a circle around the Greymaw. Break the circle and...' He mimes an explosion."),
        TEXT("The old man's eyes are clearer now. 'You earned my trust, traveler. The Greymaw has a heart -- a stone buried deep. Find it, destroy it, and maybe this ends.'"),
        TEXT("Durgan speaks freely now. 'Forty years I've kept quiet. But you need to know -- the Greymaw trail has a guardian. A thing of root and stone. Be ready.'")
    });
}
