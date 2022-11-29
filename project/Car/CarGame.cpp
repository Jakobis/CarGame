#include <sre/Inspector.hpp>
#include <iostream>
#include "CarGame.hpp"
#include "GameObject.hpp"
#include "sre/RenderPass.hpp"
#include "SpriteComponent.hpp"
#include "SpriteAnimationComponent.hpp"
#include "Box2D/Dynamics/Contacts/b2Contact.h"
#include "PhysicsComponent.hpp"
#include "Car.hpp"
#include "EnemyComponent.hpp"

using namespace std;
using namespace sre;

const glm::vec2 CarGame::windowSize(1200, 800);

CarGame *CarGame::instance = nullptr;

CarGame::CarGame()
    : debugDraw(physicsScale)
{
    instance = this;
    r.setWindowSize(windowSize);
    r.init()
        .withSdlInitFlags(SDL_INIT_EVERYTHING)
        .withSdlWindowFlags(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    init();

    // setup callback functions
    r.keyEvent = [&](SDL_Event &e)
    {
        onKey(e);
    };
    r.frameUpdate = [&](float deltaTime)
    {
        update(deltaTime);
    };
    r.frameRender = [&]()
    {
        render();
    };
    // start game loop
    r.startEventLoop();
}

void CarGame::init()
{
    if (world != nullptr)
    { // deregister call backlistener to avoid getting callbacks when recreating the world
        world->SetContactListener(nullptr);
    }
    camera.reset();
    sceneObjects.clear();
    camera.reset();
    physicsComponentLookup.clear();
    initPhysics();
    auto camObj = createGameObject();
    camObj->name = "Camera";
    camera = camObj->addComponent<SideScrollingCamera>();
    camObj->setPosition(windowSize * 0.5f);

    spriteAtlas = SpriteAtlas::create("car.json", "car.png");

    auto carObj = createGameObject();
    carObj->name = carName;
    camera->setFollowObject(carObj, {0, 0});
    auto so = carObj->addComponent<SpriteComponent>();
    auto sprite = spriteAtlas->get("Truck.png");
    sprite.setScale({2, 2});
    carObj->setPosition({-100, 200});
    so->setSprite(sprite);

    auto carComp = carObj->addComponent<Car>();
    sprite = spriteAtlas->get("Tire.png");
    sprite.setScale({2, 2});
    carComp->initTires(&sprite);

    // Absolute hack -- Sort the car and tires by name, s.t. car is drawn over tires
    std::sort(
        sceneObjects.begin(),
        sceneObjects.end(),
        [](std::shared_ptr<GameObject> a, std::shared_ptr<GameObject> b)
        {
            return a->name > b->name;
        });

    // vector<Sprite> spriteAnim({spriteAtlas->get("bird1.png"), spriteAtlas->get("bird2.png"), spriteAtlas->get("bird3.png"), spriteAtlas->get("bird2.png")});
    // for (auto &s : spriteAnim)
    // {
    //     s.setScale({2, 2});
    // }
    // anim->setSprites(spriteAnim);

    backgroundComponent.init(spriteAtlas->get("asphalt.png"));

    auto enemyObj = createGameObject();
    enemyObj->name = "Enemy";
    auto enemySpriteComponent = enemyObj->addComponent<SpriteComponent>();
    auto enemySprite = spriteAtlas->get("Truck.png");
    enemySprite.setScale({2, 2});
    enemyObj->setPosition({200, 200});
    enemySpriteComponent->setSprite(enemySprite);
    auto enemyComp = enemyObj->addComponent<EnemyComponent>();
    enemyComp->init(carObj);
}

void CarGame::update(float time)
{
    if (gameState == GameState::Running)
    {
        updatePhysics();
    }
    for (int i = 0; i < sceneObjects.size(); i++)
    {
        sceneObjects[i]->update(time);
    }
}

void CarGame::render()
{
    auto rp = RenderPass::create()
                  .withCamera(camera->getCamera())
                  .build();

    auto pos = camera->getGameObject()->getPosition();
    backgroundComponent.renderBackground(rp, pos.x, pos.y);

    auto spriteBatchBuilder = SpriteBatch::create();
    for (auto &go : sceneObjects)
    {
        go->renderSprite(spriteBatchBuilder);
    }

    if (gameState == GameState::Ready)
    {
        // auto sprite = spriteAtlas->get("get-ready.png");
        // sprite.setPosition(pos);
        // spriteBatchBuilder.addSprite(sprite);
    }
    else if (gameState == GameState::GameOver)
    {
        // auto sprite = spriteAtlas->get("game-over.png");
        // sprite.setPosition(pos);
        // spriteBatchBuilder.addSprite(sprite);
    }

    auto sb = spriteBatchBuilder.build();
    rp.draw(sb);

    if (doDebugDraw)
    {
        world->DrawDebugData();
        rp.drawLines(debugDraw.getLines());
        // rp.drawLines({{0, 0, 0}, {1000, 1000, 0}}, {1, 0, 0, 1});
        debugDraw.clear();
    }
}

void CarGame::onKey(SDL_Event &event)
{
    for (auto &gameObject : sceneObjects)
    {
        for (auto &c : gameObject->getComponents())
        {
            bool consumed = c->onKey(event);
            if (consumed)
            {
                return;
            }
        }
    }

    if (event.type == SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_ESCAPE:
            // press 'd' for physics debug
            doDebugDraw = !doDebugDraw;
            if (doDebugDraw)
            {
                world->SetDebugDraw(&debugDraw);
            }
            else
            {
                world->SetDebugDraw(nullptr);
            }
            break;
        case SDLK_r:
            init();
            break;
        case SDLK_SPACE:
            if (gameState == GameState::GameOver)
            {
                init();
                gameState = GameState::Ready;
            }
            else if (gameState == GameState::Ready)
            {
                gameState = GameState::Running;
            }
            break;
        }
    }
}

