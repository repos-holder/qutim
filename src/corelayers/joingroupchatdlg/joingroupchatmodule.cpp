#include "joingroupchatmodule.h"
#include "modulemanagerimpl.h"
#include <libqutim/icon.h>
#include <libqutim/menucontroller.h>
#include "joingroupchat.h"
#include <libqutim/account.h>
#include <libqutim/protocol.h>

namespace Core
{
	static Core::CoreModuleHelper<JoinGroupChatModule> join_groupchat_module_static(
			QT_TRANSLATE_NOOP("Plugin", "Join GroupChat dialog"),
			QT_TRANSLATE_NOOP("Plugin", "Simple groupchat join dialog")
			);


	bool isSupportGroupchat()
	{
		foreach (Protocol *p,allProtocols()) {
			bool support = p->data(qutim_sdk_0_3::Protocol::ProtocolSupportGroupChat).toBool();
			if (support) {
				foreach (Account *a,p->accounts()) {
					if (a->status() != Status::Offline) {
						return true;
					}
				}
			}
		}
		return false;
	}
	
	JoinGroupChatModule::JoinGroupChatModule()
	{
		QObject *contactList = getService("ContactList");
		if (contactList) {
			MenuController *controller = qobject_cast<MenuController*>(contactList);
			Q_ASSERT(controller);
			static QScopedPointer<ActionGenerator> button(new JoinGroupChatGenerator(this));
			controller->addAction(button.data());
		}
	}

	JoinGroupChatModule::~JoinGroupChatModule()
	{

	}

	void JoinGroupChatModule::onJoinGroupChatTriggered()
	{
		if (!m_chat)
			m_chat = new JoinGroupChat();
		
#if defined (QUTIM_MOBILE_UI)
		m_chat->showMaximized();
#else
		m_chat->show();
		centerizeWidget(m_chat);
#endif
	}

	JoinGroupChatGenerator::JoinGroupChatGenerator(QObject* module):
			ActionGenerator(Icon("meeting-attending"),
							QT_TRANSLATE_NOOP("JoinGroupChat", "Join groupchat"),
							module,
							SLOT(onJoinGroupChatTriggered())
							)
	{

	}

	void JoinGroupChatGenerator::showImpl(QAction *action, QObject *)
	{
		action->setEnabled(isSupportGroupchat());
	}
}
