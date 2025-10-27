#pragma once


enum class ActionType {
    Wander,
    Eat,
	BuildHouse,
	BringFoodToHouse,
	SellFoodAtMarket,
	BuyFoodAtMarket,
    // Add more as needed
};

struct Action {
    ActionType type;
    int priority;
    // Optionally, add more data (e.g., target position, item, etc.)

    Action(ActionType type, int priority) : type(type), priority(priority) {}

};


struct ActionComparator {
    bool operator()(const Action& a, const Action& b) const {
        return a.priority < b.priority; // higher priority first
    }
};
