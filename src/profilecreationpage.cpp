#include "profilecreationpage.h"
#include "ui_profilecreationpage.h"
#include "libqutim/cryptoservice.h"
#include "libqutim/objectgenerator.h"
#include "libqutim/extensioninfo.h"
#include <QDebug>
#include <QStringBuilder>
#include <QCryptographicHash>
#include "libqutim/json.h"
#include "libqutim/systeminfo.h"
#include "libqutim/configbase_p.h"

using namespace qutim_sdk_0_3;
namespace qutim_sdk_0_3
{ LIBQUTIM_EXPORT QVector<QDir> *system_info_dirs(); }

namespace Core
{
ProfileCreationPage::ProfileCreationPage(const QString &password, QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::ProfileCreationPage),m_is_valid(false)
{
    ui->setupUi(this);
	m_password = password;
	registerField("name", ui->nameEdit);
	registerField("id", ui->idEdit);
	registerField("portable", ui->portableBox);
	registerField("configDir", ui->configEdit);
	registerField("historyDir", ui->historyEdit);
	registerField("dataDir", ui->dataEdit);
	registerField("crypto", ui->cryptoBox);
	bool first = true;
	foreach (const ObjectGenerator *gen, moduleGenerators<CryptoService>()) {
		const ExtensionInfo info = gen->info();
		ui->cryptoBox->addItem(info.icon(), info.name(), qVariantFromValue(info));
		if (first) {
			ui->cryptoBox->setCurrentIndex(0);
			ui->cryptoDescription->setText(info.description());
		}
	}
	foreach (const ObjectGenerator *gen, moduleGenerators<ConfigBackend>()) {
		const ExtensionInfo info = gen->info();
		ui->configBox->addItem(info.icon(), info.name(), qVariantFromValue(info));
		if (first) {
			ui->configBox->setCurrentIndex(0);
			ui->configDescription->setText(info.description());
		}
	}
}

ProfileCreationPage::~ProfileCreationPage()
{
    delete ui;
}

bool ProfileCreationPage::validatePage()
{
	//FIXME Elessar, WTF? why the generators are run on several times? 
	if (m_is_valid)
		return true; //dummy
	QDir dir;
	dir.mkpath(ui->configEdit->text());
	dir.mkpath(ui->historyEdit->text());
	dir.mkpath(ui->dataEdit->text());
	QVector<QDir> &systemDirs = *system_info_dirs();
	systemDirs[SystemInfo::ConfigDir] = QDir::cleanPath(ui->configEdit->text());
	systemDirs[SystemInfo::HistoryDir] = QDir::cleanPath(ui->historyEdit->text());
	systemDirs[SystemInfo::ShareDir] = QDir::cleanPath(ui->dataEdit->text());
	QFile file(QDir(ui->configEdit->text()).absoluteFilePath("profilehash"));
	if (!file.open(QIODevice::WriteOnly))
		return false;
	ExtensionInfo info = ui->cryptoBox->itemData(ui->cryptoBox->currentIndex()).value<ExtensionInfo>();
	CryptoService *service = info.generator()->generate<CryptoService>();
	QByteArray data;
	QVariant serviceData = service->generateData(ui->idEdit->text());
	{
		QDataStream out(&data, QIODevice::WriteOnly);
		// We shouldn't store password as is
		QByteArray passwordHash = QCryptographicHash::hash(m_password.toUtf8()
														   + "5667dd05fbe97bb238711a3af63",
														   QCryptographicHash::Sha1);
		out << ui->idEdit->text() << passwordHash
				<< QByteArray(service->metaObject()->className());
	}
	service->setPassword(m_password, serviceData);
	QVariant hash = service->crypt(data);
	file.write(hash.toByteArray());
	file.flush();
	file.close();
	m_password.clear();
	if (ui->portableBox->isChecked()) {
		dir = qApp->applicationDirPath();
	} else {
#if defined(Q_OS_WIN)
		dir = QString::fromLocal8Bit(qgetenv("APPDATA")) + "/qutim";
#elif defined(Q_OS_MAC)
		dir = QDir::homePath() + "/Library/Application Support/qutIM";
#elif defined(Q_OS_UNIX)
		dir = QDir::home().absoluteFilePath(".config/qutim");
#else
# Undefined OS
#endif
	}
	file.setFileName(dir.absoluteFilePath("profiles/profiles.json"));
	QVariantMap map;
	if (file.open(QFile::ReadOnly))
		map = Json::parse(file.readAll()).toMap();
	file.close();
	QVariantList profiles = map.value("list").toList();
	info = ui->configBox->itemData(ui->configBox->currentIndex()).value<ExtensionInfo>();
	for (int i = 0; i < ui->configBox->count(); i++) {
		ExtensionInfo extInfo = ui->configBox->itemData(i).value<ExtensionInfo>();
		ConfigBackend *backend = extInfo.generator()->generate<ConfigBackend>();
		ConfigBackendInfo back = ConfigBackendInfo(metaInfo(backend->metaObject(),
															"Extension"), backend);
		if (i == ui->configBox->currentIndex())
			ConfigPrivate::config_backends.prepend(back);
		else
			ConfigPrivate::config_backends.append(back);
	}
	{
		QVariantMap profile;
		profile.insert("name", ui->nameEdit->text());
		profile.insert("id", ui->idEdit->text());
		profile.insert("crypto", QLatin1String(service->metaObject()->className()));
		profile.insert("config", QLatin1String(ConfigPrivate::config_backends.first()
											   .second->metaObject()->className()));
		profile.insert("portable", ui->portableBox->isChecked());
		if (ui->portableBox->isChecked()) {
			QDir app = qApp->applicationDirPath();
			profile.insert("configDir", app.relativeFilePath(SystemInfo::getPath(SystemInfo::ConfigDir)));
			profile.insert("historyDir", app.relativeFilePath(SystemInfo::getPath(SystemInfo::HistoryDir)));
			profile.insert("shareDir", app.relativeFilePath(SystemInfo::getPath(SystemInfo::ShareDir)));
		} else {
			profile.insert("configDir", SystemInfo::getPath(SystemInfo::ConfigDir));
			profile.insert("historyDir", SystemInfo::getPath(SystemInfo::HistoryDir));
			profile.insert("shareDir", SystemInfo::getPath(SystemInfo::ShareDir));
		}
		profiles.append(profile);
	}
	map.insert("list", profiles);
	if (!map.value("current").isValid()) {
		map.insert("current", ui->idEdit->text());
	}
	file.open(QFile::WriteOnly);
	file.write(Json::generate(map));
	file.flush();
	file.close();
	m_is_valid = true;
	return true;
}

void ProfileCreationPage::on_portableBox_toggled(bool)
{
	rebaseDirs();
}

void ProfileCreationPage::on_idEdit_textChanged(const QString &)
{
	rebaseDirs();
}

void ProfileCreationPage::on_cryptoBox_currentIndexChanged(int index)
{
	ExtensionInfo info = ui->cryptoBox->itemData(index).value<ExtensionInfo>();
	ui->cryptoDescription->setText(info.description());
}

void ProfileCreationPage::on_configBox_currentIndexChanged(int index)
{
	ExtensionInfo info = ui->configBox->itemData(index).value<ExtensionInfo>();
	ui->configDescription->setText(info.description());
}

void ProfileCreationPage::rebaseDirs()
{
	if (ui->portableBox->isChecked()) {
		QDir dir = qApp->applicationDirPath();
		ui->dataEdit->setText(dir.absoluteFilePath("share"));
		dir = QDir::cleanPath(dir.absolutePath() % "/profiles/" % ui->idEdit->text());
		ui->configEdit->setText(dir.absoluteFilePath("config"));
		ui->historyEdit->setText(dir.absoluteFilePath("history"));
	} else {
#if defined(Q_OS_WIN)
		QDir dir = QString::fromLocal8Bit(qgetenv("APPDATA"));
		ui->dataEdit->setText(dir.absolutePath() % "/qutim/share/");
		dir = QDir::cleanPath(dir.absolutePath() % "/qutim/profiles/" % ui->idEdit->text());
		ui->configEdit->setText(dir.absoluteFilePath("config"));
		ui->historyEdit->setText(dir.absoluteFilePath("history"));
#elif defined(Q_OS_MAC)
		QDir dir = QDir::homePath() + "/Library/Application Support/qutIM";
		ui->dataEdit->setText(dir.absoluteFilePath("share"));
		dir = QDir::cleanPath(dir.absolutePath() % "/profiles/" % ui->idEdit->text());
		ui->configEdit->setText(dir.absoluteFilePath("config"));
		ui->historyEdit->setText(dir.absoluteFilePath("history"));
#elif defined(Q_OS_UNIX)
		QDir dir = QDir::home();
		ui->dataEdit->setText(dir.absoluteFilePath(".local/share/qutim"));
		dir = dir.absoluteFilePath(".config/qutim/profiles/" + ui->idEdit->text());
		ui->configEdit->setText(dir.absoluteFilePath("config"));
		ui->historyEdit->setText(dir.absoluteFilePath("history"));
#else
# error Strange os.. yeah..
#endif
	}
}

void ProfileCreationPage::changeEvent(QEvent *e)
{
    QWizardPage::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
}
