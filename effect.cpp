#include "effect.h"



void DummyDriveEffect::setParameter(int index, double value) {
    if (index == 0) {
        m_drive = std::max(1.0, value);
    }
}

double DummyDriveEffect::process(double input) {
    double processed = input * m_drive;
    

    if (processed > 1.0) return 1.0;
    if (processed < -1.0) return -1.0;
    
    return processed;
}