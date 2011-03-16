/****************************************************************************
 *  simplefiletransfer.h
 *
 *  Copyright (c) 2011 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *                        Prokhin Alexey <alexey.prokhin@yandex.ru>
 *
 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*****************************************************************************/

#ifndef SIMPLEFILETRANSFER_H
#define SIMPLEFILETRANSFER_H

#include <QPointer>
#include <qutim/filetransfer.h>
#include "filetransferdialog.h"
#include "filetransferjobmodel.h"

using namespace qutim_sdk_0_3;
namespace Core
{

class SimpleFileTransfer : public FileTransferManager
{
	Q_OBJECT
	Q_CLASSINFO("Uses", "IconLoader")
public:
	explicit SimpleFileTransfer();
	virtual QIODevice *doOpenFile(FileTransferJob *job);
	virtual void handleJob(FileTransferJob *job, FileTransferJob *oldJob);
	bool event(QEvent *ev);
private slots:
	void openFileTransferDialog();
	void onSendFile(QObject *controller);
	void onJobDestroyed(QObject *obj);
private:
	QHash<FileTransferJob*, QString> m_paths;
	FileTransferJobModel *m_model;
	QPointer<FileTransferDialog> m_dialog;
};

}

#endif // SIMPLEFILETRANSFER_H
