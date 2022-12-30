#include "sre/SDLRenderer.hpp"
#include "sre/SpriteAtlas.hpp"
#include <vector>
#include "Box2D/Dynamics/b2World.h"
#include "GameObject.hpp"
#include "components/ScrollingCamera.hpp"
#include "Background.hpp"
#include "Box2DDebugDraw.hpp"
#include <random>
#include "components/PowerupComponent.hpp"
#include <deque>
#include "FrameTiming.hpp"

class PhysicsComponent;

enum class GameState
{
    Ready,
    Running,
    GameOver,
    GameWon
};

class CarGame : public b2ContactListener
{
public:
    CarGame();

    std::shared_ptr<GameObject> createGameObject();
    static const glm::vec2 windowSize;

    void BeginContact(b2Contact *contact) override;

    void EndContact(b2Contact *contact) override;

    static CarGame *instance;

    void setGameState(GameState newState);
    std::string carName = "Car";

private:
    sre::SDLRenderer r;

    void init();
    void initPhysics();

    void update(float time);

    void render();

    void onKey(SDL_Event &event);

    void handleContact(b2Contact *contact, bool begin);

    void spawnBuilding(glm::vec2 position);
    void spawnEnemy(glm::vec2 position = glm::vec2(0, 0));
    void spawnNPC(glm::vec2 position = glm::vec2(0, 0));
    void spawnPowerup(PowerupType type, glm::vec2 position = glm::vec2(0, 0));
    void spawnExplosion(glm::vec2 position);
    void handleSpawning(float time);
    void drawCarDebugMenu();
    void drawFrameTimingDebug();
    glm::vec2 getRandomSpawnPosition();

    std::shared_ptr<ScrollingCamera> camera;
    std::shared_ptr<GameObject> carObj;
    std::shared_ptr<sre::SpriteAtlas> spriteAtlas;
    std::vector<sre::Sprite> explosionSprites;
    std::vector<sre::Sprite> enemySprites;
    std::deque<FrameTiming> frameTimingHistory;
    int maxFrameHistory = 5000;

    std::uniform_real_distribution<double> ran;
    std::mt19937 gen;

    std::vector<std::shared_ptr<GameObject>> sceneObjects;
    Background background;

    void updatePhysics();
    b2World *world = nullptr;
    float enemySpawnTimer = 0;
    float NPCSpawnTimer = 0;
    const float physicsScale = 10;
    void registerPhysicsComponent(PhysicsComponent *r);
    void deregisterPhysicsComponent(PhysicsComponent *r);
    std::map<b2Fixture *, PhysicsComponent *> physicsComponentLookup;
    Box2DDebugDraw debugDraw;
    bool doDebugDraw = false;
    GameState gameState = GameState::Running;
    friend class PhysicsComponent;
    int winningScore = 100;
    int currentScore = 0;
    bool mute = false;
};
