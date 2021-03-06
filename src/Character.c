#include "pebble.h"

    #include "Adventure.h"
    #include "Battle.h"
    #include "Character.h"
    #include "Items.h"
    #include "Logging.h"
    #include "Menu.h"
    #include "UILayers.h"
    #include "Utils.h"

    CharacterData characterData;
static int statPointsToSpend = 0;

void AddStatPointToSpend(void)
{
    ++statPointsToSpend;
}

void UpdateCharacterHealth(void)
{
    UpdateHealthText(characterData.stats.currentHealth, characterData.stats.maxHealth);
}

void UpdateCharacterLevel(void)
{
    UpdateLevelLayerText(characterData.level);
}

const char *UpdateLevelText(void)
{
    static char levelText[] = "00"; // Needs to be static because it's used by the system later.
    IntToString(levelText, 2, characterData.level);
    return levelText;
}

const char *UpdateXPText(void)
{
    static char xpText[] = "0000"; // Needs to be static because it's used by the system later.
    IntToString(xpText, 4, characterData.xp);
    return xpText;
}

const char *UpdateNextXPText(void)
{
    static char nextXPText[] = "0000"; // Needs to be static because it's used by the system later.
    IntToString(nextXPText, 4, characterData.xpForNextLevel);
    return nextXPText;
}

const char *UpdateGoldText(void)
{
    static char goldText[] = "00000"; // Needs to be static because it's used by the system later.
    IntToString(goldText, 5, characterData.gold);
    return goldText;
}

const char *UpdateEscapeText(void)
{
    static char escapeText[] = "000"; // Needs to be static because it's used by the system later.
    IntToString(escapeText, 3, characterData.escapes);
    return escapeText;
}

void IncrementEscapes(void)
{
    ++characterData.escapes;
}

int ComputePlayerHealth(int level)
{
    return 10 + ((level-1)*(level)/2) + ((level-1)*(level)*(level+1)/(6*32));
}

int ComputeXPForNextLevel(int level)
{
    return XP_FOR_NEXT_LEVEL;
}

void InitializeCharacter(void)
{
    INFO_LOG("Initializing character.");
    characterData.xp = 0;
    characterData.level = 1;
    characterData.gold = 0;
    characterData.escapes = 0;
    characterData.xpForNextLevel = ComputeXPForNextLevel(1);
    characterData.stats.maxHealth = ComputePlayerHealth(1);
    characterData.stats.currentHealth = characterData.stats.maxHealth;
    characterData.stats.strength = 1;
    characterData.stats.magic = 1;
    characterData.stats.defense = 1;
    characterData.stats.magicDefense = 1;
    statPointsToSpend = 0;

    UpdateCharacterLevel();
    UpdateCharacterHealth();
}

// Returns true on levelup
bool GrantExperience(int xp)
{
    characterData.xp += xp;
    if(characterData.xp >= characterData.xpForNextLevel)
    {
        return true;
    }
    return false;
}

void GrantGold(int gold)
{
    characterData.gold += gold;
    if(characterData.gold > 9999)
        characterData.gold = 9999;
}

bool DealPlayerDamage(int damage)
{
    int returnValue = false;
    characterData.stats.currentHealth = characterData.stats.currentHealth - damage;
    if(characterData.stats.currentHealth <= 0)
    {
        characterData.stats.currentHealth = 0;
        returnValue = true;
    }
    UpdateCharacterHealth();
    return returnValue;
}

CharacterData *GetCharacter(void)
{
    return &characterData;
}

void HealPlayerByPercent(int percent)
{
    characterData.stats.currentHealth += (characterData.stats.maxHealth * percent) / 100;
    if(characterData.stats.currentHealth > characterData.stats.maxHealth)
        characterData.stats.currentHealth = characterData.stats.maxHealth;

    UpdateCharacterHealth();
}

bool PlayerIsInjured(void)
{
    return characterData.stats.currentHealth < characterData.stats.maxHealth;
}

void EndMenuDisappear(Window *window)
{
    ResetGame();
}

void EndMenuAppear(Window *window);

MenuDefinition endMenuDef = 
{
    .menuEntries = 
    {
        {"OK", "Restart game", PopMenu}
    },
        .disappear = EndMenuDisappear,
    .appear = EndMenuAppear,
    .mainImageId = -1
};

void EndMenuAppear(Window *window)
{
    MenuAppear(window);
    if(characterData.stats.currentHealth <= 0)
        ShowMainWindowRow(0, "You lose", "");
    else
        ShowMainWindowRow(0, "You win", "");
    ShowMainWindowRow(1, "Floor", UpdateFloorText());
    ShowMainWindowRow(2, "Level", UpdateLevelText());
    ShowMainWindowRow(3, "Gold", UpdateGoldText());
    ShowMainWindowRow(4, "Escapes", UpdateEscapeText());
}