std::shared_ptr<GameObject> CarGame::createGameObject()
{
    auto obj = shared_ptr<GameObject>(new GameObject());
    sceneObjects.push_back(obj);
    return obj;
}

void CarGame::updatePhysics()
{
    const float32 timeStep = 1.0f / 60.0f;
    const int positionIterations = 2;
    const int velocityIterations = 6;
    world->Step(timeStep, velocityIterations, positionIterations);

    for (auto phys : physicsComponentLookup)
    {
        if (phys.second->rbType == b2_staticBody)
            continue;
        auto position = phys.second->body->GetPosition();
        float angle = phys.second->body->GetAngle();
        auto gameObject = phys.second->getGameObject();
        gameObject->setPosition(glm::vec2(position.x * physicsScale, position.y * physicsScale));
        gameObject->setRotation(glm::degrees(angle));
    }
}

void CarGame::initPhysics()
{
    float gravity = 0; // 9.8 m/s2
    delete world;
    world = new b2World(b2Vec2(0, gravity));
    world->SetContactListener(this);

    if (doDebugDraw)
    {
        world->SetDebugDraw(&debugDraw);
    }
}

void CarGame::BeginContact(b2Contact *contact)
{
    b2ContactListener::BeginContact(contact);
    handleContact(contact, true);
}

void CarGame::EndContact(b2Contact *contact)
{
    b2ContactListener::EndContact(contact);
    handleContact(contact, false);
}

void CarGame::deregisterPhysicsComponent(PhysicsComponent *r)
{
    auto iter = physicsComponentLookup.find(r->fixture);
    if (iter != physicsComponentLookup.end())
    {
        physicsComponentLookup.erase(iter);
    }
    else
    {
        assert(false); // cannot find physics object
    }
}

void CarGame::registerPhysicsComponent(PhysicsComponent *r)
{
    physicsComponentLookup[r->fixture] = r;
}

void CarGame::handleContact(b2Contact *contact, bool begin)
{
    auto fixA = contact->GetFixtureA();
    auto fixB = contact->GetFixtureB();
    auto physA = physicsComponentLookup.find(fixA);
    auto physB = physicsComponentLookup.find(fixB);
    if (physA != physicsComponentLookup.end() && physB != physicsComponentLookup.end())
    {
        auto &aComponents = physA->second->getGameObject()->getComponents();
        auto &bComponents = physB->second->getGameObject()->getComponents();
        for (auto &c : aComponents)
        {
            if (begin)
            {
                c->onCollisionStart(physB->second);
            }
            else
            {
                c->onCollisionEnd(physB->second);
            }
        }
        for (auto &c : bComponents)
        {
            if (begin)
            {
                c->onCollisionStart(physA->second);
            }
            else
            {
                c->onCollisionEnd(physA->second);
            }
        }
    }
}

void CarGame::setGameState(GameState newState)
{
    this->gameState = newState;
}
