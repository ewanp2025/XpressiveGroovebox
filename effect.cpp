#include "effect.h"

void DummyDriveEffect::setParameter(int index, double value) { if (index == 0) m_drive = std::max(1.0, value); }
double DummyDriveEffect::process(double input) {
    double processed = input * m_drive;
    if (processed > 1.0) return 1.0;
    if (processed < -1.0) return -1.0;
    return processed;
}

void BitcrushEffect::setParameter(int index, double value) {
    if (index == 0) m_rate = std::clamp(value, 0.01, 1.0);
    if (index == 1) m_depth = std::clamp(value, 1.0, 16.0);
}
double BitcrushEffect::process(double input) {
    m_phase += m_rate;
    if (m_phase >= 1.0) {
        m_phase -= 1.0;
        double maxVal = std::pow(2.0, m_depth) - 1.0;
        m_heldSample = std::round(input * maxVal) / maxVal;
    }
    return m_heldSample;
}

DelayEffect::DelayEffect(double sampleRate) {
    m_buffer.resize((size_t)(sampleRate * 2.0), 0.0);
    m_delaySamples = (int)(sampleRate * 0.3);
}
void DelayEffect::setParameter(int index, double value) {
    if (index == 0) m_delaySamples = (int)(value * 44100.0);
    if (index == 1) m_feedback = std::clamp(value, 0.0, 0.95);
    if (index == 2) m_mix = std::clamp(value, 0.0, 1.0);
}
double DelayEffect::process(double input) {
    int readIdx = m_writeIdx - m_delaySamples;
    if (readIdx < 0) readIdx += m_buffer.size();
    double delayedSample = m_buffer[readIdx];
    m_buffer[m_writeIdx] = input + (delayedSample * m_feedback);
    m_writeIdx++;
    if (m_writeIdx >= m_buffer.size()) m_writeIdx = 0;
    return (input * (1.0 - m_mix)) + (delayedSample * m_mix);
}


ReverbEffect::ReverbEffect(double sampleRate) {

    m_combs[0].init(1116 * sampleRate / 44100.0);
    m_combs[1].init(1188 * sampleRate / 44100.0);
    m_combs[2].init(1277 * sampleRate / 44100.0);
    m_combs[3].init(1356 * sampleRate / 44100.0);
    m_allpasses[0].init(225 * sampleRate / 44100.0);
    m_allpasses[1].init(341 * sampleRate / 44100.0);
}
void ReverbEffect::setParameter(int index, double value) {
    if (index == 0) m_size = std::clamp(value, 0.0, 0.98);
    if (index == 1) m_damp = std::clamp(value, 0.0, 1.0);
    if (index == 2) m_mix = std::clamp(value, 0.0, 1.0);
}
double ReverbEffect::process(double input) {
    double out = 0.0;

    for (int i=0; i<4; i++) {
        out += m_combs[i].process(input, m_size, m_damp);
    }

    out = m_allpasses[0].process(out);
    out = m_allpasses[1].process(out);

    return (input * (1.0 - m_mix)) + (out * m_mix);
}

void EffectChain::addEffect(std::unique_ptr<Effect> effect) {
    if (effect) m_effects.push_back(std::move(effect));
}
void EffectChain::clear() {
    m_effects.clear();
}
double EffectChain::process(double input) {
    double sample = input;
    for (auto& effect : m_effects) sample = effect->process(sample);
    return sample;
}