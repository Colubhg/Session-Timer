#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
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

		this->tick(true);
		return true;
	}

public:
	// called every frame from the director's draw loop
	void tick(bool force = false) {
		auto opacity = Mod::get()->getSettingValue<double>("opacity");
		auto scale = Mod::get()->getSettingValue<double>("scale");
		m_label->setScale(static_cast<float>(scale));
		m_label->setOpacity(static_cast<GLubyte>(opacity * 255));

		// pin to top-right of whatever screen size we're on
		auto winSize = CCDirector::get()->getWinSize();
		this->setPosition({winSize.width - 6.f, winSize.height - 4.f});

		using namespace std::chrono;
		auto secs = duration_cast<seconds>(steady_clock::now() - s_startTime).count();
		if (!force && secs == m_lastShown) return;
		m_lastShown = secs;

		char buf[32];
		std::snprintf(buf, sizeof(buf), "%02lld:%02lld:%02lld",
			secs / 3600, (secs / 60) % 60, secs % 60);
		m_label->setString(buf);
	}

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

static Ref<SessionTimer> s_timer;

class $modify(TimerDirector, CCDirector) {
	void willSwitchToScene(CCScene* scene) {
		CCDirector::willSwitchToScene(scene);

		if (!s_timer) {
			s_timer = SessionTimer::create();
		}
		if (s_timer && scene) {
			// move the one timer into every new scene (menus, levels, editor...)
			s_timer->removeFromParent();
			scene->addChild(s_timer, 10000);
		}
	}

	void drawScene() {
		if (s_timer) {
			s_timer->tick();

			// safety net: if the timer ever lost its scene, put it back
			auto scene = this->getRunningScene();
			if (scene && s_timer->getParent() != scene &&
				!typeinfo_cast<CCTransitionScene*>(scene)) {
				s_timer->removeFromParent();
				scene->addChild(s_timer, 10000);
			}
		}
		CCDirector::drawScene();
	}
};
