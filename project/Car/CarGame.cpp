#include <sre/Inspector.hpp>
#include <iostream>
#include "CarGame.hpp"
#include "GameObject.hpp"
#include "sre/RenderPass.hpp"
#include "components/SpriteComponent.hpp"
#include "components/SpriteAnimationComponent.hpp"
#include "Box2D/Dynamics/Contacts/b2Contact.h"
#include "components/PhysicsComponent.hpp"
#include "components/Car.hpp"
#include "components/EnemyComponent.hpp"
#include "components/KillableComponent.hpp"
#include <random>
#include "glm/gtc/matrix_transform.hpp"
#include "SDL_mixer.h"
#include "components/PowerupComponent.hpp"
#include "picojson.h"
#include <fstream>

#define _countof(array) (sizeof(array) / sizeof(array[0]))

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

    // initialize music - shamelessly stolen from slides.
    Mix_OpenAudio(
        22050,              // int frequency
        MIX_DEFAULT_FORMAT, // Uint16 format
        2,                  // int channels
        2048                // int buffer size (controls the latency, 2048 good default)
    );
    Mix_Music *music = Mix_LoadMUS("./assets/music.mp3");
    Mix_PlayMusic(music, -1);
    my_sound = Mix_LoadWAV("./assets/explosion.wav");

    spriteAtlas = SpriteAtlas::create("car.json", "car.png");
    // initialize explosion sprites
    explosionSprites = std::vector<sre::Sprite>();
    for (int i = 0; i < 16; i++)
    {
        auto name = "explosion/tile0" + std::to_string(i) + ".png";
        auto sprite = spriteAtlas->get(name);
        sprite.setScale({2, 2});
        explosionSprites.push_back(sprite);
    }

    // initialize enemy sprites
    enemySprites = std::vector<sre::Sprite>();
    enemySprites.push_back(spriteAtlas->get("enemy1.png"));
    enemySprites.push_back(spriteAtlas->get("enemy2.png"));
    for (int i = 0; i < 2; i++)
    {
        enemySprites.at(i).setScale({2, 2});
    }

    carObj = createGameObject();
    carObj->name = carName;
    auto so = carObj->addComponent<SpriteComponent>();
    auto sprite = spriteAtlas->get("Truck.png");
    sprite.setScale({2, 2});
    carObj->setPosition({0, 0});
    so->setSprite(sprite);

    auto carComp = carObj->addComponent<Car>();
    sprite = spriteAtlas->get("Tire.png");
    sprite.setScale({2, 2});
    carComp->initTires(&sprite);
    carComp->endGame = [this]
    {
        setGameState(GameState::GameOver);
    };

    // Absolute hack -- Sort the Truck and Tires by name, s.t. car is drawn after/over tires
    std::sort(
        sceneObjects.begin(),
        sceneObjects.end(),
        [](std::shared_ptr<GameObject> a, std::shared_ptr<GameObject> b)
        {
            return a->name > b->name;
        });

    // initialize camera
    auto camObj = createGameObject();
    camObj->name = "Camera";
    camera = camObj->addComponent<ScrollingCamera>();
    camObj->setPosition(windowSize * 0.5f);
    camera->setFollowObject(carObj, {0, 0});

    background.init(spriteAtlas->get("asphalt.png"));

    // Create grid of buildings around middle of play area
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

    currentScore = 0;
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
        position = getRandomSpawnPosition();
    }

    auto enemyObj = createGameObject();
    enemyObj->name = "Enemy";
    auto enemySpriteComponent = enemyObj->addComponent<SpriteComponent>();
    enemySpriteComponent->setSprite(enemySprites.at(0));
    auto sac = enemyObj->addComponent<SpriteAnimationComponent>();
    sac->setSprites(enemySprites);
    sac->setAnimationTime(0.5);
    auto enemyComp = enemyObj->addComponent<EnemyComponent>();
    enemyComp->init(carObj, position);
    enemyObj->setPosition(position);
    enemyObj->addComponent<KillableComponent>();
}

