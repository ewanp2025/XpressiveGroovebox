#ifndef EFFECT_H
#define EFFECT_H

#include <algorithm>
#include <vector>
#include <memory>
#include <cmath>

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

class BitcrushEffect : public Effect {
    double m_rate = 1.0;
    double m_depth = 16.0;
    double m_heldSample = 0.0;
    double m_phase = 0.0;
public:
    void setParameter(int index, double value) override;
    double process(double input) override;
};

class DelayEffect : public Effect {
    std::vector<double> m_buffer;
    int m_writeIdx = 0;
    double m_feedback = 0.5;
    double m_mix = 0.5;
    int m_delaySamples = 0;
public:
    DelayEffect(double sampleRate = 44100.0);
    void setParameter(int index, double value) override;
    double process(double input) override;
};


class ReverbEffect : public Effect {
    struct Comb {
        std::vector<double> buf;
        int pos = 0;
        double filterStore = 0.0;
        void init(int size) { buf.assign(size, 0.0); pos = 0; filterStore = 0.0; }
        double process(double in, double feedback, double damp) {
            double out = buf[pos];
            filterStore = (out * (1.0 - damp)) + (filterStore * damp);
            buf[pos] = in + (filterStore * feedback);
            if (++pos >= buf.size()) pos = 0;
            return out;
        }
    };
    struct Allpass {
        std::vector<double> buf;
        int pos = 0;
        void init(int size) { buf.assign(size, 0.0); pos = 0; }
        double process(double in) {
            double out = buf[pos] - in;
            buf[pos] = in + (buf[pos] * 0.5);
            if (++pos >= buf.size()) pos = 0;
            return out;
        }
    };

    Comb m_combs[4];
    Allpass m_allpasses[2];
    double m_size = 0.84;
    double m_damp = 0.2;
    double m_mix = 0.4;

public:
    ReverbEffect(double sampleRate = 44100.0);
    void setParameter(int index, double value) override;
    double process(double input) override;
};

class EffectChain {
    std::vector<std::unique_ptr<Effect>> m_effects;
public:
    void addEffect(std::unique_ptr<Effect> effect);
    void clear();
    double process(double input);
};

#endif // EFFECT_H