#pragma once


enum class ActionType {
    Wander,
    Eat,
	BuildHouse,
    BringItemToHouse,
    EatFromHouse,
    CollectSeed,
    BuildFarm,
    PlantSeed,
    HarvestFood,
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
