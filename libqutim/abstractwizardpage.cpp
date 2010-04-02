/****************************************************************************
 *  abstractwizardpage.cpp
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

#include "abstractwizardpage.h"

namespace qutim_sdk_0_3
{
	AbstractWizardPage::AbstractWizardPage()
	{
	}

	AbstractWizardPage::~AbstractWizardPage()
	{
	}

	void AbstractWizardPage::virtual_hook(int id, void *data)
	{
		Q_UNUSED(id);
		Q_UNUSED(data);
	}
}
