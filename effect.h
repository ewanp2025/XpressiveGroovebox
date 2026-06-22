#ifndef EFFECT_H
#define EFFECT_H

#include <algorithm>


class Effect {
public:
    virtual ~Effect() = default;


    virtual double process(double input) = 0;


    virtual void setParameter(int index, double value) {}
};


class DummyDriveEffect : public Effect {
    double m_drive = 1.0;
public:
    void setParameter(int index, double value) override;
    double process(double input) override;
};

#endif // EFFECT_H