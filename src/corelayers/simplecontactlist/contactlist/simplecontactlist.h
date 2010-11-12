#ifndef SIMPLECONTACTLIST_H
#define SIMPLECONTACTLIST_H

#include <qutim/actiontoolbar.h>
#include <qutim/status.h>
#include <qutim/contact.h>
#include "abstractcontactlist.h"

using namespace qutim_sdk_0_3;

namespace Core
{
	namespace SimpleContactList
	{
		struct ModulePrivate;

		class Module : public AbstractContactList
		{
			Q_OBJECT
		public:
			Module();
			virtual ~Module();			
			virtual void addButton(ActionGenerator *generator);
			virtual QWidget *widget();
		protected:
			bool event(QEvent *);
			bool eventFilter(QObject *, QEvent *);
		public slots:
			void show();
			void hide();
			void changeVisibility();
			void addContact(qutim_sdk_0_3::Contact *contact);
		private slots:
			void onConfigureClicked(QObject*);
			void onAccountCreated(qutim_sdk_0_3::Account *account);
			void onAccountStatusChanged(const qutim_sdk_0_3::Status &status);
			void onAccountDestroyed(QObject *obj);
			void onStatusChanged();
			void onSearchButtonToggled(bool toggled);
			void showStatusDialog();
			void changeStatusTextAccepted();
			void onCopyIdTriggered(QObject *obj);
			void onQuitTriggered(QObject*);
		private:
			QAction *createGlobalStatusAction(Status::Type type);
			QScopedPointer<ModulePrivate> p;
		};
	}
}

#endif // SIMPLECONTACTLIST_H
