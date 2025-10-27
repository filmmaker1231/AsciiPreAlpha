#pragma once
#include <string>
#include <vector>
#include <queue>
#include <SDL.h>
#include <SDL_ttf.h>




class CellGrid; // Forward declaration
struct TTF_Font;

class Food {
public:
	std::string type;   // type of food
    int x, y;           // Position on the grid
    char symbol;        // Character to display (e.g., '@')
    int foodValue;
    int foodId;
    int carriedByUnitId; // -1 if not carried, otherwise the unit ID carrying it
    int ownedByHouseId;  // -1 if not owned, otherwise the house owner's unit ID
    
     Food(int x, int y, char symbol, const std::string& type, int foodValue = 100, int foodId = 0)
		 : x(x), y(y), symbol(symbol), type(type), foodValue(foodValue), foodId(foodId), 
		   carriedByUnitId(-1), ownedByHouseId(-1) {
	 }
};

class FoodManager {
private:
    std::vector<Food> food;
    TTF_Font* font;

public:
    FoodManager();
    ~FoodManager();

    // Initialize font for rendering
    bool initializeFont(const char* fontPath, int fontSize);

    // Spawn food with f symbol at given position
    void spawnFood(int x, int y, const std::string& type);

    // Delete food at given pixel position (returns true if food was deleted)
    bool deleteFoodAt(int x, int y);

    // Render all units
    void renderFood(SDL_Renderer* renderer);

    
    std::vector<Food>& getFood();
    const std::vector<Food>& getFood() const;

	TTF_Font* getFont() const { return font; }

};      // Manages food

class Seed {
public:
	std::string type;   // type of seed
    int x, y;           // Position on the grid
    char symbol;        // Character to display ('.')
    int seedId;
    int carriedByUnitId; // -1 if not carried, otherwise the unit ID carrying it
    int ownedByHouseId;  // -1 if not owned, otherwise the house owner's unit ID
    
    Seed(int x, int y, const std::string& type, int seedId)
		: x(x), y(y), symbol('.'), type(type), seedId(seedId), 
		  carriedByUnitId(-1), ownedByHouseId(-1) {
	}
};

class SeedManager {
private:
    std::vector<Seed> seeds;
    TTF_Font* font;

public:
    SeedManager();
    ~SeedManager();

    // Initialize font for rendering
    bool initializeFont(const char* fontPath, int fontSize);

    // Spawn seed with . symbol at given position
    void spawnSeed(int x, int y, const std::string& type);

    // Render all seeds
    void renderSeeds(SDL_Renderer* renderer);

    
    std::vector<Seed>& getSeeds();
    const std::vector<Seed>& getSeeds() const;

};      // Manages seeds

class Coin {
public:
	std::string type;   // type of coin
    int x, y;           // Position on the grid
    char symbol;        // Character to display ('$')
    bool fromMarketSale = false; 
    int coinId;
    int carriedByUnitId; // -1 if not carried, otherwise the unit ID carrying it
    int ownedByHouseId;  // -1 if not owned, otherwise the house owner's unit ID
    
    Coin(int x, int y, int coinId)
		: x(x), y(y), symbol('$'), type("coin"), coinId(coinId), 
		  carriedByUnitId(-1), ownedByHouseId(-1) {
	}
};

class CoinManager {
private:
    std::vector<Coin> coins;
    TTF_Font* font;

public:
    CoinManager();
    ~CoinManager();

    // Initialize font for rendering
    bool initializeFont(const char* fontPath, int fontSize);

    // Spawn coin with $ symbol at given position
    void spawnCoin(int x, int y);

    // Render all coins
    void renderCoins(SDL_Renderer* renderer);

    
    std::vector<Coin>& getCoins();
    const std::vector<Coin>& getCoins() const;

};      // Manages coins

// --- New Items for Firemaking, Clay Crafting, and Kiln System ---

class Stick {
public:
    int x, y;           // Position on the grid
    char symbol;        // Character to display ('-')
    int stickId;
    
    Stick(int x, int y, int stickId)
        : x(x), y(y), symbol('-'), stickId(stickId) {
    }
};

class StickManager {
private:
    std::vector<Stick> sticks;
    TTF_Font* font;

public:
    StickManager();
    ~StickManager();

    bool initializeFont(const char* fontPath, int fontSize);
    void spawnStick(int x, int y);
    void renderSticks(SDL_Renderer* renderer);
    std::vector<Stick>& getSticks();
    const std::vector<Stick>& getSticks() const;
};

class Firesticks {
public:
    int x, y;           // Position on the grid
    char symbol;        // Character to display ('-')
    int firesticksId;
    int carriedByUnitId; // -1 if not carried, otherwise the unit ID carrying it
    
    Firesticks(int x, int y, int firesticksId)
        : x(x), y(y), symbol('-'), firesticksId(firesticksId), carriedByUnitId(-1) {
    }
};

class FiresticksManager {
private:
    std::vector<Firesticks> firesticks;
    TTF_Font* font;

public:
    FiresticksManager();
    ~FiresticksManager();

    bool initializeFont(const char* fontPath, int fontSize);
    void spawnFiresticks(int x, int y);
    void renderFiresticks(SDL_Renderer* renderer);
    std::vector<Firesticks>& getFiresticks();
    const std::vector<Firesticks>& getFiresticks() const;
};

class Clay {
public:
    int x, y;           // Position on the grid
    char symbol;        // Character to display ('o')
    int clayId;
    int carriedByUnitId; // -1 if not carried, otherwise the unit ID carrying it
    
    Clay(int x, int y, int clayId)
        : x(x), y(y), symbol('o'), clayId(clayId), carriedByUnitId(-1) {
    }
};

class ClayManager {
private:
    std::vector<Clay> clays;
    TTF_Font* font;

public:
    ClayManager();
    ~ClayManager();

