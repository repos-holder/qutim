#include <QStringBuilder>
#include <qutim/icon.h>
#include "../jaccount.h"
#include "jmucjoin.h"
#include "jbookmarkmanager.h"
#include "../servicediscovery/jservicebrowser.h"
#include "ui_jmucjoin.h"

namespace Jabber
{
	using namespace qutim_sdk_0_3;

	struct JMUCJoinPrivate
	{
		Ui::JMUCJoin *ui;
		JAccount *account;
	};

	JMUCJoin::JMUCJoin(JAccount *account, QWidget *parent) : QDialog(parent), d_ptr(new JMUCJoinPrivate)
	{
		d_ptr->account = account;
		connect(d_ptr->account->bookmarkManager(), SIGNAL(bookmarksChanged()), SLOT(fillBookmarks()));
		setInterface();
		fillBookmarks();
	}

	JMUCJoin::~JMUCJoin()
	{
		delete d_ptr->ui;
	}

	void JMUCJoin::setConference(const QString &conference, const QString &nick, const QString &password, const QString &name)
	{
		Q_D(JMUCJoin);
		if (name.isEmpty()) {
			d->ui->comboEnterBookmarks->setCurrentIndex(0);
			setEnterConference(conference, nick, password);
		} else {
			d->ui->comboEditBookmarks->setCurrentIndex(0);
			setEditConference(name, conference, nick, password);
			d->ui->buttonBookmark->setChecked(true);
		}
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
		//disconnect(d->ui->comboEditBookmarks, SIGNAL(currentIndexChanged(int)), SLOT(editBookmarkChanged(int)));
		//disconnect(d->ui->comboEnterBookmarks, SIGNAL(currentIndexChanged(int)), SLOT(enterBookmarkChanged(int)));
		d->ui->comboEditBookmarks->addItem("");
		foreach (JBookmark bookmark, d->account->bookmarkManager()->bookmarks()) {
			QString text;
			if (bookmark.name.isEmpty())
				text = bookmark.name%"\n<font color=#808080>"%bookmark.conference%"\n"%bookmark.name%"</font>";
			else
				text = bookmark.conference%"\n<font color=#808080>"%bookmark.name%"</font>";
			d->ui->comboEnterBookmarks->addItem(bookmark.name);
			d->ui->comboEditBookmarks->addItem(bookmark.name);
			d->ui->comboEditBookmarks->setItemData(d->ui->comboEditBookmarks->count() - 1,
					qVariantFromValue(bookmark), Qt::UserRole+1);
			d->ui->comboEnterBookmarks->setItemData(d->ui->comboEnterBookmarks->count(),
					qVariantFromValue(bookmark), Qt::UserRole+1);
		}
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
			d->account->bookmarkManager()->saveBookmark(name, conference, nick, password);
		}
		d->account->bookmarkManager()->saveRecent(conference, nick, password);
		//joinConference(conference, nick, password);
	}

	void JMUCJoin::on_buttonSearch_clicked()
	{
		JServiceBrowser *browser = new JServiceBrowser(d_ptr->account, true);
		connect(browser, SIGNAL(joinConference(QString)), SLOT(searchConference(QString)));
	}

	void JMUCJoin::searchConference(const QString &conference)
	{
		setConference(conference, d_ptr->account->nick(), "");
	}

	void JMUCJoin::on_buttonSave_clicked()
	{
		Q_D(JMUCJoin);
		QString name(d->ui->lineEditConferenceName->text());
		QString conference(d->ui->lineEditConference->text());
		QString nick(d->ui->lineEditNick->text());
		QString password(d->ui->lineEditPassword->text());
		if (conference.isEmpty() || nick.isEmpty())
			return;
		if (name.isEmpty())
			name = conference;
		JBookmark bookmark;
		int index = d->ui->comboEditBookmarks->currentIndex();
		if (index > 0)
			bookmark = d->ui->comboEditBookmarks->itemData(index, Qt::UserRole+1).value<JBookmark >();
		int currentIndex = index > 0 ? index : d->account->bookmarkManager()->bookmarks().count();
		if (index > 0)
			d->account->bookmarkManager()->saveBookmark(bookmark, name, conference, nick, password);
		else
			d->account->bookmarkManager()->saveBookmark(name, conference, nick, password);
		fillBookmarks();
		d->ui->comboEditBookmarks->setCurrentIndex(currentIndex);
	}

	void JMUCJoin::on_buttonDelete_clicked()
	{
		JBookmark bookmark = d_ptr->ui->comboEditBookmarks->itemData(d_ptr->ui->comboEditBookmarks->currentIndex(),
				Qt::UserRole+1).value<JBookmark >();
		if (d_ptr->ui->comboEditBookmarks->currentIndex() > 0)
			d_ptr->account->bookmarkManager()->removeBookmark(bookmark);
		fillBookmarks();
	}

	void JMUCJoin::editBookmarkChanged(int index)
	{
		if (index) {
			JBookmark bookmark = d_ptr->ui->comboEditBookmarks->itemData(index, Qt::UserRole+1).value<JBookmark >();
			setEditConference(bookmark.name, bookmark.conference,
					bookmark.nick, bookmark.password, bookmark.autojoin);
		} else {
			setEditConference("", "", "", "", false);
		}
	}

	void JMUCJoin::enterBookmarkChanged(int index)
	{
		JBookmark bookmark = d_ptr->ui->comboEditBookmarks->itemData(index, Qt::UserRole+1).value<JBookmark >();
		setEnterConference(bookmark.conference, bookmark.nick, bookmark.password);
	}

	/*************************** End actions block *****************************/

	/************************ Begin interface block ****************************/

	void JMUCJoin::setInterface()
	{
		Q_D(JMUCJoin);
		d->ui = new Ui::JMUCJoin();
		d->ui->setupUi(this);
		d->ui->buttonEnter->setIcon(Icon(""));
		d->ui->buttonBookmark->setIcon(Icon(""));
		d->ui->buttonSearch->setIcon(Icon(""));
		d->ui->buttonJoin->setIcon(Icon(""));
		d->ui->buttonSave->setIcon(Icon(""));
		d->ui->buttonDelete->setIcon(Icon(""));
		connect(d->ui->buttonEnter, SIGNAL(toggled(bool)), SLOT(switchScene(bool)));
		connect(d->ui->buttonBookmark, SIGNAL(toggled(bool)), SLOT(switchScene(bool)));
	}

	void JMUCJoin::switchScene(bool state)
	{
		if (state) {
			if (sender()->objectName() == "buttonEnter") {
				d_ptr->ui->stackedWidget->setCurrentIndex(0);
				d_ptr->ui->buttonBookmark->setChecked(false);
			} else if (sender()->objectName() == "buttonBookmark") {
				d_ptr->ui->stackedWidget->setCurrentIndex(1);
				d_ptr->ui->buttonEnter->setChecked(false);
			}
		}
	}

	/************************** End interface block ****************************/
}
