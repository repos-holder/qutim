/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "plugpackagecache.h"

plugPackageCache::plugPackageCache(QProgressBar *progress_bar,QObject *parent)
: QObject(parent)
{
	m_progress_bar=progress_bar;
}

void plugPackageCache::readMirrorList() {
	mirror_list.clear();
	mirror_list.append(mirrorInfo(	"Testing",
									QUrl ("http://sauron.savel.pp.ru/files/packages.xml"),
									"linux-x86"
									));
	
}

void plugPackageCache::updatePackagesCache() {
	readMirrorList();
	foreach (mirrorInfo mirror_info, mirror_list) {
		if (mirror_info.platform==platform) {
			plugDownloader *loader = new plugDownloader(this);
			loader->setProgressbar(m_progress_bar);
			connect(loader,SIGNAL(downloadFinished(QString)),this,SLOT(onDownloadFinished(QString)));
			loader->startDownload(downloaderItem(	mirror_info.url,
													mirror_info.name + ".xml"
												));
		}
	}
}

void plugPackageCache::onDownloadFinished(const QString& path)
{
	return;
}

plugPackageCache::~plugPackageCache() {

}