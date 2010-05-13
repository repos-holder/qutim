#include <QStringBuilder>
#include <qutim/icon.h>
#include "../jaccount.h"
#include "jmucjoin.h"
#include "jbookmarkmanager.h"
#include "../servicediscovery/jservicebrowser.h"
#include "ui_jmucjoin.h"
#include "jmucmanager.h"
#include "jmucjoinbookmarksitemdelegate.h"

namespace Jabber
{
	using namespace qutim_sdk_0_3;

	struct JMUCJoinPrivate
	{
		Ui::JMUCJoin *ui;
		JAccount *account;
		JMUCManager *conferenceManager;
	};

	JMUCJoin::JMUCJoin(JAccount *account, QWidget *parent) : QDialog(parent), d_ptr(new JMUCJoinPrivate)
	{
		d_ptr->account = account;
		d_ptr->conferenceManager = d_ptr->account->conferenceManager();
		connect(d_ptr->conferenceManager->bookmarkManager(), SIGNAL(serverBookmarksChanged()), SLOT(fillBookmarks()));
		setInterface();
		fillBookmarks();
	}

	JMUCJoin::~JMUCJoin()
	{
		delete d_ptr->ui;
	}

	void JMUCJoin::setConference(const QString &conference, const QString &nick, const QString &password,
			const QString &name)
	{
		Q_D(JMUCJoin);
		if (name.isEmpty()) {
			d->ui->comboEnterBookmarks->setCurrentIndex(0);
			setEnterConference(conference, nick, password);
		} else {
			d->ui->comboEditBookmarks->setCurrentIndex(0);
			setEditConference(name, conference, nick, password);
		}
		JMUCJoinBookmarksItemDelegate *delegate = new JMUCJoinBookmarksItemDelegate(this);
		d->ui->comboEditBookmarks->setItemDelegate(delegate);
		d->ui->comboEnterBookmarks->setItemDelegate(delegate);
	}

	void JMUCJoin::setConference(const QString &conference)
	{
		setConference(conference, d_ptr->account->name(), "");
	}

	void JMUCJoin::setEditConference(const QString &name, const QString &conference,
			const QString &nick, const QString &password, bool autojoin)
	{
		Q_D(JMUCJoin);
		d->ui->lineEditConferenceName->setText(name);
		d->ui->lineEditConference->setText(conference);
		d->ui->lineEditNick->setText(nick);
		d->ui->lineEditPassword->setText(password);
		d->ui->checkEditAutojoin->setChecked(autojoin);
	}

	void JMUCJoin::setEnterConference(const QString &conference, const QString &nick, const QString &password)
	{
		Q_D(JMUCJoin);
		d->ui->lineEnterConference->setText(conference);
		d->ui->lineEnterNick->setText(nick);
		d->ui->lineEnterPassword->setText(password);
		d->ui->checkSaveBookmark->setChecked(false);
		d->ui->lineEnterConferenceName->setText("");
	}

	void JMUCJoin::fillBookmarks()
	{
		Q_D(JMUCJoin);
		d->ui->comboEditBookmarks->clear();
		d->ui->comboEnterBookmarks->clear();
		d->ui->comboEditBookmarks->addItem("");
		d->ui->comboEnterBookmarks->addItem("");
		foreach (JBookmark bookmark, d->conferenceManager->bookmarkManager()->bookmarks()) {
			d->ui->comboEnterBookmarks->addItem(bookmark.name);
			d->ui->comboEnterBookmarks->setItemData(d->ui->comboEnterBookmarks->count() - 1,
					qVariantFromValue(bookmark), Qt::UserRole+1);
			d->ui->comboEditBookmarks->addItem(bookmark.name);
			d->ui->comboEditBookmarks->setItemData(d->ui->comboEditBookmarks->count() - 1,
					qVariantFromValue(bookmark), Qt::UserRole+1);
		}
		int separator = d->ui->comboEnterBookmarks->count();
		bool isRecent = false;
		foreach (JBookmark bookmark, d->conferenceManager->bookmarkManager()->recent()) {
			if (d->conferenceManager->bookmarkManager()->bookmarks().contains(bookmark))
				continue;
			d->ui->comboEnterBookmarks->addItem(bookmark.conference);
			d->ui->comboEnterBookmarks->setItemData(d->ui->comboEnterBookmarks->count() - 1,
					qVariantFromValue(bookmark), Qt::UserRole+1);
			isRecent = true;
		}
		if (isRecent)
			d->ui->comboEnterBookmarks->insertSeparator(separator);
		connect(d->ui->comboEditBookmarks, SIGNAL(currentIndexChanged(int)), SLOT(editBookmarkChanged(int)));
		connect(d->ui->comboEnterBookmarks, SIGNAL(currentIndexChanged(int)), SLOT(enterBookmarkChanged(int)));
		d->ui->comboEnterBookmarks->setCurrentIndex(0);
		d->ui->comboEditBookmarks->setCurrentIndex(0);
	}

	/************************* Begin actions block *****************************/

