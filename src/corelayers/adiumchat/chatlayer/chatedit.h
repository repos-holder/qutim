#ifndef CHATEDIT_H
#define CHATEDIT_H

#include <QPlainTextEdit>
#include <QPointer>
#include <chatforms/abstractchatwidget.h>

namespace qutim_sdk_0_3
{

}

namespace Core
{
namespace AdiumChat
{

enum SendMessageKey
{
	SendEnter = 0,
	SendCtrlEnter,
	SendDoubleEnter
};

using namespace qutim_sdk_0_3;
class ChatSessionImpl;
class ADIUMCHAT_EXPORT ChatEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit ChatEdit(QWidget *parent = 0);
	void setSession(ChatSessionImpl *session);
	void setSendKey(SendMessageKey key);
public slots:
	void send();
protected:
	bool event(QEvent *e);
	QString textEditToPlainText();
protected slots:
	void onTextChanged();
private:
	QPointer<ChatSessionImpl> m_session;
	int m_entersCount;
	SendMessageKey m_sendKey;
	QTextCursor m_enterPosition;
};

}
}

#endif // CHATEDIT_H
