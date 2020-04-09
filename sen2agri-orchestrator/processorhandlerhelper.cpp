#include "processorhandlerhelper.h"
#include "qdiriterator.h"
#include <QRegularExpression>

#include "qdatetime.h"
#include "qjsonobject.h"
#include "logger.hpp"

//#define NDVI_PRD_NAME_REGEX R"(S2AGRI_L3B_SNDVI_A(\d{8}T\d{6})_.+\.TIF)"
//#define S1_L2A_PRD_NAME_REGEX   R"(SEN4CAP_L2A_.+_V(\d{8}T\d{6})_(\d{8}T\d{6})_.+\.tif)"

#define EMPTY_TILE_ID           "00000"
#define INVALID_FILE_SEQUENCE   "!&"
// Map from the sensor name to :
//      - product type
//      - pattern containing index of the date in the file name of the L2A product assuming _ separation
// NOTE: The key of the map is the string as it appears in the file name of the product (S2, L8, SPOT4 etc.) and
//       not the name of the satellite as it appears inside the file metadata of product (that can be SENTINEL-2A, LANDSAT_8 etc.)
/* static */
QMap<QString, ProcessorHandlerHelper::L2MetaTileNameInfos> ProcessorHandlerHelper::m_mapSensorL2ATileMetaFileInfos =
        // MACCS product name ex. S2A_OPER_SSC_L2VALD_29SMR____20151209.HDR
    {{"S2", {ProcessorHandlerHelper::L2_PRODUCT_TYPE_S2, ProcessorHandlerHelper::SATELLITE_ID_TYPE_S2, "hdr", R"(S2[A-D]_OPER_SSC_L2VALD_(\d{2}[A-Z]{3})_.*_(\d{8}).HDR)", false, 1, 2}},
     // MAJA product name ex. SENTINEL2A_20180124-110332-457_L2A_T30TYP_C_V1-0_MTD_ALL.xml
     {"SENTINEL2", {ProcessorHandlerHelper::L2_PRODUCT_TYPE_S2, ProcessorHandlerHelper::SATELLITE_ID_TYPE_S2, "xml", R"(SENTINEL2[A-D]_(\d{8})-(\d{6})-(\d{3})_L2A_T(\d{2}[A-Za-z]{3})_.*_MTD_ALL\.xml)", false, 4, 1}},
     // Sen2Cor product name ex. S2A_MSIL2A_20200205T100211_N0214_R122_T33UVQ_20200205T114355.SAFE
     {"MTD_MSIL2A", {ProcessorHandlerHelper::L2_PRODUCT_TYPE_S2, ProcessorHandlerHelper::SATELLITE_ID_TYPE_S2, "xml", R"(S2[A-D]_MSIL2A_(\d{8})T\d{6}_.*T(\d{2}[A-Z]{3})_.*\.SAFE)", true, 2, 1}},
     // L8 (MACCS and MAJA) product name ex. L8_TEST_L8C_L2VALD_196030_20191003.HDR
     {"L8", {ProcessorHandlerHelper::L2_PRODUCT_TYPE_L8, ProcessorHandlerHelper::SATELLITE_ID_TYPE_L8, "hdr", R"(L8_.*_L8C_L2VALD_(\d{6})_(\d{8}).HDR)", false, 1, 2}},
     {"SPOT4", {ProcessorHandlerHelper::L2_PRODUCT_TYPE_SPOT4, ProcessorHandlerHelper::SATELLITE_ID_TYPE_SPOT4, "xml", "", false, -1, 3}}, //SPOT4_*_*_<DATE>_*_*.xml
     {"SPOT5", {ProcessorHandlerHelper::L2_PRODUCT_TYPE_SPOT5, ProcessorHandlerHelper::SATELLITE_ID_TYPE_SPOT5, "xml", "", false, -1, 3}}, //SPOT5_*_*_<DATE>_*_*.xml
     // this prefix is impossible to occur in the file name
     {INVALID_FILE_SEQUENCE, {ProcessorHandlerHelper::L2_PRODUCT_TYPE_UNKNOWN, ProcessorHandlerHelper::SATELLITE_ID_TYPE_UNKNOWN, INVALID_FILE_SEQUENCE, "", false, -1, -1}}
};

QMap<QString, ProductType> ProcessorHandlerHelper::m_mapHighLevelProductTypeInfos = {
    {"L3A", ProductType::L3AProductTypeId},
    {"L3B", ProductType::L3BProductTypeId},
    {"L3C", ProductType::L3CProductTypeId},
    {"L3D", ProductType::L3DProductTypeId},
    {"L3E", ProductType::L3EProductTypeId},
    {"L4A", ProductType::L4AProductTypeId},
    {"L4B", ProductType::L4BProductTypeId},
    {"_AMP_", ProductType::S4CS1L2AmpProductTypeId},
    {"_COHE_", ProductType::S4CS1L2CoheProductTypeId},
    {"_S4C_L4A_", ProductType::S4CL4AProductTypeId},
    {"_S4C_L4B_", ProductType::S4CL4BProductTypeId},
    {"_S4C_L4C_", ProductType::S4CL4CProductTypeId},
};

bool compareTileInfoFilesDates(const ProcessorHandlerHelper::InfoTileFile& info1,const ProcessorHandlerHelper::InfoTileFile& info2)
{
  QDateTime dtProd1=ProcessorHandlerHelper::GetL2AProductDateFromPath(info1.file);
  QDateTime dtProd2=ProcessorHandlerHelper::GetL2AProductDateFromPath(info2.file);
  return (dtProd1 < dtProd2);
}

ProcessorHandlerHelper::ProcessorHandlerHelper() {}

