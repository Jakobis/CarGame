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
#include "KillableComponent.hpp"
#include <random>
#include "glm/gtc/matrix_transform.hpp"
#include "SDL_mixer.h"

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

    // load high resolution font into imgui
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("assets/Roboto-Medium.ttf", 36.0f);

    ImGuiStyle &style = ImGui::GetStyle();
    style.Alpha = 1;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);
    style.Colors[ImGuiCol_Text] = ImVec4(0, 0, 0, 1);
    style.Colors[ImGuiCol_Border] = ImVec4(0, 0, 0, 0);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.8, 0.1, 0.1, 1);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.1, 0.8, 0.1, 1);

    std::random_device rd;
    gen = std::mt19937(rd());
    ran = std::uniform_real_distribution<double>(0, 1);

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
    carObj = nullptr;
    sceneObjects.clear();
    camera.reset();
    physicsComponentLookup.clear();
    initPhysics();
    auto camObj = createGameObject();
    camObj->name = "Camera";
    camera = camObj->addComponent<SideScrollingCamera>();
    camObj->setPosition(windowSize * 0.5f);
    Mix_OpenAudio(
        22050,              // int frequency
        MIX_DEFAULT_FORMAT, // Uint16 format
        2,                  // int channels
        2048                // int buffer size (controls the latency, 2048 good default)
    );
    Mix_Music *music = Mix_LoadMUS("./assets/music.mp3");
    Mix_PlayMusic(music, -1);

    spriteAtlas = SpriteAtlas::create("car.json", "car.png");
    explosionSprites = std::vector<sre::Sprite>();
    for (int i = 0; i < 16; i++)
    {
        auto name = "explosion/tile0" + std::to_string(i) + ".png";
        auto sprite = spriteAtlas->get(name);
        sprite.setScale({2, 2});
        explosionSprites.push_back(sprite);
    }
    enemySprites = std::vector<sre::Sprite>();
    enemySprites.push_back(spriteAtlas->get("enemy1.png"));
    enemySprites.push_back(spriteAtlas->get("enemy2.png"));
    for (int i = 0; i < 2; i++)
    {
        enemySprites.at(i).setScale({2,2});
    }
    

    carObj = createGameObject();
    carObj->name = carName;
    camera->setFollowObject(carObj, {0, 0});
    auto so = carObj->addComponent<SpriteComponent>();
    auto sprite = spriteAtlas->get("Truck.png");
    sprite.setScale({2, 2});
    carObj->setPosition({0, 0});
    so->setSprite(sprite);

    auto carComp = carObj->addComponent<Car>();
    sprite = spriteAtlas->get("Tire.png");
    sprite.setScale({2, 2});
    carComp->initTires(&sprite);
    carComp->endGame = [this]{
            setGameState(GameState::GameOver);
        };

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

    int buildingAmount = 10;
    int buildingDistance = 2000;
    int offset = (-buildingAmount / 2) * buildingDistance + (buildingDistance / 2);
    int counter = 0;
    for (int i = 0; i < buildingAmount; i++)
    {
        for (int j = 0; j < buildingAmount; j++)
        {
            glm::vec2 position = {offset + (i * buildingDistance), offset + (j * buildingDistance)};
            spawnBuilding(position);
        }
    }
}

void CarGame::spawnBuilding(glm::vec2 position)
{
    auto buildingObj = createGameObject();
    buildingObj->name = "Building";
    auto buildingSpriteComponent = buildingObj->addComponent<SpriteComponent>();
    auto buildingSprite = spriteAtlas->get("building.jpg");
    float buildingScale = 8;
    buildingSprite.setScale({buildingScale * 2, buildingScale * 2});
    buildingObj->setPosition(position);
    buildingSpriteComponent->setSprite(buildingSprite);
    auto buildingPhys = buildingObj->addComponent<PhysicsComponent>();
    glm::vec2 size = buildingSprite.getSpriteSize();
    float physicsScale = 10;
    buildingPhys->initBox(b2_staticBody, size * buildingScale / physicsScale, position / physicsScale, 5);
}

