/****************************************************************************
 *  objectgenerator.cpp
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

#include "actiongenerator_p.h"
#include <QtCore/QVariant>
#include <QtCore/QCoreApplication>
#include "modulemanager_p.h"

namespace qutim_sdk_0_3
{
ObjectGeneratorHolderData::ObjectGeneratorHolderData(ObjectGenerator *generator) : QSharedData()
{
	m_generator = generator;
}

ObjectGenerator *ObjectGeneratorHolderData::generator() const
{
	return m_generator;
}

ActionGenerator *ObjectGeneratorHolderData::actionGenerator() const
{
	return static_cast<ActionGenerator*>(m_generator);
}

ObjectGenerator::ObjectGenerator() : d_ptr(new ObjectGeneratorPrivate)
{
}

ObjectGenerator::ObjectGenerator(ObjectGeneratorPrivate &priv) : d_ptr(&priv)
{
}

ObjectGenerator::~ObjectGenerator()
{
	Q_D(ObjectGenerator);
	if (d->pointer)
		d->pointer->m_generator = 0;
}

ObjectGenerator *ObjectGenerator::addProperty(const QByteArray &name, const QVariant &value)
{
	Q_D(ObjectGenerator);
	int index = d->names.indexOf(name);
	if (index != -1) {
		d->values[index] = value;
	} else {
		d->names.append(name);
		d->values.append(value);
	}
	return this;
}

bool ObjectGenerator::extends(const QMetaObject *super) const
{
	const QMetaObject *meta = metaObject();
	while (meta && meta != super)
		meta = meta->superClass();
	return super && meta == super;
}

bool ObjectGenerator::hasInterface(const char *id) const
{
	return interfaces().contains(QByteArray::fromRawData(id, qstrlen(id)));
}

QList<QByteArray> ObjectGenerator::interfaces() const
{
	return QList<QByteArray>();
}

bool ObjectGenerator::isInited()
{
	return isCoreInited();
}

GeneratorList ObjectGenerator::module(const QMetaObject *module)
{
	return moduleGenerators(module, 0);
}

GeneratorList ObjectGenerator::module(const char *iid)
{
	return moduleGenerators(0, iid);
}

ObjectGenerator::Ptr ObjectGenerator::pointerHolder()
{
	Q_D(ObjectGenerator);
	if (!d->pointer)
		d->pointer = new ObjectGeneratorHolderData(this);
	return d->pointer;
}

QObject *ObjectGenerator::generateHelper2() const
{
	Q_D(const ObjectGenerator);
	if (QObject *obj = generateHelper()) {
		for (int i = 0; i < d->names.size(); i++)
			obj->setProperty(d->names.at(i), d->values.at(i));
		if (d->info.data())
			obj->setProperty("extensioninfo", QVariant::fromValue(d->info));
		if (dynamic_cast<const ActionGenerator*>(this) && qobject_cast<QAction*>(obj)) {
			//				const ActionGeneratorPrivate *p = static_cast<const ActionGeneratorPrivate*>(d);
			//				if (p->connectionType == ActionConnectionLegacy) {
			//					const QList<QPointer<QObject> > &handlers = p->legacyData->handlers;
			//					ActionGenerator *gen = const_cast<ActionGenerator*>(static_cast<const ActionGenerator*>(this));
			//					ActionCreatedEvent event(static_cast<QAction*>(obj), gen);
			//					for (int i = 0; i < handlers.size(); i++) {
			//						if (QObject *handler = handlers.at(i))
			//							qApp->sendEvent(handler, &event);
			//					}
			//				}
		}
		return obj;
	}
	return NULL;
}

ExtensionInfo ObjectGenerator::info() const
{
	return d_func()->info;
}

void ObjectGenerator::virtual_hook(int id, void *data)
{
	Q_UNUSED(id);
	Q_UNUSED(data);
}
}
