#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/GJGarageLayer.hpp>
#include <Geode/modify/CurrencyRewardLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <capeling.garage-stats-menu/include/StatsDisplayAPI.h>

using namespace geode::prelude;

int calculateOrbsToNextKey() {
	return GameStatsManager::sharedState()->getTotalCollectedCurrency() % 500;
}

class $modify(MyEndLevelLayer, EndLevelLayer) {

	static void onModify(auto& self) {
		(void) self.setHookPriorityPost("EndLevelLayer::showLayer", Priority::Last);
	}

	void showLayer(bool p0) {
		EndLevelLayer::showLayer(p0);

		if (!Mod::get()->getSettingValue<bool>("show-on-end-screen")) return;

		if (m_playLayer->m_level->m_stars <= 0) return;

		CCSize winSize = CCDirector::get()->getWinSize();

		CCLabelBMFont* counter = CCLabelBMFont::create(fmt::format("Key: {}/500", calculateOrbsToNextKey()).c_str(), "goldFont.fnt");
		counter->setScale(0.8f);
		counter->setAnchorPoint({1.f, 0.f});
		counter->setPosition({0, 0});

		CCNode* labelContainer = CCNode::create();
		labelContainer->setPosition({winSize.width/2, winSize.height/2 + 8});
		labelContainer->setContentSize({200, 90});
		labelContainer->setAnchorPoint({0.5f, 0.5f});
		labelContainer->setID("label-container"_spr);

		ColumnLayout* layout = ColumnLayout::create();
		layout->setGap(3);
		layout->setAutoScale(false);
		labelContainer->setLayout(layout);

		std::vector<Ref<CCLabelBMFont>> nodesToMove;

		for (CCNode* child : CCArrayExt<CCNode*>(m_mainLayer->getChildren())) {
			if (CCLabelBMFont* label = typeinfo_cast<CCLabelBMFont*>(child)) {
				if (label->getPositionX() == winSize.width/2) {
					nodesToMove.push_back(label);
				}
			}
		}

		if (nodesToMove.size() > 3) {
			layout->setAutoScale(true);
		}

		for (auto node : nodesToMove) {
			node->removeFromParentAndCleanup(false);
			labelContainer->addChild(node);
		}

		labelContainer->addChild(counter);

		labelContainer->updateLayout();
		
		m_mainLayer->addChild(labelContainer);
	}
};

class $modify(MyCurrencyRewardLayer, CurrencyRewardLayer) {

    void createObjectsFull(CurrencySpriteType type, int count, cocos2d::CCSprite* spr, cocos2d::CCPoint position, float time) {
		CurrencyRewardLayer::createObjectsFull(type, count, spr, position, time);
		
		if (!Mod::get()->getSettingValue<bool>("show-on-orbs")) return;
		
		if (type == CurrencySpriteType::Orb) {
			if (m_orbsLabel) {
				CCLabelBMFont* counter = CCLabelBMFont::create(fmt::format("{}/500", calculateOrbsToNextKey()).c_str(), "bigFont.fnt");
				counter->setColor({45, 255, 255});
				counter->setScale(0.3f);
				counter->setAnchorPoint({1.f, 1.f});
				counter->setPosition({0, -m_orbsLabel->getScaledContentHeight() + 1});
				
				m_mainNode->addChild(counter);
			}
		}
	}
};

class $modify(MyPauseLayer, PauseLayer) {
	
    virtual void customSetup() {
		PauseLayer::customSetup();

		if (!Mod::get()->getSettingValue<bool>("show-on-pause")) return;

		auto pl = PlayLayer::get();
		if (!pl || pl->m_level->m_stars <= 0) return;

		if (auto normalModeLabel = getChildByID("normal-mode-label")) {

			CCLabelBMFont* counter = CCLabelBMFont::create(fmt::format("{}/500", calculateOrbsToNextKey()).c_str(), "bigFont.fnt");
			counter->setScale(0.35f);
			counter->setAnchorPoint({1.f, 0.5f});
			counter->setPosition({normalModeLabel->getPositionX() + 170, normalModeLabel->getPositionY()});
			counter->setColor({45, 255, 255});

			addChild(counter);
		}
	}


};

class $modify(GJGarageLayer) {
	bool init() {
		if (!GJGarageLayer::init()) return false;

		if (!Mod::get()->getSettingValue<bool>("show-on-garage")) return true;

		auto statMenu = this->getChildByID("capeling.garage-stats-menu/stats-menu");

		auto myStatItem = StatsDisplayAPI::getNewItem("demon-keys"_spr, 
			CCSprite::createWithSpriteFrameName("GJ_bigKey_001.png"), 
			0, 
			0.375f);

		auto label = static_cast<CCLabelBMFont*>(myStatItem->getChildByID(std::string("demon-keys"_spr) + "-label"));
		label->setString(fmt::format("(+{})", calculateOrbsToNextKey()).c_str());
		label->setColor({45, 255, 255});

		CCLabelBMFont* newLabel = CCLabelBMFont::create(fmt::format("{}", GameStatsManager::sharedState()->getStat("21")).c_str(), "bigFont.fnt");

		newLabel->setScale(label->getScale());
		newLabel->setAnchorPoint({1.f, 0.5f});
		label->setScale(0.2f);
		label->setID("demon-keys-orbs-label"_spr);

		newLabel->setPosition({label->getPositionX() - label->getScaledContentWidth() - 3, label->getPositionY() + 0.5f});
		newLabel->setID("demon-keys-label"_spr);
		myStatItem->addChild(newLabel);

		if (statMenu) {
			statMenu->addChild(myStatItem);
			statMenu->updateLayout();
		}

		return true;
	}
};