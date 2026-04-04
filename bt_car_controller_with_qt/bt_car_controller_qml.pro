QT += qml quick bluetooth concurrent
QT += quickcontrols2

CONFIG += c++17

SOURCES += \
    main_qml.cpp \
    bluetoothmanager.cpp \
    androidpermissionmanager.cpp

HEADERS += \
    bluetoothmanager.h \
    androidpermissionmanager.h

RESOURCES += \
    qml.qrc

# Android için
DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

win32 {
    LIBS += -lws2_32 -lbthprops
}
