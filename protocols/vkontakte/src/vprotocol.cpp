#include "vprotocol.h"
#include "vcontact.h"
#include "vaccount.h"
#include "ui/vaccountsettings.h"

#include <qutim/status.h>
#include <qutim/statusactiongenerator.h>
#include <qutim/icon.h>

#include <QDesktopServices>
#include <QUrl>

using namespace qutim_sdk_0_3;

VProtocol::VProtocol() :
    qutim_sdk_0_3::Protocol()
{
}

VProtocol::~VProtocol()
{
    Settings::removeItem(m_mainSettings.data());
	foreach (VAccount *account, m_accounts)
        account->saveSettings();
}

qutim_sdk_0_3::Account *VProtocol::account(const QString &email) const
{
    return m_accounts.value(email);
}

void VProtocol::loadAccounts()
{
    QList<Status> statuses;
    statuses << Status(Status::Online)
             << Status(Status::Offline)
             << Status(Status::Invisible);
    foreach (Status status, statuses) {
        status.initIcon("vkontakte");
        Status::remember(status, "vkontakte");
		MenuController::addAction(new StatusActionGenerator(status), &VAccount::staticMetaObject);
    }

    ActionGenerator *gen = new ActionGenerator(Icon("applications-internet"),
                               QT_TRANSLATE_NOOP("Vkontakte", "Open homepage"),
                               this,
                               SLOT(onWebPageTriggered(QObject*)));
    gen->setType(ActionTypeContactList);
    MenuController::addAction<VContact>(gen);

    QStringList accounts = config("general").value("accounts", QStringList());
    foreach (const QString &email, accounts) {
		VAccount *account = new VAccount(email, this);
        m_accounts.insert(email, account);
        account->loadSettings();
        connect(account, SIGNAL(destroyed(QObject*)), this, SLOT(onAccountDestroyed(QObject*)));
        emit accountCreated(account);
    }

    m_mainSettings.reset(new GeneralSettingsItem<VAccountSettings>(Settings::Protocol,
                                                                   Icon("im-vkontakte"),
                                                                   QT_TRANSLATE_NOOP("Vkontakte", "Account settings")));
	Settings::registerItem<VAccount>(m_mainSettings.data());
}

QList<qutim_sdk_0_3::Account *> VProtocol::accounts() const
{
    AccountList list;
	foreach (VAccount *account, m_accounts)
        list.append(account);
    return list;
}

QVariant VProtocol::data(qutim_sdk_0_3::Protocol::DataType type)
{
    switch (type) {
        case ProtocolIdName:
            return tr("id");
        case ProtocolContainsContacts:
            return true;
        default:
            return QVariant();
	}
}

VProtocol *VProtocol::instance()
{
	static VProtocol pointer;
	return &pointer;
}

void VProtocol::addAccount(VAccount *account)
{
	m_accounts.insert(account->email(), account);
    emit accountCreated(account);
}

Account *VProtocol::doCreateAccount(const QString &email, const QVariantMap &parameters)
{
	const QString password = parameters.value("password").toString();
	const bool savePassword = parameters.value("savePassword", false).toBool();

    VAccount *account = new VAccount(email, this);
	if (savePassword) {
		Config cfg = account->config();
		cfg.beginGroup("general");
		cfg.setValue("passwd", password, Config::Crypted);
	}
	Config cfg = config();
	cfg.beginGroup("general");
	QStringList accounts = cfg.value("accounts", QStringList());
	accounts << account->id();
	cfg.setValue("accounts", accounts);
	addAccount(account);
    return account;
}

void VProtocol::virtual_hook(int id, void *data)
{
    switch (id) {
	case CreateAccountHook: {
		CreateAccountArgument &argument = *reinterpret_cast<CreateAccountArgument*>(data);
		argument.account = doCreateAccount(argument.id, argument.parameters);
		break;
	}
	default:
		Protocol::virtual_hook(id, data);
	}
}

void VProtocol::onWebPageTriggered(QObject *obj)
{
    VContact *contact = qobject_cast<VContact*>(obj);
    Q_ASSERT(obj);
    QUrl url ("http://vkontakte.ru/id" + contact->id());
    QDesktopServices::openUrl(url);
}

void VProtocol::onAccountDestroyed(QObject *obj)
{
	VAccount *account = static_cast<VAccount*>(obj);
    m_accounts.remove(m_accounts.key(account));
}
