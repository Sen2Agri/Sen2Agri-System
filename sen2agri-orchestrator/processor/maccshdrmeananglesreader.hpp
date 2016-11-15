#ifndef MACCS_HDR_MEAN_ANGLES_READER_H
#define MACCS_HDR_MEAN_ANGLES_READER_H

#include <QXmlStreamReader>
#include <QString>

class MeanAngles
{
public:
    MeanAngles() {zenith = 0; azimuth = 0; valid = false;}
    double zenith;
    double azimuth;
    bool valid;
};

class MeanBandViewingAngles
{
public:
    MeanBandViewingAngles() {bandId = -1;}
    int bandId;
    MeanAngles meanViewingAngles;
};

class MaccsHdrMeanAngles
{
public:
    bool IsValid() { return (sunMeanAngles.valid && meanViewingAnglesList.size() > 0); }

    MeanAngles GetSensorMeanAngles() {
        MeanAngles angles;

        size_t nBandsCnt = meanViewingAnglesList.size();
        int nValidAzimuths = 0;
        int nValidZeniths = 0;
        for(size_t i = 0; i < nBandsCnt; i++) {
            MeanAngles bandAngles = meanViewingAnglesList[i].meanViewingAngles;
            if(!std::isnan(bandAngles.azimuth)) {
                angles.azimuth += bandAngles.azimuth;
                nValidAzimuths++;
            }
            if(!std::isnan(bandAngles.zenith)) {
                angles.zenith += bandAngles.zenith;
                nValidZeniths++;
            }
        }
        if(nValidAzimuths > 0) {
            angles.azimuth = angles.azimuth / nValidAzimuths;
        } else {
            angles.azimuth = 0;
        }
        if(nValidZeniths > 0) {
            angles.zenith = angles.zenith / nValidZeniths;
        } else {
            angles.zenith = 0;
        }

        return angles;
    }

    double GetRelativeAzimuthAngle() {
        MeanAngles sensorAngle = GetSensorMeanAngles();
        double relAzimuth = sunMeanAngles.azimuth - sensorAngle.azimuth;
        return relAzimuth;
    }

    MeanAngles sunMeanAngles;
    QList<MeanBandViewingAngles> meanViewingAnglesList;
};

class MaccsHdrMeanAnglesReader
{
public:
    MaccsHdrMeanAnglesReader(const QString filename);

    MaccsHdrMeanAngles read();

private:
    void processSunMeanAngles();
    void processL8MeanViewingAngles();
    void processS2MeanViewingIncidenceAngles();
    bool extractMeanAngles(double &outZenithAngle, double &outAzimuthAngle);
    QString extractAngle(const QStringRef &angleElement);
    QString readNextText();
    QString errorString();

    QString _filename;
    QXmlStreamReader xml;
    MaccsHdrMeanAngles meanAngles;
};

#endif // MACCS_HDR_MEAN_ANGLES_READER_H