ProductType ProcessorHandlerHelper::GetProductTypeFromFileName(const QString &path, bool useParentDirIfDir) {
    QFileInfo info(path);
    QString name;
    if(info.isDir() && useParentDirIfDir) {
        name = info.dir().dirName();
    } else {
        name = info.baseName();
    }

    // check if it is an L2A product
    const L2MetaTileNameInfos &infos = GetL2AProductTileNameInfos(info);
    if(infos.productType != L2_PRODUCT_TYPE_UNKNOWN) {
        return ProductType::L2AProductTypeId;
    }

    // check if it is a higher level product
    // TODO: Maybe here we should add a more generic mechanism to detect if is a project high level product, as for L2A
    //      to avoid the conditions below
    QStringList nameWords = name.split("_");
    if(nameWords.size() > 2) {
        // we have an higher level product or SEN4CAP L2A product
        if(nameWords[0].indexOf("S2AGRI") != -1 || nameWords[0].indexOf("SEN4CAP") != -1) {
            QMap<QString, ProductType>::iterator i;
            i = m_mapHighLevelProductTypeInfos.find(nameWords[1]);
            if(i != m_mapHighLevelProductTypeInfos.end()) {
                return i.value();
            }
        }
    }
    return ProductType::InvalidProductTypeId;
}

/* static */
QString ProcessorHandlerHelper::GetTileId(const QString &path, SatelliteIdType &satelliteId)
{
    // First remove the extension
    QFileInfo info(path);
    QString fileNameWithoutExtension = info.completeBaseName();
    satelliteId = SATELLITE_ID_TYPE_UNKNOWN;
    //QString extension = info.suffix();
    ProductType productType = GetProductTypeFromFileName(path, false);
    if(productType != ProductType::InvalidProductTypeId) {
        if(productType == ProductType::L2AProductTypeId) {
            const L2MetaTileNameInfos &infos = GetL2AProductTileNameInfos(info);
            satelliteId = infos.satelliteIdType;
            const QString &extrStr = GetL2AFieldFromPath(path, L2MetaTileNameInfos::TILE_IDX);
            if (extrStr.size() > 0) {
                return extrStr;
            }
            // TODO: The if else below I think might be removed. The generic part above should do the job
            // If tests with MACCS, MAJA (both S2 and L8) and Sen2Cor are succesful

            // /////////////    START TO REMOVE //////////////////////////////
            // Split the name by "_" and search the part having _Txxxxx (_T followed by 5 characters)
            QStringList pieces = fileNameWithoutExtension.split("_");
            if(infos.productType == L2_PRODUCT_TYPE_S2) {
                // TODO: Here we should do it generic and use the REGEX from the L2MetaTileNameInfos
                if((pieces.size() == 9) && (pieces[5] == "")) {
                    return QString(pieces[4]);
                } else {
                    // Check for MAJA
                    if((pieces.size() == 8) && path.endsWith("_MTD_ALL.xml")) {
                        QString  tileName = QString(pieces[3]);
                        if (tileName.startsWith("T")) {
                            return tileName.mid(1);
                        }
                        return tileName;
                    }
                }
            } else if(infos.productType == L2_PRODUCT_TYPE_L8) {
                if((pieces.size() == 6) && (pieces[3] == "L2VALD")) {
                    return QString(pieces[4]);
                }
            }
            // /////////////    END TO REMOVE //////////////////////////////
        } else {
            // Split the name by "_" and search the part having _Txxxxx (_T followed by 5 characters)
            QStringList pieces = fileNameWithoutExtension.split("_");
            for (const QString &piece : pieces) {
                int pieceLen = piece.length();
                if (((pieceLen == 6) || (pieceLen == 7)) && (piece.at(0) == 'T')) {
                    // return the tile without the 'T'
                    QString tileId = piece.right(pieceLen-1);
                    int tileIdLen = tileId.length();
                    if(tileIdLen == 5) {
                        satelliteId = SATELLITE_ID_TYPE_S2;
                    } else if(tileIdLen == 6) {
                        bool ok = false;
                        tileId.toInt(&ok, 10);
                        if(ok)
                            satelliteId = SATELLITE_ID_TYPE_L8;
                    }
                    return tileId;
                }
            }
        }
    }

    return QString(EMPTY_TILE_ID);
}

QMap<QString, ProcessorHandlerHelper::TileTemporalFilesInfo> ProcessorHandlerHelper::GroupTiles(const QStringList &listAllProductsTiles,
                                                                                                QList<ProcessorHandlerHelper::SatelliteIdType> &outAllSatIds,
                                                                                                ProcessorHandlerHelper::SatelliteIdType &outPrimarySatelliteId) {
    QMap<QString, TileTemporalFilesInfo> mapTiles;
    for(const QString &tileFile: listAllProductsTiles) {
        // get from the tile ID also the info about tile to determine satellite ID
        SatelliteIdType satId;
        QString tileId = GetTileId(tileFile, satId);
        if(!outAllSatIds.contains(satId))
            outAllSatIds.append(satId);
        if(mapTiles.contains(tileId)) {
            TileTemporalFilesInfo &infos = mapTiles[tileId];
            // The tile acquisition date should be filled later
            infos.temporalTilesFileInfos.append({tileFile, satId, "", {}});
            if(!infos.uniqueSatteliteIds.contains(satId))
                infos.uniqueSatteliteIds.append(satId);
        } else {
            TileTemporalFilesInfo infos;
            infos.tileId = tileId;
            // The tile acquisition date should be filled later
            infos.temporalTilesFileInfos.append({tileFile, satId, "", {}});
            infos.uniqueSatteliteIds.append(satId);
            mapTiles[tileId] = infos;
        }
    }

    // Get the primary satellite id
    outPrimarySatelliteId = ProcessorHandlerHelper::GetPrimarySatelliteId(outAllSatIds);

    // now update also the primary satelite id
    QMap<QString, TileTemporalFilesInfo>::iterator i;
    for (i = mapTiles.begin(); i != mapTiles.end(); ++i) {
        // this time, get a copy from the map and not the reference to info as we do not want to alter the input map
        TileTemporalFilesInfo &info = i.value();
        // fill the primary satellite ID here
        info.primarySatelliteId = outPrimarySatelliteId;
    }

    return mapTiles;
}

