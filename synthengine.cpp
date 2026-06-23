#include "synthengine.h"
#include <QDebug>
#include <QtEndian>
#include <QMutexLocker>

SynthEngine::SynthEngine(QObject *parent) : QIODevice(parent) {
    m_format.setSampleRate(44100);
    m_format.setChannelCount(2);
    m_format.setSampleFormat(QAudioFormat::Float);

    QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (!device.isFormatSupported(m_format)) m_format = device.preferredFormat();
    m_sampleRate = m_format.sampleRate();

    m_audioSink = new QAudioSink(device, m_format, this);
    m_audioSink->setBufferSize(16384);
    open(QIODevice::ReadOnly);
    m_audioSink->start(this);
}

SynthEngine::~SynthEngine() {
    m_audioSink->stop(); close(); delete m_audioSink;
}


void SynthEngine::addEffect(int targetIndex, int effectType) {
    QMutexLocker locker(&m_mutex);
    std::unique_ptr<Effect> fx;

    if (effectType == 0) fx = std::make_unique<DummyDriveEffect>();
    else if (effectType == 1) fx = std::make_unique<BitcrushEffect>();
    else if (effectType == 2) fx = std::make_unique<DelayEffect>(m_sampleRate);
    else if (effectType == 3) fx = std::make_unique<ReverbEffect>(m_sampleRate); // Lush Algorithmic Reverb

    if (!fx) return;

    if (targetIndex == 0) m_masterEffects.addEffect(std::move(fx));
    else if (targetIndex >= 1 && targetIndex <= 6) m_drumEffects[targetIndex - 1].addEffect(std::move(fx));
    else if (targetIndex >= 7 && targetIndex <= 9) m_synthEffects[targetIndex - 7].addEffect(std::move(fx));
}

void SynthEngine::clearEffects() {
    QMutexLocker locker(&m_mutex);
    m_masterEffects.clear();
    for(int i=0; i<6; ++i) m_drumEffects[i].clear();
    for(int i=0; i<3; ++i) m_synthEffects[i].clear();
}

bool SynthEngine::isSequential() const { return true; }

qint64 SynthEngine::bytesAvailable() const {
    if (!isOpen()) return 0;
    return m_audioSink->bufferSize() + QIODevice::bytesAvailable();
}

void SynthEngine::start() { QMutexLocker locker(&m_mutex); m_totalSamples = 0; m_isPlaying = true; }
void SynthEngine::stop() { QMutexLocker locker(&m_mutex); m_isPlaying = false; }
void SynthEngine::setAudioSource(AudioCallback func) { QMutexLocker locker(&m_mutex); m_audioSource = func; }
qint64 SynthEngine::writeData(const char *data, qint64 len) { Q_UNUSED(data); return len; }

qint64 SynthEngine::readData(char *data, qint64 maxlen) {
    QMutexLocker locker(&m_mutex);
    memset(data, 0, maxlen);

    int channels = m_format.channelCount();
    if (channels == 0) return maxlen;

    bool isFloat = (m_format.sampleFormat() == QAudioFormat::Float);
    bool isInt16 = (m_format.sampleFormat() == QAudioFormat::Int16);
    if (!isFloat && !isInt16) return maxlen;

    int bytesPerSample = isFloat ? sizeof(float) : sizeof(qint16);
    int frames = maxlen / (bytesPerSample * channels);

    float *bufferF32 = reinterpret_cast<float*>(data);
    qint16 *bufferI16 = reinterpret_cast<qint16*>(data);

    for (int i = 0; i < frames; ++i) {
        float sample = 0.0f;

        if (m_isPlaying) {
            double t = (double)m_totalSamples / m_sampleRate;
            double outDrums[6] = {0}, outSynths[3] = {0};
            double masterVol = 1.0, masterDrive = 1.0;


            if (m_currentInstType == InstrumentType::Xpressive && m_audioSource) {
                m_audioSource(t, outDrums, outSynths, masterVol, masterDrive);
            }

            else if (m_currentInstType == InstrumentType::TripleOscillator) {
                outSynths[0] = m_tripleOscProcessor.process(t, 220.0, m_tripleOscConfig, m_sampleRate);
            }
            else if (m_currentInstType == InstrumentType::Lb302) {
                bool trigger = (m_totalSamples % (int)m_sampleRate == 0);
                outSynths[0] = m_lb302Processor.process(t, 110.0, m_lb302Config, m_sampleRate, trigger);
            }

            double finalMix = 0.0;


            for(int d=0; d<6; ++d) finalMix += m_drumEffects[d].process(outDrums[d]);
            for(int s=0; s<3; ++s) finalMix += m_synthEffects[s].process(outSynths[s]);


            finalMix *= masterDrive;

            finalMix = m_masterEffects.process(finalMix);

            finalMix *= masterVol;

            sample = (float)(finalMix * 0.5f);
            if (std::isnan(sample) || std::isinf(sample)) sample = 0.0f;
            m_totalSamples++;
        }

        if (isFloat) {
            bufferF32[i * channels] = sample;
            if (channels > 1) bufferF32[i * channels + 1] = sample;
        } else {
            bufferI16[i * channels] = (qint16)(sample * 32767.0f);
            if (channels > 1) bufferI16[i * channels + 1] = (qint16)(sample * 32767.0f);
        }
    }
    return maxlen;
}