    bool initializeFont(const char* fontPath, int fontSize);
    void spawnClay(int x, int y);
    void renderClays(SDL_Renderer* renderer);
    std::vector<Clay>& getClays();
    const std::vector<Clay>& getClays() const;
};

class ShapedClay {
public:
    int x, y;           // Position on the grid
    char symbol;        // Character to display ('O')
    int shapedClayId;
    Uint32 creationTime; // Time when shaped clay was created (for auto-transform to brick)
    int carriedByUnitId; // -1 if not carried, otherwise the unit ID carrying it
    
    ShapedClay(int x, int y, int shapedClayId, Uint32 creationTime)
        : x(x), y(y), symbol('O'), shapedClayId(shapedClayId), 
          creationTime(creationTime), carriedByUnitId(-1) {
    }
};

class ShapedClayManager {
private:
    std::vector<ShapedClay> shapedClays;
    TTF_Font* font;

public:
    ShapedClayManager();
    ~ShapedClayManager();

    bool initializeFont(const char* fontPath, int fontSize);
    void spawnShapedClay(int x, int y);
    void renderShapedClays(SDL_Renderer* renderer);
    std::vector<ShapedClay>& getShapedClays();
    const std::vector<ShapedClay>& getShapedClays() const;
};

class Brick {
public:
    int x, y;           // Position on the grid
    char symbol;        // Character to display ('B')
    int brickId;
    int carriedByUnitId; // -1 if not carried, otherwise the unit ID carrying it
    
    Brick(int x, int y, int brickId)
        : x(x), y(y), symbol('B'), brickId(brickId), carriedByUnitId(-1) {
    }
};

class BrickManager {
private:
    std::vector<Brick> bricks;
    TTF_Font* font;

public:
    BrickManager();
    ~BrickManager();

    bool initializeFont(const char* fontPath, int fontSize);
    void spawnBrick(int x, int y);
    void renderBricks(SDL_Renderer* renderer);
    std::vector<Brick>& getBricks();
    const std::vector<Brick>& getBricks() const;
};

class DryGrass {
public:
    int x, y;           // Position on the grid
    char symbol;        // Character to display ('*')
    int dryGrassId;
    int carriedByUnitId; // -1 if not carried, otherwise the unit ID carrying it
    
    DryGrass(int x, int y, int dryGrassId)
        : x(x), y(y), symbol('*'), dryGrassId(dryGrassId), carriedByUnitId(-1) {
    }
};

class DryGrassManager {
private:
    std::vector<DryGrass> dryGrasses;
    TTF_Font* font;

public:
    DryGrassManager();
    ~DryGrassManager();

    bool initializeFont(const char* fontPath, int fontSize);
    void spawnDryGrass(int x, int y);
    void renderDryGrasses(SDL_Renderer* renderer);
    std::vector<DryGrass>& getDryGrasses();
    const std::vector<DryGrass>& getDryGrasses() const;
};

class PiggyBank {
public:
    int x, y;           // Position on the grid
    char symbol;        // Character to display ('p')
    int piggyBankId;
    int carriedByUnitId; // -1 if not carried, otherwise the unit ID carrying it
    int ownedByHouseId;  // -1 if not owned, otherwise the house owner's unit ID
    
    PiggyBank(int x, int y, int piggyBankId)
        : x(x), y(y), symbol('p'), piggyBankId(piggyBankId), 
          carriedByUnitId(-1), ownedByHouseId(-1) {
    }
};

class PiggyBankManager {
private:
    std::vector<PiggyBank> piggyBanks;
    TTF_Font* font;

public:
    PiggyBankManager();
    ~PiggyBankManager();

    bool initializeFont(const char* fontPath, int fontSize);
    void spawnPiggyBank(int x, int y);
    void renderPiggyBanks(SDL_Renderer* renderer);
    std::vector<PiggyBank>& getPiggyBanks();
    const std::vector<PiggyBank>& getPiggyBanks() const;
};

class UnfinishedKiln {
public:
    int x, y;           // Position on the grid (stores top-left position)
    std::string symbol; // Display as "uk"
    int unfinishedKilnId;
    bool hasFiresticks;  // Whether firesticks have been brought
    bool hasDryGrass;    // Whether dry grass has been brought
    
    UnfinishedKiln(int x, int y, int unfinishedKilnId)
        : x(x), y(y), symbol("uk"), unfinishedKilnId(unfinishedKilnId),
          hasFiresticks(false), hasDryGrass(false) {
    }
};

class UnfinishedKilnManager {
private:
    std::vector<UnfinishedKiln> unfinishedKilns;
    TTF_Font* font;

public:
    UnfinishedKilnManager();
    ~UnfinishedKilnManager();

    bool initializeFont(const char* fontPath, int fontSize);
    void spawnUnfinishedKiln(int x, int y);
    void renderUnfinishedKilns(SDL_Renderer* renderer);
    std::vector<UnfinishedKiln>& getUnfinishedKilns();
    const std::vector<UnfinishedKiln>& getUnfinishedKilns() const;
};

class Kiln {
public:
    int x, y;           // Position on the grid
    char symbol;        // Character to display ('k')
    int kilnId;
    
    Kiln(int x, int y, int kilnId)
        : x(x), y(y), symbol('k'), kilnId(kilnId) {
    }
};

class KilnManager {
private:
    std::vector<Kiln> kilns;
    TTF_Font* font;

public:
    KilnManager();
    ~KilnManager();

    bool initializeFont(const char* fontPath, int fontSize);
    void spawnKiln(int x, int y);
    void renderKilns(SDL_Renderer* renderer);
    std::vector<Kiln>& getKilns();
    const std::vector<Kiln>& getKilns() const;
};

