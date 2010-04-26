/****************************************************************************
 *  modulemanager.cpp
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

#include "modulemanager.h"
#include "plugin_p.h"
#include "deprecatedplugin_p.h"
#include "cryptoservice.h"
#include "configbase_p.h"
#include "protocol.h"
#include "contactlist.h"
#include "notificationslayer.h"
#include "systeminfo.h"
#include <QPluginLoader>
#include <QSettings>
#include <QDir>
#include <QApplication>
#include <QSet>
#include <QPointer>
#include <QMetaMethod>
#include <QDebug>
#include <QVarLengthArray>
#include <QLibrary>

// Is there any other way to init CryptoService from ModuleManager?
#define INSIDE_MODULE_MANAGER
#include "cryptoservice.cpp"

//Let's show message box with error
#if	defined(Q_OS_SYMBIAN)
#include <QMessageBox>
#endif

namespace qutim_sdk_0_3
{
	const char *qutimVersionStr()
	{
		return QUTIM_VERSION_STR;
	}

	quint32 qutimVersion()
	{
		return QUTIM_VERSION;
	}

	const char *metaInfo(const QMetaObject *meta, const char *name)
	{
		int num = meta ? meta->classInfoCount() : 0;
		while(num --> 0)
		{
			QMetaClassInfo info = meta->classInfo(num);
			if(!qstrcmp(info.name(), name))
				return info.value();
		}
		return 0;
	}

	enum ModuleFlag
	{
	};
	Q_DECLARE_FLAGS(ModuleFlags, ModuleFlag)

	/**
	 * ModuleManagerPrivate class used to hide inner structure of ModuleManager to provide binary compatibility between different versions.
	 */
	class ModuleManagerPrivate
	{
	public:
		inline ModuleManagerPrivate() : is_inited(false), protocols_hash(new QHash<QString, QPointer<Protocol> >()) {}
		inline ~ModuleManagerPrivate() { delete protocols_hash; }
		QList<QPointer<Plugin> > plugins;
		bool is_inited;
		union { // This union is intended to be used as reinterpret_cast =)
			QHash<QString, QPointer<Protocol> > *protocols_hash;
			QHash<QString, Protocol *> *protocols;
		};
		QHash<QString, QHash<QString, ModuleFlags> > choosed_modules;
		QHash<QByteArray, QObject *> services;
		QHash<QByteArray, ExtensionInfo> extensionsHash;
		ExtensionInfoList extensions;
		QSet<QByteArray> interface_modules;
		QSet<const QMetaObject *> meta_modules;
		QList<const ExtensionInfo> modules;
	};

	// Static Fields
	static ModuleManager *managerSelf = NULL;

	static ModuleManagerPrivate *p = NULL;

	ExtensionInfoList extensionList()
	{
		return (managerSelf && p && p->is_inited) ? p->extensions : ExtensionInfoList();
	}

	/**
	 * Function to detect if ModuleManager and it's inner data had been initialized.
	 */
	bool isCoreInited()
	{
		return managerSelf && p && p->is_inited;
	}

	QObject *getService(const QByteArray &name)
	{
		return p->services.value(name);
	}

	QList<QByteArray> getServiceNames()
	{
		return p->services.keys();
	}

	/**
	 * Returns list of ObjectGenerator's for extensions that match QMetaObject criterion
	 */
	GeneratorList moduleGenerators(const QMetaObject *module)
	{
		GeneratorList list;
//		if(isCoreInited())
		if (managerSelf && p)
		{
			QMultiMap<Plugin *, ExtensionInfo> exts = managerSelf->getExtensions(module);
			QMultiMap<Plugin *, ExtensionInfo>::const_iterator it = exts.constBegin();
			for(; it != exts.constEnd(); it++)
				list << it.value().generator();
		}
		return list;
	}

	/**
	 * Returns list of ObjectGenerator's for extensions that match char* interfaceID
	 */
	GeneratorList moduleGenerators(const char *iid)
	{
		GeneratorList list;
//		if(isCoreInited())
		if (managerSelf && p)
		{
			QMultiMap<Plugin *, ExtensionInfo> exts = managerSelf->getExtensions(iid);
			QMultiMap<Plugin *, ExtensionInfo>::const_iterator it = exts.constBegin();
			for(; it != exts.constEnd(); it++)
				list << it.value().generator();
		}
		return list;
	}

	/**
	 * Returns Map list of protocols
	 */
	ProtocolMap allProtocols()
	{
		ProtocolMap map;
		if(isCoreInited())
			map = *p->protocols;
		return map;
	}

	/**
	 * This is ModuleManager constructor.
	 */
	ModuleManager::ModuleManager(QObject *parent) : QObject(parent)
	{
		qDebug() << QIcon::themeSearchPaths();
		Q_ASSERT_X(!managerSelf, "ModuleManager", "Only one instance of ModuleManager can be created");
		p = new ModuleManagerPrivate;
		managerSelf = this;
		qApp->setApplicationName("qutIM");
		qApp->setApplicationVersion(qutimVersionStr());
		qApp->setOrganizationDomain("qutim.org");
//		qApp->setOrganizationName("qutIM");
	}

	/**
	 * This is ModuleManager destructor.
	 */
	ModuleManager::~ModuleManager()
	{
		managerSelf = 0;
	}

	/**
	 * This function used to search and load plugins.
	 */
	void ModuleManager::loadPlugins(const QStringList &additional_paths)
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "qutim", "qutimsettings");

		QStringList paths = additional_paths;
		QDir root_dir = QApplication::applicationDirPath();
		// 1. Windows, ./plugins
		QString plugin_path = root_dir.canonicalPath();
		plugin_path += QDir::separator();
		plugin_path += "plugins";
		paths << plugin_path;
		root_dir.cdUp();
		// 2. Linux, /usr/lib/qutim
		// May be it should be changed to /usr/lib/qutim/plugins ?..
		plugin_path = root_dir.canonicalPath();
		plugin_path += QDir::separator();
		plugin_path += "lib";
		plugin_path += QDir::separator();
		plugin_path += "qutim";
		paths << plugin_path;
		plugin_path += QDir::separator();
		plugin_path += "plugins";
		paths << plugin_path;
		// 3. MacOS X, ../PlugIns
		plugin_path = root_dir.canonicalPath();
		plugin_path += QDir::separator();
		plugin_path += "PlugIns";
		paths << plugin_path;
		// 4. Safe way, ~/.config/qutim/plugins
		plugin_path = QFileInfo(settings.fileName()).canonicalPath();
		plugin_path += QDir::separator();
		plugin_path += "plugins";
		paths << plugin_path;
		// 5. Symbian (ported from Nokia qutIM 0.2beta example for S60)
		//TODO use more flexible variant?