	void JMUCJoin::on_buttonJoin_clicked()
	{
		Q_D(JMUCJoin);
		QString name(d->ui->lineEnterConferenceName->text());
		QString conference(d->ui->lineEnterConference->text());
		QString nick(d->ui->lineEnterNick->text());
		QString password(d->ui->lineEnterPassword->text());
		if (conference.isEmpty() || nick.isEmpty())
			return;
		if (d->ui->checkSaveBookmark->isChecked() && !name.isEmpty()) {
			d->conferenceManager->bookmarkManager()->saveBookmark(d->conferenceManager->bookmarkManager()->bookmarks().count(),
					name, conference, nick, password);
		}
		d->conferenceManager->bookmarkManager()->saveRecent(conference, nick, password);
		d->conferenceManager->join(conference, nick, password);
		close();
	}

	void JMUCJoin::on_buttonSearch_clicked()
	{
		JServiceBrowser *browser = new JServiceBrowser(d_ptr->account, true);
		connect(browser, SIGNAL(joinConference(QString)), SLOT(setConference(QString)));
		browser->show();
	}

	void JMUCJoin::on_buttonSave_clicked()
	{
		Q_D(JMUCJoin);
		QString name(d->ui->lineEditConferenceName->text());
		QString conference(d->ui->lineEditConference->text());
		QString nick(d->ui->lineEditNick->text());
		QString password(d->ui->lineEditPassword->text());
		bool autojoin = d->ui->checkEditAutojoin->isChecked();
		if (conference.isEmpty())
			return;
		if (name.isEmpty())
			name = conference;
		int currentIndex = d->ui->comboEditBookmarks->currentIndex() > 0 ?
				d->ui->comboEditBookmarks->currentIndex()-1 : d->conferenceManager->bookmarkManager()->bookmarks().count();
		d->conferenceManager->bookmarkManager()->saveBookmark(currentIndex, name, conference, nick, password, autojoin);
		fillBookmarks();
		d->ui->comboEditBookmarks->setCurrentIndex(currentIndex+1);
	}

	void JMUCJoin::on_buttonDelete_clicked()
	{
		if (d_ptr->ui->comboEditBookmarks->currentIndex() > 0)
			d_ptr->conferenceManager->bookmarkManager()->removeBookmark(d_ptr->ui->comboEditBookmarks->currentIndex()-1);
		fillBookmarks();
	}

	void JMUCJoin::editBookmarkChanged(int index)
	{
		if (index) {
			JBookmark bookmark = d_ptr->ui->comboEditBookmarks->itemData(index, Qt::UserRole+1).value<JBookmark>();
			setEditConference(bookmark.name, bookmark.conference,
					bookmark.nick, bookmark.password, bookmark.autojoin);
			d_ptr->ui->comboEditBookmarks->setEditText(bookmark.name);
		} else {
			setEditConference("", "", "", "", false);
		}
	}

	void JMUCJoin::enterBookmarkChanged(int index)
	{
		JBookmark bookmark = d_ptr->ui->comboEnterBookmarks->itemData(index, Qt::UserRole+1).value<JBookmark>();
		setEnterConference(bookmark.conference, bookmark.nick, bookmark.password);
		if (bookmark.name.isEmpty())
			d_ptr->ui->comboEnterBookmarks->setEditText(bookmark.conference);
		else
			d_ptr->ui->comboEnterBookmarks->setEditText(bookmark.name);
	}

	/*************************** End actions block *****************************/

	/************************ Begin interface block ****************************/

	void JMUCJoin::setInterface()
	{
		Q_D(JMUCJoin);
		d->ui = new Ui::JMUCJoin();
		d->ui->setupUi(this);
		centerizeWidget(this);
		d->ui->stackedWidget->setCurrentIndex(0);
		d->ui->toolBar->setIconSize(QSize(32,32));
		d->ui->toolBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);

		QActionGroup *group = new QActionGroup (this);
		group->setExclusive(true);

		QAction *act = new QAction(Icon("meeting-attending"),tr("Enter"),group);
		act->setCheckable(true);
		act->setChecked(true);
		act->setData(0);
		connect(act,SIGNAL(toggled(bool)),SLOT(switchScene(bool)));
		d->ui->toolBar->addAction(act);
		act = new QAction(Icon("bookmarks-organize"),tr("Edit Bookmarks"),group);
		act->setData(1);
		act->setCheckable(true);
		connect(act,SIGNAL(toggled(bool)),SLOT(switchScene(bool)));
		d->ui->toolBar->addAction(act);

#ifdef Q_WS_WIN
		d->ui->toolBar->setStyleSheet("QToolBar{background:none;border:none;}"); //HACK

//		if (QtWin::isCompositionEnabled()) {
//			QtWin::extendFrameIntoClientArea(this,layout()->margin(),
//											 layout()->margin(),
//											 d->ui->toolBar->size().height()+ d->ui->toolBar->geometry().y(),
//											 layout()->margin());
//			d->ui->stackedWidget->setAutoFillBackground(true);
//		}

#endif
	}

	void JMUCJoin::switchScene(bool state)
	{
		if (state) {
			QAction *act = qobject_cast<QAction*>(sender());
			Q_ASSERT(act);
			d_ptr->ui->stackedWidget->setCurrentIndex(act->data().toInt());
		}
	}

	/************************** End interface block ****************************/
}
