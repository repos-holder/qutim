/****************************************************************************
 *
 *  This file is part of qutIM
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *
 ***************************************************************************
 *                                                                         *
 *   This file is part of free software; you can redistribute it and/or    *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************
 ****************************************************************************/

#include "config.h"
#include "cryptoservice.h"
#include "systeminfo.h"
#include <QSet>
#include <QStringList>
#include <QFileInfo>
#include <QDateTime>

namespace qutim_sdk_0_3
{
	Q_GLOBAL_STATIC(QList<ConfigBackend*>, all_config_backends)
	LIBQUTIM_EXPORT QList<ConfigBackend*> &get_config_backends()
	{ return *all_config_backends(); }
	
	struct ConfigAtom
	{
		inline ConfigAtom() : deleteOnDestroy(true), typeMap(true), readOnly(false), list(0) {}
		ConfigAtom(QVariant &var, bool isMap);
		inline ~ConfigAtom();
		bool deleteOnDestroy;
		bool typeMap;
		bool readOnly;
		union {
			QVariantList *list;
			QVariantMap *map;
		};
	};
	
	ConfigAtom::ConfigAtom(QVariant &var, bool isMap) : deleteOnDestroy(false), typeMap(isMap), readOnly(false)
	{
		if (isMap && var.type() != QVariant::Map)
			var = QVariantMap();
		else if (!isMap && var.type() != QVariant::List)
			var = QVariantList();
		
		if (isMap)
			map = reinterpret_cast<QVariantMap*>(var.data());
		else
			list = reinterpret_cast<QVariantList*>(var.data());
	}
	
	ConfigAtom::~ConfigAtom()
	{
		if (deleteOnDestroy && typeMap)
			delete map;
		else if (deleteOnDestroy && !typeMap)
			delete list;
	}
	
	struct ConfigLevel
	{
		inline ConfigLevel() : arrayElement(false) {}
		inline ~ConfigLevel() { qDeleteAll(atoms); }
		
		QList<ConfigAtom*> atoms;
		bool arrayElement;
	};
	
	class ConfigSource : public QSharedData
	{
		Q_DISABLE_COPY(ConfigSource)
	public:
		typedef QExplicitlySharedDataPointer<ConfigSource> Ptr;
		inline ConfigSource() : backend(0), dirty(false) { update(); }
		inline ~ConfigSource() {}
		static ConfigSource::Ptr open(const QString &path, bool systemDir, bool create);
		inline void update() { lastUse = QDateTime::currentDateTime(); }
		
		QString fileName;
		ConfigBackend *backend;
		bool dirty;
		ConfigAtom data;
		QDateTime lastUse;
	};
	
	typedef QHash<QString, ConfigSource::Ptr> ConfigSourceHash;
	
	Q_GLOBAL_STATIC(ConfigSourceHash, sourceHash)
	
	ConfigSource::Ptr ConfigSource::open(const QString &path, bool systemDir, bool create)
	{
		QString fileName = path;
		if (fileName.isEmpty())
			fileName = QLatin1String("profile");
		QFileInfo info(fileName);
		if (!info.isAbsolute()) {
			SystemInfo::DirType type = systemDir
									   ? SystemInfo::SystemConfigDir
										   : SystemInfo::ConfigDir;
			fileName = SystemInfo::getDir(type).filePath(fileName);
		} else if (systemDir) {
			// We need to open absolute dir only once
			return ConfigSource::Ptr();
		}
		fileName = QDir::cleanPath(fileName);
		
		ConfigSource::Ptr result = sourceHash()->value(path);
		if (result) {
			result->update();
			return result;
		}
		
		ConfigBackend *backend = 0;
		
		QByteArray suffix = info.suffix().toLatin1().toLower();
		const QList<ConfigBackend*> &backends = *all_config_backends();
		
		if (backends.isEmpty())
			return ConfigSource::Ptr();
		
		if (!suffix.isEmpty()) {
			for (int i = 0; i < backends.size(); i++) {
				if (backends.at(i)->name() == suffix) {
					backend = backends.at(i);
					break;
				}
			}
		}
		if (!backend) {
			backend = backends.at(0);
			fileName += QLatin1Char('.');
			fileName += QLatin1String(backend->name());
			
			result = sourceHash()->value(path);
			if (result) {
				result->update();
				return result;
			}
			info.setFile(fileName);
		}
		
		if (!info.exists() && !create)
			return result;
		
		QDir dir = info.absoluteDir();
		if (!dir.exists()) {
			if (!create)
				return result;
			dir.mkpath(info.absolutePath());
		}
		
		result = new ConfigSource;
		ConfigSource *d = result.data();
		d->backend = backend;
		d->fileName = fileName;
		// QFileInfo says that we can't write to non-exist files but we can
		d->data.readOnly = !info.isWritable() && (systemDir || info.exists());
		
		QVariant var = d->backend->load(d->fileName);
		if (var.type() == QVariant::Map) {
			d->data.map = new QVariantMap(var.toMap());
		} else if (var.type() == QVariant::List) {
			d->data.typeMap = false;
			d->data.list = new QVariantList(var.toList());
		} else {
			if (!create) {
				d->data.map = 0;
				// result will be cleared automatically
				return ConfigSource::Ptr();
			}
			d->data.map = new QVariantMap();
		}
		sourceHash()->insert(path, result);
		return result;
	}

