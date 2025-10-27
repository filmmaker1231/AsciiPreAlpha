#pragma once


enum class ActionType {
    Wander,
    Eat,
	BuildHouse,
    BringItemToHouse,
    EatFromHouse,
    CollectSeed,
    CollectCoin,
    BuildFarm,
    PlantSeed,
    HarvestFood,
    StealFood,
    Fight,
    SellAtMarket,
    BuyAtMarket,
    BringCoinToHouse,
    MakeFire,           // Rub two adjacent sticks to make fire
    ShapeClay,          // Shape clay into shaped clay
    CreateBrick,        // Transform shaped clay to brick (passive)
    BuildUnfinishedKiln, // Create unfinished kiln from brick
    BringFiresticksToKiln, // Bring firesticks to unfinished kiln
    BringDryGrassToKiln,   // Bring dry grass to unfinished kiln
    FinishKiln,         // Complete kiln construction
    MakePiggyBank,      // Create piggy bank at kiln from clay
    BringPiggyBankHome, // Bring piggy bank home
    // Add more as needed
};

struct Action { 
    ActionType type;
    int priority;
    std::string itemType;

    Action(ActionType type, int priority, const std::string& itemType = "")
        : type(type), priority(priority), itemType(itemType) {}
};


struct ActionComparator {
    bool operator()(const Action& a, const Action& b) const {
        return a.priority < b.priority; // higher priority first
    }
};