#if	defined(Q_OS_SYMBIAN)
		paths << "c:/resource/qt/plugins/qutimplugins";
		paths << "e:/resource/qt/plugins/qutimplugins";
#endif
		// 6. From config
		QStringList config_paths = settings.value("General/libpaths", QStringList()).toStringList();
		paths << config_paths;
		paths.removeDuplicates();
		QSet<QString> plugin_paths_list;
		p->extensions << coreExtensions();
		foreach (const QString &path, paths) {
			QDir plugins_dir = path;
			QFileInfoList files = plugins_dir.entryInfoList(QDir::AllEntries);
			for (int i = 0; i < files.count(); ++i) {
				QString filename = files[i].canonicalFilePath();
				if(plugin_paths_list.contains(filename) || !QLibrary::isLibrary(filename) || !files[i].isFile())
					continue;
				plugin_paths_list << filename;
				// Just don't load old plugins
				{
					typedef const char * Q_STANDARD_CALL (*QutimPluginVerificationFunction)();
					QutimPluginVerificationFunction verificationFunction = NULL;
					QScopedPointer<QLibrary> lib(new QLibrary(filename));
					if (lib->load()) {
						verificationFunction = (QutimPluginVerificationFunction)lib->resolve("qutim_plugin_query_verification_data");
						lib->unload();
						if (!verificationFunction) {
//							qDebug("'%s' has no valid verification data", qPrintable(filename));
							continue;
						}
					} else {
						qDebug("%s", qPrintable(lib->errorString()));
#if	defined(Q_OS_SYMBIAN)
						QMessageBox msg;
						msg.setText(tr("Library: %1").arg(lib->errorString()));
						msg.exec();
#endif
						continue;
					}
				}
				QPluginLoader *loader = new QPluginLoader(filename);
				if (filename.contains("quetzal"))
					loader->setLoadHints(QLibrary::ExportExternalSymbolsHint);
				QObject *object = loader->instance();
				if (Plugin *plugin = qobject_cast<Plugin *>(object)) {
					plugin->info().data()->fileName = filename;
					plugin->init();
					if (plugin->p->validate()) {
						plugin->p->is_inited = true;
						p->plugins.append(plugin);
						p->extensions << plugin->avaiableExtensions();
					} else {
						delete object;
					}
				} else {
					if (object)
						delete object;
					else {
						qWarning("%s", qPrintable(loader->errorString()));
#if	defined(Q_OS_SYMBIAN)
						QMessageBox msg;
						msg.setText(tr("Plugin: %1").arg(loader->errorString()));
						msg.exec();
#endif
					}
					loader->unload();
				}
			}
		}
		foreach (const ExtensionInfo &info, p->extensions)
			p->extensionsHash.insert(info.generator()->metaObject()->className(), info);
	}

	/**
	 * Selects all available extensions by QMetaObject criterion
	 */
	QMultiMap<Plugin *, ExtensionInfo> ModuleManager::getExtensions(const QMetaObject *service_meta) const
	{
		p->meta_modules.insert(service_meta);
		QMultiMap<Plugin *, ExtensionInfo> result;
		if (!service_meta)
			return result;
		for (int i = -1; i < p->plugins.size(); i++) {
			Plugin *plugin;
			ExtensionInfoList extensions;
			if (i < 0) {
				plugin = 0;
				extensions = managerSelf->coreExtensions();
			} else {
				plugin = p->plugins.at(i);
				if(!plugin)
					continue;
				extensions = plugin->avaiableExtensions();
			}
			for (int j = 0; j < extensions.size(); j++)
			{
				if(extensions.at(j).generator()->extends(service_meta))
					result.insert(plugin, extensions.at(j));
			}
		}
		return result;
	}

	/**
	 * Selects all available extensions by char* interface id criterion
	 */
	QMultiMap<Plugin *, ExtensionInfo> ModuleManager::getExtensions(const char *interface_id) const
	{
		QMultiMap<Plugin *, ExtensionInfo> result;
		if(!interface_id)
			return result;
		for(int i = -1; i < p->plugins.size(); i++)
		{
			Plugin *plugin;
			ExtensionInfoList extensions;
			if(i < 0)
			{
				plugin = 0;
				extensions = managerSelf->coreExtensions();
			}
			else
			{
				plugin = p->plugins.at(i);
				if(!plugin)
					continue;
				extensions = plugin->avaiableExtensions();
			}
			for(int j = 0; j < extensions.size(); j++)
			{
				if(extensions.at(j).generator()->extends(interface_id))
					result.insert(plugin, extensions.at(j));
			}
		}
		return result;
	}

	/**
	 * Initializes specific extension. To select extension type QMetaObject used.
	 */
	QObject *ModuleManager::initExtension(const QMetaObject *service_meta)
	{
		QMultiMap<Plugin *, ExtensionInfo> exts = getExtensions(service_meta);
		QMultiMap<Plugin *, ExtensionInfo>::const_iterator it = exts.begin();
		for(; it != exts.end(); it++)
		{
			QObject *obj = it.value().generator()->generate();
			qDebug("Found %s for %s", it.value().generator()->metaObject()->className(), service_meta->className());
			return obj;
		}
		qWarning("%s extension isn't found", service_meta->className());
		return 0;
	}
	
	typedef QHash<QByteArray, const ObjectGenerator*> ServiceHash;
	void initService(const QByteArray &name, QHash<QByteArray, QObject *> &services,
					 const ServiceHash &hash, QSet<QByteArray> &used,
					 QVariantMap &selected)
	{
		if (used.contains(name))
			return;
		used.insert(name);
		QString stringName = QString::fromLatin1(name, name.size());
		QString previous = selected.value(stringName).toString();
		const ObjectGenerator *gen = 0;
		if (!previous.isEmpty()) {
			gen = p->extensionsHash.value(previous.toLatin1()).generator();
			if (!gen || name != metaInfo(gen->metaObject(), "Service"))
				gen = 0;
		}
		if (!gen)
			gen = hash.value(name);
		const QMetaObject *meta = gen->metaObject();
		for (int i = 0, size = meta->classInfoCount(); i < size; i++) {
			QMetaClassInfo info = meta->classInfo(i);
			selected.insert(stringName, QString::fromLatin1(meta->className()));
			if (!qstrcmp(info.name(), "Uses"))
				initService(info.value(), services, hash, used, selected);
		}
		services.insert(name, gen->generate<QObject>());
	}

	/**
	 * Don't know
	 */
	void ModuleManager::initExtensions()
	{
//		qutim_sdk_0_3::managerSelf = initExtension<CryptoService>();
		if (ConfigPrivate::config_backends.isEmpty()) {
			// Is it really possible?.. May be we should remove it?
			QMultiMap<Plugin *, ExtensionInfo> exts = getExtensions<ConfigBackend>();
			QMultiMap<Plugin *, ExtensionInfo>::const_iterator it = exts.begin();
			for(; it != exts.end(); it++)
			{
				const QMetaObject *meta = it.value().generator()->metaObject();
				QByteArray name = metaInfo(meta, "Extension");
				if(name.isEmpty())
				{
					qWarning("%s has no 'Extension' class info", meta->className());
					continue;
				}
				else
				{
					qDebug("Found '%s' for '%s'", meta->className(), name.constData());
				}
				ConfigPrivate::config_backends << qMakePair(name, it.value().generator()->generate<ConfigBackend>());
			}
		}
		{
			const QHash<QByteArray, ExtensionInfo> &extsHash = p->extensionsHash;
			ConfigGroup group = Config().group("protocols");
			QVariantMap selected = group.value("list", QVariantMap());
			bool changed = false;
			QVariantMap::const_iterator it = selected.constBegin();
			for (; it != selected.constEnd(); it++) {
				const ExtensionInfo info = extsHash.value(it.value().toString().toLatin1());
				if (info.generator() && info.generator()->extends<Protocol>()) {
					const QMetaObject *meta = info.generator()->metaObject();
					QByteArray name = metaInfo(meta, "Protocol");
					if (name.isEmpty() || name != it.key())
						continue;
					Protocol *protocol = info.generator()->generate<Protocol>();
					p->protocols_hash->insert(protocol->id(), protocol);
				}
			}
			const ExtensionInfoList &exts = p->extensions;
			ExtensionInfoList::const_iterator it2 = exts.constBegin();
			for(; it2 != exts.end(); it2++) {
				const ObjectGenerator *gen = it2->generator();
				const QMetaObject *meta = gen->metaObject();
				if (!gen->extends<Protocol>())
					continue;
				QString name = QString::fromLatin1(metaInfo(meta, "Protocol"));
				if (name.isEmpty() || p->protocols->contains(name))
					continue;
				Protocol *protocol = gen->generate<Protocol>();
				p->protocols_hash->insert(protocol->id(), protocol);
				selected.insert(protocol->id(), QString::fromLatin1(meta->className()));
				changed = true;
			}
			if (changed) {
				group.setValue("list", selected);
				group.sync();
			}
		}
		p->is_inited = true;
		{
			ConfigGroup group = Config().group("services");
			QVariantMap selected = group.value("list", QVariantMap());
			ServiceHash serviceGens;
			const ExtensionInfoList &exts = p->extensions;
			for (int i = 0; i < exts.size(); i++) {
				const ExtensionInfo &info = exts.at(i);
				const char *serviceName = metaInfo(info.generator()->metaObject(), "Service");
				if (serviceName && *serviceName) {
					QByteArray name = serviceName;
					serviceGens.insert(name, info.generator());
				}
			}
			QSet<QByteArray> used;
			ServiceHash::iterator it;
			for (it = serviceGens.begin(); it != serviceGens.end(); it++)
				initService(it.key(), p->services, serviceGens, used, selected);
			qDebug() << "Inited Services" << used;
			group.setValue("list", selected);
			group.sync();
		}
		{
			QMultiMap<Plugin *, ExtensionInfo> exts = getExtensions(qobject_interface_iid<StartupModule *>());
			QMultiMap<Plugin *, ExtensionInfo>::const_iterator it = exts.begin();
			for(; it != exts.end(); it++)
			{
				qDebug("Startup: %s", it.value().generator()->metaObject()->className());
				it.value().generator()->generate<StartupModule>();
			}
		}
		foreach(Plugin *plugin, p->plugins) {
			if (plugin)
				plugin->load();
		}

		foreach(Protocol *proto, allProtocols())
			proto->loadAccounts();
		Notifications::sendNotification(Notifications::Startup, 0);
	}

	void ModuleManager::virtual_hook(int id, void *data)
	{
		Q_UNUSED(id);
		Q_UNUSED(data);
	}
}
