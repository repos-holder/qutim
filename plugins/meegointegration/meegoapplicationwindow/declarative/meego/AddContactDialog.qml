/*
 * Copyright (C) 2011 Evgeniy Degtyarev <degtep@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

import QtQuick 1.0
import com.nokia.meego 1.0
import org.qutim 0.3

Sheet {
	id: addContactDialog
	AddContactDialogWrapper {
		id: handler
	}
	acceptButtonText: qsTr("Close")

	content: Item {
		id: contentArea
		objectName: "contentArea"
		anchors.fill: parent
		anchors { top: parent.top; left: parent.left; right: parent.right; bottom: parent.bottom; }
		anchors.bottomMargin: toolBar.visible || (toolBar.opacity==1)? toolBar.height : 0

		PageStack {
			id: pageStack
			anchors { top: parent.top; left: parent.left; right: parent.right; bottom: parent.bottom; }
			//anchors.bottomMargin: toolBar.visible || (toolBar.opacity==1)? toolBar.height : 0
			toolBar: toolBar
		}
		ToolBar {
			objectName: "toolBar"
			anchors.top: contentArea.bottom
			id: toolBar
			// Don't know why I have to do it manually
			onVisibleChanged: if (__currentContainer) __currentContainer.visible = visible;
		}
	}
	Page
	{
		id:mainPage
		anchors.margins:10
		anchors.fill: parent


		Text {
			id:header
			anchors{top:parent.top; left:parent.left; right:parent.right}
			font.pointSize: 40
			text:qsTr("Select protocol:")
		}

		ListView {
			id:mainInfo
			spacing: 20
			anchors{top:header.bottom; left:parent.left; right:parent.right;bottom:parent.bottom}

			model: handler
			delegate: ItemDelegate {
				title: account.id
				subtitle: account.id
				onClicked: {
					channel.setAccount(account);
					//channel.active = true
					//channel.showChat()
				}

			}
		}


	}

	Page
	{
		id:addContactPage
		Column {
				anchors.fill: parent
				id: addContactDialogContent
				spacing: 10

				Row {
					spacing: 10
					Label {
						id:addContactIdLabel
						text: handler.contactIdLabel
					}
					TextField {
						id:addContactIdText
					}
				}

				Row {
					spacing: 10
					Label {
						id:addContactNameLabel

						text: qsTr("Name")
					}
					TextField {
						id:addContactNameText
					}
				}
				Button { id:acceptButton; text: qsTr("Add contact"); onClicked: handler.addContact(addContactIdText.text,addContactNameText.text); }
		}
		tools: ToolBarLayout {
			ToolIcon {
				visible: true
				platformIconId: "toolbar-previous"
				onClicked: pageStack.pop()
			}
		}

	}

	onStatusChanged: {
		if (status == PageStatus.Inactive && pageStack.currentPage != mainPage) {
			pageStack.clear();
			pageStack.push(mainPage);
		}
		handler.loadAccounts();
	}
	Component.onCompleted: {
		__owner = parent;
		pageStack.push(mainPage);
	}
}
