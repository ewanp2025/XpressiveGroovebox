#ifndef PROJECTIO_H
#define PROJECTIO_H

#include <QString>
#include "instrument.h"

class ProjectIO {
public:

    static bool saveMmp(const QString& filePath, 
                        int bpm,
                        const ChannelConfig channels[6],
                        const bool drumSteps[6][16],
                        const int arrangerState[4][8],
                        const MelodicConfig melodic[3]);


    static bool loadMmp(const QString& filePath, 
                        int& bpm,
                        ChannelConfig channels[6],
                        bool drumSteps[6][16],
                        int arrangerState[4][8],
                        MelodicConfig melodic[3]);
};

#endif // PROJECTIO_H