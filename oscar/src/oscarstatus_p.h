/****************************************************************************
 *  oscarstatus_p.h
 *
 *  Copyright (c) 2010 by Prokhin Alexey <alexey.prokhin@yandex.ru>
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

#ifndef OSCARSTATUS_P_H
#define OSCARSTATUS_P_H

#include "oscarstatus.h"

namespace Icq
{

typedef QSet<QString> CapsTypes;
typedef QHash<QString, Capability> CapsList;
CapsTypes &capsTypes();
typedef QHash<quint16, Status> OscarStatusList;

} // namespace Icq

Q_DECLARE_METATYPE(Icq::CapsList);

#endif // OSCARSTATUS_P_H
