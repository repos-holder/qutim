import qbs.base 1.0
import "./3rdPartyLibrary.qbs" as ThirdPartyLibrary

ThirdPartyLibrary {
    name: "qtdwm"

	Depends { name: "qt"; submodules: [ 'core', 'gui' ] }

    files: [
        "qtdwm/qtdwm.h",
        "qtdwm/qtdwm_p.h",
        "qtdwm/qtdwm.cpp",
    ]
}
