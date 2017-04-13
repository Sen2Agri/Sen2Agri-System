#include "processorhandlerhelper.h"
#include "qdiriterator.h"

#include "qdatetime.h"

#define EMPTY_TILE_ID           "00000"
#define INVALID_FILE_SEQUENCE   "!&"
// Map from the sensor name to :
//      - product type
//      - pattern containing index of the date in the file name of the L2A product assuming _ separation
// NOTE: The key of the map is the string as it appears in the file name of the product (S2, L8, SPOT4 etc.) and
//       not the name of the satellite as it appears inside the file metadata of product (that can be SENTINEL-2A, LANDSAT_8 etc.)
/* static */
QMap<QString, ProcessorHandlerHelper::L2MetaTileNameInfos> ProcessorHandlerHelper::m_mapSensorL2ATileMetaFileInfos =
    {{"S2", {ProcessorHandlerHelper::L2_PRODUCT_TYPE_S2, ProcessorHandlerHelper::SATELLITE_ID_TYPE_S2, 8, "hdr", "S2A|S2B_*_*_L2VALD_<TILEID>_*_*_*_<DATE>.HDR"}},
     {"L8", {ProcessorHandlerHelper::L2_PRODUCT_TYPE_L8, ProcessorHandlerHelper::SATELLITE_ID_TYPE_L8, 5, "hdr", "L8_*_L8C_L2VALD_<TILEID>_<DATE>.HDR"}},
     {"SPOT4", {ProcessorHandlerHelper::L2_PRODUCT_TYPE_SPOT4, ProcessorHandlerHelper::SATELLITE_ID_TYPE_SPOT4, 3, "xml", "SPOT4_*_*_<DATE>_*_*.xml"}},
     {"SPOT5", {ProcessorHandlerHelper::L2_PRODUCT_TYPE_SPOT5, ProcessorHandlerHelper::SATELLITE_ID_TYPE_SPOT5, 3, "xml", "SPOT5_*_*_<DATE>_*_*.xml"}},
     // this prefix is impossible to occur in the file name
     {INVALID_FILE_SEQUENCE, {ProcessorHandlerHelper::L2_PRODUCT_TYPE_UNKNOWN, ProcessorHandlerHelper::SATELLITE_ID_TYPE_UNKNOWN, -1, INVALID_FILE_SEQUENCE, ""}}
};

