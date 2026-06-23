#ifndef SYNTHENGINE_H
#define SYNTHENGINE_H

#include <QIODevice>
#include <QAudioSink>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QMutex>
#include <cmath>
#include <functional>
#include <QString>

#include "effect.h"
#include "instrument.h"


using AudioCallback = std::function<void(double t, double outDrums[6], double outSynths[3], double& masterVol, double& masterDrive)>;

class SynthEngine : public QIODevice {
    Q_OBJECT

public:
    explicit SynthEngine(QObject *parent = nullptr);
    ~SynthEngine();

    void start();
    void stop();
    void setAudioSource(AudioCallback func);

    void setInstrumentType(InstrumentType type) { m_currentInstType = type; }
    void setTripleOscConfig(const TripleOscConfig& config) { m_tripleOscConfig = config; }
    void setLb302Config(const Lb302Config& config) { m_lb302Config = config; }


    void addEffect(int targetIndex, int effectType);
    void clearEffects();

    bool isSequential() const override;
    qint64 bytesAvailable() const override;

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    QAudioSink *m_audioSink = nullptr;
    QAudioFormat m_format;
    AudioCallback m_audioSource;

    QMutex m_mutex;
    double m_sampleRate = 44100.0;
    qint64 m_totalSamples = 0;
    bool m_isPlaying = false;


    EffectChain m_masterEffects;
    EffectChain m_drumEffects[6];
    EffectChain m_synthEffects[3];

    InstrumentType m_currentInstType = InstrumentType::Xpressive;
    TripleOscConfig m_tripleOscConfig;
    Lb302Config m_lb302Config;
    TripleOscProcessor m_tripleOscProcessor;
    Lb302Processor m_lb302Processor;
};

#endif // SYNTHENGINE_H