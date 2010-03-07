#ifndef CHATSESSIONMODEL_H
#define CHATSESSIONMODEL_H

#include <QAbstractListModel>
#include "chatsessionimpl.h"

namespace AdiumChat
{
using namespace qutim_sdk_0_3;

class ChatSessionModel : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit ChatSessionModel(ChatSessionImpl *parent = 0);
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	void addContact(qutim_sdk_0_3::Buddy *c);
	void removeContact(qutim_sdk_0_3::Buddy *c);
private slots:
	void onNameChanged(const QString &name);
	void onStatusChanged(const qutim_sdk_0_3::Status &status);
private:
	QList<Buddy*> m_units;
};
}

#endif // CHATSESSIONMODEL_H
