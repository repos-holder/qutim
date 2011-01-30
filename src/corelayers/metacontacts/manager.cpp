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

#include "manager.h"
#include <qutim/protocol.h>
#include <qutim/debug.h>
#include <qutim/actiongenerator.h>
#include <qutim/icon.h>
#include "mergedialog.h"
#include <qutim/systemintegration.h>
#include <qutim/contactlist.h>
#include "factory.h"

namespace Core
{
namespace MetaContacts
{
using namespace qutim_sdk_0_3;

Manager::Manager() : 
	m_storage(RosterStorage::instance()),
	m_factory(new Factory(this)),
	m_blockUpdate(false)
{
	ActionGenerator *gen = new ActionGenerator(Icon("list-remove-user"),
											   QT_TRANSLATE_NOOP("MetaContact","Split Metacontact"),
											   this,
											   SLOT(onSplitTriggered(QObject*)));
	gen->setType(ActionTypeContactList);
	MenuController::addAction<MetaContactImpl>(gen);
	gen = new ActionGenerator(Icon("list-add-user"),
							  QT_TRANSLATE_NOOP("MetaContact","Manage metacontacts"),
							  this,
							  SLOT(onCreateTriggered(QObject*)));
	gen->setType(ActionTypeContactList);
	MenuController::addAction<MetaContactImpl>(gen);
	MenuController::addAction<ContactList>(gen);

	connect(this, SIGNAL(contactCreated(qutim_sdk_0_3::Contact*)), SLOT(onContactCreated(qutim_sdk_0_3::Contact*)));

	setContactsFactory(m_factory.data());
}

Manager::~Manager()
{
}

ChatUnit *Manager::getUnit(const QString &unitId, bool create)
{
	MetaContactImpl *contact = m_contacts.value(unitId);
	if (!contact && create) {
		contact = new MetaContactImpl(unitId);
		m_contacts.insert(unitId, contact);
		emit contactCreated(contact);
	}
	return contact;
}

void Manager::loadContacts()
{
	m_blockUpdate = true;
	m_storage->load(this);
	m_blockUpdate = false;
}

void Manager::onSplitTriggered(QObject *object)
{
	//TODO implement logic
	MetaContactImpl *metaContact = qobject_cast<MetaContactImpl*>(object);
	foreach (Contact *c, metaContact->contacts()) {
		metaContact->removeContact(c);
	}
}

void Manager::onCreateTriggered(QObject *obj)
{
	MergeDialog *dialog = new MergeDialog;
	if(MetaContactImpl *m = qobject_cast<MetaContactImpl*>(obj))
		dialog->setMetaContact(m);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	centerizeWidget(dialog);
	SystemIntegration::show(dialog);
}

QString Manager::name() const
{
	//TODO implement logic
	return (QT_TRANSLATE_NOOP("Metacontact","You")).toString();
}

void Manager::onContactCreated(qutim_sdk_0_3::Contact *contact)
{
	if(!m_blockUpdate) {
		m_storage->addContact(contact);
	}
}

}
}