void CarGame::spawnEnemy(glm::vec2 position)
{
    if (position == glm::vec2(0, 0))
    {
        float distance = 300;
        auto angle = ran(gen) * M_PI * 2;
        position = carObj->getPosition() + (distance * glm::vec2(glm::sin(angle), glm::cos(angle)));
    }

    auto enemyObj = createGameObject();
    enemyObj->name = "Enemy";
    auto enemySpriteComponent = enemyObj->addComponent<SpriteComponent>();
    enemySpriteComponent->setSprite(enemySprites.at(0));
    auto sac = enemyObj->addComponent<SpriteAnimationComponent>();
    sac->setSprites(enemySprites);
    sac->setAnimationTime(0.5);
    auto enemyComp = enemyObj->addComponent<EnemyComponent>();
    enemyObj->setPosition(position);
    enemyComp->init(carObj, position);
    enemyObj->addComponent<KillableComponent>();
}

void CarGame::spawnNPC(glm::vec2 position)
{
    if (position == glm::vec2(0, 0))
    {
        float distance = 30;
        auto angle = ran(gen) * M_PI * 2;
        position = carObj->getPosition() + (distance * glm::vec2(glm::sin(angle), glm::cos(angle)));
    }

    auto NPCObj = createGameObject();
    NPCObj->name = "NPC";
    auto NPCSpriteComponent = NPCObj->addComponent<SpriteComponent>();
    auto NPCSprite = spriteAtlas->get("man.png");
    NPCSprite.setScale({2, 2});
    NPCSpriteComponent->setSprite(NPCSprite);
    NPCObj->setPosition(position);
    auto kc = NPCObj->addComponent<KillableComponent>();
    kc->damageSpeedThreshold = 0;
    auto phys = NPCObj->addComponent<PhysicsComponent>();
    phys->initCircle(b2_dynamicBody, 30 / 10, position, 5);
}

void CarGame::handleSpawning(float time)
{
    float baseSpawnTime = 5;
    enemySpawnTimer -= time;
    if (enemySpawnTimer <= 0)
    {
        float denominator = winningScore - currentScore;
        if (denominator < 1) {
            denominator = 1;
        }
        float divisor = winningScore / baseSpawnTime;
        enemySpawnTimer += denominator / divisor;
        spawnEnemy();
    }
    NPCSpawnTimer -= time;
    if (NPCSpawnTimer <= 0) {

        NPCSpawnTimer += baseSpawnTime;
        spawnNPC();
    }
}

void CarGame::update(float time)
{
    if (gameState == GameState::Running)
    {
        updatePhysics();
        handleSpawning(time);
    }
    for (int i = 0; i < sceneObjects.size(); i++)
    {
        sceneObjects[i]->update(time);
        if (sceneObjects[i]->shouldRemove && sceneObjects[i]->getComponent<KillableComponent>() != nullptr)
        {
            spawnExplosion(sceneObjects[i]->getPosition());
            currentScore += sceneObjects[i]->getComponent<KillableComponent>()->getPointValue();
            if (currentScore >= winningScore) {
                std::cout << "you win" << std::endl;
                gameState = GameState::GameWon;
            }
        }
    }
    if (gameState == GameState::GameWon) 
    {
        for (int i = 0; i < sceneObjects.size(); i++)
    {
        if (sceneObjects[i]->getComponent<KillableComponent>() != nullptr)
        {
            sceneObjects[i]->shouldRemove = true;
            spawnExplosion(sceneObjects[i]->getPosition());
        }
    }
    }
    // remove_if from <algorithm> moves elements to the end of the vector...
    auto it = std::remove_if(
        sceneObjects.begin(),
        sceneObjects.end(),
        [](const std::shared_ptr<GameObject> &obj)
        {
            return obj->shouldRemove;
        });

    // ...so that we may remove them in O(n)

    sceneObjects.erase(it, sceneObjects.end());
}

