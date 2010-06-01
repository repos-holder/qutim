#include "captchasender.h"
#include <qutim/messagesession.h>
#include <qutim/configbase.h>
#include <qutim/NotificationsLayer>

namespace Antispam
{
	
	CaptchaSender::CaptchaSender(QObject* parent): QObject(parent)
	{

	}
	
	void CaptchaSender::sendQuestion(qutim_sdk_0_3::Message* message)
	{
		QString question = Config("plugins").group("antispam").value<QString>("question",QString()); //TODO
		QStringList answers = Config("plugins").group("antispam").value<QStringList>("answers",QStringList()); //TODO
		ChatUnit *unit = const_cast<ChatUnit *>(message->chatUnit());
		ChatSession *session = ChatLayer::get(unit,false);
		session->setProperty("question",question);
		session->setProperty("answers",answers);
		unit->sendMessage(question);
	}

	void CaptchaSender::checkAnswer(qutim_sdk_0_3::Message* message)
	{
		ChatUnit *unit = const_cast<ChatUnit *>(message->chatUnit());
		ChatSession *session = ChatLayer::get(unit,false);
		
		QStringList right_answers = session->property("answers").toStringList(); 
		QString answer = message->text();
		foreach (QString right_answer, right_answers) {
			if (answer.contains(right_answer)) {
				session->setProperty("suspicious",false);
				return;
			}
		}
		message->setProperty("spam",true);
		Notifications::sendNotification(qutim_sdk_0_3::Notifications::BlockedMessage,
										session
										);
	}

	void CaptchaSender::messageReceived(qutim_sdk_0_3::Message* message)
	{
		ChatUnit *unit = const_cast<ChatUnit *>(message->chatUnit());
		ChatSession *session = ChatLayer::get(unit,false);
		Q_ASSERT(session);		
		Contact *c = qobject_cast<Contact *>(unit);
		if (!session->property("suspicious").toBool() || !c || c->isInList())
			return;
		
		session->setProperty("suspicious",true);
		message->setProperty("hide",true);
		
		if (session->property("question").isNull()) {
			sendQuestion(message);
		}
		else {
			checkAnswer(message);
		}
	}

}