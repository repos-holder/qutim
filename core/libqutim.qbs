import qbs.base 1.0

DynamicLibrary {
    name: "libqutim"
    destination: "lib"
    
    property string versionMajor: project.qutim_version_major
    property string versionMinor: project.qutim_version_minor
    property string versionRelease: project.qutim_version_release
    property string versionPatch: project.qutim_version_patch
    property string version: project.qutim_version
    property string shareDir: {
        if (qbs.targetOS === "mac")
            return "/Resources/share";
        else
            return project.shareDir;
    }

    Depends { name: "qutim-headers" }
    Depends { name: "k8json" }
    Depends { name: "qxt" }
    Depends { name: "qxdg" }
    Depends { name: "qtdwm" }
    Depends { name: "flowlayout" }
    Depends { name: "slidingstackedwidget" }
    Depends { name: "qtsolutions" }
    Depends { name: "cpp" }
    Depends { name: "qt"; submodules: [ 'core', 'gui', 'network', 'script', 'quick1' ] }
    Depends { name: "qt.widgets"; condition: qt.core.versionMajor === 5 }
    Depends { name: "mac.carbon"; condition: qbs.targetOS === 'mac' }
    Depends { name: "mac.cocoa"; condition: qbs.targetOS === 'mac' }

    cpp.includePaths: [
        buildDirectory + "/include/qutim"
    ]

    cpp.dynamicLibraries: {
        if (qbs.targetOS === 'linux' || qbs.targetOS === 'freebsd') {
            return [ 'X11' ];
        }
        return [];
    }

    cpp.dynamicLibraryPrefix: ""
    cpp.staticLibraryPrefix: ""
    cpp.defines: [
        "LIBQUTIM_LIBRARY",
        "QUTIM_SHARE_DIR=\"" + shareDir + "\"",
        "NO_SYSTEM_QXT"
    ]

    ProductModule {
        property string basePath

        Depends { name: "cpp" }
        cpp.includePaths: [
            product.buildDirectory + "/include/qutim",
            product.buildDirectory + "/include"
        ]
        Properties {
            condition: project.declarativeUi
            cpp.defines: [ "QUTIM_DECLARATIVE_UI" ]
        }
    }
//    Properties {
//        condition: qbs.platform !== "windows"
//        cpp.cxxFlags: "-fvisibility=hidden"
//    }

    files: [
        'libqutim/*.h',
        'libqutim/*.cpp',
        'libqutim/utils/*.h',
        'libqutim/utils/*.cpp'
    ]
}
