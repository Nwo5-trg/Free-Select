#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

#ifdef GEODE_IS_WINDOWS
    #include <geode.custom-keybinds/include/Keybinds.hpp>
    using namespace keybinds;
#endif

#ifdef GEODE_IS_MACOS
    #include <CoreGraphics/CGEventSource.h>
#endif

using namespace geode::prelude;

class $modify(EditorUIHook, EditorUI) {
    struct Fields {
        bool touchEndedFlag;
        bool lassoEnabled;
        std::vector<CCPoint> lassoPoints;
        std::vector<CCPoint> lassoPointsToCheck;

        CCNodeRGBA* chromaNode;

        bool lassoAlwaysEnabled;
        bool guaranteeCenter;
        bool makePointsBoxes;
        bool chroma;
        #ifdef GEODE_IS_MACOS
            unsigned short macKeycode;
            unsigned short secondMacKeycode;
        #endif
        float gridSize;
        float chromaSpeed;
        ccColor4B selectColor;
    };

    void draw() {;
        auto fields = m_fields.self();

        if (m_swipeActive) {
            if (fields->chroma) {
                auto col = fields->chromaNode->getColor();
                ccDrawColor4B(col.r, col.g, col.b, fields->selectColor.a);
            } else ccDrawColor4B(fields->selectColor);

            if (fields->lassoEnabled) {
                auto& points = fields->lassoPoints;
                auto size = points.size();

                for (size_t i = 0; i + 1 < size; i++) {
                    ccDrawLine(points[i], points[i + 1]);
                }
            } else {
                ccDrawRect(m_swipeStart, m_swipeEnd);
            }
        }

        // already hooking draw might as well use it as my update function

        #ifdef GEODE_IS_MOBILE
            fields->lassoEnabled = !fields->lassoAlwaysEnabled;
        #endif

        #ifdef GEODE_IS_MACOS
            bool pressed = CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, fields->macKeycode);
            if (!pressed && fields->secondMacKeycode != 0) {
                pressed = CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, fields->secondMacKeycode);
            }
            fields->lassoEnabled = fields->lassoAlwaysEnabled ? !pressed : pressed;
        #endif

        bool ret = m_swipeActive;
        m_swipeActive = false;
        EditorUI::draw();
        m_swipeActive = ret;
    }

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;
        auto fields = m_fields.self();

        auto mod = Mod::get();
        fields->lassoAlwaysEnabled = mod->getSettingValue<bool>("lasso-always-enabled");
        fields->guaranteeCenter = mod->getSettingValue<bool>("guarantee-center");
        fields->makePointsBoxes = mod->getSettingValue<bool>("make-points-boxes");
        fields->chroma = mod->getSettingValue<bool>("chroma");

        #ifdef GEODE_IS_MACOS
            fields->macKeycode = mod->getSettingValue<int64_t>("mac-keycode");
            fields->secondMacKeycode = mod->getSettingValue<int64_t>("second-mac-keycode");
        #endif
        
        fields->gridSize = mod->getSettingValue<double>("grid-size");
        fields->chromaSpeed = mod->getSettingValue<double>("chroma-speed");

        fields->selectColor = mod->getSettingValue<ccColor4B>("select-color");

        #ifdef GEODE_IS_WINDOWS
            this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
                fields->lassoEnabled = fields->lassoAlwaysEnabled ? !event->isDown() : event->isDown();
                return ListenerResult::Propagate;
            }, "free-select-lasso-select-modifier"_spr);
        #endif

        auto chromaNode = CCNodeRGBA::create();
        chromaNode->runAction(CCRepeatForever::create(
            CCSequence::create(
                CCTintTo::create(fields->chromaSpeed, 128, 255, 255),
                CCTintTo::create(fields->chromaSpeed, 128, 128, 255), 
                CCTintTo::create(fields->chromaSpeed, 255, 128, 255),
                CCTintTo::create(fields->chromaSpeed, 255, 128, 128), 
                CCTintTo::create(fields->chromaSpeed, 255, 255, 128),
                CCTintTo::create(fields->chromaSpeed, 128, 255, 128), 
                nullptr
            )
        ));
        chromaNode->setID("chroma-node"_spr);
        this->addChild(chromaNode);
        fields->chromaNode = chromaNode;

        return true;
    }
    
    bool ccTouchBegan(CCTouch* p0, CCEvent* p1) {
        if (!EditorUI::ccTouchBegan(p0, p1)) return false;

        auto fields = m_fields.self();
        if (fields->lassoEnabled) fields->lassoPoints.clear();

        return true;
    }
    
    void ccTouchMoved(CCTouch* p0, CCEvent* p1) {
        m_fields->lassoPoints.push_back(convertTouchToNodeSpace(p0));
        EditorUI::ccTouchMoved(p0, p1);
    }

    void ccTouchEnded(CCTouch* p0, CCEvent* p1) {
        auto fields = m_fields.self();

        if (!fields->lassoPoints.empty()) {
            fields->lassoPoints.push_back(fields->lassoPoints.front());
        }

        if (fields->lassoEnabled && m_swipeActive) {
            fields->touchEndedFlag = true;
            EditorUI::ccTouchEnded(p0, p1);
            fields->touchEndedFlag = false;
            selectLassoObjects();
        } else {
            EditorUI::ccTouchEnded(p0, p1);
        }
    }

    void selectObjects(CCArray* p0, bool p1) {
        if (m_fields->touchEndedFlag) return;
        EditorUI::selectObjects(p0, p1);
    }
    
    void selectLassoObjects() {
        auto fields = m_fields.self();

        auto& points = fields->lassoPoints;
        auto size = points.size();

        if (size < 3) return;

        CCPoint min = {FLT_MAX, FLT_MAX};
        CCPoint max = {-FLT_MAX, -FLT_MAX};

        for (const auto& point : points) {
            if (point.x < min.x) min.x = point.x;
            if (point.y < min.y) min.y = point.y;
            if (point.x > max.x) max.x = point.x;
            if (point.y > max.y) max.y = point.y;
        }

        auto& pointsToCheck = fields->lassoPointsToCheck;
        pointsToCheck.clear();

        float gridSize = fields->gridSize * m_editorLayer->m_objectLayer->getScale();
        auto gridUnit = fields->makePointsBoxes ? ccp(fields->gridSize, fields->gridSize) : ccp(0.0f, 0.0f);

        // side note, this is a push_back situation right? cuz operators construct a new object
        // idk and doesnt rly change much anyway
        if (fields->guaranteeCenter) pointsToCheck.push_back((min + max) / 2);

        // 3 nested loops </3
        for (float x = min.x; x < max.x; x += gridSize) {
            for (float y = min.y; y < max.y; y += gridSize) {

                CCPoint pos = {x, y};
                bool inside = false;
                
                for (size_t i = 1; i < size; i++) {
                    const auto& p1 = points[i];
                    const auto& p2 = points[i - 1];

                    if (p1.y == p2.y) continue;

                    if (
                        (p1.y > pos.y) != (p2.y > pos.y) &&
                        (pos.x < ((p2.x - p1.x) * (pos.y - p1.y) / (p2.y - p1.y) + p1.x))
                    ) inside = !inside;
                }

                if (inside) pointsToCheck.push_back(pos);
            }
        }

        auto objs = CCArray::create();

        for (const auto& point : pointsToCheck) {
            for (auto obj : CCArrayExt<GameObject*>(m_editorLayer->objectsInRect(
                {m_editorLayer->m_drawGridLayer->convertToNodeSpace(point) - gridUnit / 2, gridUnit}, false))
            ) {
                if (!objs->containsObject(obj)) objs->addObject(obj);
            }
        }

        createUndoSelectObject(false);
        selectObjects(objs, false);
        updateButtons();
        updateObjectInfoLabel();
    }
};

#ifdef GEODE_IS_WINDOWS
    $execute {
        BindManager::get()->registerBindable(
            "free-select-lasso-select-modifier"_spr,
            "", { Keybind::create(KEY_Alt, Modifier::None) },
            "Editor/Modify"
        );
    }
#endif