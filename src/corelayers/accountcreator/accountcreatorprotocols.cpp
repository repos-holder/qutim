#include "accountcreatorprotocols.h"
#include "accountcreatorwizard.h"
#include "libqutim/extensioninfo.h"
#include "libqutim/icon.h"
#include "ui_accountcreatorprotocols.h"
#include <QPushButton>
#include <QScrollBar>

namespace Core
{
	AccountCreatorProtocols::AccountCreatorProtocols(AccountCreatorWizard *parent) :
		QWizardPage(parent),
		m_ui(new Ui::AccountCreatorProtocols),
		m_wizard(parent)
	{
		m_ui->setupUi(this);
		
		connect(m_ui->protocolList->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(sliderMoved(int)));

		m_lastId = Id;
		QSet<QByteArray> protocols;
		foreach (Protocol *protocol, allProtocols()) {
			protocols.insert(protocol->metaObject()->className());
		}

		foreach(const ObjectGenerator *gen, moduleGenerators<AccountCreationWizard>()) {
			const char *proto = metaInfo(gen->metaObject(), "DependsOn");
			if (!protocols.contains(proto))
				continue;
			AccountCreationWizard *wizard = gen->generate<AccountCreationWizard>();
			m_wizards.insert(wizard->info().name(), wizard);
		}

		m_ui->upButton->setIcon(Icon("arrow-up"));
		m_ui->downButton->setIcon(Icon("arrow-down"));

		int h = (height() - 180 < height() - 120 ? 180 : 120);
		m_ui->protocolList->setGridSize(QSize(0, 60));
		m_ui->protocolList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		m_ui->protocolList->setFrameStyle(QFrame::NoFrame);
		m_ui->protocolList->setMinimumSize(m_ui->protocolList->minimumSize().width(), h);
		m_ui->protocolList->setMaximumSize(m_ui->protocolList->maximumSize().width(), h);
		m_ui->upButton->setDisabled(true);

		foreach (AccountCreationWizard *wizard, m_wizards) {
			ExtensionInfo info = wizard->info();
			QIcon icon = info.icon();
			if (icon.isNull())
				icon = Icon(QLatin1String("im-") + info.name().original().toLower());

			QListWidgetItem *item = new QListWidgetItem(m_ui->protocolList);
			item->setData(Qt::UserRole + 1,reinterpret_cast<qptrdiff>(wizard));
			item->setData(Qt::UserRole + 2,qVariantFromValue(info));
			item->setFlags(Qt::NoItemFlags);
			item->setSizeHint(QSize( 0, 60 ));

			QPushButton *b = new QPushButton(icon, info.name());
			connect(b, SIGNAL(clicked()), this, SLOT(protocolSelected()));
			b->setMinimumSize(b->minimumSize().width(), 60);
			b->setFocusPolicy(Qt::ClickFocus);
			b->setToolTip(info.description());
			m_ui->protocolList->setItemWidget(item, b);

			m_items.insert(b, item);
		}
		setTitle(tr("Select protocol"));
	}

	AccountCreatorProtocols::~AccountCreatorProtocols()
	{
		delete m_ui;
		qDeleteAll(m_wizards);
	}

	bool AccountCreatorProtocols::validatePage()
	{
		return true;
	}

	bool AccountCreatorProtocols::isComplete() const
	{
		return false;
	}

	int AccountCreatorProtocols::nextId() const
	{
		if (m_ui->protocolList->count() == 0)
			return -1;
		QMap<AccountCreationWizard *, int>::iterator it
				= const_cast<AccountCreatorProtocols *>(this)->ensureCurrentProtocol();
		return (it == m_wizardIds.end()  || it.value() == -1) ? m_lastId + 1 : it.value();
	}

	QMap<AccountCreationWizard *, int>::iterator AccountCreatorProtocols::ensureCurrentProtocol()
	{
		QListWidgetItem *item = m_ui->protocolList->currentItem();

		if (!item)
			return m_wizardIds.end();

		qptrdiff wizardPtr = item->data(Qt::UserRole + 1).value<qptrdiff>();
		AccountCreationWizard *wizard = reinterpret_cast<AccountCreationWizard *>(wizardPtr);
		if (!wizard)
			return m_wizardIds.end();

		QMap<AccountCreationWizard *, int>::iterator it = m_wizardIds.find(wizard);
		if (it == m_wizardIds.end()) {
			QList<QWizardPage *> pages = wizard->createPages(m_wizard);
			if (pages.isEmpty()) {
				it = m_wizardIds.insert(wizard, -1);
			} else {
				it = m_wizardIds.insert(wizard, m_lastId + 1);
				for (int i = 0; i < pages.size(); i++)
					m_wizard->setPage(++m_lastId, pages.at(i));
			}
		}
		return it;
	}

	void AccountCreatorProtocols::changeEvent(QEvent *e)
	{
		QWizardPage::changeEvent(e);
		switch (e->type()) {
			case QEvent::LanguageChange:
				m_ui->retranslateUi(this);
				break;
			default:
				break;
		}
	}

	void AccountCreatorProtocols::protocolSelected()
	{
		m_ui->protocolList->setCurrentItem(m_items.value(qobject_cast<QPushButton *>(sender())));
		wizard()->next();
	}

	void AccountCreatorProtocols::on_upButton_clicked()
	{
		m_ui->protocolList->verticalScrollBar()->setValue(m_ui->protocolList->verticalScrollBar()->value() - 1);
	}

	void AccountCreatorProtocols::on_downButton_clicked()
	{
		m_ui->protocolList->verticalScrollBar()->setValue(m_ui->protocolList->verticalScrollBar()->value() + 1);
	}

	void AccountCreatorProtocols::sliderMoved(int val)
	{
		if (val == m_ui->protocolList->verticalScrollBar()->minimum())
		{
			m_ui->upButton->setDisabled(true);
			m_ui->downButton->setDisabled(false);
		}
		else if (val == m_ui->protocolList->verticalScrollBar()->maximum())
		{
			m_ui->upButton->setDisabled(false);
			m_ui->downButton->setDisabled(true);
		}
		else
		{
			m_ui->upButton->setDisabled(false);
			m_ui->downButton->setDisabled(false);
		}
	}
}
