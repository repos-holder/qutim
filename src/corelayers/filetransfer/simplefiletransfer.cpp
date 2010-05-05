#include "simplefiletransfer.h"
#include "filetransferdialog.h"
#include "src/modulemanagerimpl.h"
#include "libqutim/actiongenerator.h"
#include "libqutim/menucontroller.h"
#include "libqutim/icon.h"
#include "libqutim/buddy.h"
#include <QFileDialog>

namespace Core
{

static Core::CoreModuleHelper<SimpleFileTransfer> filetransfer_static(
	QT_TRANSLATE_NOOP("Plugin", "File transfer manager"),
	QT_TRANSLATE_NOOP("Plugin", "Default qutIM file transfer manager")
);

class FileTransferActionGenerator : public ActionGenerator
{
public:
	FileTransferActionGenerator(QObject *receiver) :
			ActionGenerator(Icon("document-save"), QT_TRANSLATE_NOOP("FileTransfer", "Send file"),
							receiver, SLOT(onSendFile())) {}
protected:
	virtual QObject *generateHelper() const
	{
		ChatUnit *buddy = qobject_cast<ChatUnit*>(const_cast<FileTransferActionGenerator*>(this)->controller());
		if (buddy && FileTransferManager::instance()->checkAbility(buddy))
			return ActionGenerator::generateHelper();
		return 0;
	}
};

SimpleFileTransfer::SimpleFileTransfer()
{
	static bool init = false;
	if (!init) {
		MenuController::addAction<ChatUnit>(new FileTransferActionGenerator(this));
		init = true;
	}
}

void SimpleFileTransfer::send(ChatUnit *unit, const QStringList &files)
{
	FileTransferEngine *engine = getEngine(unit);
	if (!engine)
		return;
	engine->setFiles(files);
	engine->start();
	FileTransferDialog *dialog = new FileTransferDialog(engine);
	dialog->show();
}

void SimpleFileTransfer::receive(FileTransferEngine *engine)
{
	QString path;
	if (engine->remoteFiles().count() == 1) {
		path = QFileDialog::getSaveFileName(0, QString(), QDir::homePath() + "/" + engine->remoteFiles().first());
	} else {
		path = QFileDialog::getExistingDirectory(0, QString(), QDir::homePath());
	}
	if (!path.isEmpty()) {
		FileTransferDialog *dialog = new FileTransferDialog(engine);
		dialog->setPath(path);
		dialog->show();
	} else {
		engine->cancel();
	}
}

void SimpleFileTransfer::onSendFile()
{
	Q_ASSERT(qobject_cast<QAction*>(sender()));
	QAction *action = reinterpret_cast<QAction *>(sender());
	Q_ASSERT(action);
	ChatUnit *unit = qobject_cast<ChatUnit*>(action->data().value<MenuController*>());
	Q_ASSERT(unit);
	QStringList files = QFileDialog::getOpenFileNames(0, QString(), QDir::homePath());
	if (!files.isEmpty()) {
		send(unit, files);
	}
}

}
