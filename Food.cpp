#include "Food.h"
#include <iostream>
#include "CellGrid.h"

FoodManager::FoodManager() : font(nullptr) {
}

FoodManager::~FoodManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool FoodManager::initializeFont(const char* fontPath, int fontSize) {
    // Try provided font path first (skip if nullptr or empty)
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) {
            return true;
        }
    }
    
    // Try to load a default system font
    // On Windows, try Arial
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        // On Linux, try DejaVu Sans
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void FoodManager::spawnFood(int x, int y, const std::string& type) {
    static int nextFoodId = 1; // Static to ensure unique IDs
    food.emplace_back(x, y, 'f', type, 100, nextFoodId++);
    std::cout << "Spawned food '" << type << "' at (" << x << ", " << y << ") with id " << (nextFoodId-1) << std::endl;
}

bool FoodManager::deleteFoodAt(int x, int y) {
    // Use a larger click area to make it easier to select food
    const int clickRadius = 20;
    
    for (auto it = food.begin(); it != food.end(); ++it) {
        // Check if click is within the food's bounding box
        if (x >= it->x - clickRadius && x <= it->x + clickRadius &&
            y >= it->y - clickRadius && y <= it->y + clickRadius) {
            std::cout << "Deleted food '" << it->type << "' (id " << it->foodId << ") at (" << it->x << ", " << it->y << ")" << std::endl;
            food.erase(it);
            return true;
        }
    }
    return false;
}

void FoodManager::renderFood(SDL_Renderer* renderer) {
    if (!font) {
        return;
    }
    
    SDL_Color color = {255, 255, 0, 255}; // Yellow color
    
    for (const auto& foodItem : food) {
        // Create surface with the food symbol
        std::string symbolStr(1, foodItem.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) {
            continue;
        }
        
        // Create texture from surface
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {foodItem.x, foodItem.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        
        SDL_FreeSurface(surface);
    }
}

std::vector<Food>& FoodManager::getFood() {
    return food;
}

const std::vector<Food>& FoodManager::getFood() const {
    return food;
}

SeedManager::SeedManager() : font(nullptr) {
}

SeedManager::~SeedManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool SeedManager::initializeFont(const char* fontPath, int fontSize) {
    // Try provided font path first (skip if nullptr or empty)
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) {
            return true;
        }
    }
    
    // Try to load a default system font
    // On Windows, try Arial
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        // On Linux, try DejaVu Sans
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void SeedManager::spawnSeed(int x, int y, const std::string& type) {
    static int nextSeedId = 1; // Static to ensure unique IDs
    seeds.emplace_back(x, y, type, nextSeedId++);
    std::cout << "Spawned seed '" << type << "' at (" << x << ", " << y << ") with id " << (nextSeedId-1) << std::endl;
}

void SeedManager::renderSeeds(SDL_Renderer* renderer) {
    if (!font) {
        return;
    }
    
    SDL_Color color = {255, 255, 255, 255}; // White color for seeds (high visibility)
    
    for (const auto& seedItem : seeds) {
        // Create surface with the seed symbol
        std::string symbolStr(1, seedItem.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) {
            continue;
        }
        
        // Create texture from surface
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {seedItem.x, seedItem.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        
        SDL_FreeSurface(surface);
    }
}

std::vector<Seed>& SeedManager::getSeeds() {
    return seeds;
}

const std::vector<Seed>& SeedManager::getSeeds() const {
    return seeds;
}

CoinManager::CoinManager() : font(nullptr) {
}

CoinManager::~CoinManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool CoinManager::initializeFont(const char* fontPath, int fontSize) {
    // Try provided font path first (skip if nullptr or empty)
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) {
            return true;
        }
    }
    
    // Try to load a default system font
    // On Windows, try Arial
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        // On Linux, try DejaVu Sans
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void CoinManager::spawnCoin(int x, int y) {
    static int nextCoinId = 1; // Static to ensure unique IDs
    coins.emplace_back(x, y, nextCoinId++);
    std::cout << "Spawned coin at (" << x << ", " << y << ") with id " << (nextCoinId-1) << std::endl;
}

void CoinManager::renderCoins(SDL_Renderer* renderer) {
    if (!font) {
        return;
    }
    
    SDL_Color color = {255, 215, 0, 255}; // Gold color
    
    for (const auto& coinItem : coins) {
        // Render all coins - carried coins move with units via Unit::processAction
        // Coins in houses and at stalls are positioned at their storage locations
        
        // Create surface with the coin symbol
        std::string symbolStr(1, coinItem.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) {
            continue;
        }
        
        // Create texture from surface
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {coinItem.x, coinItem.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        
        SDL_FreeSurface(surface);
    }
}

std::vector<Coin>& CoinManager::getCoins() {
    return coins;
}

const std::vector<Coin>& CoinManager::getCoins() const {
    return coins;
}

// --- Stick Manager Implementation ---
StickManager::StickManager() : font(nullptr) {}

StickManager::~StickManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool StickManager::initializeFont(const char* fontPath, int fontSize) {
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) return true;
    }
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void StickManager::spawnStick(int x, int y) {
    static int nextStickId = 1;
    sticks.emplace_back(x, y, nextStickId++);
    std::cout << "Spawned stick at (" << x << ", " << y << ") with id " << (nextStickId-1) << std::endl;
}

