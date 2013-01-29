import qbs.base 1.0

StaticLibrary {
    property string sourcePath

    destination: "lib"

    Depends { name: "cpp" }

    Group {
        condition: sourcePath !== undefined
        prefix: sourcePath + '/**/'
        files: [ '*.cpp', '*.h', '*.ui', "*.c", "*.mm" ]
    }

    ProductModule {
        Depends { name: "cpp" }
        cpp.includePaths: [
            "."
        ]
    }
}

