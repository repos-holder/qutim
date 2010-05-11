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

#include "multimediasoundbackend.h"
#include "modulemanagerimpl.h"
#include <QAudioDeviceInfo>
#include <QFile>
#include <QDataStream>
#include <QtEndian>
#include <QDebug>

static Core::CoreModuleHelper<Core::MultimediaSoundBackend> sound_theme_static(
		QT_TRANSLATE_NOOP("Plugin", "Mulimedia engine"),
		QT_TRANSLATE_NOOP("Plugin", "Sound engine based on QtMultimedia")
		);

namespace Core
{
MultimediaSoundBackend::MultimediaSoundBackend()
{
}

void MultimediaSoundBackend::playSound(const QString &filename)
{
	QScopedPointer<QFile> file(new QFile(filename));
	if (file->size() < 8 || !file->open(QIODevice::ReadOnly))
		return;
	
	quint32 RIFF_str = 0x46464952;
	quint32 WAVE_str = 0x45564157;
	quint32 fmt_str  = 0x20746D66;
	quint32 data_str = 0x61746164;
	
	QDataStream in(file.data());
	in.setByteOrder(QDataStream::LittleEndian);
	
	quint32 riffId, riffLength, waveId, waveFmt, waveLength;
	in >> riffId >> riffLength >> waveId >> waveFmt >> waveLength;
	if (riffId != RIFF_str || waveId != WAVE_str || waveFmt != fmt_str) {
		qWarning() << filename << "is not valid WAV file";
		return;
	}
	
	quint16 type, channels, align, bitsPerSample;
	quint32 frequency, bytesPerSec;
	in >> type >> channels >> frequency >> bytesPerSec >> align >> bitsPerSample;
	in.skipRawData(waveLength - 16);
	if (type != 1) {
		qWarning("Unsupported WAV type: 0x%s", qPrintable(QString::number(type, 16)));
		return;
	}
	quint32 dataId, dataSize;
	in >> dataId >> dataSize;
	if (dataId != data_str) {
		qWarning() << filename << "is not valid WAV file";
		return;
	}
	QAudioFormat format;
	format.setChannels(channels);
	format.setFrequency(frequency);
	format.setSampleSize(bitsPerSample);
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(bitsPerSample == 8 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);
	format.setCodec("audio/pcm");
	QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
	
	if (!info.isFormatSupported(format)) {
		qWarning() << "Raw audio format not supported by backend, cannot play audio.";
		return;
	}
	QAudioOutput *audio = new QAudioOutput(format, this);
	file->setParent(audio);
	connect(audio, SIGNAL(stateChanged(QAudio::State)), SLOT(finishedPlaying(QAudio::State)));
	audio->start(file.take());
}

QStringList MultimediaSoundBackend::supportedFormats()
{
	return QStringList() << "wav";
}

void MultimediaSoundBackend::finishedPlaying(QAudio::State state)
{   
	if(state == QAudio::IdleState) {
		QAudioOutput *audio = qobject_cast<QAudioOutput*>(sender());
		audio->stop();
		delete audio;
	}
}
}
