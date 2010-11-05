#ifndef CONFERENCECONTACTSVIEW_H
#define CONFERENCECONTACTSVIEW_H

#include <QListView>
#include <chatlayerplugin.h>

namespace Core
{
namespace AdiumChat
{

class ChatSessionImpl;
class ADIUMCHAT_EXPORT ConferenceContactsView : public QListView
{
    Q_OBJECT
public:
    explicit ConferenceContactsView(QWidget *parent = 0);
	void setSession(ChatSessionImpl *session);
protected:
	virtual bool event(QEvent *event);
private slots:
	void onActivated(const QModelIndex &);
	void onBuddiesChanged();
private:
	ChatSessionImpl *m_session;
};

}
}
#endif // CONFERENCECONTACTSVIEW_H
