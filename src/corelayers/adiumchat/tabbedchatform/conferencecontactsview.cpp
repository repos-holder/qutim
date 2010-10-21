#include "conferencecontactsview.h"
#include "contactdelegate.h"
#include <chatlayer/chatsessionimpl.h>
#include <qutim/conference.h>
#include <qutim/mimeobjectdata.h>
#include <QDropEvent>

namespace Core
{
namespace AdiumChat
{
using namespace qutim_sdk_0_3;

ConferenceContactsView::ConferenceContactsView(QWidget *parent) :
    QListView(parent)
{
	setItemDelegate(new ContactDelegate(this));
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	setSizePolicy(sizePolicy);
	setAcceptDrops(true);
}

void ConferenceContactsView::setSession(ChatSessionImpl *session)
{
	m_session = session;
	setModel(session->getModel());
	bool isContactsViewVisible = session->getModel()->rowCount(QModelIndex()) > 0;
	if(qobject_cast<Conference *>(session->getUnit()))
		isContactsViewVisible = true;

	setVisible(isContactsViewVisible);
}

bool ConferenceContactsView::event(QEvent *event)
{
	if (event->type() == QEvent::ContextMenu) {
		QContextMenuEvent *menuEvent = static_cast<QContextMenuEvent*>(event);
		QModelIndex index = indexAt(menuEvent->pos());
		Buddy *buddy = index.data(Qt::UserRole).value<Buddy*>();
		if (buddy)
			buddy->showMenu(menuEvent->globalPos());
		return true;
	} else if (event->type() == QEvent::DragEnter) {
		QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent*>(event);
		if (const MimeObjectData *data = qobject_cast<const MimeObjectData*>(dragEvent->mimeData())) {
			Contact *contact = qobject_cast<Contact*>(data->object());
			Conference *conf = qobject_cast<Conference*>(m_session->getUnit());
			if (contact && conf && contact->account() == conf->account())
				dragEvent->acceptProposedAction();
		}
		return true;
	} else if (event->type() == QEvent::Drop) {
		QDropEvent *dropEvent = static_cast<QDropEvent*>(event);
		if (const MimeObjectData *mimeData
				= qobject_cast<const MimeObjectData*>(dropEvent->mimeData())) {
			if (Contact *contact = qobject_cast<Contact*>(mimeData->object())) {
				ChatUnit *unit = m_session->getUnit();
				if (Conference *conf = qobject_cast<Conference*>(unit))
					conf->invite(contact);
				dropEvent->setDropAction(Qt::CopyAction);
				dropEvent->accept();
				return true;
			}
		}
	}
	return QListView::event(event);
}

}
}