	class ConfigPrivate : public QSharedData
	{
		Q_DISABLE_COPY(ConfigPrivate)
	public:
		inline ConfigPrivate()  { levels << new ConfigLevel(); }
		inline ~ConfigPrivate() { if (!memoryGuard) sync(); }
		inline ConfigLevel *current() const { return levels.at(0); }
		void sync();
		void init(const QStringList &paths);
		QList<ConfigLevel *> levels;
		QList<ConfigSource::Ptr> sources;
		QExplicitlySharedDataPointer<ConfigPrivate> memoryGuard;
	};
	
	void ConfigPrivate::sync()
	{
		if (sources.isEmpty())
			return;
		ConfigSource *source = sources.value(0).data();
		if (source && source->dirty) {
			const ConfigAtom * const level = &source->data;
			if (level->typeMap)
				source->backend->save(source->fileName, *(level->map));
			else
				source->backend->save(source->fileName, *(level->list));
			source->dirty = false;
		}
	}
	
	void ConfigPrivate::init(const QStringList &paths)
	{
		QSet<QString> opened;
		ConfigSource::Ptr source;
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < paths.size(); i++) {
				// Firstly we should open user-specific configs
				source = ConfigSource::open(paths.at(i), j == 1, sources.isEmpty());
				if (source && !opened.contains(source->fileName)) {
					opened.insert(source->fileName);
					sources << source;
				}
			}
		}
		for (int i = 0; i < sources.size(); i++) {
			source = sources.at(i);
			ConfigAtom *atom = new ConfigAtom(source->data);
			atom->deleteOnDestroy = false;
			atom->readOnly = atom->readOnly || i > 0;
			current()->atoms << atom;
		}
	}

	Config::Config(const QVariantList &list) : d_ptr(new ConfigPrivate)
	{
		Q_D(Config);
		ConfigAtom *atom = new ConfigAtom();
		atom->typeMap = false;
		atom->deleteOnDestroy = true;
		atom->readOnly = true;
		atom->list = new QVariantList(list);
		d->current()->atoms << atom;
	}

	Config::Config(QVariantList *list) : d_ptr(new ConfigPrivate)
	{
		Q_D(Config);
		ConfigAtom *atom = new ConfigAtom();
		atom->typeMap = false;
		atom->deleteOnDestroy = false;
		atom->list = list;
		d->current()->atoms << atom;
	}

	Config::Config(const QVariantMap &map) : d_ptr(new ConfigPrivate)
	{
		Q_D(Config);
		ConfigAtom *atom = new ConfigAtom();
		atom->readOnly = true;
		atom->map = new QVariantMap(map);
		d->current()->atoms << atom;
	}

	Config::Config(QVariantMap *map) : d_ptr(new ConfigPrivate)
	{
		Q_D(Config);
		ConfigAtom *atom = new ConfigAtom();
		atom->deleteOnDestroy = false;
		atom->map = map;
		d->current()->atoms << atom;
	}

	Config::Config(const QString &path) : d_ptr(new ConfigPrivate)
	{
		Q_D(Config);
		d->init(QStringList() << path);
	}
	
	Config::Config(const QStringList &paths) : d_ptr(new ConfigPrivate)
	{
		Q_D(Config);
		d->init(paths);
	}
	
	Config::Config(const Config &other) : d_ptr(other.d_ptr)
	{
	}

	Config &Config::operator =(const Config &other)
	{
		d_ptr = other.d_ptr;
		return *this;
	}

	Config::~Config()
	{
	}

	Config Config::group(const QString &fullName)
	{
		Q_D(Config);
		Q_ASSERT(!fullName.isEmpty());
		Config cfg(reinterpret_cast<QVariantMap*>(0));
		ConfigPrivate *p = cfg.d_func();
		p->memoryGuard = d_ptr;
		p->sources = d->sources;
		qDeleteAll(p->current()->atoms);
		p->current()->atoms = d->current()->atoms;
		cfg.beginGroup(fullName);
		p->levels.removeLast();
		return cfg;
	}

	QStringList Config::childGroups() const
	{
		Q_D(const Config);
		QStringList groups;
		QList<ConfigAtom*> &atoms = d->current()->atoms;
		for (int i = 0; i < atoms.size(); i++) {
			ConfigAtom *atom = atoms.at(i);
			if (!atom->typeMap)
				continue;
			QVariantMap::iterator it = atom->map->begin();
			for (; it != atom->map->end(); it++) {
				if (it.value().type() == QVariant::Map && !groups.contains(it.key()))
					groups << it.key();
			}
		}
		return groups;
	}

	QStringList Config::childKeys() const
	{
		Q_D(const Config);
		QStringList groups;
		QList<ConfigAtom*> &atoms = d->current()->atoms;
		for (int i = 0; i < atoms.size(); i++) {
			ConfigAtom *atom = atoms.at(i);
			if (!atom->typeMap)
				continue;
			QVariantMap::iterator it = atom->map->begin();
			for (; it != atom->map->end(); it++) {
				if (it.value().type() != QVariant::Map && !groups.contains(it.key()))
					groups << it.key();
			}
		}
		return groups;
	}
	
	bool Config::hasChildGroup(const QString &name) const
	{
		Q_D(const Config);
		QList<ConfigAtom*> &atoms = d->current()->atoms;
		for (int i = 0; i < atoms.size(); i++) {
			ConfigAtom *atom = atoms.at(i);
			if (!atom->typeMap)
				continue;
			QVariantMap::iterator it = atom->map->find(name);
			if (it != atom->map->end() && it.value().type() == QVariant::Map)
				return true;
		}
		return false;
	}
	
	bool Config::hasChildKey(const QString &name) const
	{
		Q_D(const Config);
		QList<ConfigAtom*> &atoms = d->current()->atoms;
		for (int i = 0; i < atoms.size(); i++) {
			ConfigAtom *atom = atoms.at(i);
			if (!atom->typeMap)
				continue;
			QVariantMap::iterator it = atom->map->find(name);
			if (it != atom->map->end() && it.value().type() != QVariant::Map)
				return true;
		}
		return false;
	}
	
	QStringList parseNames(const QString &fullName)
	{
		QStringList names;
		int first = 0;
		do {
			int last = fullName.indexOf('/', first);
			QString name = fullName.mid(first, last != -1 ? last - first : last);
			first = last + 1;
			if (!name.isEmpty())
				names << name;
		} while(first != 0);
		return names;
	}

	void Config::beginGroup(const QString &fullName)
	{
		Q_D(Config);
		Q_ASSERT(!fullName.isEmpty());
		ConfigLevel * const l = new ConfigLevel;
		QList<ConfigAtom*> &atoms = d->current()->atoms;
		QStringList names = parseNames(fullName);
		for (int i = 0; i < atoms.size(); i++) {
			ConfigAtom *current = atoms.at(i);
			Q_ASSERT(current->typeMap);
			if (!current->typeMap)
				continue;
			ConfigAtom *atom = new ConfigAtom;
			atom->deleteOnDestroy = false;
			atom->readOnly = current->readOnly /*|| i > 0*/;
			atom->map = current->map;
			for (int j = 0; j < names.size(); j++) {
				QVariant &var = (*(atom->map))[names.at(j)];
				if (var.type() != QVariant::Map) {
					if (atom->readOnly) {
						delete atom;
						atom = 0;
						break;
					} else {
						if (!d->sources.isEmpty())
							d->sources.at(0)->dirty = true;
						var = QVariantMap();
					}
				}
				atom->map = reinterpret_cast<QVariantMap*>(var.data());
			}
			if (atom)
				l->atoms << atom;
		}
		d->levels.prepend(l);
	}

	void Config::endGroup()
	{
		Q_D(Config);
		Q_ASSERT(d->levels.size() > 1);
#ifdef DEBUG		
		ConfigLevel * const level = d->levels.at(0);
		ConfigAtom * const atom = level->atoms.value(0);
		Q_ASSERT(!atom || atom->typeMap);
#endif
		delete d->levels.takeFirst();
	}

	void Config::remove(const QString &name)
	{
		Q_D(Config);
		ConfigLevel * const level = d->levels.at(0);
		ConfigAtom * const atom = level->atoms.value(0);
		Q_ASSERT(!atom || atom->typeMap);
		if (atom && !atom->readOnly) {
			if (atom->map->remove(name) != 0)
				d->sources.at(0)->dirty = true;
		}
	}

	Config Config::arrayElement(int index)
	{
		Q_D(Config);
		Config cfg(reinterpret_cast<QVariantList*>(0));
		ConfigPrivate *p = cfg.d_func();
		p->memoryGuard = d_ptr;
		p->sources = d->sources;
		qDeleteAll(p->current()->atoms);
		p->current()->atoms = d->current()->atoms;
		cfg.setArrayIndex(index);
		p->levels.removeLast();
		return cfg;
	}

	int Config::beginArray(const QString &name)
	{
		Q_D(Config);
		ConfigLevel * const l = new ConfigLevel;
		QList<ConfigAtom*> &atoms = d->current()->atoms;
		int size = 0;
		for (int i = 0; i < atoms.size(); i++) {
			ConfigAtom *current = atoms.at(i);
			if (current->readOnly) {
				if (current->typeMap) {
					QVariant &var = (*(current->map))[name];
					if (var.type() == QVariant::List) {
						l->atoms << new ConfigAtom(var, false);
						if (!size)
							size = var.toList().size();
						break;
					}
				}
			} else if (current->typeMap) {
				QVariant &var = (*(current->map))[name];
				if (var.type() != QVariant::List && !d->sources.isEmpty())
					d->sources.at(0)->dirty;
				l->atoms << new ConfigAtom(var, false);
				if (!size)
					size = var.toList().size();
			}
		}
		d->levels.prepend(l);
		return size;
	}

	void Config::endArray()
	{
		Q_D(Config);
		Q_ASSERT(d->levels.size() > 1);
		ConfigLevel * const level = d->levels.at(0);
		if (level->atoms.isEmpty())
			return;
		ConfigAtom * const atom = level->atoms.value(0);
		Q_ASSERT(!atom || level->arrayElement || !atom->typeMap);
		if (level->arrayElement) {
			Q_ASSERT(d->levels.size() > 2);
			delete d->levels.takeFirst();
			delete d->levels.takeFirst();
		} else if (!atom->typeMap) {
			Q_ASSERT(d->levels.size() > 1);
			delete d->levels.takeFirst();
		}
	}
	
	int Config::arraySize() const
	{
		Q_D(const Config);
		ConfigLevel * const level = d->levels.at(0);
		ConfigAtom * const atom = level->atoms.value(0);
		if (!atom || atom->typeMap) {
			return 0;
		} else {
			int size = atom->list->size();
			if (!size && level->atoms.size() > 1)
				size = level->atoms.at(1)->list->size();
			return size;
		}
	}

	void Config::setArrayIndex(int index)
	{
		Q_D(Config);
		ConfigLevel * const level = d->levels.at(0);
		ConfigAtom *atom = level->atoms.value(0);
		Q_ASSERT(level->arrayElement || !atom || !atom->typeMap);
		if (!atom) {
			if (!level->arrayElement) {
				d->levels.prepend(new ConfigLevel);
				d->current()->arrayElement = true;
			}
			return;
		}
		if (!level->arrayElement) {
			ConfigLevel * const l = new ConfigLevel();
			l->arrayElement = true;
			d->levels.prepend(l);
		}
		ConfigLevel * const l = d->levels.at(1);
		QList<ConfigAtom*> &atoms = l->atoms;
		qDeleteAll(d->current()->atoms);
		d->current()->atoms.clear();
		for (int i = 0; i < atoms.size(); i++) {
			atom = atoms.at(i);
			if (atom->readOnly && !atom->typeMap && atom->list->size() > index) {
				QVariant &var = (*(atom->list))[index];
				if (var.type() == QVariant::Map)
					d->current()->atoms << new ConfigAtom(var, true);
			} else if (!atom->readOnly && !atom->typeMap) {
				bool *changed = !d->sources.isEmpty() ? &d->sources.at(0)->dirty : 0;
				while (atom->list->size() <= index) {
					if (changed)
						*changed = true;
					atom->list->append(QVariantMap());
				}
				QVariant &var = (*(atom->list))[index];
				if (changed)
					*changed = var.type() == QVariant::Map;
				d->current()->atoms << new ConfigAtom(var ,true);
			}
		}
	}

	void Config::remove(int index)
	{
		Q_D(Config);
		ConfigLevel *level = d->levels.at(0);
		if (level->arrayElement) {
			Q_ASSERT(d->levels.size() > 1);
			delete d->levels.takeFirst();
			level = d->levels.at(0);
		}
		ConfigAtom *atom = level->atoms.value(0);
		Q_ASSERT(!atom || !atom->typeMap);
		if (atom && !atom->readOnly && atom->list->size() > index) {
			atom->list->removeAt(index);
			if (!d->sources.isEmpty())
				d->sources.at(0)->dirty = true;
		}
	}

	QVariant Config::value(const QString &key, const QVariant &def, ValueFlags type) const
	{
		Q_D(const Config);
		ConfigLevel *level = d->levels.at(0);
		if (level->atoms.isEmpty())
			return def;
		QString name = key;
		int slashIndex = name.lastIndexOf('/');
		if (slashIndex != -1) {
			const_cast<Config*>(this)->beginGroup(name.mid(0, slashIndex));
			name = name.mid(slashIndex);
		}
		QVariant var;
		QList<ConfigAtom*> &atoms = d->current()->atoms;
		for (int i = 0; i < atoms.size(); i++) {
			ConfigAtom *atom = level->atoms.at(i);
			Q_ASSERT(atom->typeMap);
			var = atom->map->value(name);
			if (!var.isNull())
				break;
		}
		if (slashIndex != -1)
			const_cast<Config*>(this)->endGroup();
		if (type & Config::Crypted)
			return var.isNull() ? def : CryptoService::decrypt(var);
		else
			return var.isNull() ? def : var;
	}

	void Config::setValue(const QString &key, const QVariant &value, ValueFlags type)
	{
		Q_D(Config);
		ConfigLevel *level = d->levels.at(0);
		if (level->atoms.isEmpty())
			return;
		QString name = key;
		int slashIndex = name.lastIndexOf('/');
		if (slashIndex != -1) {
			beginGroup(name.mid(0, slashIndex));
			name = name.mid(slashIndex);
		}
		ConfigAtom *atom = level->atoms.at(0);
		Q_ASSERT(atom->typeMap);
		QVariant var = (type & Config::Crypted) ? CryptoService::crypt(value) : value;
		atom->map->insert(name, var);
		if (!d->sources.isEmpty())
			d->sources.at(0)->dirty = true;
		if (slashIndex != -1)
			endGroup();
	}

	void Config::sync()
	{
		d_func()->sync();
	}
	
	class ConfigBackendPrivate
	{
	public:
		mutable QByteArray extension;
	};
	
	ConfigBackend::ConfigBackend() : d_ptr(new ConfigBackendPrivate)
	{
	}

	ConfigBackend::~ConfigBackend()
	{
	}
	
	QByteArray ConfigBackend::name() const
	{
		Q_D(const ConfigBackend);
		if(d->extension.isNull()) {
			d->extension = metaInfo(metaObject(), "Extension");
			d->extension = d->extension.toLower();
		}
		return d->extension;
	}
	
	void ConfigBackend::virtual_hook(int id, void *data)
	{
		Q_UNUSED(id);
		Q_UNUSED(data);
	}
}
