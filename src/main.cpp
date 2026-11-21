#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <algorithm>

// if yall think is vibecode dm me on discord my user is idzito

using namespace geode::prelude;
using namespace cocos2d;

namespace {
    constexpr int SHAKE_TAG = 8472;

    struct ShakeStep {
        float x;
        float y;
        float rot;
    };
}

class $modify(ShakePlayLayer, PlayLayer) {
    void resetShake() {
        this->stopActionByTag(SHAKE_TAG);
        this->setPosition({ 0.f, 0.f });
        this->setRotation(0.f);
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        bool wasDead = player->m_isDead;
        PlayLayer::destroyPlayer(player, object);

        if (!wasDead && player->m_isDead) {
            this->triggerDeathShake();
        }
    }

    void resetLevel() {
        this->resetShake();
        PlayLayer::resetLevel();
    }

    void onQuit() {
        this->resetShake();
        PlayLayer::onQuit();
    }

    void triggerDeathShake() {
        auto mod = Mod::get();
        if (!mod->getSettingValue<bool>("enabled")) return;
        if (this->m_isPracticeMode && !mod->getSettingValue<bool>("practice")) return;

        this->resetShake();

        float intensity = static_cast<float>(mod->getSettingValue<double>("intensity"));
        float speed = std::max(0.05f, static_cast<float>(mod->getSettingValue<double>("speed")));
        float stepTime = 0.025f / speed;
        float rotFactor = mod->getSettingValue<bool>("rotation") ? (intensity * 0.12f) : 0.f;

        const ShakeStep steps[] = {
            { -intensity,         intensity * 0.7f, -rotFactor },
            {  intensity,        -intensity * 0.8f,  rotFactor },
            { -intensity * 0.6f, -intensity * 0.5f, -rotFactor * 0.6f },
            {  intensity * 0.5f,  intensity * 0.4f,  rotFactor * 0.5f },
            { -intensity * 0.25f, intensity * 0.2f, -rotFactor * 0.25f },
            {  0.f,               0.f,               0.f }
        };

        auto actions = CCArray::create();
        for (const auto& step : steps) {
            actions->addObject(CCSpawn::createWithTwoActions(
                CCMoveTo::create(stepTime, { step.x, step.y }),
                CCRotateTo::create(stepTime, step.rot)
            ));
        }

        auto seq = CCSequence::create(actions);
        seq->setTag(SHAKE_TAG);
        this->runAction(seq);
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

        if (auto menu = this->getChildByID("right-button-menu")) {
            menu->addChild(button);
            menu->updateLayout();
        }
    }

    void onShakeSettings(CCObject*) {
        geode::openSettingsPopup(Mod::get());
    }
};