void CarGame::spawnExplosion(glm::vec2 position)
{
    auto obj = createGameObject();
    obj->setPosition(position);
    auto sc = obj->addComponent<SpriteComponent>();
    sc->setSprite(explosionSprites.at(0));
    auto sac = obj->addComponent<SpriteAnimationComponent>();
    sac->setSprites(explosionSprites);
    sac->setAnimationTime(0.1);
    sac->setRepeating(false);
    Mix_Chunk *my_sound = Mix_LoadWAV("./assets/explosion.wav");
    // Plays an SFX on the desired SFX channel
    Mix_PlayChannel(
        -1,       // int channel to play on (-1 is first available)
        my_sound, // Mix_Chunk* chunk to play
        0         // int number loops
    );
}

void CarGame::render()
{
    auto rp = RenderPass::create()
                  .withCamera(camera->getCamera())
                  .build();

    auto pos = camera->getGameObject()->getPosition();
    backgroundComponent.renderBackground(rp, pos.x, pos.y);

    auto spriteBatchBuilder = SpriteBatch::create();
    int healthCount = 0;
    for (auto &go : sceneObjects)
    {
        go->renderSprite(spriteBatchBuilder);
        if (camera->isInView(go->position))
        {
            auto killable = go->getComponent<KillableComponent>();
            if (killable != nullptr)
            {
                auto pos = camera->getScreenPosition(go->position);
                ImGui::SetNextWindowPos(ImVec2(pos.x - 50, pos.y - 40), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(100, 10), ImGuiCond_Always);
                ImGui::Begin(("health" + std::to_string(healthCount++)).data(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
                ImGui::SetWindowFontScale(0.05);
                ImGui::ProgressBar(killable->health / killable->maxHealth, ImVec2(-1, 0));
                ImGui::End();
            }
        }
    }
    if (gameState == GameState::Ready)
    {
        // auto sprite = spriteAtlas->get("get-ready.png");
        // sprite.setPosition(pos);
        // spriteBatchBuilder.addSprite(sprite);
    }
    else if (gameState == GameState::GameOver)
    {
        auto sprite = spriteAtlas->get("game-over.png");
        sprite.setPosition(pos);
        spriteBatchBuilder.addSprite(sprite);
    }
    else if (gameState == GameState::GameWon)
    {
        auto sprite = spriteAtlas->get("youwin.png");
        sprite.setPosition(pos);
        spriteBatchBuilder.addSprite(sprite);
    }

    auto sb = spriteBatchBuilder.build();
    rp.draw(sb);

    if (doDebugDraw)
    {
        world->DrawDebugData();
        rp.drawLines(debugDraw.getLines());
        debugDraw.clear();
    }
    auto window = (glm::vec2)sre::Renderer::instance->getWindowSize();
    ImGui::SetNextWindowPos(ImVec2(0 * window.x, .0f * window.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(0.25 * window.x, 0.06* window.y), ImGuiCond_Always);
    ImGui::Begin("Health Bar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    auto car = carObj->getComponent<Car>();
    ImGui::ProgressBar(car->health / car->maxHealth, ImVec2(-1, 0), std::to_string((int)ceil(car->health)).data());
    ImGui::End();

    //score counter
    auto size = ImVec2(0.25 * window.x, 0.06* window.y);
    ImGui::SetNextWindowPos(ImVec2(0.5 * window.x - (size.x * 0.5), .0f * window.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    ImGui::Begin("Score", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    ImGui::ProgressBar((float)currentScore / (float)winningScore, ImVec2(-1, 0), std::to_string(currentScore).data());
    ImGui::End();

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
            if (gameState == GameState::GameOver || gameState == GameState::GameWon)
            {
                init();
                gameState = GameState::Running;
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
    auto obj = std::shared_ptr<GameObject>(new GameObject());
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
