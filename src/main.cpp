#include <Geode/Geode.hpp>
#include <Geode/ui/SceneManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <chrono>
#include <cstdio>

using namespace geode::prelude;

// Starts counting the moment the mod is loaded (i.e. when GD launches)
static const auto s_startTime = std::chrono::steady_clock::now();

class SessionTimer : public CCNode {
protected:
	CCLabelBMFont* m_label = nullptr;
	long long m_lastShown = -1;

	bool init() override {
		if (!CCNode::init()) return false;

		m_label = CCLabelBMFont::create("00:00:00", "bigFont.fnt");
		m_label->setAnchorPoint({1.f, 1.f});
		this->addChild(m_label);

		this->refreshStyle();
		this->refreshText(true);
		this->scheduleUpdate();
		return true;
	}

	void refreshStyle() {
		auto opacity = Mod::get()->getSettingValue<double>("opacity");
		auto scale = Mod::get()->getSettingValue<double>("scale");
		m_label->setScale(static_cast<float>(scale));
		m_label->setOpacity(static_cast<GLubyte>(opacity * 255));

		// pin to top-right of whatever screen size we're on
		auto winSize = CCDirector::get()->getWinSize();
		this->setPosition({winSize.width - 6.f, winSize.height - 4.f});
	}

	void refreshText(bool force = false) {
		using namespace std::chrono;
		auto secs = duration_cast<seconds>(steady_clock::now() - s_startTime).count();
		if (!force && secs == m_lastShown) return;
		m_lastShown = secs;

		char buf[32];
		std::snprintf(buf, sizeof(buf), "%02lld:%02lld:%02lld",
			secs / 3600, (secs / 60) % 60, secs % 60);
		m_label->setString(buf);
	}

	void update(float) override {
		this->refreshStyle();
		this->refreshText();
	}

public:
	static SessionTimer* create() {
		auto ret = new SessionTimer();
		if (ret && ret->init()) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
};

class $modify(TimerMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		// create the timer once and keep it alive across every scene
		// (menus, levels, editor, etc.)
		static bool created = false;
		if (!created) {
			created = true;
			auto timer = SessionTimer::create();
			if (timer) {
				timer->setZOrder(10000);
				SceneManager::get()->keepAcrossScenes(timer);
			}
		}
		return true;
	}
};
