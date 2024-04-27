project(GoogleMapsQtGeoService
    VERSION 1.0.0
)

set(PROJECT_VERSION "6.6.0")
set(PROJECT_VERSION_MAJOR "6")
set(PROJECT_VERSION_MINOR "6")
set(PROJECT_VERSION_PATCH "0")
set(QT_USE_FIXED_QT_ADD_RESOURCE_BASE TRUE)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core) # special case

# WebAssembly not supported
if(WASM)
    message(NOTICE "Skipping the build as the condition \"NOT WASM\" is not met.")
    return()
endif()

qt_prepare_standalone_project()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Location Positioning Network) # special case

qt_internal_add_plugin(QGeoServiceProviderFactoryGoogleMapsPlugin
    OUTPUT_NAME qtgeoservices_googlemaps
    CLASS_NAME QGeoServiceProviderFactoryGoogleMaps
    PLUGIN_TYPE geoservices
SOURCES
    qgeoserviceproviderplugingooglemaps.h qgeoserviceproviderplugingooglemaps.cpp
    qgeocodingmanagerenginegooglemaps.h qgeocodingmanagerenginegooglemaps.cpp
    qgeocodereplygooglemaps.h qgeocodereplygooglemaps.cpp
    qgeoroutingmanagerenginegooglemaps.h qgeoroutingmanagerenginegooglemaps.cpp
    qgeoroutereplygooglemaps.h qgeoroutereplygooglemaps.cpp
    qplacemanagerenginegooglemaps.h qplacemanagerenginegooglemaps.cpp
    qplacesearchreplygooglemaps.h qplacesearchreplygooglemaps.cpp
    qplacecategoriesreplygooglemaps.h qplacecategoriesreplygooglemaps.cpp
    qgeomapreplygooglemaps.h qgeomapreplygooglemaps.cpp
    qgeotiledmapgooglemaps.h qgeotiledmapgooglemaps.cpp
    qgeotiledmappingmanagerenginegooglemaps.h qgeotiledmappingmanagerenginegooglemaps.cpp
    qgeotilefetchergooglemaps.h qgeotilefetchergooglemaps.cpp
    qplacesearchsuggestionreplyimpl.h qplacesearchsuggestionreplyimpl.cpp
    qgeoerror_messages.h qgeoerror_messages.cpp
LIBRARIES
    Qt::Core
    Qt::Network
    Qt::LocationPrivate
    Qt::PositioningPrivate
DEFINES
    QT_NO_FOREACH
)