void StickManager::renderSticks(SDL_Renderer* renderer) {
    if (!font) return;
    SDL_Color color = {101, 67, 33, 255}; // Dark brown
    for (const auto& stick : sticks) {
        std::string symbolStr(1, stick.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) continue;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {stick.x, stick.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

std::vector<Stick>& StickManager::getSticks() { return sticks; }
const std::vector<Stick>& StickManager::getSticks() const { return sticks; }

// --- Firesticks Manager Implementation ---
FiresticksManager::FiresticksManager() : font(nullptr) {}

FiresticksManager::~FiresticksManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool FiresticksManager::initializeFont(const char* fontPath, int fontSize) {
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) return true;
    }
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void FiresticksManager::spawnFiresticks(int x, int y) {
    static int nextFiresticksId = 1;
    firesticks.emplace_back(x, y, nextFiresticksId++);
    std::cout << "Spawned firesticks at (" << x << ", " << y << ") with id " << (nextFiresticksId-1) << std::endl;
}

void FiresticksManager::renderFiresticks(SDL_Renderer* renderer) {
    if (!font) return;
    SDL_Color color = {255, 165, 0, 255}; // Orange
    for (const auto& fs : firesticks) {
        std::string symbolStr(1, fs.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) continue;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {fs.x, fs.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

std::vector<Firesticks>& FiresticksManager::getFiresticks() { return firesticks; }
const std::vector<Firesticks>& FiresticksManager::getFiresticks() const { return firesticks; }

// --- Clay Manager Implementation ---
ClayManager::ClayManager() : font(nullptr) {}

ClayManager::~ClayManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool ClayManager::initializeFont(const char* fontPath, int fontSize) {
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) return true;
    }
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void ClayManager::spawnClay(int x, int y) {
    static int nextClayId = 1;
    clays.emplace_back(x, y, nextClayId++);
    std::cout << "Spawned clay at (" << x << ", " << y << ") with id " << (nextClayId-1) << std::endl;
}

void ClayManager::renderClays(SDL_Renderer* renderer) {
    if (!font) return;
    SDL_Color color = {139, 69, 19, 255}; // Brown
    for (const auto& clay : clays) {
        std::string symbolStr(1, clay.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) continue;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {clay.x, clay.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

std::vector<Clay>& ClayManager::getClays() { return clays; }
const std::vector<Clay>& ClayManager::getClays() const { return clays; }

// --- ShapedClay Manager Implementation ---
ShapedClayManager::ShapedClayManager() : font(nullptr) {}

ShapedClayManager::~ShapedClayManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool ShapedClayManager::initializeFont(const char* fontPath, int fontSize) {
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) return true;
    }
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void ShapedClayManager::spawnShapedClay(int x, int y) {
    static int nextShapedClayId = 1;
    Uint32 currentTime = SDL_GetTicks();
    shapedClays.emplace_back(x, y, nextShapedClayId++, currentTime);
    std::cout << "Spawned shaped clay at (" << x << ", " << y << ") with id " << (nextShapedClayId-1) << std::endl;
}

void ShapedClayManager::renderShapedClays(SDL_Renderer* renderer) {
    if (!font) return;
    SDL_Color color = {139, 69, 19, 255}; // Brown
    for (const auto& sc : shapedClays) {
        std::string symbolStr(1, sc.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) continue;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {sc.x, sc.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

std::vector<ShapedClay>& ShapedClayManager::getShapedClays() { return shapedClays; }
const std::vector<ShapedClay>& ShapedClayManager::getShapedClays() const { return shapedClays; }

// --- Brick Manager Implementation ---
BrickManager::BrickManager() : font(nullptr) {}

BrickManager::~BrickManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool BrickManager::initializeFont(const char* fontPath, int fontSize) {
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) return true;
    }
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void BrickManager::spawnBrick(int x, int y) {
    static int nextBrickId = 1;
    bricks.emplace_back(x, y, nextBrickId++);
    std::cout << "Spawned brick at (" << x << ", " << y << ") with id " << (nextBrickId-1) << std::endl;
}

void BrickManager::renderBricks(SDL_Renderer* renderer) {
    if (!font) return;
    SDL_Color color = {178, 34, 34, 255}; // Reddish-brown (Firebrick)
    for (const auto& brick : bricks) {
        std::string symbolStr(1, brick.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) continue;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {brick.x, brick.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

std::vector<Brick>& BrickManager::getBricks() { return bricks; }
const std::vector<Brick>& BrickManager::getBricks() const { return bricks; }

// --- DryGrass Manager Implementation ---
DryGrassManager::DryGrassManager() : font(nullptr) {}

DryGrassManager::~DryGrassManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool DryGrassManager::initializeFont(const char* fontPath, int fontSize) {
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) return true;
    }
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void DryGrassManager::spawnDryGrass(int x, int y) {
    static int nextDryGrassId = 1;
    dryGrasses.emplace_back(x, y, nextDryGrassId++);
    std::cout << "Spawned dry grass at (" << x << ", " << y << ") with id " << (nextDryGrassId-1) << std::endl;
}

void DryGrassManager::renderDryGrasses(SDL_Renderer* renderer) {
    if (!font) return;
    SDL_Color color = {154, 205, 50, 255}; // Yellow-green (YellowGreen)
    for (const auto& grass : dryGrasses) {
        std::string symbolStr(1, grass.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) continue;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {grass.x, grass.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

std::vector<DryGrass>& DryGrassManager::getDryGrasses() { return dryGrasses; }
const std::vector<DryGrass>& DryGrassManager::getDryGrasses() const { return dryGrasses; }

// --- PiggyBank Manager Implementation ---
PiggyBankManager::PiggyBankManager() : font(nullptr) {}

PiggyBankManager::~PiggyBankManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool PiggyBankManager::initializeFont(const char* fontPath, int fontSize) {
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) return true;
    }
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void PiggyBankManager::spawnPiggyBank(int x, int y) {
    static int nextPiggyBankId = 1;
    piggyBanks.emplace_back(x, y, nextPiggyBankId++);
    std::cout << "Spawned piggy bank at (" << x << ", " << y << ") with id " << (nextPiggyBankId-1) << std::endl;
}

void PiggyBankManager::renderPiggyBanks(SDL_Renderer* renderer) {
    if (!font) return;
    SDL_Color color = {255, 192, 203, 255}; // Pink
    for (const auto& pb : piggyBanks) {
        std::string symbolStr(1, pb.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) continue;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {pb.x, pb.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

std::vector<PiggyBank>& PiggyBankManager::getPiggyBanks() { return piggyBanks; }
const std::vector<PiggyBank>& PiggyBankManager::getPiggyBanks() const { return piggyBanks; }

// --- UnfinishedKiln Manager Implementation ---
UnfinishedKilnManager::UnfinishedKilnManager() : font(nullptr) {}

UnfinishedKilnManager::~UnfinishedKilnManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool UnfinishedKilnManager::initializeFont(const char* fontPath, int fontSize) {
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) return true;
    }
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void UnfinishedKilnManager::spawnUnfinishedKiln(int x, int y) {
    static int nextUnfinishedKilnId = 1;
    unfinishedKilns.emplace_back(x, y, nextUnfinishedKilnId++);
    std::cout << "Spawned unfinished kiln at (" << x << ", " << y << ") with id " << (nextUnfinishedKilnId-1) << std::endl;
}

void UnfinishedKilnManager::renderUnfinishedKilns(SDL_Renderer* renderer) {
    if (!font) return;
    SDL_Color color = {139, 0, 0, 255}; // Dark red
    for (const auto& uk : unfinishedKilns) {
        SDL_Surface* surface = TTF_RenderText_Solid(font, uk.symbol.c_str(), color);
        if (!surface) continue;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {uk.x, uk.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

std::vector<UnfinishedKiln>& UnfinishedKilnManager::getUnfinishedKilns() { return unfinishedKilns; }
const std::vector<UnfinishedKiln>& UnfinishedKilnManager::getUnfinishedKilns() const { return unfinishedKilns; }

// --- Kiln Manager Implementation ---
KilnManager::KilnManager() : font(nullptr) {}

KilnManager::~KilnManager() {
    if (font) {
        TTF_CloseFont(font);
    }
}

bool KilnManager::initializeFont(const char* fontPath, int fontSize) {
    if (fontPath && fontPath[0] != '\0') {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) return true;
    }
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", fontSize);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    return true;
}

void KilnManager::spawnKiln(int x, int y) {
    static int nextKilnId = 1;
    kilns.emplace_back(x, y, nextKilnId++);
    std::cout << "Spawned kiln at (" << x << ", " << y << ") with id " << (nextKilnId-1) << std::endl;
}

void KilnManager::renderKilns(SDL_Renderer* renderer) {
    if (!font) return;
    SDL_Color color = {255, 0, 0, 255}; // Red
    for (const auto& kiln : kilns) {
        std::string symbolStr(1, kiln.symbol);
        SDL_Surface* surface = TTF_RenderText_Solid(font, symbolStr.c_str(), color);
        if (!surface) continue;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {kiln.x, kiln.y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

std::vector<Kiln>& KilnManager::getKilns() { return kilns; }
const std::vector<Kiln>& KilnManager::getKilns() const { return kilns; }