QMap<QDate, QStringList> ProcessorHandlerHelper::GroupL2AProductTilesByDate(const QMap<QString, QStringList> &inputProductToTilesMap)
{
    QMap<QDate, QStringList> mapDateTiles;
    QMap<QDate, QList<ProcessorHandlerHelper::SatelliteIdType>> mapDateTilesSats;
    // group tiles by their date
    for(const auto &prd : inputProductToTilesMap.keys()) {
        // iterate the list of tiles for the current product
        const QStringList &prdTilesList = inputProductToTilesMap[prd];
        for(const QString &prdTile: prdTilesList) {
            // get the date of the current tile (normally, we could take only for the first tile
            // and then add all other tiles of the product directly but is simpler this way)
            const QDateTime &tileDateTime = ProcessorHandlerHelper::GetL2AProductDateFromPath(prdTile);
            const QDate &tileDate = tileDateTime.date();
            // add it to the list of the tiles from the current date
            if(mapDateTiles.contains(tileDate)) {
                QStringList &dateTiles = mapDateTiles[tileDate];
                if(!dateTiles.contains(prdTile)) {
                    dateTiles.append(prdTile);

                    // add also the satellite
                    QList<ProcessorHandlerHelper::SatelliteIdType> &listSat = mapDateTilesSats[tileDate];
                    listSat.append(ProcessorHandlerHelper::GetL2ASatelliteFromTile(prdTile));
                }
            } else {
                QStringList dateTiles;
                dateTiles.append(prdTile);
                mapDateTiles[tileDate] = dateTiles;

                // add also the satellite
                QList<ProcessorHandlerHelper::SatelliteIdType> listSat;
                listSat.append(ProcessorHandlerHelper::GetL2ASatelliteFromTile(prdTile));
                mapDateTilesSats[tileDate] = listSat;
            }
        }
    }
    // Now, we should iterate the tiles list and remove the secondary satellites from that date
    // as if we have the primary satellite, there is no need to consider also the secondary satellite for that date
    QMap<QDate, QStringList> retMapDateTiles;
    for(const auto &date : mapDateTiles.keys()) {
        QStringList filteredDateTilesList;
        const QStringList &dateTilesList = mapDateTiles[date];
        const QList<ProcessorHandlerHelper::SatelliteIdType> &dateTilesSatsList = mapDateTilesSats[date];
        ProcessorHandlerHelper::SatelliteIdType primarySatId = ProcessorHandlerHelper::GetPrimarySatelliteId(dateTilesSatsList);
        // normally we must have the same number as we above added in the same time in the two lists
        for(int i = 0; i<dateTilesList.size(); i++) {
            if(primarySatId == dateTilesSatsList[i]) {
                filteredDateTilesList.append(dateTilesList[i]);
            }
        }
        // normally, we must have here at least one tile so there is no need to make any check on size of list
        retMapDateTiles[date] = filteredDateTilesList;
    }
    return retMapDateTiles;
}

ProcessorHandlerHelper::SatelliteIdType ProcessorHandlerHelper::GetPrimarySatelliteId(
        const QList<ProcessorHandlerHelper::SatelliteIdType> &satIds) {
    // Get the primary satellite id
    SatelliteIdType retSatId = SATELLITE_ID_TYPE_S2;
    if(satIds.contains(SATELLITE_ID_TYPE_S2)) {
        retSatId = SATELLITE_ID_TYPE_S2;
    } else if (satIds.contains(SATELLITE_ID_TYPE_SPOT4)) {
        retSatId = SATELLITE_ID_TYPE_SPOT4;
    } else if (satIds.contains(SATELLITE_ID_TYPE_SPOT5)) {
        retSatId = SATELLITE_ID_TYPE_SPOT5;
    } else if(satIds.size() >= 1) {
        // check if all satellites in the list are the same
        const SatelliteIdType &refSatId = satIds[0];
        bool bAllSameSat = true;
        for(const SatelliteIdType &satId: satIds) {
            if(satId != refSatId) {
                bAllSameSat = false;
                break;
            }
        }
        if(bAllSameSat) {
            retSatId = satIds[0];
        }
    }

    return retSatId;
}

// NOTE: returning string is the name of the satellite as it appears inside the file metadata of product
//       (that can be SENTINEL-2A, LANDSAT_8 etc.) and NOT the small identifier that appears in the file name
//       of the product (like S2, L8 etc.)
QString ProcessorHandlerHelper::GetMissionNamePrefixFromSatelliteId(ProcessorHandlerHelper::SatelliteIdType satId) {
    switch (satId) {
        case SATELLITE_ID_TYPE_S2:
            return "SENTINEL";
        case SATELLITE_ID_TYPE_SPOT4:
        case SATELLITE_ID_TYPE_SPOT5:
            return "SPOT";
        case SATELLITE_ID_TYPE_L8:
            return "LANDSAT";
        default:
            return "SENTINEL";
    }
}

bool ProcessorHandlerHelper::IsValidHighLevelProduct(const QString &path) {
    QDirIterator it(path, QStringList() << "*.xml", QDir::Files);
    if (!it.hasNext()) {
        return false;
    }

    QDirIterator itTiles(path + "/TILES/", QStringList() << "*", QDir::Dirs);
    if (!itTiles.hasNext()) {
        return false;
    }
    bool bAtLeastOneValidTile = false;
    while(itTiles.hasNext()) {
        QString tileDir = itTiles.next();
        // get the dir name
        QString tileDirName = QFileInfo(tileDir).fileName();
        if(tileDirName == "." || tileDirName == "..") {
            continue;
        }
        bool bValidTile = true;
        // we should have some TIF files in this folder
        QDirIterator itImgData(tileDir + "/IMG_DATA/", QStringList() << "*.TIF", QDir::Files);
        if (!itImgData.hasNext()) {
            bValidTile = false;
        }
        // check if we have some files here (we cannot assume they are TIF or other format)
        QDirIterator itQiData(tileDir + "/QI_DATA/", QStringList() << "*.*", QDir::Files);
        if (!itQiData.hasNext()) {
            bValidTile = false;
        }
        if(bValidTile) {
            bAtLeastOneValidTile = true;
            break;
        }
    }
    return bAtLeastOneValidTile;
}

