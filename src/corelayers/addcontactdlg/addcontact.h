#ifndef ADDCONTACT_H
#define ADDCONTACT_H
#include <QScopedPointer>
#include <QDialog>
#include <libqutim/account.h>

namespace Core {
	using namespace qutim_sdk_0_3;
	struct AddContactPrivate;

	class AddContact : public QDialog
	{
		Q_OBJECT
		Q_DECLARE_PRIVATE(AddContact)
		public:
			AddContact(Account *account = 0, QWidget *parent = 0);
			~AddContact();
		protected:
			void setAccount(Account *account);
		private slots:
			void on_okButton_clicked();
			void on_cancelButton_clicked();
			void setAccount();
		private:
			QScopedPointer<AddContactPrivate> d_ptr;
	};

	class AddContactModule : public QObject
	{
		Q_OBJECT
		Q_CLASSINFO("Service", "AddContact")
		Q_CLASSINFO("Uses", "IconLoader")
		Q_CLASSINFO("Uses", "ContactList")
		public:
   		AddContactModule();
		private slots:
			void show();
	};
}
#endif // ADDCONTACT_H
