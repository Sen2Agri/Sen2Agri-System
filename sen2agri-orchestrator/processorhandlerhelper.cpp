#include "processorhandlerhelper.h"
#include "qdiriterator.h"

#include "qdatetime.h"


#define INVALID_FILE_SEQUENCE   "!&"
// Map from the sensor name to :
//      - product type
//      - pattern containing index of the date in the file name of the L2A product assuming _ separation
/* static */
QMap<QString, ProcessorHandlerHelper::L2MetaTileNameInfos> ProcessorHandlerHelper::m_mapSensorL2ATileMetaFileInfos =
    {{"S2", {ProcessorHandlerHelper::S2, 8, "hdr", "S2A|S2B_*_*_L2VALD_<TILEID>_*_*_*_<DATE>.HDR"}},
     {"L8", {ProcessorHandlerHelper::L8, 5, "hdr", "L8_*_L8C_L2VALD_<TILEID>_<DATE>.HDR"}},
     {"SPOT4", {ProcessorHandlerHelper::SPOT4, 3, "xml", "SPOT4_*_*_<DATE>_*_*.xml"}},
     {"SPOT5", {ProcessorHandlerHelper::SPOT5, 3, "xml", "SPOT5_*_*_<DATE>_*_*.xml"}},
     {INVALID_FILE_SEQUENCE, {ProcessorHandlerHelper::UNKNOWN, -1, INVALID_FILE_SEQUENCE, ""}}    // this prefix is impossible to occur in the file name
};

ProcessorHandlerHelper::ProcessorHandlerHelper() {}

/* static */
QString ProcessorHandlerHelper::GetTileId(const QString &path, bool *ok)
{
    if(ok) *ok = false;
    // First remove the extension
    QFileInfo info(path);
    QString fileNameWithoutExtension = info.completeBaseName();
    QString extension = info.suffix();
    // Split the name by "_" and search the part having _Txxxxx (_T followed by 5 characters)
    QStringList pieces = fileNameWithoutExtension.split("_");
    for (const QString &piece : pieces) {
        if (((piece.length() == 6) || (piece.length() == 7)) && (piece.at(0) == 'T')) {
            if(ok) *ok = true;
            return QString("TILE_" + piece);
        }
    }
    if(QString::compare(extension, "hdr", Qt::CaseInsensitive) == 0) {
        // check if is S2
        if((pieces.size() == 9) && (pieces[0].indexOf("S2") != -1) && (pieces[5] == "")) {
            if(ok) *ok = true;
            return QString("TILE_" + pieces[4]);
        } else if((pieces.size() == 6) && (pieces[0].indexOf("L8") != -1) && (pieces[3] == "L2VALD")) {
            if(ok) *ok = true;
            return QString("TILE_" + pieces[4]);
        }
    }

    return QString("TILE_00000");
}

/* static */
QString ProcessorHandlerHelper::GetTileId(const QStringList &xmlFileNames, bool *ok)
{
    bool bOk = false;
    if(ok) *ok = bOk;
    // we might have combined tiles (from S2 and L8 for example but L8 does not have a tile ID)
    for (const QString &xmlFileName : xmlFileNames) {
        QString tileId = GetTileId(xmlFileName, &bOk);
        if(bOk) {
            if(ok) *ok = bOk;
            return tileId;
        }
    }
    return QString("TILE_00000");
}

QMap<QString, QStringList> ProcessorHandlerHelper::GroupTiles(const QStringList &listAllProductsTiles) {
    QMap<QString, QStringList> mapTiles;
    for(const QString &tileFile: listAllProductsTiles) {
        QString tileId = GetTileId(tileFile);
        if(mapTiles.contains(tileId)) {
            mapTiles[tileId].append(tileFile);
        } else {
            mapTiles[tileId] = QStringList{tileFile};
        }
    }
    return mapTiles;
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
    while(itTiles.hasNext()) {
        QString tileDir = itTiles.next();
        // get the dir name
        QString tileDirName = QFileInfo(tileDir).fileName();
        if(tileDirName == "." || tileDirName == "..") {
            continue;
        }
        // we should have some TIF files in this folder
        QDirIterator itImgData(tileDir + "/IMG_DATA/", QStringList() << "*.TIF", QDir::Files);
        if (!itImgData.hasNext()) {
            return false;
        }
        // check if we have some files here (we cannot assume they are TIF or other format)
        QDirIterator itQiData(tileDir + "/QI_DATA/", QStringList() << "*.*", QDir::Files);
        if (!itQiData.hasNext()) {
            return false;
        }
    }
    // apparently, everything OK
    return true;
}

bool ProcessorHandlerHelper::GetHigLevelProductAcqDatesFromName(const QString &productName, QDateTime &minDate, QDateTime &maxDate) {
    QStringList pieces = productName.split("_");
    for (int i = 0; i < pieces.size(); i++) {
        const QString &piece = pieces[i];
        if(piece.indexOf("V") == 0) {
            minDate = QDateTime::fromString(piece.right(piece.size()-1), "yyyyMMdd");
            if((i+1) < pieces.size())
                maxDate = QDateTime::fromString(pieces[i+1], "yyyyMMdd");
            else
                maxDate = minDate;
            return true;
        }
        if (piece.indexOf("A") == 0) {
            minDate = QDateTime::fromString(piece.right(piece.size()-1), "yyyyMMdd");
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
    // TODO: Maybe we should check if file really exists
    if(isQiData) {
        return tileDir + "/QI_DATA/" + fileName + ".TIF";
    }
    return tileDir + "/IMG_DATA/" + fileName + ".TIF";
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
            for (const QString &piece : pieces) {
                // we might have 6 (for S2 and others) or 7 (for LANDSAT)
                if (((piece.length() == 6) || (piece.length() == 7)) && (piece.at(0) == 'T')) {
                    // We have the tile
                    QString tileId = "TILE_" + piece.right(piece.length()-1);
                    //QString tileDirPath = tilesDir+ "/" + tileDirName;
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
QStringList ProcessorHandlerHelper::GetProductTileIds(const QStringList &listFiles) {
    bool bOk;
    QStringList result;
    for (const QString &fileName : listFiles) {
        QString tileId = GetTileId(fileName, &bOk);
        if(bOk) {
            result.append(tileId);
        }
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
    QString metaFile = info.fileName();
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

    if((infos.productType != UNKNOWN) && ext.compare(infos.extension, Qt::CaseInsensitive) != 0)  {
        return false;
    }
    switch (infos.productType) {
        case S2:
            if(listComponents.size() < 4 || listComponents[3] != "L2VALD" || listComponents[2] == "QCK") {
                return false;
            }
            break;
        case L8:
            if((listComponents.size() < 4) || (listComponents[2] != "L8C") || (listComponents[3] != "L2VALD")) {
                return false;
            }
            break;
        case SPOT4:
        case SPOT5:
            // TODO: see if something else should be added here
            break;
        case UNKNOWN:
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