bool ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(const QString &productName, QDateTime &minDate, QDateTime &maxDate) {
    QString prodName = productName.trimmed();
    // remove any / at the end of the product name
    if(prodName.endsWith('/')) {
        prodName.chop( 1 );
    }
    // if the name is actually the full path, then keep only the last part to avoid interpreting
    // incorrectly other elements in path
    const QStringList &els = prodName.split( "/" );
    prodName = els.value( els.length() - 1 );

    QStringList pieces = prodName.split("_");
    for (int i = 0; i < pieces.size(); i++) {
        const QString &piece = pieces[i];
        if(piece.length() == 0) // is it possible?
            continue;
        bool bIsInterval = (piece[0] == 'V');
        bool bIsAcquisition = (piece[0] == 'A');
        if(bIsInterval || bIsAcquisition) {
            QString timeFormat("yyyyMMdd");
            // Remove the A or V from the name
            QString trimmedPiece = piece.right(piece.size()-1);
            // check if the date is in formate yyyyMMddTHHmmss (ISO 8601)
            if(trimmedPiece.length() == 15 && trimmedPiece[8] == 'T') {
                // use this format, more precise
                timeFormat = "yyyyMMddTHHmmss";
            }
            minDate = QDateTime::fromString(trimmedPiece, timeFormat);
            maxDate = minDate;
            if(bIsInterval && (i+1) < pieces.size()) {
                const QDateTime tmpDate = QDateTime::fromString(pieces[i+1], timeFormat);
                // Do not make assumption min is first - choose the min between them
                if (tmpDate <= minDate) {
                    minDate = tmpDate;
                }
                if (tmpDate >= maxDate) {
                    maxDate = tmpDate;
                }
            }
            return true;
        }
    }

    Logger::error(QStringLiteral("Cannot extract acquisition dates from product %1").arg(productName));
    return false;
}

// Returns the images from a product for all tiles according to the given identifier
QMap<QString, QString> ProcessorHandlerHelper::GetHigLevelProductFiles(const QString &productDir,
                                                                       const QString &fileIdentif,
                                                                       bool isQiData) {
    QMap<QString, QString> retMap;
    QMap<QString, QStringList> mapTiles = GroupHighLevelProductTiles(QStringList(productDir));
    for(auto tile : mapTiles.keys())
    {
       QStringList listTemporalTiles = mapTiles.value(tile);
       // we should normally have always 1 tile folder
       if(listTemporalTiles.size() == 1) {
           retMap[tile] = GetHigLevelProductTileFile(listTemporalTiles[0], fileIdentif, isQiData);
       }
    }
    return retMap;
}

QString ProcessorHandlerHelper::GetHigLevelProductTileFile(const QString &tileDir, const QString &fileIdentif, bool isQiData) {
    // get the dir name
    QString tileFolder = tileDir + (isQiData ? "/QI_DATA/" : "/IMG_DATA/");
    QString filesFilter = QString("*_%1_*.TIF").arg(fileIdentif);
    QDirIterator it(tileFolder, QStringList() << filesFilter, QDir::Files);
    QStringList listFoundFiles;
    while (it.hasNext()) {
        listFoundFiles.append(it.next());
    }
    if(listFoundFiles.size() == 0) {
        return "";
    } else if (listFoundFiles.size() == 1) {
        return listFoundFiles[0];
    } else {
        // we have several files
        QString tileDirName = QFileInfo(tileDir).fileName();
        QStringList pieces = tileDirName.split("_");
        QString fileName;
        int curPiece = 0;
        // We have a tilename something like S2AGRI_L3A_V2013xxx_2013yyyy
        // the file that we are looking for is something like S2AGRI_L3A_CM_V2013xxx_2013yyyy.zzz
        // if there are multiple files with different timestamps, we get the one that has
        // the same timestamp as the tile/product
        int numPieces = pieces.size();
        for (int i = 0; i<numPieces; i++) {
            const QString &piece = pieces[i];
            fileName += piece;

            // we do not add the _ if it is the last piece
            if(curPiece != numPieces - 1)
                fileName += "_";

            // add the identifier after the product TYPE
            if(curPiece == 1)
                fileName += fileIdentif + "_";
            curPiece++;
        }
        QString filePath = tileDir + (isQiData ? "/QI_DATA/" : "/IMG_DATA/") + fileName + ".TIF";
        if(listFoundFiles.contains(filePath)) {
            return filePath;
        }
        // otherwise return the last in the list as it might possibly have the latest date
        return listFoundFiles[listFoundFiles.size()-1];
    }
}

QString ProcessorHandlerHelper::GetHighLevelProductIppFile(const QString &productDir) {
    QString auxDir = productDir + "/AUX_DATA/";
    QDirIterator it(auxDir, QStringList() << "*_IPP_*.xml", QDir::Files);
    // get the last strata shape file found
    while (it.hasNext()) {
        return it.next();
    }
    return "";
}

// TODO: This function should be extended to return a list of all the L2A products if the tile filter is empty or if it is a time series
//      (see the implementation in xxx_l3b_new.cpp
QString ProcessorHandlerHelper::GetSourceL2AFromHighLevelProductIppFile(const QString &productDir, const QString &tileFilter) {
    const QString ippXmlFile = GetHighLevelProductIppFile(productDir);
    if(ippXmlFile.length() == 0) {
        // incomplete product
        return "";
    }
    const QString startTag((tileFilter.length() == 0) ? "<XML_0>" : "<XML_");
    const QString endTag  ((tileFilter.length() == 0) ? "</XML_0>" : "</XML_");
    bool bFullTag = (tileFilter.length() == 0);
    QFile inputFile(ippXmlFile);
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
           const QString &line = in.readLine();
           int startTagIdx = line.indexOf(startTag);
           if(startTagIdx >= 0) {
               int endTagIdx = line.indexOf(endTag);
               if(endTagIdx > startTagIdx) {
                   if ((tileFilter.length() == 0) || (line.indexOf(tileFilter) >= 0))
                   {
                       int startIdx;
                       if (bFullTag)
                       {
                            startIdx = startTagIdx + startTag.length();

                       } else {
                           // we need to search for the closing of xml i.e the ">" character
                           int closingBracketIdx = line.indexOf('>', startTagIdx);
                           if(closingBracketIdx > startTagIdx)
                           {
                               startIdx = closingBracketIdx+1;
                           } else {
                               continue;
                           }
                       }
                       // check for the new values of startIdx
                       if(endTagIdx > startIdx) {
                            return line.mid(startIdx, endTagIdx - startIdx);
                       }
                   }
               }
           }
       }
       inputFile.close();
    }

    return "";
}


