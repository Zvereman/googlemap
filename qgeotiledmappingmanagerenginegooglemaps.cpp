#include "QtLocation/private/qgeocameracapabilities_p.h"
#include "qgeotiledmappingmanagerenginegooglemaps.h"
#include "qgeotiledmapgooglemaps.h"
#include "qgeotilefetchergooglemaps.h"
#include "QtLocation/private/qgeotilespec_p.h"
#if QT_VERSION < QT_VERSION_CHECK(5,6,0)
#include <QStandardPaths>
#include "QtLocation/private/qgeotilecache_p.h"
#else
#include "QtLocation/private/qgeofiletilecache_p.h"
#endif

#include <QDebug>
#include <QDir>
#include <QVariant>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/qmath.h>
#include <QtCore/qstandardpaths.h>

QT_BEGIN_NAMESPACE

QGeoTiledMappingManagerEngineGooglemaps::QGeoTiledMappingManagerEngineGooglemaps(const QVariantMap &parameters,
    QGeoServiceProvider::Error *error,
    QString *errorString)
    : QGeoTiledMappingManagerEngine()
{
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    QGeoCameraCapabilities capabilities;
    capabilities.setMinimumZoomLevel(0.0);
    capabilities.setMaximumZoomLevel(21.0);

    setCameraCapabilities(capabilities);

    int tile = parameters.value(QStringLiteral("googlemaps.maps.tilesize"), 256).toInt();

    setTileSize(QSize(tile, tile));

    QList<QGeoMapType> types;
#if QT_VERSION < QT_VERSION_CHECK(5,9,0)
    types << QGeoMapType(QGeoMapType::StreetMap, tr("Road Map"), tr("Normal map view in daylight mode"), false, false, 1);
    types << QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Satellite"), tr("Satellite map view in daylight mode"), false, false, 2);
    types << QGeoMapType(QGeoMapType::TerrainMap, tr("Terrain"), tr("Terrain map view in daylight mode"), false, false, 3);
    types << QGeoMapType(QGeoMapType::HybridMap, tr("Hybrid"), tr("Satellite map view with streets in daylight mode"), false, false, 4);
#elif QT_VERSION < QT_VERSION_CHECK(5,10,0)
    types << QGeoMapType(QGeoMapType::StreetMap, tr("Road Map"), tr("Normal map view in daylight mode"), false, false, 1, "googlemaps");
    types << QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Satellite"), tr("Satellite map view in daylight mode"), false, false, 2, "googlemaps");
    types << QGeoMapType(QGeoMapType::TerrainMap, tr("Terrain"), tr("Terrain map view in daylight mode"), false, false, 3, "googlemaps");
    types << QGeoMapType(QGeoMapType::HybridMap, tr("Hybrid"), tr("Satellite map view with streets in daylight mode"), false, false, 4, "googlemaps");
#else
    types << QGeoMapType(QGeoMapType::StreetMap, tr("Road Map"), tr("Normal map view in daylight mode"), false, false, 1, "googlemaps", capabilities, parameters);
    types << QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Satellite"), tr("Satellite map view in daylight mode"), false, false, 2, "googlemaps", capabilities, parameters);
    types << QGeoMapType(QGeoMapType::TerrainMap, tr("Terrain"), tr("Terrain map view in daylight mode"), false, false, 3, "googlemaps", capabilities, parameters);
    types << QGeoMapType(QGeoMapType::HybridMap, tr("Hybrid"), tr("Satellite map view with streets in daylight mode"), false, false, 4, "googlemaps", capabilities, parameters);
#endif
    setSupportedMapTypes(types);

    const QByteArray pluginName = "googlemaps";
    if (parameters.contains(QStringLiteral("googlemaps.maps.cache.directory"))) {
        m_cacheDirectory = parameters.value(QStringLiteral("googlemaps.maps.cache.directory")).toString();
    } else {
        // managerName() is not yet set, we have to hardcode the plugin name below
        m_cacheDirectory = QAbstractGeoTileCache::baseLocationCacheDirectory() + QLatin1String(pluginName);
    }

    /* TILE CACHE */
    //if (parameters.contains(QStringLiteral("googlemaps.maps.offline.directory")))
    //    m_offlineDirectory = parameters.value(QStringLiteral("googlemaps.maps.offline.directory")).toString();
    QGeoFileTileCache *tileCache = new QGeoFileTileCache(m_cacheDirectory);

    /*
     * Disk cache setup -- defaults to ByteSize (old behavior)
     */
    if (parameters.contains(QStringLiteral("googlemaps.maps.cache.disk.cost_strategy"))) {
        QString cacheStrategy = parameters.value(QStringLiteral("googlemaps.maps.cache.disk.cost_strategy")).toString().toLower();
        if (cacheStrategy == QLatin1String("bytesize"))
            tileCache->setCostStrategyDisk(QGeoFileTileCache::ByteSize);
        else
            tileCache->setCostStrategyDisk(QGeoFileTileCache::Unitary);
    } else {
        tileCache->setCostStrategyDisk(QGeoFileTileCache::ByteSize);
    }
    if (parameters.contains(QStringLiteral("googlemaps.maps.cache.disk.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("googlemaps.maps.cache.disk.size")).toString().toInt(&ok);
        if (ok)
            tileCache->setMaxDiskUsage(cacheSize);
    }

    /*
     * Memory cache setup -- defaults to ByteSize (old behavior)
     */
    if (parameters.contains(QStringLiteral("googlemaps.maps.cache.memory.cost_strategy"))) {
        QString cacheStrategy = parameters.value(QStringLiteral("googlemaps.maps.cache.memory.cost_strategy")).toString().toLower();
        if (cacheStrategy == QLatin1String("bytesize"))
            tileCache->setCostStrategyMemory(QGeoFileTileCache::ByteSize);
        else
            tileCache->setCostStrategyMemory(QGeoFileTileCache::Unitary);
    } else {
        tileCache->setCostStrategyMemory(QGeoFileTileCache::ByteSize);
    }
    if (parameters.contains(QStringLiteral("googlemaps.maps.cache.memory.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("googlemaps.maps.cache.memory.size")).toString().toInt(&ok);
        if (ok)
            tileCache->setMaxMemoryUsage(cacheSize);
    }

    /*
     * Texture cache setup -- defaults to ByteSize (old behavior)
     */
    if (parameters.contains(QStringLiteral("googlemaps.maps.cache.texture.cost_strategy"))) {
        QString cacheStrategy = parameters.value(QStringLiteral("googlemaps.maps.cache.texture.cost_strategy")).toString().toLower();
        if (cacheStrategy == QLatin1String("bytesize"))
            tileCache->setCostStrategyTexture(QGeoFileTileCache::ByteSize);
        else
            tileCache->setCostStrategyTexture(QGeoFileTileCache::Unitary);
    } else {
        tileCache->setCostStrategyTexture(QGeoFileTileCache::ByteSize);
    }
    if (parameters.contains(QStringLiteral("googlemaps.maps.cache.texture.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("googlemaps.maps.cache.texture.size")).toString().toInt(&ok);
        if (ok)
            tileCache->setExtraTextureUsage(cacheSize);
    }

    setTileCache(tileCache);

    QGeoTileFetcherGooglemaps *fetcher = new QGeoTileFetcherGooglemaps(parameters, this, tileSize());
    setTileFetcher(fetcher);

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoTiledMappingManagerEngineGooglemaps::~QGeoTiledMappingManagerEngineGooglemaps()
{
}

QGeoMap *QGeoTiledMappingManagerEngineGooglemaps::createMap()
{
    return new QGeoTiledMapGooglemaps(this);
}

QT_END_NAMESPACE

