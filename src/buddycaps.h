/****************************************************************************
 *  buddycaps.h
 *
 *  Copyright (c) 2008 by Alexey Ignatiev <twosev@gmail.com>
 *  Copyright (c) 2009 by Prokhin Alexey <alexey.prokhin@yandex.ru>
 *
 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
****************************************************************************/

#ifndef CAPABILITIES_H_
#define CAPABILITIES_H_

#include <capability.h>

namespace Icq
{

const Capability ICQ_CAPABILITY_SRVxRELAY    (0x09, 0x46, 0x13, 0x49, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_SHORTCAPS    (0x09, 0x46, 0x00, 0x00, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIMVOICE     (0x09, 0x46, 0x13, 0x41, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIMSENDFILE  (0x09, 0x46, 0x13, 0x43, 0x4c, 0x7f,
                                              0x11, 0xd1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_DIRECT       (0x09, 0x46, 0x13, 0x44, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIMIMAGE     (0x09, 0x46, 0x13, 0x45, 0x4c, 0x7f,
                                              0x11, 0xd1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIMICON      (0x09, 0x46, 0x13, 0x46, 0x4c, 0x7f,
                                              0x11, 0xd1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIM_STOCKS   (0x09, 0x46, 0x13, 0x47, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIMGETFILE   (0x09, 0x46, 0x13, 0x48, 0x4c, 0x7f,
                                              0x11, 0xd1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIM_GAMES    (0x09, 0x46, 0x13, 0x4A, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_BUDDY_LIST   (0x09, 0x46, 0x13, 0x4B, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AVATAR       (0x09, 0x46, 0x13, 0x4C, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIM_SUPPORT  (0x09, 0x46, 0x13, 0x4D, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_UTF8         (0x09, 0x46, 0x13, 0x4E, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_RTFxMSGS     (0x97, 0xB1, 0x27, 0x51, 0x24, 0x3C,
                                              0x43, 0x34, 0xAD, 0x22, 0xD6, 0xAB,
                                              0xF7, 0x3F, 0x14, 0x92);

const Capability ICQ_CAPABILITY_TYPING       (0x56, 0x3F, 0xC8, 0x09, 0x0B, 0x6F,
                                              0x41, 0xBD, 0x9F, 0x79, 0x42, 0x26,
                                              0x09, 0xDF, 0xA2, 0xF3);

const Capability ICQ_CAPABILITY_AIMxINTER    (0x09, 0x46, 0x13, 0x4D, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_XTRAZ        (0x1A, 0x09, 0x3C, 0x6C, 0xD7, 0xFD,
                                              0x4E, 0xC5, 0x9D, 0x51, 0xA6, 0x47,
                                              0x4E, 0x34, 0xF5, 0xA0);

const Capability ICQ_CAPABILITY_BART         (0x09, 0x46, 0x13, 0x46, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIMCHAT      (0x74, 0x8F, 0x24, 0x20, 0x62, 0x87,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_HTMLMSGS     (0x01, 0x38, 0xca, 0x7b, 0x76, 0x9a,
                                              0x49, 0x15, 0x88, 0xf2, 0x13, 0xfc,
                                              0x00, 0x97, 0x9e, 0xa8);

const Capability ICQ_CAPABILITY_LIVEVIDEO    (0x09, 0x46, 0x01, 0x01, 0x4c, 0x7f,
                                              0x11, 0xd1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_IMSECURE     ('I', 'M', 's', 'e', 'c', 'u', 'r',
                                              'e', 'C', 'p', 'h', 'r', 0x00,
                                              0x00, 0x06, 0x01);

const Capability ICQ_CAPABILITY_MSGTYPE2     (0x09, 0x49, 0x13, 0x49, 0x4c, 0x7f,
                                              0x11, 0xd1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIMICQ       (0x09, 0x46, 0x13, 0x4D, 0x4C, 0x7F,
                                              0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIMAUDIO     (0x09, 0x46, 0x01, 0x04, 0x4c, 0x7f,
                                              0x11, 0xd1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

//AIM Client version 5.9 capabilities
const Capability ICQ_CAPABILITY_AIMADDINGS   (0x09, 0x46, 0x13, 0x47, 0x4c, 0x7f,
                                              0x11, 0xd1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIMCONTSEND  (0x09, 0x46, 0x13, 0x4b, 0x4c, 0x7f,
                                              0x11, 0xd1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIMUNK2      (0x09, 0x46, 0x01, 0x02, 0x4c, 0x7f,
                                              0x11, 0xd1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x00, 0x00);

const Capability ICQ_CAPABILITY_AIMSNDBDDLST (0x09, 0x46, 0x00, 0x00, 0x4c, 0x7f,
                                              0x11, 0xd1, 0x82, 0x22, 0x44, 0x45,
                                              0x53, 0x54, 0x13, 0x4B);

const Capability ICQ_CAPABILITY_IMSECKEY1    (0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                              0x00, 0x00, 0x00, 0x00);

const Capability ICQ_CAPABILITY_IMSECKEY2    (0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                              0x00, 0x00, 0x00, 0x00);

const Capability ICQ_CAPABILITY_PSIG_MESSAGE (0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											  0x00, 0x00, 0x00, 0x00);

} // namespace Icq

#endif /*CAPABILITIES_H_*/
