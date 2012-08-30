import qbs.base 1.0
import "./3rdPartyLibrary.qbs" as ThirdPartyLibrary

ThirdPartyLibrary {
    name: "qxt"

	Depends { name: "qt"; submodules: [ 'core', 'gui' ] }

    files: [
        "qxt/qxtcommandoptions.h",
        "qxt/qxtglobal.h",
        "qxt/qxtcommandoptions.cpp",
    ]

}
