#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

constexpr int SHAKE_ACTION_TAG = 8472;

class $modify(ShakePlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        bool wasDead = player->m_isDead;

        PlayLayer::destroyPlayer(player, object);

        if (!wasDead && player->m_isDead) {
            this->triggerDeathShake();
        }
    }

    void triggerDeathShake() {
        auto mod = Mod::get();
        if (!mod->getSettingValue<bool>("enabled")) return;
        if (this->m_isPracticeMode && !mod->getSettingValue<bool>("practice")) return;

        this->stopActionByTag(SHAKE_ACTION_TAG);
        this->setPosition({ 0.f, 0.f });
        this->setRotation(0.f);

        float intensity = static_cast<float>(mod->getSettingValue<double>("intensity"));
        float speed = static_cast<float>(mod->getSettingValue<double>("speed"));
        if (speed <= 0.05f) speed = 0.05f;
        float stepTime = 0.025f / speed;
        bool useRotation = mod->getSettingValue<bool>("rotation");
        float rot = useRotation ? (intensity * 0.12f) : 0.f;

        auto sequence = CCSequence::create(
            CCSpawn::createWithTwoActions(
                CCMoveTo::create(stepTime, { -intensity, intensity * 0.7f }),
                CCRotateTo::create(stepTime, -rot)
            ),
            CCSpawn::createWithTwoActions(
                CCMoveTo::create(stepTime, { intensity, -intensity * 0.8f }),
                CCRotateTo::create(stepTime, rot)
            ),
            CCSpawn::createWithTwoActions(
                CCMoveTo::create(stepTime, { -intensity * 0.6f, -intensity * 0.5f }),
                CCRotateTo::create(stepTime, -rot * 0.6f)
            ),
            CCSpawn::createWithTwoActions(
                CCMoveTo::create(stepTime, { intensity * 0.5f, intensity * 0.4f }),
                CCRotateTo::create(stepTime, rot * 0.5f)
            ),
            CCSpawn::createWithTwoActions(
                CCMoveTo::create(stepTime, { -intensity * 0.25f, intensity * 0.2f }),
                CCRotateTo::create(stepTime, -rot * 0.25f)
            ),
            CCSpawn::createWithTwoActions(
                CCMoveTo::create(stepTime, { 0.f, 0.f }),
                CCRotateTo::create(stepTime, 0.f)
            ),
            nullptr
        );

        sequence->setTag(SHAKE_ACTION_TAG);
        this->runAction(sequence);
    }
};

class $modify(ShakePauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto sprite = CircleButtonSprite::createWithSpriteFrameName(
            "GJ_optionsBtn_001.png",
            1.f,
            CircleBaseColor::Green,
            CircleBaseSize::Small
        );

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(ShakePauseLayer::onShakeSettings)
        );
        button->setID("death-shake-settings-btn"_spr);

        if (auto rightMenu = this->getChildByID("right-button-menu")) {
            rightMenu->addChild(button);
            rightMenu->updateLayout();
        } else if (auto leftMenu = this->getChildByID("left-button-menu")) {
            leftMenu->addChild(button);
            leftMenu->updateLayout();
        } else {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            auto menu = CCMenu::create();
            menu->setPosition({ winSize.width - 35.f, 35.f });
            menu->addChild(button);
            this->addChild(menu);
        }
    }

    void onShakeSettings(CCObject*) {
        geode::openSettingsPopup(Mod::get());
    }
};