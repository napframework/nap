macro(find_qt)
    # Search for hints about our Qt library location
    if(DEFINED ENV{QT_DIR})
        # TODO After changing to run this once only, using global scope with CMake 3.24+ (see TODO below), remove this
        if(NOT NAP_QT_ENV_VAR_LOGGED)
            message(STATUS "Using QT_DIR environment variable: $ENV{QT_DIR}")
            set(NAP_QT_ENV_VAR_LOGGED TRUE PARENT_SCOPE)
        endif()
        set(QTDIR $ENV{QT_DIR})
    else()
        # Also allow for NAP_QT_DIR set directly (not using QT_DIR so we don't conflict with the variable below)
        if(DEFINED NAP_QT_DIR)
            set(QTDIR, NAP_QT_DIR)
        elseif(DEFINED NAP_PACKAGED_BUILD)
            # If we're doing a platform release let's enforce the an explicit Qt path so that we're
            # certain what we're bundling with the release
            message(FATAL_ERROR "Please set the QT_DIR environment variable to define the Qt6 version"
                                "to be installed with the platform release, eg. \"C:/dev/qt/6.7.2/msvc2019_64\"")
        endif()
    endif()

    # Add possible Qt installation paths to the HINTS section
    # The version probably doesn't have to match exactly (5.8.? is probably fine)
    find_path(QT_DIR lib/cmake/Qt6/Qt6Config.cmake
              HINTS
              ${QTDIR}
              )

    if(QT_DIR)
        if(APPLE AND DEFINED NAP_PACKAGED_BUILD)
              # Ensure we're not using Qt from homebrew as we don't know the legal situation with packaging homebrew's packages.
              # Plus Qt's own opensource packages should have wider macOS version support.
              if(EXISTS ${QT_DIR}/INSTALL_RECEIPT.json)
                  message(FATAL_ERROR "Homebrew's Qt packages aren't allowed due largely to a legal unknown.
                          Install Qt's own opensource release and point environment variable QT_DIR there.")
              endif()
        endif()

        # TODO Ensure we're not packaging system Qt on Linux, we only want to use a download from qt.io

        # Find_package for Qt6 will pick up the Qt installation from CMAKE_PREFIX_PATH
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})
    else()
        message(FATAL_ERROR
                "Qt6 could not be found, please set the QT_DIR environment variable, eg.:"
                "\n Win64 - \"C:/dev/Qt/6.5.3/msvc2015_64\""
                "\n macOS - \"/Users/username/dev/Qt/6.5.3/clang_64\""
                "\n Linux - \"/home/username/dev/Qt/6.5.3/gcc_64\"")
    endif()

    # TODO Update to CMake 3.24+ and use global scope here to avoid redefining
    find_package(Qt6Core REQUIRED)
    find_package(Qt6Widgets REQUIRED)
    find_package(Qt6Gui REQUIRED)
    find_package(Qt6OpenGL REQUIRED)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    add_definitions(-DQT_NO_KEYWORDS)

    set(QT_LIBS Qt6::Widgets Qt6::Core Qt6::Gui Qt6::OpenGL)
endmacro()