/// @brief Returns a position 30 units away from player in random direction
glm::vec2 CarGame::getRandomSpawnPosition()
{
    float distance = 30;
    auto angle = ran(gen) * M_PI * 2;
    auto position = carObj->getPosition() + (distance * glm::vec2(glm::sin(angle), glm::cos(angle)));
    return position;
}

void CarGame::spawnNPC(glm::vec2 position)
{
    if (position == glm::vec2(0, 0))
    {
        position = getRandomSpawnPosition();
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

void CarGame::spawnPowerup(PowerupType type, glm::vec2 position)
{
    if (position == glm::vec2(0, 0))
    {
        position = getRandomSpawnPosition();
    }

    auto obj = createGameObject();
    obj->name = "Powerup";
    auto spriteComponent = obj->addComponent<SpriteComponent>();
    auto sprite = spriteAtlas->get("healing.png");
    sprite.setScale({2, 2});
    spriteComponent->setSprite(sprite);
    obj->setPosition(position);
    auto phys = obj->addComponent<PhysicsComponent>();
    phys->initCircle(b2_dynamicBody, 30 / 10, position, 5);
    phys->setSensor(true);
    auto pc = obj->addComponent<PowerupComponent>();
}

void CarGame::handleSpawning(float time)
{
    float baseSpawnTime = 3;
    enemySpawnTimer -= time;
    if (enemySpawnTimer <= 0)
    {
        float denominator = winningScore - currentScore;
        if (denominator < 1)
        {
            denominator = 1;
        }
        float divisor = winningScore / baseSpawnTime;
        enemySpawnTimer += denominator / divisor;
        spawnEnemy();
    }
    NPCSpawnTimer -= time;
    if (NPCSpawnTimer <= 0)
    {

        NPCSpawnTimer += baseSpawnTime;
        spawnNPC();
        spawnPowerup(PowerupType::Heal);
    }
}

void CarGame::update(float time)
{
    if (gameState == GameState::Running)
    {
        handleSpawning(time);
        updatePhysics();
    }
    for (int i = 0; i < sceneObjects.size(); i++)
    {
        sceneObjects[i]->update(time);
        if (sceneObjects[i]->shouldRemove && sceneObjects[i]->getComponent<KillableComponent>() != nullptr)
        {
            spawnExplosion(sceneObjects[i]->getPosition());

            currentScore += sceneObjects[i]->getComponent<KillableComponent>()->getPointValue();
            if (currentScore >= winningScore)
            {
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

    // Update queue of frame times, ensuring elements are capped to max size
    this->frameTimingHistory.push_front(r.getLastFrameStats());
    while (frameTimingHistory.size() > maxFrameHistory)
    {
        frameTimingHistory.pop_back();
    }
}

void CarGame::spawnExplosion(glm::vec2 position)
{
    auto obj = createGameObject();
    obj->setPosition(position);
    obj->setRotation(ran(gen) * 360);
    auto sc = obj->addComponent<SpriteComponent>();
    sc->setSprite(explosionSprites.at(0));
    auto sac = obj->addComponent<SpriteAnimationComponent>();
    sac->setSprites(explosionSprites);
    sac->setAnimationTime(0.1);
    sac->setRepeating(false);
    if (!mute)
    {
        // Plays an SFX on the desired SFX channel
        Mix_PlayChannel(
            -1,       // int channel to play on (-1 is first available)
            my_sound, // Mix_Chunk* chunk to play
            0         // int number loops
        );
    }
}

void CarGame::render()
{
    auto rp = RenderPass::create()
                  .withCamera(camera->getCamera())
                  .build();

    auto pos = camera->getGameObject()->getPosition();
    background.renderBackground(rp, pos.x, pos.y);

    auto spriteBatchBuilder = SpriteBatch::create();
    int healthCount = 0;
    // Create health bars for each killable object in view
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
    if (gameState == GameState::GameOver)
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
        drawCarDebugMenu();
        drawFrameTimingDebug();
        debugDraw.clear();
    }
    // Health bar
    auto window = (glm::vec2)sre::Renderer::instance->getWindowSize();
    ImGui::SetNextWindowPos(ImVec2(0 * window.x, .0f * window.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(0.25 * window.x, 0.06 * window.y), ImGuiCond_Always);
    ImGui::Begin("Health Bar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    auto car = carObj->getComponent<Car>();
    ImGui::ProgressBar(car->health / car->maxHealth, ImVec2(-1, 0), std::to_string((int)ceil(car->health)).data());
    ImGui::End();

    // Score counter
    auto size = ImVec2(0.25 * window.x, 0.06 * window.y);
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
        case SDLK_ESCAPE: // Toggle debug view and menus
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
        case SDLK_r: // Instantly reset game
            init();
            break;
        case SDLK_SPACE: // Restart game if game over or won
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
        case SDLK_o: // Spawn an enemy at a random position
            spawnEnemy();
            break;
        case SDLK_m: // Toggle sound mute
            if (mute)
                Mix_ResumeMusic();
            else
                Mix_PauseMusic();
            mute = !mute;
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

/// @brief Debug window for modifying car variables and saving/loading them to/from JSON
void CarGame::drawCarDebugMenu()
{
    auto window = (glm::vec2)sre::Renderer::instance->getWindowSize();
    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::Begin("Car Debug Menu", nullptr);
    ImGui::SetWindowFontScale(0.50f);
    auto car = carObj->getComponent<Car>();
    ImGui::DragFloat("Drag Ratio", &car->dragRatio, 0.01f, 0.0f, 5.0f);
    ImGui::DragFloat("Current Traction", &car->currentTraction, 0.1f, 0.0f, 5.0f);
    ImGui::DragFloat("Max Forward Speed", &car->maxForwardSpeed, 1.0f, 0.0f, 1000.0f);
    ImGui::DragFloat("Max Backwards Speed", &car->maxBackwardSpeed, 1.0f, -1000.0f, 0.0f);
    ImGui::DragFloat("Back Tire Max Lateral Impulse", &car->backTireMaxLateralImpulse, 0.5f, 0.0f, 50.0f);
    ImGui::DragFloat("Front Tire Max Lateral Impulse", &car->frontTireMaxLateralImpulse, 0.5f, 0.0f, 50.0f);
    ImGui::DragFloat("Back Tire Max Drive Force", &car->backTireMaxDriveForce, 500.0f, 0.0f, 100000.0f);
    ImGui::DragFloat("Front Tire Max Drive Force", &car->frontTireMaxDriveForce, 500.0f, 0.0f, 100000.0f);
    auto save = ImGui::Button("Save Settings");
    ImGui::SameLine();
    auto load = ImGui::Button("Load Settings");
    ImGui::End();
    if (save)
    {
        auto f = std::ofstream("carSettings.json");
        picojson::object o;
        o["dragRatio"] = picojson::value(car->dragRatio);
        o["currentTraction"] = picojson::value(car->currentTraction);
        o["maxBackwardSpeed"] = picojson::value(car->maxBackwardSpeed);
        o["maxForwardSpeed"] = picojson::value(car->maxForwardSpeed);
        o["backTireMaxLateralImpulse"] = picojson::value(car->backTireMaxLateralImpulse);
        o["frontTireMaxLateralImpulse"] = picojson::value(car->frontTireMaxLateralImpulse);
        o["backTireMaxDriveForce"] = picojson::value(car->backTireMaxDriveForce);
        o["frontTireMaxDriveForce"] = picojson::value(car->frontTireMaxDriveForce);
        f << picojson::value(o);
        f.flush();
    }
    if (load)
    {
        auto f = std::ifstream("carSettings.json");
        picojson::value v;
        if (!f)
        {
            std::cerr << "carSettings.json could not be read, maybe it has not been saved yet";
        }
        else
        {
            f >> v;
            car->dragRatio = v.get("dragRatio").get<double>();
            car->currentTraction = v.get("currentTraction").get<double>();
            car->maxBackwardSpeed = v.get("maxBackwardSpeed").get<double>();
            car->maxForwardSpeed = v.get("maxForwardSpeed").get<double>();
            car->backTireMaxLateralImpulse = v.get("backTireMaxLateralImpulse").get<double>();
            car->frontTireMaxLateralImpulse = v.get("frontTireMaxLateralImpulse").get<double>();
            car->backTireMaxDriveForce = v.get("backTireMaxDriveForce").get<double>();
            car->frontTireMaxDriveForce = v.get("frontTireMaxDriveForce").get<double>();
        }
    }
}

/// @brief Pick a color based on frame time
static glm::vec4 DeltaTimeToColor(float dt)
{
    // From https://github.com/sawickiap/RegEngine/blob/613c31fd60558a75c5b8902529acfa425fc97b2a/Source/Game.cpp#L331
    constexpr glm::vec3 colors[] = {
        {0.f, 0.f, 1.f}, // blue
        {0.f, 1.f, 0.f}, // green
        {1.f, 1.f, 0.f}, // yellow
        {1.f, 0.f, 0.f}, // red
    };
    constexpr float dts[] = {
        1.f / 120.f,
        1.f / 60.f,
        1.f / 30.f,
        1.f / 15.f,
    };
    if (dt < dts[0])
        return glm::vec4(colors[0], 1.f);
    for (size_t i = 1; i < _countof(dts); ++i)
    {
        if (dt < dts[i])
        {
            const float t = (dt - dts[i - 1]) / (dts[i] - dts[i - 1]);
            return glm::vec4(glm::mix(colors[i - 1], colors[i], t), 1.f);
        }
    }
    return glm::vec4(colors[_countof(dts) - 1], 1.f);
}

/// @brief Debug window containing frame timing histogram
void CarGame::drawFrameTimingDebug()
{
    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::Begin("Frame Timings", nullptr);
    ImGui::SetWindowFontScale(0.50f);

    if (frameTimingHistory.size() > 0)
    {
        auto lastFrame = frameTimingHistory[0];
        std::string frameInfo = "Delta Last frame " + std::to_string(lastFrame.delta() * 1000) + "ms";
        ImGui::Text(frameInfo.c_str());
    }

    // From https://github.com/sawickiap/RegEngine/blob/613c31fd60558a75c5b8902529acfa425fc97b2a/Source/Game.cpp#L331
    auto frameStats = r.getLastFrameStats();
    const float width = ImGui::GetWindowWidth();
    const size_t frameCount = frameTimingHistory.size();
    if (width > 0.f && frameCount > 0)
    {
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImVec2 basePos = ImGui::GetCursorScreenPos();
        constexpr float minHeight = 2.f;
        constexpr float maxHeight = 64.f;
        float endX = width;
        constexpr float dtMin = 1.f / 120.f;
        constexpr float dtMax = 1.f / 15.f;
        const float dtMin_Log2 = log2(dtMin);
        const float dtMax_Log2 = log2(dtMax);
        drawList->AddRectFilled(basePos, ImVec2(basePos.x + width, basePos.y + maxHeight), 0xFF404040);
        for (size_t frameIndex = 0; frameIndex < frameCount && endX > 0.f; ++frameIndex)
        {
            const float delta = this->frameTimingHistory[frameIndex].delta();
            const float frameWidth = delta / dtMin;
            const float frameHeightFactor = (log2(delta) - dtMin_Log2) / (dtMax_Log2 - dtMin_Log2);
            const float frameHeightFactor_Nrm = std::min(std::max(0.f, frameHeightFactor), 1.f);
            const float frameHeight = glm::mix(minHeight, maxHeight, frameHeightFactor_Nrm);
            const float begX = endX - frameWidth;
            const uint32_t color = glm::packUnorm4x8(DeltaTimeToColor(delta));
            drawList->AddRectFilled(
                ImVec2(basePos.x + std::max(0.f, floor(begX)), basePos.y + maxHeight - frameHeight),
                ImVec2(basePos.x + ceil(endX), basePos.y + maxHeight),
                color);
            endX = begX;
        }
        ImGui::Dummy(ImVec2(width, maxHeight));
    }
    ImGui::End();
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
