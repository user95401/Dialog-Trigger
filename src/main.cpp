#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
using namespace geode::prelude;

#include <regex>

int DIALOG_TRIGGER_ID = "dialog_trigger"_h * 0.00001; //based of 1613

// Returns a CCArrayExt of GameObjects that was changed
inline static auto exchangeMyCheatsToObjectID(
    GameObject* object, CCArray* objects = nullptr, 
    CCArrayExt<GameObject*> changedObjects = {}
) {
    if (!object and !objects) return changedObjects;
    // ID mapping
    auto static IDs = std::map<int, int>{ {}
        , {DIALOG_TRIGGER_ID, 1613}
    };
    // Restore IDs mapping
    auto static restoreIDs = std::map<int, int>{};
    if (restoreIDs.size() == 0) for (auto& [to, from] : IDs) restoreIDs[from] = to;
    // Restore replaced IDs
    if (changedObjects.size() > 0) {
        for (auto object : changedObjects) {
            if (restoreIDs.contains(object->m_objectID)) object->m_objectID = restoreIDs[object->m_objectID];
        };
        return changedObjects;
    }
    // Replace ID of objects in CCArray (selected objects f.e.)
    if (objects) {
        for (auto object : CCArrayExt<GameObject*>(objects)) {
			if (IDs.contains(object->m_objectID)) {
				object->m_objectID = IDs[object->m_objectID];
                changedObjects.push_back(object);
            }
        };
    };
    // Replace ID of object...
    if (object) {
		if (IDs.contains(object->m_objectID)) {
			object->m_objectID = IDs[object->m_objectID];
			changedObjects.push_back(object);
		};
    };
    return changedObjects;
}

#include <Geode/modify/GJBaseGameLayer.hpp>
class $modify(GJBaseGameLayerDialogTriggerExt, GJBaseGameLayer) {
    virtual void spawnObject(GameObject * object, double delay, gd::vector<int> const& remapKeys) {
        auto changedObjects = exchangeMyCheatsToObjectID(object);
        GJBaseGameLayer::spawnObject(object, delay, remapKeys);
        exchangeMyCheatsToObjectID(object, nullptr, changedObjects);
    }
};

#include <Geode/modify/EffectGameObject.hpp>
class $modify(EffectGameObjectDialogTriggerExt, EffectGameObject) {
    struct Fields : DialogDelegate {
        DialogLayer* m_dialogLayer = nullptr;
        GJBaseGameLayer* m_game = nullptr;
        virtual void dialogClosed(DialogLayer* p0) {
            m_dialogLayer = nullptr;
            if (m_game) m_game->resumeSchedulerAndActions();
        };
    };
    virtual void customSetup() {
        auto changedObjects = exchangeMyCheatsToObjectID(this);
        EffectGameObject::customSetup();
        exchangeMyCheatsToObjectID(this, nullptr, changedObjects);
    }
    virtual void triggerActivated(float p0) {
        auto changedObjects = exchangeMyCheatsToObjectID(this);
		EffectGameObject::triggerActivated(p0);
		exchangeMyCheatsToObjectID(this, nullptr, changedObjects);
    }
    virtual void triggerObject(GJBaseGameLayer* p0, int p1, gd::vector<int> const* p2) {
        log::debug(
            "{}->{}(GJBaseGameLayer* {}, int {}, gd::vector<int> const* {})", 
            this, __func__, p0, p1, p2 ? *p2 : gd::vector<int>()
        );
        log::debug("{}->m_objectID {}", this, this->m_objectID);
        m_fields->m_game = p0;
        EffectGameObject::triggerObject(p0, p1, p2);
        if (auto DialogTriggerDataNode = typeinfo_cast<CCNode*>(this->getUserObject("DialogTriggerDataNode"_spr))) {
            auto raw_data = "[" + DialogTriggerDataNode->getID() + "]";
            auto parse = matjson::parse(raw_data);
            matjson::Value data = parse.err()
                ? matjson::parse("[ \"<cr>err: " + parse.err().value().message + "</c>\" ]").unwrapOrDefault()
                : parse.unwrapOrDefault();

            /*
                example text:
                ```
                1,"char:the noone","<cr>hi!</c> why ar u here ever?","!","its feels bad you here"
                ```
            */

            DialogChatPlacement placement = DialogChatPlacement::Center;

            auto not_skippable = true;
            auto character = std::string("");
            auto characterFrame = 0;

            auto dialogObjectsArr = CCArrayExt<DialogObject>();

            for (auto& val : data) {
                if (val.isNumber()) {
                    characterFrame = val.asInt().unwrapOrDefault();
                }
                if (val.isString()) {
                    auto text = val.asString().unwrapOrDefault();
                    if (string::startsWith(text, "!char:")) {
                        character = string::replace(text, "!char:", "");
                        continue;
                    }
                    if (string::startsWith(text, "!place:")) {
                        auto place = string::replace(text, "!place:", "");
						if (place == "t") placement = DialogChatPlacement::Top;
						if (place == "c") placement = DialogChatPlacement::Center;
						if (place == "b") placement = DialogChatPlacement::Bottom;
						continue;
					}
                    if (string::startsWith(text, "!")) {
                        not_skippable = false;
                        continue;
                    }
                    dialogObjectsArr.push_back(DialogObject::create(
                        character, text, characterFrame, 1.f, not not_skippable, ccWHITE
                    ));
                }
            }

            log::debug("placement {}", static_cast<int>(placement));
            
            auto& dialog = m_fields->m_dialogLayer;
            if (dialog) dialog->removeFromParent();
            dialog = DialogLayer::createDialogLayer(
                dialogObjectsArr[0], dialogObjectsArr.inner(), 1
            );
            dialog->m_delegate = m_fields.self();
            dialog->updateChatPlacement(placement);
            dialog->animateInRandomSide();
            dialog->addToMainScene();

            if (p0) p0->pauseSchedulerAndActions();
        }
    }
};

