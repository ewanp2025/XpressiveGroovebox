#include "synthengine.h"
#include <QDebug>
#include <QtEndian>
#include <QMutexLocker>
#include <QDebug>

SynthEngine::SynthEngine(QObject *parent) : QIODevice(parent) {
    m_format.setSampleRate(44100);
    m_format.setChannelCount(2);
    m_format.setSampleFormat(QAudioFormat::Float);

    QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (!device.isFormatSupported(m_format)) {
        m_format = device.preferredFormat();
    }
    m_audioSink = new QAudioSink(device, m_format, this);
    m_audioSink->setBufferSize(16384);

    open(QIODevice::ReadOnly);
    m_audioSink->start(this);
}

SynthEngine::~SynthEngine() {
    m_audioSink->stop();
    close();
    delete m_audioSink;
}

bool SynthEngine::isSequential() const { return true; }

qint64 SynthEngine::bytesAvailable() const {
    if (!isOpen()) return 0;
    return m_audioSink->bufferSize() + QIODevice::bytesAvailable();
}

void SynthEngine::start() {
    QMutexLocker locker(&m_mutex);
    m_totalSamples = 0;
    m_isPlaying = true;
}
void SynthEngine::stop() {
    QMutexLocker locker(&m_mutex);
    m_isPlaying = false;
}

void SynthEngine::setAudioSource(std::function<double(double)> func) {
    QMutexLocker locker(&m_mutex);
    m_oscillator = func;
}

void SynthEngine::setExpression(QString code) {
    m_currentCode = code;
}

qint64 SynthEngine::readData(char *data, qint64 maxlen) {
    QMutexLocker locker(&m_mutex);
    memset(data, 0, maxlen);

    int channels = m_format.channelCount();
    if (channels == 0) return maxlen;

    bool isFloat = (m_format.sampleFormat() == QAudioFormat::Float);
    bool isInt16 = (m_format.sampleFormat() == QAudioFormat::Int16);

    // If neither format is met, return silence
    if (!isFloat && !isInt16) return maxlen;

    int bytesPerSample = isFloat ? sizeof(float) : sizeof(qint16);
    int frames = maxlen / (bytesPerSample * channels);

    float *bufferF32 = reinterpret_cast<float*>(data);
    qint16 *bufferI16 = reinterpret_cast<qint16*>(data);

    for (int i = 0; i < frames; ++i) {
        float sample = 0.0f;

        if (m_isPlaying && m_oscillator) {
            double t = (double)m_totalSamples / m_format.sampleRate();
            sample = (float)m_oscillator(t) * 0.5f;

            if (std::isnan(sample) || std::isinf(sample)) sample = 0.0f;
            m_totalSamples++;
        }

        for (int c = 0; c < channels; ++c) {
            if (isFloat) {
                *bufferF32++ = sample;
            } else {
                // Clamp and convert floating point to 16-bit integer for Android
                float clamped = std::max(-1.0f, std::min(1.0f, sample));
                *bufferI16++ = static_cast<qint16>(clamped * 32767.0f);
            }
        }
    }
    return maxlen;
}

qint64 SynthEngine::writeData(const char *data, qint64 len) {
    Q_UNUSED(data);
    return len;
}