bool ProcessorHandlerHelper::HighLevelPrdHasL2aSource(const QString &highLevelPrd, const QString &l2aPrd) {
    QString ippXmlFile = GetHighLevelProductIppFile(highLevelPrd);
    if(ippXmlFile.length() == 0) {
        // incomplete product
        return false;
    }
    QFile inputFile(ippXmlFile);
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
           QString line = in.readLine();
           if(line.contains(l2aPrd)) {
               inputFile.close();
               return true;
           }
       }
       inputFile.close();
    }
    return false;

}


QMap<QString, QString> ProcessorHandlerHelper::GetHighLevelProductTilesDirs(const QString &productDir) {
    QMap<QString, QString> mapTiles;

    QString tilesDir = productDir + "/TILES/";
    QDirIterator it(tilesDir, QStringList() << "*", QDir::Dirs);
    SatelliteIdType satId;
    while (it.hasNext()) {
        QString subDir = it.next();
       // get the dir name
        QString tileDirName = QFileInfo(subDir).fileName();
        if(tileDirName == "." || tileDirName == "..") {
            continue;
        }
        QString tileId = GetTileId(subDir, satId);
        if(tileId != EMPTY_TILE_ID) {
            mapTiles[tileId] = subDir;
        }
    }
    return mapTiles;
}

QMap<QString, QStringList> ProcessorHandlerHelper::GroupHighLevelProductTiles(const QStringList &listAllProductFolders) {
    QMap<QString, QStringList> mapTiles;
    for(const QString &productDir: listAllProductFolders) {
        QString tilesDir = productDir + "/TILES/";
        QDirIterator it(tilesDir, QStringList() << "*", QDir::Dirs);
        while (it.hasNext()) {
            QString subDir = it.next();
            // get the dir name
            QString tileDirName = QFileInfo(subDir).fileName();
            if(tileDirName == "." || tileDirName == "..") {
                continue;
            }
            QStringList pieces = tileDirName.split("_");
            int pieceLen;
            for (const QString &piece : pieces) {
                // we might have 6 (for S2 and others) or 7 (for LANDSAT)
                pieceLen = piece.length();
                if (((pieceLen == 6) || (pieceLen == 7)) && (piece.at(0) == 'T')) {
                    // We have the tile
                    QString tileId = piece.right(pieceLen-1);
                    if(mapTiles.contains(tileId)) {
                        mapTiles[tileId].append(subDir);
                    } else {
                        mapTiles[tileId] = QStringList{subDir};
                    }
                }
            }
        }
    }
    return mapTiles;
}

/* static */
QStringList ProcessorHandlerHelper::GetTileIdsFromHighLevelProduct(const QString &prodFolder) {
    QStringList result;
    const QMap<QString, QString> &mapProdTiles = GetHighLevelProductTilesDirs(prodFolder);
    for(auto tile : mapProdTiles.keys())
    {
        result.append(tile);
    }
    return result;
}


QStringList ProcessorHandlerHelper::GetTextFileLines(const QString &filePath) {
    QFile inputFile(filePath);
    QStringList lines;
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
          lines.append(in.readLine());
       }
       inputFile.close();
    }
    return lines;
}

QString ProcessorHandlerHelper::GetFileNameFromPath(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.fileName();
}

/*
QString ProcessorHandlerHelper::GetL2ATileMainImageFilePath(const QString &tileMetadataPath) {
    QFileInfo info(tileMetadataPath);
    QString parentFolder = info.absoluteDir().absolutePath();
    QString metaFile = info.fileName().split(".",QString::SkipEmptyParts).at(0);
    //QString extension = info.suffix();
    // check if is S2
    if(metaFile.indexOf("S2") == 0) {
        QDirIterator it(parentFolder + "/" + metaFile + ".DBL.DIR/", QStringList() << "*_FRE_R1.DBL.TIF", QDir::Files);
        if (it.hasNext()) {
            return it.next();
        }
    } else if(metaFile.indexOf("L8") == 0) {
        QDirIterator it(parentFolder + "/" + metaFile + ".DBL.DIR/", QStringList() << "*_FRE.DBL.TIF", QDir::Files);
        if (it.hasNext()) {
            return it.next();
        }
    } else if(metaFile.indexOf("SPOT") == 0) {
        QDirIterator it(parentFolder + "/", QStringList() << "*_PENTE_*.TIF", QDir::Files);
        if (it.hasNext()) {
            return it.next();
        }
    }
    return "";
}
*/

const ProcessorHandlerHelper::L2MetaTileNameInfos &ProcessorHandlerHelper::GetL2AProductTileNameInfos(const QFileInfo &metaFileInfo) {
    QMap<QString, L2MetaTileNameInfos>::iterator i;
    QString metaFileName;
    if(metaFileInfo.isDir()) {
        metaFileName = metaFileInfo.dir().dirName();
    } else {
        metaFileName = metaFileInfo.fileName();
    }

    for (i = m_mapSensorL2ATileMetaFileInfos.begin(); i != m_mapSensorL2ATileMetaFileInfos.end(); ++i) {
        if(metaFileName.indexOf(i.key()) == 0) {
            return i.value();
        }
    }
    return m_mapSensorL2ATileMetaFileInfos[INVALID_FILE_SEQUENCE];
}

ProcessorHandlerHelper::L2ProductType ProcessorHandlerHelper::GetL2AProductTypeFromTile(const QString &tileMetadataPath) {
    QFileInfo info(tileMetadataPath);
    return GetL2AProductTileNameInfos(info).productType;
}

ProcessorHandlerHelper::SatelliteIdType ProcessorHandlerHelper::GetL2ASatelliteFromTile(const QString &tileMetadataPath) {
    QFileInfo info(tileMetadataPath);
    return GetL2AProductTileNameInfos(info).satelliteIdType;
}