#include <Geode/modify/GameObject.hpp>
class $modify(GameObjectDialogTriggerExt, GameObject) {
    static GameObject* createWithKey(int p0) {
        if (p0 == DIALOG_TRIGGER_ID) {//bg effect trigger sets
            auto object = typeinfo_cast<EffectGameObject*>(GameObject::createWithKey(1613));
            object->firstSetup();
            object->customSetup();

            object->initWithSpriteFrameName("edit_eEventLinkBtn_001.png");
			object->m_objectID = DIALOG_TRIGGER_ID;

            auto DialogTriggerDataNode = CCNode::create();
            object->setUserObject("DialogTriggerDataNode"_spr, DialogTriggerDataNode);

			return object;
		}
		return GameObject::createWithKey(p0);
    };
    static GameObject* objectFromVector(gd::vector<gd::string>& p0, gd::vector<void*>& p1, GJBaseGameLayer* p2, bool p3) {
		auto object = GameObject::objectFromVector(p0, p1, p2, p3);
        log::debug("p0 {}", p0);
        if (object->m_objectID == DIALOG_TRIGGER_ID) {
            auto effectObj = typeinfo_cast<EffectGameObject*>(object);

            auto changedObjects = exchangeMyCheatsToObjectID(effectObj);
            effectObj->customObjectSetup(p0, p1);
			exchangeMyCheatsToObjectID(effectObj, nullptr, changedObjects);

            auto DialogTriggerDataNode = typeinfo_cast<CCNode*>(
                effectObj->getUserObject("DialogTriggerDataNode"_spr)
            );
            if (DialogTriggerDataNode) {
                auto data = ZipUtils::base64URLDecode(p0[228].c_str());
                DialogTriggerDataNode->setID(data);
            };
        };
		return object;
    }
    virtual gd::string getSaveString(GJBaseGameLayer* p0) {
		auto str = GameObject::getSaveString(p0);
        if (auto data = typeinfo_cast<CCNode*>(this->getUserObject("DialogTriggerDataNode"_spr))) {
            str += ",228," + ZipUtils::base64URLEncode(data->getID().c_str());
        }
        return str;
    }
};

