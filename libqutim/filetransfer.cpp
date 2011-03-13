/****************************************************************************
 *  filetransfer.cpp
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
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

#include "filetransfer.h"
#include "objectgenerator.h"
#include "servicemanager.h"
#include "chatunit.h"
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QBitArray>

namespace qutim_sdk_0_3
{
#define REMEMBER_ALL_ABILITIES 1
namespace Games
{
struct FileTransferScope
{
	struct Observer
	{
		QList<FileTransferObserver *> list;
		ChatUnit *unit;
#ifdef REMEMBER_ALL_ABILITIES
		QBitArray abilities;
		int setCount;
#else
		bool ability;
#endif
	};
	FileTransferScope() : manager(0), inited(false) {}
	bool init();
	QList<FileTransferFactory*> factories;
	QMap<ChatUnit*, Observer> observers;
	FileTransferManager *manager;
	bool inited;
};

bool FileTransferScope::init()
{
	if (!inited && ObjectGenerator::isInited()) {
		inited = true;
		manager = ServiceManager::getByName<FileTransferManager*>("FileTransferManager");
	}
	return manager != NULL;
}

Q_GLOBAL_STATIC(FileTransferScope, scope)

class FileTransferManagerPrivate
{
	Q_DECLARE_PUBLIC(FileTransferManager)
public:
	FileTransferManagerPrivate(FileTransferManager *q) : q_ptr(q) {}
	static FileTransferManagerPrivate *get(FileTransferManager *q) { return q->d_func(); }
	void handleJob(FileTransferJob *job, FileTransferJob *oldJob) { q_func()->handleJob(job, oldJob); }
	FileTransferManager *q_ptr;
};
}

class FileTransferInfoPrivate : public QSharedData
{
public:
	FileTransferInfoPrivate() : fileSize(0) {}
	FileTransferInfoPrivate(const FileTransferInfoPrivate &o) :
	    QSharedData(o), fileName(o.fileName), fileSize(o.fileSize) {}
	QString fileName;
	qint64 fileSize;
};

FileTransferInfo::FileTransferInfo() : d_ptr(new FileTransferInfoPrivate)
{
}

FileTransferInfo::FileTransferInfo(const FileTransferInfo &other) : d_ptr(other.d_ptr)
{
}

FileTransferInfo::~FileTransferInfo()
{
}

FileTransferInfo &FileTransferInfo::operator =(const FileTransferInfo &other)
{
	d_ptr = other.d_ptr;
	return *this;
}

QString FileTransferInfo::fileName() const
{
	return d_ptr->fileName;
}

void FileTransferInfo::setFileName(const QString &fileName)
{
	d_ptr->fileName = fileName;
}

qint64 FileTransferInfo::fileSize() const
{
	return d_ptr->fileSize;
}

void FileTransferInfo::setFileSize(qint64 fileSize)
{
	d_ptr->fileSize = fileSize;
}

class FileTransferJobPrivate
{
	Q_DECLARE_PUBLIC(FileTransferJob)
public:
	FileTransferJobPrivate(FileTransferJob::Direction d, FileTransferJob *q) :
	    direction(d), error(FileTransferJob::NoError),
	    state(FileTransferJob::Initiation), currentIndex(-1),
	    progress(0), fileProgress(0), totalSize(0), q_ptr(q) {}
	void addFile(const QFileInfo &info);
	QIODevice *device(int index);
	ChatUnit *unit;
	QString title;
	FileTransferJob::Direction direction;
	FileTransferJob::ErrorType error;
	FileTransferJob::State state;
	QVector<FileTransferInfo> files;
	QVector<QIODevice*> devices;
	int currentIndex;
	qint64 progress;
	qint64 fileProgress;
	qint64 totalSize;
	Games::FileTransferFactory *factory;
	FileTransferJob *q_ptr;
	QDir dir;
};

void FileTransferJobPrivate::addFile(const QFileInfo &info)
{
	FileTransferInfo ftInfo;
	ftInfo.setFileName(dir.relativeFilePath(info.absoluteFilePath()));
	ftInfo.setFileSize(info.size());
	files.append(ftInfo);
	totalSize += ftInfo.fileSize();
}

QIODevice *FileTransferJobPrivate::device(int index)
{
	if (!devices[index])
		devices[index] = Games::FileTransferManager::openFile(q_func());
	return devices[index];
}

FileTransferJob::FileTransferJob(ChatUnit *unit, FileTransferJob::Direction direction,
                                 Games::FileTransferFactory *factory) :
    QObject(unit), d_ptr(new FileTransferJobPrivate(direction, this))
{
	Q_D(FileTransferJob);
	d->unit = unit;
	d->factory = factory;
}

FileTransferJob::~FileTransferJob()
{
}

void FileTransferJob::send(const QUrl &url, const QString &title)
{
	Q_D(FileTransferJob);
	QFileInfo info = url.toLocalFile();
	d->title = title.isEmpty() ? info.fileName() : title;
	QStringList files;
	if (info.isDir()) {
		d->dir.setPath(info.absoluteFilePath());
		QDirIterator it(d->dir, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			info = it.fileInfo();
			if (!info.isFile())
				continue;
			d->addFile(info);
			files << d->files.last().fileName();
		}
	} else {
		d->dir.setPath(info.absolutePath());
		d->addFile(info);
		files << d->files.last().fileName();
	}
	doSend();
	//	if (Games::scope()->init())
	//		Games::FileTransferManagerPrivate::get(Games::scope()->manager)->handleJob(this, 0);
}

void FileTransferJob::send(const QDir &baseDir, const QStringList &files, const QString &title)
{
	Q_D(FileTransferJob);
	d->dir = baseDir;
	d->title = title;
	for (int i = 0; i < files.size(); i++) {
		QFileInfo info = d->dir.filePath(files.at(i));
		FileTransferInfo ftInfo;
		ftInfo.setFileName(files.at(i));
		ftInfo.setFileSize(info.size());
		d->files << ftInfo;
	}
	doSend();
}

void FileTransferJob::stop()
{
	doStop();
}

FileTransferJob::Direction FileTransferJob::direction() const
{
	return d_func()->direction;
}

QString FileTransferJob::title() const
{
	return d_func()->title;
}

QString FileTransferJob::fileName() const
{
	Q_D(const FileTransferJob);
	return d->currentIndex == -1 ? QString() : d->files.at(d->currentIndex).fileName();
}

FileTransferInfo FileTransferJob::info(int index) const
{
	return d_func()->files.value(index);
}

int FileTransferJob::filesCount() const
{
	return d_func()->files.size();
}

int FileTransferJob::currentIndex() const
{
	return d_func()->currentIndex;
}

qint64 FileTransferJob::fileSize() const
{
	Q_D(const FileTransferJob);
	return d->currentIndex == -1 ? 0 : d->files.at(d->currentIndex).fileSize();
}

qint64 FileTransferJob::progress() const
{
	return d_func()->progress;
}

qint64 FileTransferJob::totalSize() const
{
	return d_func()->totalSize;
}

FileTransferJob::State FileTransferJob::state() const
{
	return d_func()->state;
}

FileTransferJob::ErrorType FileTransferJob::error() const
{
	return d_func()->error;
}

void FileTransferJob::init(int filesCount, qint64 totalSize, const QString &title)
{
	Q_D(FileTransferJob);
	d->files.resize(filesCount);
	d->totalSize = totalSize;
	d->title = title;
	emit titleChanged(title);
	emit totalSizeChanged(totalSize);
	if (Games::scope()->init())
		Games::FileTransferManagerPrivate::get(Games::scope()->manager)->handleJob(this, 0);
}

QDir FileTransferJob::baseDir() const
{
	return d_func()->dir;
}

QIODevice *FileTransferJob::setCurrentIndex(int index)
{
	Q_D(FileTransferJob);
	if (d->currentIndex != index) {
		d->currentIndex = index;
		d->fileProgress = 0;
		const FileTransferInfo &info = d->files.at(index);
		emit currentIndexChanged(index);
		emit fileNameChanged(info.fileName());
		emit fileSizeChanged(info.fileSize());
	}
	return d->device(index);
}

void FileTransferJob::setFileProgress(qint64 fileProgress)
{
	Q_D(FileTransferJob);
	qint64 delta = fileProgress - d->fileProgress;
	Q_ASSERT(delta > 0);
	d->fileProgress = fileProgress;
	d->progress += delta;
	emit progressChanged(d->progress);
}

void FileTransferJob::setError(FileTransferJob::ErrorType err)
{
	Q_D(FileTransferJob);
	if (d->error != err) {
		d->error = err;
		FileTransferJob *job = 0;
		if (d->direction == Outgoing) {
			QList<Games::FileTransferFactory*> &list = Games::scope()->factories;
			for (int i = list.indexOf(d->factory) + 1; !job && i < list.size(); i++) {
				Games::FileTransferFactory *factory = list.at(i);
				if (factory->checkAbility(d->unit)) {
					job = factory->create(d->unit);
					FileTransferJobPrivate *p = job->d_func();
					p->files = d->files;
					p->dir = d->dir;
					job->doSend();
					Games::FileTransferManagerPrivate::get(Games::scope()->manager)->handleJob(job, this);
				}
			}
		}
		emit error(d->error, job);
	}
}

void FileTransferJob::setState(FileTransferJob::State state)
{
	Q_D(FileTransferJob);
	if (d->state != state) {
		d->state = state;
		emit stateChanged(state);
	}
}

void FileTransferJob::setFileInfo(int index, const FileTransferInfo &info)
{
	Q_D(FileTransferJob);
	FileTransferInfo old = info;
	qSwap(d->files[index], old);
	if (index == d->currentIndex) {
		if (old.fileName() != info.fileName())
			emit fileNameChanged(info.fileName());
		if (old.fileSize() != info.fileSize())
			emit fileSizeChanged(info.fileSize());
	}
}

void FileTransferJob::virtual_hook(int id, void *data)
{
	Q_UNUSED(id);
	Q_UNUSED(data);
}

class FileTransferObserverPrivate
{
	Q_DECLARE_PUBLIC(FileTransferObserver)
public:
	FileTransferObserverPrivate(FileTransferObserver *q) : q_ptr(q), scope(0) {}
	static FileTransferObserverPrivate *get(FileTransferObserver *o) { return o->d_func(); }
	void emitAbilityChanged(bool ability) { emit q_func()->abilityChanged(ability); }
	FileTransferObserver *q_ptr;
	Games::FileTransferScope::Observer *scope;
};

FileTransferObserver::FileTransferObserver(ChatUnit *unit) :
    d_ptr(new FileTransferObserverPrivate(this))
{
	Q_D(FileTransferObserver);
	d->scope = &Games::scope()->observers[unit];
	if (d->scope->list.isEmpty()) {
		d->scope->unit = unit;
		QList<Games::FileTransferFactory*> &list = Games::scope()->factories;
#ifdef REMEMBER_ALL_ABILITIES
		d->scope->setCount = 0;
		d->scope->abilities.resize(list.size());
		for (int i = 0; i < list.size(); i++) {
			list.at(i)->startObserve(unit);
			if (list.at(i)->checkAbility(unit)) {
				++d->scope->setCount;
				d->scope->abilities.setBit(i, true);
			}
		}
#else
		for (int i = 0; !d->scope->ability && i < list.size(); i++) {
			list.at(i)->startObserve(unit);
			d->scope->ability |= list.at(i)->checkAbility(unit);
		}
#endif
	}
	d->scope->list.append(this);
}

FileTransferObserver::~FileTransferObserver()
{
	Q_D(FileTransferObserver);
	d->scope->list.removeOne(this);
	if (d->scope->list.isEmpty()) {
		Games::scope()->observers.remove(d->scope->unit);
		QList<Games::FileTransferFactory*> &list = Games::scope()->factories;
		for (int i = 0; i < list.size(); i++)
			list.at(i)->stopObserve(d->scope->unit);
	}
}

bool FileTransferObserver::checkAbility() const
{
#ifdef REMEMBER_ALL_ABILITIES
	return d_func()->scope->setCount > 0;
#else
	return d_func()->scope->ability;
#endif
}

namespace Games
{
class FileTransferFactoryPrivate
{
public:
	LocalizedString name;
	LocalizedString description;
	FileTransferFactory::Capabilities capabilities;
};

FileTransferFactory::FileTransferFactory(const LocalizedString &name,
                                         FileTransferFactory::Capabilities capabilities) :
    d_ptr(new FileTransferFactoryPrivate)
{
	Q_D(FileTransferFactory);
	d->capabilities = capabilities;
	d->name = name;
}

FileTransferFactory::~FileTransferFactory()
{
}

FileTransferFactory::Capabilities FileTransferFactory::capabilities() const
{
	return d_func()->capabilities;
}

void FileTransferFactory::changeAvailability(ChatUnit *unit, bool canSend)
{
	FileTransferScope::Observer &scp = scope()->observers[unit];
#ifdef REMEMBER_ALL_ABILITIES
	QList<Games::FileTransferFactory*> &list = Games::scope()->factories;
	int index = list.indexOf(this);
	bool changed = scp.abilities.testBit(index) != canSend;
	if (!changed)
		return;
	scp.setCount = (scp.abilities.toggleBit(index) << 1) - 1;
	if (scp.setCount == 0 || (scp.setCount == 1 && !canSend)) {
		for (int i = 0; i < scp.list.size(); i++) {
			FileTransferObserver *obs = scp.list.at(i);
			FileTransferObserverPrivate::get(obs)->emitAbilityChanged(scp.setCount > 0);
		}
	}
#else
	if (!scp.ability && canSend) {
		scp.ability = true;
	} else if (scp.ability && !canSend) {
		// Need recalc abilities
		bool oldAbility = scp.ability;
		scp.ability = false;
		QList<Games::FileTransferFactory*> &list = Games::scope()->factories;
		for (int i = 0; !scp.ability && i < list.size(); i++)
			scp.ability |= list.at(i)->checkAbility(scp.unit);
		if (scp.ability == oldAbility)
			return;
	} else {
		return;
	}
	for (int i = 0; i < scp.list.size(); i++) {
		FileTransferObserver *obs = scp.list.at(i);
		FileTransferObserverPrivate::get(obs)->emitAbilityChanged(scp.ability);
	}
#endif
}

void FileTransferFactory::virtual_hook(int id, void *data)
{
	Q_UNUSED(id);
	Q_UNUSED(data);
}

FileTransferManager::FileTransferManager() : d_ptr(new FileTransferManagerPrivate(this))
{
	Games::scope()->manager = this;
	Games::scope()->inited = true;
	foreach (const ObjectGenerator *gen, ObjectGenerator::module<FileTransferFactory>())
		Games::scope()->factories << gen->generate<FileTransferFactory>();
}

FileTransferManager::~FileTransferManager()
{
}

bool FileTransferManager::checkAbility(ChatUnit *unit)
{
	QMap<ChatUnit*,FileTransferScope::Observer>::iterator it = scope()->observers.find(unit);
	if (it != scope()->observers.end()) {
#ifdef REMEMBER_ALL_ABILITIES
		return it.value().setCount > 0;
#else
		return it.value().ability;
#endif
	}
	QList<Games::FileTransferFactory*> &list = scope()->factories;
	bool ok = false;
	for (int i = 0; !ok && i < list.size(); i++)
		ok |= list.at(i)->checkAbility(unit);
	return ok;
}

FileTransferJob *FileTransferManager::send(ChatUnit *unit, const QUrl &url, const QString &title)
{
	if (!scope()->init())
		return 0;
	QList<Games::FileTransferFactory*> &list = scope()->factories;
	for (int i = 0; i < list.size(); i++) {
		Games::FileTransferFactory *factory = list.at(i);
		if (factory->checkAbility(unit)) {
			FileTransferJob *job = factory->create(unit);
			job->send(url, title);
			scope()->manager->handleJob(job, 0);
			return job;
		}
	}
	return 0;
}

QIODevice *FileTransferManager::openFile(FileTransferJob *job)
{
	if (!scope()->init())
		return 0;
	return scope()->manager->doOpenFile(job);
}

void FileTransferManager::virtual_hook(int id, void *data)
{
	Q_UNUSED(id);
	Q_UNUSED(data);
}
}


class FileTransferEnginePrivate
{
public:
	ChatUnit *chatUnit;
	FileTransferFactory *factory;
	FileTransferEngine::Direction direction;
};

class FileTransferManagerPrivate
{
};

struct FileTransferData
{
	QList<FileTransferFactory *> factories;
	QPointer<FileTransferManager> manager;
};

void init(FileTransferData *data)
{
	GeneratorList gens = ObjectGenerator::module<FileTransferFactory>();
	foreach(const ObjectGenerator *gen, gens)
		data->factories << gen->generate<FileTransferFactory>();
	gens = ObjectGenerator::module<FileTransferManager>();
	if (!gens.isEmpty())
		data->manager = gens.first()->generate<FileTransferManager>();
}

Q_GLOBAL_STATIC_WITH_INITIALIZER(FileTransferData, data, init(x.data()));

FileTransferEngine::FileTransferEngine(ChatUnit *chatUnit, Direction direction, FileTransferFactory *factory) :
	QObject(factory), d_ptr(new FileTransferEnginePrivate)
{
	Q_D(FileTransferEngine);
	d->chatUnit = chatUnit;
	d->factory = factory;
	d->direction = direction;
}

FileTransferEngine::~FileTransferEngine()
{
}

ChatUnit *FileTransferEngine::chatUnit() const
{
	return d_func()->chatUnit;
}

int FileTransferEngine::localPort() const
{
	return -1;
}

int FileTransferEngine::remotePort() const
{
	return -1;
}

QHostAddress FileTransferEngine::remoteAddress() const
{
	return QHostAddress();
}

FileTransferEngine::Direction FileTransferEngine::direction() const
{
	return d_func()->direction;
}

void FileTransferEngine::virtual_hook(int id, void *data)
{
	Q_UNUSED(id);
	Q_UNUSED(data);
}

FileTransferFactory::FileTransferFactory()
{
}

FileTransferFactory::~FileTransferFactory()
{
}

FileTransferManager::FileTransferManager() : d_ptr(new FileTransferManagerPrivate)
{
}

FileTransferManager::~FileTransferManager()
{
}

FileTransferManager *FileTransferManager::instance()
{
	return data()->manager;
}

bool FileTransferManager::checkAbility(ChatUnit *unit)
{
	foreach (FileTransferFactory *factory, data()->factories) {
		if (factory->check(unit))
			return true;
	}
	return false;
}

FileTransferEngine *FileTransferManager::getEngine(ChatUnit *unit, FileTransferEngine *last)
{
	if (last && last->direction() == FileTransferEngine::Receive)
		return 0;

	FileTransferFactory *lastFactory = last ? last->d_func()->factory : 0;
	FileTransferData *d = data();
	int index = lastFactory ? d->factories.indexOf(lastFactory) : -1;
	index++;
	for (; index < d->factories.size(); index++) {
		if (d->factories.at(index)->check(unit))
			return d->factories.at(index)->create(unit);
	}
	return 0;
}
}