QString ProcessorHandlerHelper::GetL2AFieldFromPath(const QString &path, ProcessorHandlerHelper::L2MetaTileNameInfos::REGEX_IDX regexIdx) {
    QFileInfo info(path);

    const ProcessorHandlerHelper::L2MetaTileNameInfos &infos = GetL2AProductTileNameInfos(info);
    int fieldIdxInName = -1;
    switch (regexIdx)
    {
        case L2MetaTileNameInfos::TILE_IDX:
            fieldIdxInName = infos.tileIdxInName;
            break;
        case L2MetaTileNameInfos::DATE_IDX:
            fieldIdxInName = infos.dateIdxInName;
            break;
    }
    QString extractedStr;
    if (fieldIdxInName < 0) {
        return extractedStr;
    }

    QString name;
    if(info.isDir()) {
        name = info.dir().dirName();
    } else {
        // For Sen2Cor products, the relevant information are not in metadata file name but in the parent folder name
        if (infos.regexOnParentFolder) {
            name = info.absoluteDir().dirName();
        } else {
            name = info.fileName();
        }
    }
    if (infos.tileNameRegex.size() > 0) {
        QRegularExpression rex(infos.tileNameRegex);
        QRegularExpression re(rex);
        const QRegularExpressionMatch &match = re.match(name);
        if (match.hasMatch()) {
            extractedStr = match.captured(fieldIdxInName);
        }
    } else {
        // otherwise, check by splitting by _
        QStringList nameWords = name.split("_");
        if(fieldIdxInName >= 0 && fieldIdxInName < nameWords.size()) {
            extractedStr = nameWords[fieldIdxInName];
        }
    }
    return extractedStr;
}

QDateTime ProcessorHandlerHelper::GetL2AProductDateFromPath(const QString &path) {
    const QString &extrStr = GetL2AFieldFromPath(path, L2MetaTileNameInfos::DATE_IDX);
    if (extrStr.size() > 0) {
        return QDateTime::fromString(extrStr, "yyyyMMdd");
    }

    return QDateTime();
}

bool ProcessorHandlerHelper::IsValidL2AMetadataFileName(const QString &path) {
    QFileInfo info(path);
    if(!info.isFile()) {
        return false;
    }
    QString fileName = info.fileName();
    QStringList listComponents = fileName.split(".",QString::SkipEmptyParts);
    QString ext;

    QString fileNameNoExt = fileName;
    if(listComponents.size() > 1) {
        ext = listComponents[listComponents.size()-1];
        fileNameNoExt = info.baseName();
    }
    const L2MetaTileNameInfos &infos = GetL2AProductTileNameInfos(info);

    if((infos.productType != L2_PRODUCT_TYPE_UNKNOWN) && ext.compare(infos.extension, Qt::CaseInsensitive) != 0)  {
        return false;
    }

    listComponents = fileNameNoExt.split("_");
    switch (infos.productType) {
        case L2_PRODUCT_TYPE_S2:
            // check if it is a S2 MACCS or MAJA format
            if (fileNameNoExt != "MTD_MSIL2A") {
                if(listComponents.size() < 4 || ((listComponents[3] != "L2VALD" || listComponents[2] == "QCK") &&
                                                 (listComponents[6] != "MTD" || listComponents[7] != "ALL")))
                {
                    return false;
                }
            }
            break;
        case L2_PRODUCT_TYPE_L8:
            if((listComponents.size() < 4) || (listComponents[2] != "L8C") || (listComponents[3] != "L2VALD")) {
                return false;
            }
            break;
        case L2_PRODUCT_TYPE_SPOT4:
        case L2_PRODUCT_TYPE_SPOT5:
            // TODO: see if something else should be added here
            break;
        case L2_PRODUCT_TYPE_UNKNOWN:
            return false;
    }

    return true;
}

bool ProcessorHandlerHelper::GetL2AIntevalFromProducts(const QStringList &productsList, QDateTime &minTime, QDateTime &maxTime) {

    minTime = QDateTime();
    maxTime = QDateTime();
    for(const QString &prod: productsList) {
        QDateTime curDate = GetL2AProductDateFromPath(prod);
        if(curDate.isValid()) {
            if(!minTime.isValid() || minTime > curDate)
                minTime = curDate;
            if(!maxTime.isValid() || maxTime < curDate)
                maxTime = curDate.addSecs(SECONDS_IN_DAY-1);
        }
    }
    if(minTime.isValid() && maxTime.isValid())
        return true;
    return false;
}

bool ProcessorHandlerHelper::GetStrataFile(const QString &refDir, QString &strataShapeFile) {
    bool bRet = false;
    QDirIterator itStrataDir(refDir, QStringList() << "strata" << "STRATA", QDir::Dirs);
    while (itStrataDir.hasNext()) {
        QDirIterator itStrataFile(itStrataDir.next(), QStringList() << "*.shp" << "*.SHP", QDir::Files);
        // get the last strata shape file found
        while (itStrataFile.hasNext()) {
            strataShapeFile = itStrataFile.next();
            bRet = true;
        }
    }

    return bRet;
}

bool ProcessorHandlerHelper::GetCropReferenceFile(const QString &refDir, QString &shapeFile, QString &referenceRasterFile, QString &strataShapeFile) {
    bool bRet = GetCropReferenceFile(refDir, shapeFile, referenceRasterFile);
    GetStrataFile(refDir, strataShapeFile);

    return bRet;
}

bool ProcessorHandlerHelper::GetCropReferenceFile(const QString &refDir, QString &shapeFile, QString &referenceRasterFile) {
    bool bRet = false;
    if(refDir.isEmpty()) {
        return bRet;
    }

    QDirIterator it(refDir, QStringList() << "*.shp" << "*.SHP", QDir::Files);
    // get the last shape file found
    while (it.hasNext()) {
        shapeFile = it.next();
        bRet = true;
    }
    // even if the shape file was found, search also for the reference raster for no-insitu case
    QDirIterator it2(refDir, QStringList() << "*.tif" << "*.TIF", QDir::Files);
    // get the last reference raster file found
    while (it2.hasNext()) {
        referenceRasterFile = it2.next();
        bRet = true;
    }

    return bRet;
}

