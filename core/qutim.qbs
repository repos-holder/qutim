import qbs.base 1.0

Application {
	name: "qutim"
	destination: "bin"

	Depends { name: "cpp" }
	Depends { name: "libqutim" }
    Depends { name: "qt"; submodules: [ 'core', 'gui', 'network', "script" ] }
    Depends { name: "qt.widgets"; condition: qt.core.versionMajor === 5 }
	
	cpp.includePaths: [
        buildDirectory + "/include/qutim",
        buildDirectory + "/include"
	]
	
	files: [
		"main.cpp"
	]
	
	Group {
		condition: project.declarativeUi
		files: [
			"src/declarative/modulemanagerimpl.h",
			"src/declarative/modulemanagerimpl.cpp",
			"src/declarative/requesthelper.cpp",
			"src/declarative/requesthelper.h"
		]
        cpp.defines: [ "QUTIM_DECLARATIVE_UI" ]

        Depends { name: "qt.quick1" }

	}
	Group {
		condition: !project.declarativeUi
		files: [
			"src/widgets/modulemanagerimpl.cpp",
			"src/widgets/modulemanagerimpl.h",
			"src/widgets/profilecreationpage.cpp",
			"src/widgets/profilecreationpage.h",
			"src/widgets/profilecreationpage.ui",
			"src/widgets/profilecreationwizard.cpp",
			"src/widgets/profilecreationwizard.h",
			"src/widgets/profiledialog.cpp",
			"src/widgets/profiledialog.h",
			"src/widgets/profiledialog.ui",
			"src/widgets/profilelistwidget.cpp",
			"src/widgets/profilelistwidget.h",
			"src/widgets/submitpage.cpp",
			"src/widgets/submitpage.h"
		]
	}
}
