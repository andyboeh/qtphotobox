PROJECT(photobooth-rewrite)
SET(NAME "photobooth-rewrite")
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.1 FATAL_ERROR)

CMAKE_POLICY(SET CMP0020 NEW)
IF(CMP0028)
  CMAKE_POLICY(SET CMP0028 NEW)
ENDIF(CMP0028)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

FIND_PACKAGE(Qt5Core)
FIND_PACKAGE(Qt5Gui)
FIND_PACKAGE(Qt5Widgets)
FIND_PACKAGE(Qt5DBus)
FIND_PACKAGE(Qt5Network)
FIND_PACKAGE(Gphoto2)
FIND_PACKAGE(Cups)
FIND_LIBRARY(PIGPIO_LIBRARY pigpiod_if2)

include_directories(SYSTEM ${Gphoto2_INCLUDE_DIRS})
include_directories(${CUPS_INCLUDE_DIRS})
add_definitions(${Gphoto2_DEFINITIONS})

get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

SET(OWRT_INSTALL_RUNTIME_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${NAME}")

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
  photobooth-rewrite.cxx
  settings/settings.cxx
  camera/pbcamera.cxx
  camera/camera_interface.cxx
  camera/camera_gphoto2.cxx
  camera/camera_dummy.cxx
  gpio/gpioWorker.cxx
  printer/printer.cxx
  printer/printer_interface.cxx
  printer/printer_selphy.cxx
  printer/printer_cups.cxx
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
  worker/pictureWorker.cxx
  worker/pictureTask.cxx
  worker/postprocessWorker.cxx
  worker/postprocessTask.cxx
  worker/showWorker.cxx
  storage/storageManager.cxx
)

SET(${NAME}_HEADERS
  photobooth-rewrite.h
  settings/settings.h
  camera/pbcamera.h
  camera/camera_interface.h
  camera/camera_gphoto2.h
  camera/camera_dummy.h
  gpio/gpioWorker.h
  printer/printer.h
  printer/printer_interface.h
  printer/printer_cups.h
  printer/printer_selphy.h
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
  #photobooth-rewrite.ui
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
)

SET(${NAME}_RES
    photobooth-rewrite.qrc
)

IF(WIN32)
    SET(WIN32_DEFINE "WIN32")
ELSE(WIN32)
    SET(WIN32_DEFINE)
ENDIF(WIN32)

QT5_WRAP_CPP(${NAME}_HEADERS_MOC ${${NAME}_HEADERS})
QT5_WRAP_UI(${NAME}_HEADERS_UI ${${NAME}_UI})
QT5_ADD_RESOURCES(${NAME}_RES_RCC ${${NAME}_RES})
QT5_ADD_DBUS_INTERFACE(${NAME}_DBUS_INTERFACE_OM storage/org.freedesktop.UDisks2.ObjectManager.xml objectManager)

ADD_EXECUTABLE(${NAME} ${WIN32_DEFINE}
  ${${NAME}_HEADERS}
  ${${NAME}_SOURCES}
  ${${NAME}_HEADERS_MOC}
  ${${NAME}_HEADERS_NO_MOC}
  ${${NAME}_HEADERS_UI}
  ${${NAME}_RES_RCC}
  ${${NAME}_DBUS_INTERFACE_OM}
)

TARGET_LINK_LIBRARIES(${NAME}
  ${QT_LIBRARIES}
  ${Gphoto2_LIBRARIES}
  ${CUPS_LIBRARIES}
  ${PIGPIO_LIBRARY}
)

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