void ProcessorHandlerHelper::AddSatteliteIntersectingProducts(QMap<QString, TileTemporalFilesInfo> &mapSatellitesTilesInfos,
                                                     QStringList &listSecondarySatLoadedProds, SatelliteIdType secondarySatId,
                                                     TileTemporalFilesInfo &primarySatInfos) {

    // mapSatellitesTilesInfos contains the sattelites infos but each tile has only  products from its satellite
    // (note that we have also here tiles from the secondary satellite)
    QMap<QString, TileTemporalFilesInfo>::iterator i;
    // iterate all tiles in the map
    for (i = mapSatellitesTilesInfos.begin(); i != mapSatellitesTilesInfos.end(); ++i) {
        TileTemporalFilesInfo &info = i.value();

        // if we have a secondary product type
        if(info.uniqueSatteliteIds.contains(secondarySatId)) {
            // check if the tile meta appears in the list of loaded secondary products that were intersecting the primary ones
            for(const InfoTileFile &temporalTileFileInfo: info.temporalTilesFileInfos) {
                if(listSecondarySatLoadedProds.contains(temporalTileFileInfo.file) &&
                        !TemporalTileInfosHasFile(primarySatInfos, temporalTileFileInfo.file)) {
                    // add it to the target primary sattelite information list
                    primarySatInfos.temporalTilesFileInfos.append({temporalTileFileInfo.file, temporalTileFileInfo.satId, "", {}});
                    if(!primarySatInfos.uniqueSatteliteIds.contains(secondarySatId)) {
                        primarySatInfos.uniqueSatteliteIds.append(secondarySatId);
                    }
                }
            }
        }
    }
}

QString ProcessorHandlerHelper::BuildShapeName(const QString &shapeFilesDir, const QString &tileId, int jobId, int taskId)
{
    QDir dir(shapeFilesDir);
    if (!dir.exists()){
        if (!QDir::root().mkpath(shapeFilesDir)) {
            throw std::runtime_error(
                QStringLiteral("Unable to create output path for tiles shape files: %1").arg(shapeFilesDir).toStdString());
        }
    }
    return QStringLiteral("%1/%2_%3_%4.shp").arg(shapeFilesDir).arg(tileId).arg(jobId).arg(taskId);
}

QString ProcessorHandlerHelper::BuildProjectionFileName(const QString &projFilesDir, const QString &tileId, int jobId, int taskId)
{
    QDir dir(projFilesDir);
    if (!dir.exists()){
        if (!QDir::root().mkpath(projFilesDir)) {
            throw std::runtime_error(
                QStringLiteral("Unable to create output path for tiles shape files: %1").arg(projFilesDir).toStdString());
        }
    }
    return QStringLiteral("%1/%2_%3_%4.proj").arg(projFilesDir).arg(tileId).arg(jobId).arg(taskId);
}


QString ProcessorHandlerHelper::GetShapeForTile(const QString &shapeFilesDir, const QString &tileId)
{
    QDirIterator it(shapeFilesDir, QStringList() << "*.shp", QDir::Files);
    while(it.hasNext()) {
        QString shapeFileFullName = it.next();
        QFileInfo shapefileFileInfo(shapeFileFullName);
        // it should be actually equals with 0
        if(shapefileFileInfo.fileName().indexOf(tileId + "_") >= 0) {
            return shapeFileFullName;
        }
    }

    return "";
}

QString ProcessorHandlerHelper::GetProjectionForTile(const QString &projFilesDir, const QString &tileId)
{
    QDirIterator it(projFilesDir, QStringList() << "*.proj", QDir::Files);
    while(it.hasNext()) {
        QString projFileFullName = it.next();
        QFileInfo projfileFileInfo(projFileFullName);
        // it should be actually equals with 0
        if(projfileFileInfo.fileName().indexOf(tileId + "_") >= 0) {
            return projFileFullName;
        }
    }

    return "";
}


QStringList ProcessorHandlerHelper::GetTemporalTileFiles(const TileTemporalFilesInfo &temporalTileInfo)
{
    QStringList retList;
    for(const InfoTileFile &fileInfo: temporalTileInfo.temporalTilesFileInfos) {
        retList.append(fileInfo.file);
    }
    return retList;
}

QStringList ProcessorHandlerHelper::GetTemporalTileAcquisitionDates(const TileTemporalFilesInfo &temporalTileInfo)
{
    QStringList retList;
    for(const InfoTileFile &fileInfo: temporalTileInfo.temporalTilesFileInfos) {
        retList.append(fileInfo.acquisitionDate);
    }
    return retList;
}

bool ProcessorHandlerHelper::TemporalTileInfosHasFile(const TileTemporalFilesInfo &temporalTileInfo, const QString &filePath)
{
    for(const InfoTileFile &fileInfo: temporalTileInfo.temporalTilesFileInfos) {
        if(fileInfo.file == filePath)
            return true;
    }
    return false;
}

void ProcessorHandlerHelper::SortTemporalTileInfoFiles(TileTemporalFilesInfo &temporalTileInfo)
{
    qSort(temporalTileInfo.temporalTilesFileInfos.begin(), temporalTileInfo.temporalTilesFileInfos.end(), compareTileInfoFilesDates);
}

/**
 * @brief ProcessorHandlerHelper::TrimLeftSecondarySatellite
 * @param productsList
 * @param temporalTileInfo
 * Removes from the list of products and from the map of tiles the secondary products that appear before the primary products.
 * This is needed to avoid any import the wrong UTM or other infos from the secondary product in the final composition product.
 * The difference of the previous function is that this ones checks at tile level if there are secondary products before primary
 * ones and it removes from the tile infos but also from the list of products if does not appears anymore in the list of map tiles
 */
void ProcessorHandlerHelper::TrimLeftSecondarySatellite(QStringList &productsList, QMap<QString, TileTemporalFilesInfo> mapTiles)
{
    for(auto tileId : mapTiles.keys())
    {
        const TileTemporalFilesInfo &listTemporalTiles = mapTiles.value(tileId);
        QList<InfoTileFile> fileInfos;
        bool bAddAllowed = false;
        for(const InfoTileFile &fileInfo: listTemporalTiles.temporalTilesFileInfos) {
            if(fileInfo.satId == listTemporalTiles.primarySatelliteId) {
                bAddAllowed = true;
            }
            if(bAddAllowed) {
                fileInfos.append(fileInfo);
            }
        }
        mapTiles[tileId].temporalTilesFileInfos = fileInfos;
    }

    QStringList tempProdList(productsList);
    // now search the products that do not exist in any of the tile temporal files
    for(const QString &prod: tempProdList) {
        bool prodFound = false;
        for(const TileTemporalFilesInfo &listTemporalTiles : mapTiles.values()) {
            for(const InfoTileFile &infoFile : listTemporalTiles.temporalTilesFileInfos) {
                if(infoFile.file == prod) {
                    prodFound = true;
                    break;
                }
            }
        }
        // if the product was not found anymore in none of the tiles temporal files, then remove it also from
        // the input product list
        if(!prodFound) {
            productsList.removeAll(prod);
        }
    }
}

