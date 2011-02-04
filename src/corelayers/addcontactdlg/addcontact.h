#ifndef ADDCONTACT_H
#define ADDCONTACT_H
#include <QScopedPointer>
#include <QDialog>
#include <qutim/account.h>
#include <qutim/status.h>

namespace Core {
	using namespace qutim_sdk_0_3;
	class AddContactPrivate;

	class AddContact : public QDialog
	{
		Q_OBJECT
		Q_DECLARE_PRIVATE(AddContact)
	public:
				AddContact(Account *account = 0, QWidget *parent = 0);
		~AddContact();
	protected:
		void setAccount(Account *account);
		void changeState(Account *account, const qutim_sdk_0_3::Status &status);
	private slots:
		void on_okButton_clicked();
		void on_cancelButton_clicked();
		void onStartChatClicked();
		void onShowInfoClicked();
		void setAccount();
		void changeState(const qutim_sdk_0_3::Status &status);
		void currentChanged(int index);
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
		~AddContactModule();
	protected:
		bool event(QEvent *ev);
	private slots:
		void show();
		void onAccountStatusChanged(qutim_sdk_0_3::Status);
		void onContactAddRemoveAction(QObject*);
	private:
		QScopedPointer<ActionGenerator> m_addRemoveGen;
		QScopedPointer<ActionGenerator> m_addUserGen;
	};
}
#endif // ADDCONTACT_H
