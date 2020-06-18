PROJECT(QtPhotobox)
SET(NAME "qtphotobox")
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.1 FATAL_ERROR)

CMAKE_POLICY(SET CMP0020 NEW)
IF(CMP0028)
  CMAKE_POLICY(SET CMP0028 NEW)
ENDIF(CMP0028)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui REQUIRED)
FIND_PACKAGE(Qt5Widgets REQUIRED)
FIND_PACKAGE(Qt5DBus)
FIND_PACKAGE(Qt5Network)
FIND_PACKAGE(GPHOTO2)
FIND_PACKAGE(Cups)
FIND_PACKAGE(LibUSB)
FIND_LIBRARY(PIGPIO_LIBRARY pigpiod_if2)
find_package(Qt5LinguistTools REQUIRED)

SET(TL_LIBRARIES
    ${Qt5Core_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
)

INCLUDE_DIRECTORIES(${Qt5Core_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Gui_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS})

IF(NOT Qt5Network_FOUND)
	MESSAGE(STATUS "Not building Canon Selphy WiFi driver: Qt5Network not found.")
	SET(BUILD_SELPHY_WIFI FALSE)
ELSE()
    SET(TL_LIBRARIES
        ${TL_LIBRARIES}
        ${Qt5Network_LIBRARIES}
    )
	add_definitions("-DBUILD_SELPHY_WIFI")
	SET(BUILD_SELPHY_WIFI TRUE)
ENDIF()

IF(NOT Qt5Dbus_FOUND)
    MESSAGE(STATUS "Not building support for waiting for removal device: Qt5DBus not found.")
	SET(BUILD_WAIT_USB FALSE)
ELSE()
    SET(TL_LIBRARIES
        ${TL_LIBRARIES}
        ${Qt5Dbus_LIBRARIES}
    )
	SET(BUILD_WAIT_USB TRUE)
	add_definitions("-DBUILD_WAIT_USB")
ENDIF()

IF(NOT GPHOTO2_FOUND)
	MESSAGE(STATUS "Not building support for DSLR cameras: Gphoto2 not found.")
	SET(BUILD_GPHOTO2 FALSE)
ELSE()
	SET(BUILD_GPHOTO2 TRUE)
	add_definitions("-DBUILD_GPHOTO2")
	SET(TL_LIBRARIES
		${TL_LIBRARIES}
		${GPHOTO2_LIBRARIES}
	)
ENDIF()

IF(NOT Cups_FOUND)
	MESSAGE(STATUS "Not building support for CUPS: CUPS not found.")
	SET(BUILD_CUPS FALSE)
ELSE()
	SET(BUILD_CUPS TRUE)
	add_definitions("-DBUILD_CUPS")
	SET(TL_LIBRARIES
		${TL_LIBRARIRES}
		${CUPS_LIBRARIES}
	)
ENDIF()

IF(NOT LIBUSB_FOUND)
	MESSAGE(STATUS "Not building support for Canon Selphy USB driver: LibUSB not found.")
	SET(BUILD_SELPHY_USB FALSE)
ELSE()
	SET(BUILD_SELPHY_USB TRUE)
	add_definitions("-DBUILD_SELPHY_USB")
	SET(TL_LIBRARIES
		${TL_LIBRARIES}
		${LIBUSB_LIBRARIES}
	)
ENDIF()

IF(PIGPIO_LIBRARY STREQUAL "PIGPIO_LIBRARY-NOTFOUND")
	MESSAGE(STATUS "Not building support for GPIO: pigpio not found.")
	SET(BUILD_PIGPIO FALSE)
ELSE()
	SET(BUILD_PIGPIO TRUE)
	add_definitions("-DBUILD_PIGPIO")
	SET(TL_LIBRARIES
		${TL_LIBRARIES}
		${PIGPIO_LIBRARY}
	)
ENDIF()

IF(BUILD_GPHOTO2)
	include_directories(SYSTEM ${GPHOTO2_INCLUDE_DIRS})
	add_definitions(${GPHOTO2_DEFINITIONS})
ENDIF()

IF(BUILD_CUPS)
	include_directories(${CUPS_INCLUDE_DIRS})
ENDIF()

IF(BUILD_SELPHY_USB)
	include_directories(${LIBUSB_INCLUDE_DIRS})
ENDIF()

IF(BUILD_SELPHY_WIFI)
    INCLUDE_DIRECTORIES(${Qt5Network_INCLUDE_DIRS})
ENDIF()

IF(BUILD_WAIT_USB)
    INCLUDE_DIRECTORIES(${Qt5Dbus_INCLUDE_DIRS})
ENDIF()

#get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
#get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
#find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

#SET(OWRT_INSTALL_RUNTIME_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${NAME}")

INCLUDE_DIRECTORIES(${QT5_DIR})
SET(QT_LIBRARIES
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5DBus_LIBRARIES}
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}
    camera
    gpio
    gui
    settings
    printer
    statemachine
    worker
    storage
    )

