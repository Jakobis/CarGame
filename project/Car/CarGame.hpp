#include "sre/SDLRenderer.hpp"
#include "sre/SpriteAtlas.hpp"
#include <vector>
#include "Box2D/Dynamics/b2World.h"
#include "GameObject.hpp"
#include "SideScrollingCamera.hpp"
#include "BackgroundComponent.hpp"
#include "Box2DDebugDraw.hpp"
#include <random>

class PhysicsComponent;

enum class GameState
{
    Ready,
    Running,
    GameOver
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

    void spawnEnemy(glm::vec2 position = glm::vec2(0, 0));

    std::shared_ptr<SideScrollingCamera> camera;
    std::shared_ptr<GameObject> carObj;
    std::shared_ptr<sre::SpriteAtlas> spriteAtlas;

    std::uniform_real_distribution<double> ran;
    std::mt19937 gen;

    std::vector<std::shared_ptr<GameObject>> sceneObjects;
    BackgroundComponent backgroundComponent;

    void updatePhysics();
    b2World *world = nullptr;
    const float physicsScale = 10;
    void registerPhysicsComponent(PhysicsComponent *r);
    void deregisterPhysicsComponent(PhysicsComponent *r);
    std::map<b2Fixture *, PhysicsComponent *> physicsComponentLookup;
    Box2DDebugDraw debugDraw;
    bool doDebugDraw = false;
    GameState gameState = GameState::Running;
    friend class PhysicsComponent;
};
