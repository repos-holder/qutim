import "../UreenPlugin.qbs" as UreenPlugin

UreenPlugin {
    pluginTags: 'linux'

    Depends { name: "qt.dbus" }
}