#include <Geode/modify/EditorUI.hpp>
class $modify(EditorUIDialogTriggerExt, EditorUI) {
    void editObject(cocos2d::CCObject * p0) {
        auto changedObjects = exchangeMyCheatsToObjectID(m_selectedObject, m_selectedObjects);
		EditorUI::editObject( p0);
        exchangeMyCheatsToObjectID(m_selectedObject, m_selectedObjects, changedObjects);
    }
    bool editButtonUsable() {
        auto changedObjects = exchangeMyCheatsToObjectID(m_selectedObject, m_selectedObjects);
        auto rtn = EditorUI::editButtonUsable();
        exchangeMyCheatsToObjectID(m_selectedObject, m_selectedObjects, changedObjects);
        return rtn;
    };
    void setupCreateMenu() {
        EditorUI::setupCreateMenu();
        //12
        if (auto tab = (EditButtonBar*)(this->m_createButtonBars->objectAtIndex(12))) {
            log::info("dialog trigger added {}", DIALOG_TRIGGER_ID);
            tab->m_buttonArray->addObject(this->getCreateBtn(DIALOG_TRIGGER_ID, 4));
            tab->reloadItems(
                GameManager::get()->getIntGameVariable("0049"),
                GameManager::get()->getIntGameVariable("0050")
            );
        }
    };
};

#include <Geode/modify/EditTriggersPopup.hpp>
class $modify(EditTriggersPopupDialogTriggerExt, EditTriggersPopup) {
    class EditDialogDataPopup : public Popup<TextInput*> {
    public:
        class Item : public CCNode {
        public:
            matjson::Value m_value;
            CREATE_FUNC(Item);
        };
        bool m_shouldUpdateStrings = false;
        Ref<TextInput> m_pTextInput;
        matjson::Value m_value;
        void stringsUpdateSch(float dt = 1337.f) {
            if (!m_shouldUpdateStrings and (dt != 1337.f)) return;
            m_shouldUpdateStrings = false;
            auto newStr = std::string();
            //...
            m_pTextInput->setString(newStr.c_str());
        }
        bool setup(TextInput*) override {
            this->setTitle("no impl for now... (in dev)");
            m_value = matjson::parse(m_pTextInput->getString()).unwrapOrDefault();
            for (auto item : m_value) {
				auto node = Item::create();
                node->m_value = item;
                if (item.isNumber()) {
                    node->addChild(CCLabelBMFont::create("char frame", "goldFont.fnt"));
                    node->setLayout(RowLayout::create());
                }
				if (item.isString()) {
					node->addChild(CCLabelBMFont::create("text", "goldFont.fnt"));
					node->setLayout(RowLayout::create());
				}
				this->addChild(node);
            }
            return true;
        }
        static EditDialogDataPopup* create(TextInput* textinput) {
            auto ret = new EditDialogDataPopup();
            ret->m_pTextInput = textinput; //ae
            if (ret->initAnchored(440.000f, 280.000f, textinput)) {
                ret->autorelease();
                return ret;
            }
            delete ret;
            return nullptr;
        }
    };
    bool init(EffectGameObject * p0, cocos2d::CCArray * p1) {
		if (!EditTriggersPopup::init(p0, p1)) return false;
        if (auto data = typeinfo_cast<CCNode*>(p0->getUserObject("DialogTriggerDataNode"_spr))) {
            if (auto title = this->getChildByType<CCLabelBMFont*>(0)) {
				title->setString("Edit Dialog Trigger");
                title->setAnchorPoint(CCPointMake(0.5f, 0.3f));
            }
            if (auto inf = this->m_buttonMenu->getChildByType<InfoAlertButton*>(0)) {
				inf->setVisible(false);
            }

            auto input = TextInput::create(312.f, "Dialog data string...");
            input->setFilter(" !\"#$ % &'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
            input->getInputNode()->m_allowedChars = " !\"#$ % &'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
            input->setString(data->getID());
            input->setPositionY(55.000f);
            input->setCallback(
                [data = Ref(data)](const std::string& p0) {
                    data->setID(p0);
                }
            );
            this->m_buttonMenu->addChild(input);

            auto editor = CCMenuItemExt::createSpriteExtra(
                ButtonSprite::create("Dialog Editor"), [input = Ref(input)](CCMenuItem*) {
                    EditDialogDataPopup::create(input)->show();
                    openInfoPopup(getMod());
                }
            );
            editor->setPosition(CCPointMake(100.f, 28.f));
            editor->setScale(0.55f);
            editor->m_baseScale = (0.55f);
            editor->m_scaleMultiplier = (1.0f + (0.61f - 0.55f));
            this->m_buttonMenu->addChild(editor);
		}
		return true;
    }
};