QMap<QString, ProductType> ProcessorHandlerHelper::m_mapHighLevelProductTypeInfos = {
    {"L3A", ProductType::L3AProductTypeId},
    {"L3B", ProductType::L3BProductTypeId},
    {"L3C", ProductType::L3CProductTypeId},
    {"L3D", ProductType::L3DProductTypeId},
    {"L3E", ProductType::L3EProductTypeId},
    {"L4A", ProductType::L4AProductTypeId},
    {"L4B", ProductType::L4BProductTypeId},
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
    QStringList nameWords = name.split("_");
    if(nameWords.size() > 2) {
        // we have an higher level product
        if(nameWords[0].indexOf("S2AGRI") != -1) {
            QMap<QString, ProductType>::iterator i;
            i = m_mapHighLevelProductTypeInfos.find(nameWords[1]);
            if(i != m_mapHighLevelProductTypeInfos.end()) {
                return i.value();
            }
        } else {
            const L2MetaTileNameInfos &infos = GetL2AProductTileNameInfos(name);
            if(infos.productType != L2_PRODUCT_TYPE_UNKNOWN) {
                return ProductType::L2AProductTypeId;
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
    // Split the name by "_" and search the part having _Txxxxx (_T followed by 5 characters)
    QStringList pieces = fileNameWithoutExtension.split("_");

    ProductType productType = GetProductTypeFromFileName(path, false);
    if(productType != ProductType::InvalidProductTypeId) {
        if(productType == ProductType::L2AProductTypeId) {
            const L2MetaTileNameInfos &infos = GetL2AProductTileNameInfos(fileNameWithoutExtension);
            satelliteId = infos.satelliteIdType;
            if(infos.productType == L2_PRODUCT_TYPE_S2) {
                if((pieces.size() == 9) && (pieces[5] == "")) {
                    return QString(pieces[4]);
                }
            } else if(infos.productType == L2_PRODUCT_TYPE_L8) {
                if((pieces.size() == 6) && (pieces[3] == "L2VALD")) {
                    return QString(pieces[4]);
                }
            }
        } else {
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
    QStringList pieces = productName.split("_");
    for (int i = 0; i < pieces.size(); i++) {
        const QString &piece = pieces[i];
        if(piece.length() == 0) // is it possible?
            continue;
        bool bIsInterval = (piece[0] == 'V');
        bool bIsAcquisition = (piece[0] == 'A');
        if(bIsInterval || bIsAcquisition) {
            QString timeFormat("yyyyMMdd");
            QString trimmedPiece = piece.right(piece.size()-1);
            // check if the date is in formate yyyyMMddTHHmmss (ISO 8601)
            if(trimmedPiece.length() == 15 && trimmedPiece[8] == 'T') {
                // use this format, more precise
                timeFormat = "yyyyMMddTHHmmss";
            }
            minDate = QDateTime::fromString(trimmedPiece, timeFormat);
            if(bIsInterval && (i+1) < pieces.size())
                maxDate = QDateTime::fromString(pieces[i+1], timeFormat);
            else
                maxDate = minDate;
            return true;
        }
    }

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


const ProcessorHandlerHelper::L2MetaTileNameInfos &ProcessorHandlerHelper::GetL2AProductTileNameInfos(const QString &metaFileName) {
    QMap<QString, L2MetaTileNameInfos>::iterator i;
    for (i = m_mapSensorL2ATileMetaFileInfos.begin(); i != m_mapSensorL2ATileMetaFileInfos.end(); ++i) {
        if(metaFileName.indexOf(i.key()) == 0) {
            return i.value();
        }
    }
    return m_mapSensorL2ATileMetaFileInfos[INVALID_FILE_SEQUENCE];
}

ProcessorHandlerHelper::L2ProductType ProcessorHandlerHelper::GetL2AProductTypeFromTile(const QString &tileMetadataPath) {
    QFileInfo info(tileMetadataPath);
    QString metaFile = info.fileName();

    return GetL2AProductTileNameInfos(metaFile).productType;
}

ProcessorHandlerHelper::SatelliteIdType ProcessorHandlerHelper::GetL2ASatelliteFromTile(const QString &tileMetadataPath) {
    QFileInfo info(tileMetadataPath);
    QString metaFile = info.fileName();

    return GetL2AProductTileNameInfos(metaFile).satelliteIdType;
}

QDateTime ProcessorHandlerHelper::GetL2AProductDateFromPath(const QString &path) {
    QFileInfo info(path);
    QString name;
    if(info.isDir()) {
        name = info.dir().dirName();
    } else {
        name = info.baseName();
    }

    int idx = GetL2AProductTileNameInfos(name).dateIdxInName;
    QStringList nameWords = name.split("_");
    if(idx >= 0 && idx < nameWords.size())
        return QDateTime::fromString(nameWords[idx], "yyyyMMdd");
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

    if(listComponents.size() > 1) {
        ext = listComponents[listComponents.size()-1];
    }
    listComponents = fileName.split("_");

    const L2MetaTileNameInfos &infos = GetL2AProductTileNameInfos(fileName);

    if((infos.productType != L2_PRODUCT_TYPE_UNKNOWN) && ext.compare(infos.extension, Qt::CaseInsensitive) != 0)  {
        return false;
    }
    switch (infos.productType) {
        case L2_PRODUCT_TYPE_S2:
            if(listComponents.size() < 4 || listComponents[3] != "L2VALD" || listComponents[2] == "QCK") {
                return false;
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

