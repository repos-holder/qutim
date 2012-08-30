import qbs.base 1.0

DynamicLibrary {
    name: "libqutim"
    destination: "lib"
    
    property string versionMajor: project.qutim_version_major
    property string versionMinor: project.qutim_version_minor
    property string versionRelease: project.qutim_version_release
    property string versionPatch: project.qutim_version_patch
    property string version: project.qutim_version

    Depends { name: "qutim-headers" }
    Depends { name: "k8json" }
    Depends { name: "qxt" }
    Depends { name: "qxdg" }
    Depends { name: "qtdwm" }
    Depends { name: "flowlayout" }
    Depends { name: "slidingstackedwidget" }
    Depends { name: "qtsolutions" }
    Depends { name: "cpp" }
    Depends { name: "qt"; submodules: [ 'core', 'gui', 'network', 'script', 'declarative' ] }

    cpp.includePaths: [
        "3rdparty",
        "3rdparty/qxt",
        buildDirectory + "/include/qutim"
    ]

    cpp.dynamicLibraries: {
        if (qbs.targetOS == 'linux' || qbs.targetOS == 'freebsd') {
            return [ 'X11' ];
        }
        return [];
    }

    cpp.dynamicLibraryPrefix: ""
    cpp.staticLibraryPrefix: ""
    cpp.defines: [ "LIBQUTIM_LIBRARY", "QUTIM_SHARE_DIR=\"" + project.shareDir + "\""]

    files: [
        "libqutim/abstractsearchrequest.h",
        "libqutim/abstractwizardpage.h",
        "libqutim/account.h",
        "libqutim/accountmanager.h",
        "libqutim/actionbox.h",
        "libqutim/actiongenerator.h",
        "libqutim/actiontoolbar.h",
        "libqutim/authorizationdialog.h",
        "libqutim/buddy.h",
        "libqutim/chatsession.h",
        "libqutim/chatunit.h",
        "libqutim/conference.h",
        "libqutim/configbase.h",
        "libqutim/config.h",
        "libqutim/contact.h",
        "libqutim/contactsearch.h",
        "libqutim/cryptoservice.h",
        "libqutim/dataforms.h",
        "libqutim/datasettingsobject.h",
        "libqutim/debug.h",
        "libqutim/declarativeview.h",
        "libqutim/emoticons.h",
        "libqutim/event.h",
        "libqutim/extensionicon.h",
        "libqutim/extensioninfo.h",
        "libqutim/filetransfer.h",
        "libqutim/groupchatmanager.h",
        "libqutim/history.h",
        "libqutim/icon.h",
        "libqutim/iconloader.h",
        "libqutim/inforequest.h",
        "libqutim/jsonfile.h",
        "libqutim/json.h",
        "libqutim/libqutim_global.h",
        "libqutim/localizedstring.h",
        "libqutim/menucontroller.h",
        "libqutim/message.h",
        "libqutim/messagehandler.h",
        "libqutim/metacontact.h",
        "libqutim/metacontactmanager.h",
        "libqutim/metaobjectbuilder.h",
        "libqutim/mimeobjectdata.h",
        "libqutim/modulemanager.h",
        "libqutim/networkproxy.h",
        "libqutim/notification.h",
        "libqutim/objectgenerator.h",
        "libqutim/passworddialog.h",
        "libqutim/personinfo.h",
        "libqutim/plugin.h",
        "libqutim/profilecreatorpage.h",
        "libqutim/profile.h",
        "libqutim/protocol.h",
        "libqutim/tooltip.h",
        "libqutim/qtwin.h",
        "libqutim/rosterstorage.h",
        "libqutim/scripttools.h",
        "libqutim/servicemanager.h",
        "libqutim/settingslayer.h",
        "libqutim/settingswidget.h",
        "libqutim/shortcut.h",
        "libqutim/sound.h",
        "libqutim/spellchecker.h",
        "libqutim/startupmodule.h",
        "libqutim/statusactiongenerator.h",
        "libqutim/status.h",
        "libqutim/systeminfo.h",
        "libqutim/systemintegration.h",
        "libqutim/tcpsocket.h",
        "libqutim/thememanager.h",
        "libqutim/utils/avatarfilter.h",
        "libqutim/utils.h",
        "libqutim/utils/itemdelegate.h",
        "libqutim/widgetactiongenerator.h",
        "libqutim/abstractsearchrequest_p.h",
        "libqutim/account_p.h",
        "libqutim/actionbox_p.h",
        "libqutim/actiongenerator_p.h",
        "libqutim/actiontoolbar_p.h",
        "libqutim/buddy_p.h",
        "libqutim/chatunit_p.h",
        "libqutim/contact_p.h",
        "libqutim/datasettingsobject_p.h",
        "libqutim/dglobalhotkey_p.h",
        "libqutim/dynamicpropertydata_p.h",
        "libqutim/event_test_p.h",
        "libqutim/extensionsearch_p.h",
        "libqutim/groupchatmanager_p.h",
        "libqutim/iconbackend_p.h",
        "libqutim/menucontroller_p.h",
        "libqutim/metacontactprotocol_p.h",
        "libqutim/modulemanager_p.h",
        "libqutim/objectgenerator_p.h",
        "libqutim/plugin_p.h",
        "libqutim/protocol_p.h",
        "libqutim/servicemanager_p.h",
        "libqutim/settingslayer_p.h",
        "libqutim/shortcut_p.h",
        "libqutim/sound_p.h",
        "libqutim/statisticshelper_p.h",
        "libqutim/statusactiongenerator_p.h",
        "libqutim/systemintegration_p.h",
        "libqutim/utils/avatariconengine_p.h",
        "libqutim/varianthook_p.h",
        "libqutim/abstractsearchrequest.cpp",
        "libqutim/abstractwizardpage.cpp",
        "libqutim/account.cpp",
        "libqutim/accountmanager.cpp",
        "libqutim/actionbox.cpp",
        "libqutim/actiongenerator.cpp",
        "libqutim/actiontoolbar.cpp",
        "libqutim/authorizationdialog.cpp",
        "libqutim/buddy.cpp",
        "libqutim/chatsession.cpp",
        "libqutim/chatunit.cpp",
        "libqutim/conference.cpp",
        "libqutim/config.cpp",
        "libqutim/contact.cpp",
        "libqutim/contactsearch.cpp",
        "libqutim/cryptoservice.cpp",
        "libqutim/dataforms.cpp",
        "libqutim/datasettingsobject.cpp",
        "libqutim/debug.cpp",
        "libqutim/declarativeview.cpp",
        "libqutim/dglobalhotkey.cpp",
        "libqutim/dynamicpropertydata.cpp",
        "libqutim/emoticons.cpp",
        "libqutim/event.cpp",
        "libqutim/event_test.cpp",
        "libqutim/extensionicon.cpp",
        "libqutim/extensioninfo.cpp",
        "libqutim/extensionsearch.cpp",
        "libqutim/filetransfer.cpp",
        "libqutim/groupchatmanager.cpp",
        "libqutim/history.cpp",
        "libqutim/iconbackend.cpp",
        "libqutim/icon.cpp",
        "libqutim/iconloader.cpp",
        "libqutim/inforequest.cpp",
        "libqutim/json.cpp",
        "libqutim/jsonfile.cpp",
        "libqutim/localizedstring.cpp",
        "libqutim/menucontroller.cpp",
        "libqutim/message.cpp",
        "libqutim/messagehandler.cpp",
        "libqutim/metacontact.cpp",
        "libqutim/metacontactmanager.cpp",
        "libqutim/metacontactprotocol.cpp",
        "libqutim/metaobjectbuilder.cpp",
        "libqutim/mimeobjectdata.cpp",
        "libqutim/modulemanager.cpp",
        "libqutim/networkproxy.cpp",
        "libqutim/notification.cpp",
        "libqutim/objectgenerator.cpp",
        "libqutim/passworddialog.cpp",
        "libqutim/personinfo.cpp",
        "libqutim/plugin.cpp",
        "libqutim/profile.cpp",
        "libqutim/profilecreatorpage.cpp",
        "libqutim/protocol.cpp",
        "libqutim/qtwin.cpp",
        "libqutim/rosterstorage.cpp",
        "libqutim/scripttools.cpp",
        "libqutim/servicemanager.cpp",
        "libqutim/settingslayer.cpp",
        "libqutim/settingswidget.cpp",
        "libqutim/shortcut.cpp",
        "libqutim/sound.cpp",
        "libqutim/spellchecker.cpp",
        "libqutim/startupmodule.cpp",
        "libqutim/statisticshelper.cpp",
        "libqutim/statusactiongenerator.cpp",
        "libqutim/status.cpp",
        "libqutim/systeminfo.cpp",
        "libqutim/systemintegration.cpp",
        "libqutim/tcpsocket.cpp",
        "libqutim/thememanager.cpp",
        "libqutim/tooltip.cpp",
        "libqutim/utils/avatarfilter.cpp",
        "libqutim/utils/avatariconengine.cpp",
        "libqutim/utils.cpp",
        "libqutim/utils/itemdelegate.cpp",
        "libqutim/varianthook.cpp",
        "libqutim/version.cpp",
        "libqutim/widgetactiongenerator.cpp",
    ]

}