void ShowEndWindow(void)
{
    PushNewMenu(&endMenuDef);
}

const char  *UpdateStatPointText(void)
{
    static char statText[] = "00"; // Needs to be static because it's used by the system later.

    IntToString(statText, 2, statPointsToSpend);
    return statText;
}

const char  *UpdateStrengthText(void)
{
    static char strengthText[] = "00"; // Needs to be static because it's used by the system later.

    IntToString(strengthText, 2, characterData.stats.strength);
    return strengthText;
}

const char  *UpdateDefenseText(void)
{
    static char defenseText[] = "00"; // Needs to be static because it's used by the system later.

    IntToString(defenseText, 2, characterData.stats.defense);
    return defenseText;
}

const char  *UpdateMagicText(void)
{
    static char magicText[] = "00"; // Needs to be static because it's used by the system later.

    IntToString(magicText, 2, characterData.stats.magic);
    return magicText;
}

const char  *UpdateMagicDefenseText(void)
{
    static char magicDefenseText[] = "00"; // Needs to be static because it's used by the system later.

    IntToString(magicDefenseText, 2, characterData.stats.magicDefense);
    return magicDefenseText;
}

void DrawStatWindow(void)
{
    ShowMainWindowRow(0, "Stat Points", UpdateStatPointText());	
    ShowMainWindowRow(1, "Strength", UpdateStrengthText());
    ShowMainWindowRow(2, "Defense", UpdateDefenseText());
    ShowMainWindowRow(3, "Magic", UpdateMagicText());
    ShowMainWindowRow(4, "MagicDef", UpdateMagicDefenseText());
}

void IncrementStat(int *stat)
{
    if(statPointsToSpend && (*stat) < characterData.level)
    {
        ++(*stat);
        --statPointsToSpend;
        DrawStatWindow();
    }
    // after adding last stat point, change focus to quit
    if(statPointsToSpend == 0) {
        SetMenuSelection(0);
    }
}

void IncrementStrength(void)
{
    IncrementStat(&characterData.stats.strength);
}

void IncrementDefense(void)
{
    IncrementStat(&characterData.stats.defense);
}

void IncrementMagic(void)
{
    IncrementStat(&characterData.stats.magic);
}

void IncrementMagicDefense(void)
{
    IncrementStat(&characterData.stats.magicDefense);
}

void LevelUp(void)
{
    INFO_LOG("Level up.");
    statPointsToSpend += STAT_POINTS_PER_LEVEL;
    ++characterData.level;
    characterData.xpForNextLevel += ComputeXPForNextLevel(characterData.level);
    characterData.stats.maxHealth = ComputePlayerHealth(characterData.level);
    if(characterData.stats.maxHealth > 9999)
        characterData.stats.maxHealth = 9999;
    characterData.stats.currentHealth = characterData.stats.maxHealth;
    UpdateCharacterLevel();
    UpdateCharacterHealth();
    ShowStatMenu();
}

void StatMenuAppear(Window *window);

MenuDefinition statMenuDef = 
{
    .menuEntries = 
    {
        {"Quit", "Return to main menu", PopMenu},
        {"Increase", "Increase strength", IncrementStrength},
        {"Increase", "Increase defense", IncrementDefense},
        {"Increase", "Increase magic", IncrementMagic},
        {"Increase", "Increase magic defense", IncrementMagicDefense},
    },
        .appear = StatMenuAppear,
    .mainImageId = -1
};

void StatMenuAppear(Window *window)
{
    MenuAppear(window);
    DrawStatWindow();
}

void ShowStatMenu(void)
{
    PushNewMenu(&statMenuDef);
}

void ProgressMenuAppear(Window *window);

MenuDefinition progressMenuDef = 
{
    .menuEntries = 
    {
        {"Quit", "Return to main menu", PopMenu},
    },
        .appear = ProgressMenuAppear,
    .mainImageId = -1
};

void ProgressMenuAppear(Window *window)
{
    MenuAppear(window);
    ShowMainWindowRow(0, "Progress", "");	
    ShowMainWindowRow(1, "Level", UpdateLevelText());
    ShowMainWindowRow(2, "XP", UpdateXPText());
    ShowMainWindowRow(3, "Next XP", UpdateNextXPText());
    ShowMainWindowRow(4, "Gold", UpdateGoldText());
    ShowMainWindowRow(5, "Escapes", UpdateEscapeText());
}

void ShowProgressMenu(void)
{
    PushNewMenu(&progressMenuDef);
}