SET(${NAME}_SOURCES
  qtphotobox.cxx
  settings/settings.cxx
  camera/pbcamera.cxx
  camera/camera_interface.cxx
  camera/camera_dummy.cxx
  printer/printer.cxx
  printer/printer_interface.cxx
  printer/printJob.cxx
  statemachine/statemachine.cxx
  gui/initwidget.cxx
  gui/settingswidget.cxx
  gui/idlewidget.cxx
  gui/greeterwidget.cxx
  gui/countdownwidget.cxx
  gui/startwidget.cxx
  gui/capturewidget.cxx
  gui/assemblewidget.cxx
  gui/waitingspinnerwidget.cpp
  gui/reviewwidget.cxx
  gui/postprocesswidget.cxx
  gui/archivewidget.cxx
  gui/iconlistmodel.cxx
  gui/picturedetailwidget.cxx
  gui/showwidget.cxx
  gui/errorwidget.cxx
  gui/waitremovablewidget.cxx
  gui/screensaver.cxx
  gui/screensaverwidget.cxx
  worker/pictureWorker.cxx
  worker/pictureTask.cxx
  worker/postprocessWorker.cxx
  worker/postprocessTask.cxx
  worker/showWorker.cxx
  storage/storageManager.cxx
)

SET(${NAME}_HEADERS
  qtphotobox.h
  settings/settings.h
  camera/pbcamera.h
  camera/camera_interface.h
  camera/camera_dummy.h
  printer/printer.h
  printer/printer_interface.h
  printer/printJob.h
  statemachine/statemachine.h
  gui/errorwidget.h
  gui/initwidget.h
  gui/settingswidget.h
  gui/idlewidget.h
  gui/greeterwidget.h
  gui/countdownwidget.h
  gui/startwidget.h
  gui/capturewidget.h
  gui/assemblewidget.h
  gui/waitingspinnerwidget.h
  gui/reviewwidget.h
  gui/postprocesswidget.h
  gui/archivewidget.h
  gui/iconlistmodel.h
  gui/picturedetailwidget.h
  gui/showwidget.h
  gui/waitremovablewidget.h
  gui/screensaver.h
  gui/screensaverwidget.h
  worker/worker.h
  worker/pictureWorker.h
  worker/pictureTask.h
  worker/postprocessWorker.h
  worker/postprocessTask.h
  worker/showWorker.h
  storage/storageManager.h
)

SET(${NAME}_HEADERS_NO_MOC

)

SET(${NAME}_UI
  gui/initwidget.ui
  gui/settingswidget.ui
  gui/idlewidget.ui
  gui/greeterwidget.ui
  gui/countdownwidget.ui
  gui/startwidget.ui
  gui/capturewidget.ui
  gui/assemblewidget.ui
  gui/reviewwidget.ui
  gui/postprocesswidget.ui
  gui/archivewidget.ui
  gui/picturedetailwidget.ui
  gui/showwidget.ui
  gui/errorwidget.ui
  gui/waitremovablewidget.ui
  gui/screensaverwidget.ui
)

SET(${NAME}_RES
    qtphotobox.qrc
)

SET(${NAME}_TRANSLATIONS
    translations/english.ts
    translations/german.ts
    )

IF(BUILD_SELPHY_USB)
    SET(${NAME}_SOURCES
        ${${NAME}_SOURCES}
        printer/printer_selphyusb.cxx
    )

    SET(${NAME}_HEADERS
        ${${NAME}_HEADERS}
        printer/printer_selphyusb.h
    )
ENDIF()

IF(BUILD_SELPHY_WIFI)
    SET(${NAME}_SOURCES
        ${${NAME}_SOURCES}
        printer/printer_selphy.cxx
    )

    SET(${NAME}_HEADERS
        ${${NAME}_HEADERS}
        printer/printer_selphy.h
    )
ENDIF()

IF(BUILD_GPHOTO2)
    SET(${NAME}_SOURCES
        ${${NAME}_SOURCES}
        camera/camera_gphoto2.cxx
    )

    SET(${NAME}_HEADERS
        ${${NAME}_HEADERS}
        camera/camera_gphoto2.h
    )
ENDIF()