ProcessorHandlerHelper::SatelliteIdType ProcessorHandlerHelper::ConvertSatelliteType(Satellite satId)
{
    switch (satId)
    {
        case Satellite::Sentinel2:
            return SATELLITE_ID_TYPE_S2;
        case Satellite::Landsat8:
            return SATELLITE_ID_TYPE_L8;
        default:
            return SATELLITE_ID_TYPE_UNKNOWN;
    }
}

void ProcessorHandlerHelper::UpdateMinMaxTimes(const QDateTime &newTime, QDateTime &minTime, QDateTime &maxTime)
{
    if (newTime.isValid()) {
        if (!minTime.isValid() && !maxTime.isValid()) {
            minTime = newTime;
            maxTime = newTime;
        } else if (newTime < minTime) {
            minTime = newTime;
        } else if (newTime > maxTime) {
            maxTime = newTime;
        }
    }
}

QString ProcessorHandlerHelper::GetMapValue(const std::map<QString, QString> &configParameters, const QString &key, const QString &defVal) {
    std::map<QString, QString>::const_iterator it = configParameters.find(key);
    if(it != configParameters.end()) {
        return it->second;
    }
    return defVal;
}

bool ProcessorHandlerHelper::GetBoolConfigValue(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                                const QString &key, const QString &cfgPrefix) {
    const QString &strVal = GetStringConfigValue(parameters, configParameters, key, cfgPrefix);
    if (QString::compare(strVal, "true", Qt::CaseInsensitive) == 0) {
        return true;
    }
    if (QString::compare(strVal, "false", Qt::CaseInsensitive) == 0) {
        return false;
    }
    return (GetIntConfigValue(parameters, configParameters, key, cfgPrefix) != 0);
}

int ProcessorHandlerHelper::GetIntConfigValue(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                                const QString &key, const QString &cfgPrefix) {
    return GetStringConfigValue(parameters, configParameters, key, cfgPrefix).toInt();
}

QString ProcessorHandlerHelper::GetStringConfigValue(const QJsonObject &parameters, const std::map<QString, QString> &configParameters,
                                                const QString &key, const QString &cfgPrefix) {
    QString fullKey(cfgPrefix);
    fullKey += key;

    QString retVal;
    if(parameters.contains(key)) {
        retVal = parameters[key].toString();
    } else {
        retVal = GetMapValue(configParameters, fullKey);
    }
    return retVal;
}

bool ProcessorHandlerHelper::GetParameterValueAsInt(const QJsonObject &parameters, const QString &key,
                                              int &outVal) {
    bool bRet = false;
    if(parameters.contains(key)) {
        // first try to get it as string
        const auto &value = parameters[key];
        if(value.isDouble()) {
            outVal = value.toInt();
            bRet = true;
        }
        if(value.isString()) {
            outVal = value.toString().toInt(&bRet);
        }
    }
    return bRet;
}

bool ProcessorHandlerHelper::GetParameterValueAsString(const QJsonObject &parameters, const QString &key,
                                              QString &outVal) {
    bool bRet = false;
    if(parameters.contains(key)) {
        // first try to get it as string
        const auto &value = parameters[key];
        if(value.isString()) {
            outVal = value.toString();
            bRet = true;
        }
    }
    return bRet;
}

TQStrQStrMap ProcessorHandlerHelper::FilterConfigParameters(const TQStrQStrMap &configParameters,
                                                const QString &cfgPrefix) {
    TQStrQStrMap retMap;
    TQStrQStrMap::const_iterator i = configParameters.lower_bound(cfgPrefix);
    while(i != configParameters.end()) {
        const QString& key = i->first;
        if (key.toStdString().compare(0, cfgPrefix.size(), cfgPrefix.toStdString()) == 0) { // Really a prefix?
            retMap.insert(TQStrQStrPair(i->first, i->second));
        }
        ++i;
    }
    return retMap;
}

/*
int ProcessorHandlerHelper::GuessYear(const QDateTime &startDateTime, const QDateTime &endDateTime) {
    const QDate &startDate = startDateTime.date();
    const QDate &endDate = endDateTime.date();

    int startYear = startDate.year();
    int endYear = endDate.year();
    if (startYear >= endYear) {
        return endYear;
    }
    if (endYear - startYear > 1) {
        return startYear;   // cannot deduce anything from here but we should not have this situation
    }
    QDate startDateEoy(startYear, 12, 31);
    QDate endDateSoy(endYear, 1, 1);
    qint64 startDiff = startDate.daysTo(startDateEoy);
    qint64 endDiff = endDateSoy.daysTo(endDate);
    // if there are more days in the first year, we consider this year, otherwise the end year
    return startDiff > endDiff ? startYear : endYear;
}
*/

QDateTime ProcessorHandlerHelper::GetDateTimeFromString(const QString &strTime) {
    QDateTime date = QDateTime::fromString(strTime, "yyyyMMdd");
    if (!date.isValid()) {
        date = QDateTime::fromString(strTime, "yyyy-MM-dd");
    }
    return date;
}

QDateTime ProcessorHandlerHelper::GetLocalDateTime(const QString &strTime) {
    QDateTime dateTime = GetDateTimeFromString(strTime);
    dateTime.setTimeSpec(Qt::UTC); // mark the timestamp as UTC (but don't convert it)
    dateTime = dateTime.toLocalTime();
    if (dateTime.isDaylightTime()) {
        dateTime = dateTime.addSecs(-3600);
    }
    return dateTime;
}

