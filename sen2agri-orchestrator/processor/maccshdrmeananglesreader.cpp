#include "maccshdrmeananglesreader.hpp"

#include <QFile>
#include <QDebug>

MaccsHdrMeanAnglesReader::MaccsHdrMeanAnglesReader(const QString filename) :
    _filename(filename)
{}

MaccsHdrMeanAngles MaccsHdrMeanAnglesReader::read() {
    QFile xmlFile(_filename);
    xmlFile.open(QIODevice::ReadOnly);
    xml.setDevice(&xmlFile);
    bool extracted = false;
    QStringRef xmlName;
    while (!extracted && xml.readNextStartElement())
    {
        xmlName = xml.name();
        if(xml.name() == "Earth_Explorer_Header")
        {
            while (!extracted && xml.readNextStartElement())
            {
                xmlName = xml.name();
                if(xml.name() == "Variable_Header")
                {
                    while (!extracted && xml.readNextStartElement())
                    {
                        xmlName = xml.name();
                        if(xml.name() == "Specific_Product_Header")
                        {
                            while (!extracted && xml.readNextStartElement())
                            {
                                xmlName = xml.name();
                                if(xml.name() == "Product_Information")
                                {
                                    bool bSunAnglesExtracted = false;
                                    bool bViewAnglesExtracted = false;
                                    while (xml.readNextStartElement())
                                    {
                                        xmlName = xml.name();
                                        if(xml.name() == "Mean_Sun_Angle" || xml.name() == "Mean_Solar_Angles")
                                        {
                                            processSunMeanAngles();
                                            xml.skipCurrentElement();
                                            bSunAnglesExtracted = true;
                                        } else if(xml.name() == "Mean_Viewing_Incidence_Angle_List") {
                                            // S2
                                            processS2MeanViewingIncidenceAngles();
                                            xml.skipCurrentElement();
                                            bViewAnglesExtracted = true;
                                        } else if(xml.name() == "Mean_Viewing_Angles") {
                                            // L8
                                            processL8MeanViewingAngles();
                                            xml.skipCurrentElement();
                                            bViewAnglesExtracted = true;
                                        } else {
                                            xml.skipCurrentElement();
                                        }
                                        if(bSunAnglesExtracted && bViewAnglesExtracted) {
                                            break;
                                        }
                                    }
                                    extracted = true;
                                } else {
                                    xml.skipCurrentElement();
                                }
                            }
                        } else {
                            xml.skipCurrentElement();
                        }
                    }
                } else {
                    xml.skipCurrentElement();
                }
            }
        }
    }

    // readNextStartElement() leaves the stream in
    // an invalid state at the end. A single readNext()
    // will advance us to EndDocument.
    if (xml.tokenType() == QXmlStreamReader::Invalid)
        xml.readNext();

    if (xml.hasError()) {
        xml.raiseError();
        qDebug() << errorString();
    }
    return meanAngles;
}

void MaccsHdrMeanAnglesReader::processSunMeanAngles() {
    if (!xml.isStartElement() || (xml.name() != "Mean_Sun_Angle" && xml.name() != "Mean_Solar_Angles"))
        return;
    meanAngles.sunMeanAngles.valid = extractMeanAngles(meanAngles.sunMeanAngles.zenith, meanAngles.sunMeanAngles.azimuth);
}

void MaccsHdrMeanAnglesReader::processL8MeanViewingAngles() {
    if (!xml.isStartElement() || xml.name() != "Mean_Viewing_Angles")
        return;
    MeanBandViewingAngles bandViewingAngles;
    // for L8 we have only one entry
    if(extractMeanAngles(bandViewingAngles.meanViewingAngles.zenith, bandViewingAngles.meanViewingAngles.azimuth)) {
        bandViewingAngles.meanViewingAngles.valid = true;
        meanAngles.meanViewingAnglesList.append(bandViewingAngles);
    }
}

void MaccsHdrMeanAnglesReader::processS2MeanViewingIncidenceAngles() {
    if (!xml.isStartElement() || xml.name() != "Mean_Viewing_Incidence_Angle_List")
        return;

    while (xml.readNextStartElement()) {
        if (xml.name() == "Mean_Viewing_Incidence_Angle")
        {
            int bandId = -1;
            foreach(const QXmlStreamAttribute &attr, xml.attributes()) {
                 if (attr.name().toString() == QLatin1String("bandId")) {
                    QString strBandId = attr.value().toString();
                    bool bOkBndId = false;
                    bandId = strBandId.toInt(&bOkBndId);
                    if (!bOkBndId) {bandId  = -1;}
                }
            }
            MeanBandViewingAngles bandViewingAngles;
            bandViewingAngles.bandId = bandId;
            if(extractMeanAngles(bandViewingAngles.meanViewingAngles.zenith, bandViewingAngles.meanViewingAngles.azimuth)) {
                bandViewingAngles.meanViewingAngles.valid = true;
                meanAngles.meanViewingAnglesList.append(bandViewingAngles);
            }
            xml.skipCurrentElement();
        } else {
            xml.skipCurrentElement();
        }
    }
}

bool MaccsHdrMeanAnglesReader::extractMeanAngles(double &outZenithAngle, double &outAzimuthAngle) {
    QString strZenithAngle;
    QString strAzimuthAngle;
    QStringRef xmlName;
    while (xml.readNextStartElement()) {
        xmlName = xml.name();
        if ((xmlName == "ZENITH_ANGLE") || (xmlName == "Zenith"))
            strZenithAngle = extractAngle(xmlName);
        else if ((xmlName == "AZIMUTH_ANGLE") || (xmlName == "Azimuth"))
            strAzimuthAngle = extractAngle(xmlName);
        else
            xml.skipCurrentElement();
        if(!strZenithAngle.isEmpty() && !strAzimuthAngle.isEmpty())
            break;
    }
    bool ok1 = false;
    bool ok2 = false;
    if(!strZenithAngle.isEmpty() && !strAzimuthAngle.isEmpty())
    {
        outZenithAngle = strZenithAngle.toDouble(&ok1);
        outAzimuthAngle = strAzimuthAngle.toDouble(&ok2);
    }
    return (ok1 && ok2);
}


// Uncomment this to see another way to read element
// text. It returns the concatenation of the text
// from all child elements.
//#define USE_READ_ELEMENT_TEXT 1

QString MaccsHdrMeanAnglesReader::extractAngle(const QStringRef &angleElement) {
    if (!xml.isStartElement() || xml.name() != angleElement)
        return "";
    QString angle = readNextText();
    xml.skipCurrentElement();
    return angle;
}

QString MaccsHdrMeanAnglesReader::readNextText() {
#ifndef USE_READ_ELEMENT_TEXT
    xml.readNext();
    return xml.text().toString();
#else
    return xml.readElementText();
#endif
}

QString MaccsHdrMeanAnglesReader::errorString() {
    return QObject::tr("%1\nLine %2, column %3")
            .arg(xml.errorString())
            .arg(xml.lineNumber())
            .arg(xml.columnNumber());
}