IF(BUILD_PIGPIO)
    SET(${NAME}_SOURCES
        ${${NAME}_SOURCES}
        gpio/gpioWorker.cxx
    )

    SET(${NAME}_HEADERS
        ${${NAME}_HEADERS}
        gpio/gpioWorker.h
    )
ENDIF()

IF(BUILD_CUPS)
    SET(${NAME}_SOURCES
        ${${NAME}_SOURCES}
        printer/printer_cups.cxx
    )

    SET(${NAME}_HEADERS
        ${${NAME}_HEADERS}
        printer/printer_cups.h
    )
ENDIF()

IF(WIN32)
    SET(WIN32_DEFINE "WIN32")
ELSE(WIN32)
    SET(WIN32_DEFINE)
ENDIF(WIN32)

SET(${NAME}_TRANSLATIONS_SRC
    ${${NAME}_SOURCES}
    ${${NAME}_UI}
    )

QT5_WRAP_CPP(${NAME}_HEADERS_MOC ${${NAME}_HEADERS})
QT5_WRAP_UI(${NAME}_HEADERS_UI ${${NAME}_UI})
QT5_ADD_RESOURCES(${NAME}_RES_RCC ${${NAME}_RES})
IF(BUILD_WAIT_USB)
	QT5_ADD_DBUS_INTERFACE(${NAME}_DBUS_INTERFACE_OM storage/org.freedesktop.UDisks2.ObjectManager.xml objectManager)
ENDIF()
QT5_CREATE_TRANSLATION(${NAME}_TRANSLATIONS_BIN ${${NAME}_TRANSLATIONS_SRC} ${${NAME}_TRANSLATIONS})
add_custom_target(translations DEPENDS ${${NAME}_TRANSLATIONS_BIN})


if(NOT QT_TRANSLATIONS_DIR)
    # If this directory is missing, we are in a Qt5 environment.
    # Extract the qmake executable location
    get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    # Ask Qt5 where to put the translations
    execute_process( COMMAND ${QT5_QMAKE_EXECUTABLE} -query QT_INSTALL_TRANSLATIONS
    OUTPUT_VARIABLE qt_translations_dir OUTPUT_STRIP_TRAILING_WHITESPACE )
    # make sure we have / and not \ as qmake gives on windows
    file( TO_CMAKE_PATH "${qt_translations_dir}" qt_translations_dir)
    set( QT_TRANSLATIONS_DIR ${qt_translations_dir} CACHE PATH
    "The location of the Qt translations" FORCE)
endif()

install(FILES ${${NAME}_TRANSLATIONS_BIN} DESTINATION ${QT_TRANSLATIONS_DIR})

ADD_EXECUTABLE(${NAME} ${WIN32_DEFINE}
  ${${NAME}_HEADERS}
  ${${NAME}_SOURCES}
  ${${NAME}_HEADERS_MOC}
  ${${NAME}_HEADERS_NO_MOC}
  ${${NAME}_HEADERS_UI}
  ${${NAME}_RES_RCC}
  ${${NAME}_DBUS_INTERFACE_OM}
  ${${NAME}_TRANSLATIONS_BIN}
)

TARGET_LINK_LIBRARIES(${NAME}
	${TL_LIBRARIES}
  )

IF(FALSE)
IF(WIN32)
    add_custom_command(TARGET ${NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
                COMMAND set PATH="${_qt_bin_dir}"
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
                --verbose 0 --no-compiler-runtime --no-translations --no-angle --release --no-opengl-sw "$<TARGET_FILE:${NAME}>"
    )

    install(
        DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
        DESTINATION ${OWRT_INSTALL_RUNTIME_DESTINATION}
    )


    INSTALL(TARGETS ${NAME}
      RUNTIME
      DESTINATION ${OWRT_INSTALL_RUNTIME_DESTINATION}
      COMPONENT Runtime)

  # install system runtime libs

    #set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)

    if(MINGW)
        message(STATUS "    Installing system-libraries: MinGW DLLs.")
        get_filename_component(Mingw_Path ${CMAKE_CXX_COMPILER} PATH)
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
            ${Mingw_Path}/libwinpthread-1.dll
            ${Mingw_Path}/libstdc++-6.dll
            ${Mingw_Path}/libgcc_s_seh-1.dll
        )
    endif(MINGW)

    SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${OWRT_INSTALL_RUNTIME_DESTINATION})
    include(InstallRequiredSystemLibraries)

    # Actually install it when make install is called.
    # Note, this works with CPack
    #if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
    #    install(
    #        PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    #        DESTINATION ${OWRT_INSTALL_RUNTIME_DESTINATION}
    #    )
    #endif(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
ENDIF(WIN32)
ENDIF()
