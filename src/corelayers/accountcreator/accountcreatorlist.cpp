#include "accountcreatorlist.h"
#include "libqutim/icon.h"
#include "libqutim/protocol.h"
#include "libqutim/account.h"
#include "ui_accountcreatorlist.h"
#include <QListWidgetItem>
#include <QContextMenuEvent>
#include "itemdelegate.h"

namespace Core
{
	AccountCreatorList::AccountCreatorList() :
			SettingsWidget(),
			ui(new Ui::AccountCreatorList)
	{
		ui->setupUi(this);

		ui->listWidget->installEventFilter(this);
		ItemDelegate *delegate = new ItemDelegate(this);
		delegate->setCommandLinkStyle();
		ui->listWidget->setItemDelegate(delegate);

		connect(ui->listWidget,
				SIGNAL(itemActivated(QListWidgetItem*)),
				SLOT(listViewClicked(QListWidgetItem*))
				);
		QListWidgetItem *addItem = new QListWidgetItem(ui->listWidget);
		addItem->setText(QT_TRANSLATE_NOOP("Account","Add new account"));
		addItem->setToolTip(QT_TRANSLATE_NOOP("Account","Just add or create new account"));
		addItem->setData(DescriptionRole,addItem->toolTip());
		addItem->setIcon(Icon("list-add-user"));

		addItem = new QListWidgetItem(ui->listWidget);
		addItem->setText(QT_TRANSLATE_NOOP("Account","Accounts"));
		addItem->setData(SeparatorRole,true);

		foreach(Protocol *protocol, allProtocols())
		{
			connect(protocol,SIGNAL(accountCreated(qutim_sdk_0_3::Account*)),SLOT(addAccount(qutim_sdk_0_3::Account*)));
			foreach(Account *account, protocol->accounts())
			{
				addAccount(account);
			}
		}

	}

	AccountCreatorList::~AccountCreatorList()
	{
		delete ui;
	}

	void AccountCreatorList::loadImpl()
	{
	}

	void AccountCreatorList::saveImpl()
	{
	}

	void AccountCreatorList::cancelImpl()
	{
	}

	void AccountCreatorList::onWizardDestroyed()
	{
		QWidget *parent = window();
		if (parent)
			window()->setEnabled(true);
	}

	void AccountCreatorList::changeEvent(QEvent *e)
	{
		QWidget::changeEvent(e);
		switch (e->type())
		{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
		}
	}

	void AccountCreatorList::addAccount(qutim_sdk_0_3::Account *account)
	{
		Icon protoIcon(QLatin1String("im-user") + account->protocol()->id()); //FIXME wtf?

//		if (protoIcon.isNull())
			protoIcon = Icon("applications-internet");

		QListWidgetItem *accountItem = new QListWidgetItem(ui->listWidget);
		accountItem->setText(account->name());
		accountItem->setToolTip(account->name());
		accountItem->setIcon(protoIcon);
		accountItem->setData(Qt::UserRole,qVariantFromValue<Account *>(account));

		QVariantMap fields;
		QString id = account->protocol()->data(Protocol::ProtocolIdName).toString();
		fields.insert(id,account->id());
		accountItem->setData(DescriptionRole,fields);
	}

	bool AccountCreatorList::eventFilter(QObject *obj, QEvent *ev)
	{
		if (ev->type() ==  QEvent::ContextMenu) {
			if (QListWidget *widget = qobject_cast<QListWidget *>(obj)) {
				QContextMenuEvent *event = static_cast<QContextMenuEvent*>(ev);
				QModelIndex index = widget->indexAt(event->pos());
				Account *account = index.data(Qt::UserRole).value<Account *>();
				if (account) {
					QMenu *menu = new QMenu();
					menu->setAttribute(Qt::WA_DeleteOnClose,true);
					QAction *act = new QAction(menu);
					act->setText(tr("Edit info"));
					act->setIcon(Icon("document-properties"));
					menu->addAction(act);

					act = new QAction(menu);
					act->setText(tr("Remove account"));
					act->setIcon(Icon("list-remove-user"));
					menu->addAction(act);
					menu->popup(QCursor::pos());
				}
			}
		}
		return SettingsWidget::eventFilter(obj,ev);
	}

	void AccountCreatorList::listViewClicked(QListWidgetItem *item)
	{
		if (item->data(SeparatorRole).toBool() || item->data(Qt::UserRole).value<Account *>())
			return;

		if (!m_wizard.isNull())
			return;
		QWidget *parent = window();
		if (parent)
			window()->setEnabled(false);
		AccountCreatorWizard *wizard = new AccountCreatorWizard();
		connect(wizard,SIGNAL(destroyed()),SLOT(onWizardDestroyed()));
#if defined(Q_OS_SYMBIAN)
		wizard->showMaximized();
#else
		wizard->show();
#endif
	}
}
