#include "instrument.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double TripleOscProcessor::process(double t, double freq, const TripleOscConfig& config, double sampleRate) {
    auto getWave = [](int type, double phase) {
        double p = fmod(phase, 1.0);
        if (type == 0) return sin(p * 2.0 * M_PI);
        if (type == 1) return 2.0 * (p < 0.5 ? p : 1.0 - p) - 1.0;
        if (type == 2) return 2.0 * p - 1.0;
        return p < 0.5 ? 1.0 : -1.0;
    };
    m_phase1 += (freq * pow(2.0, config.osc1Coarse / 12.0)) / sampleRate;
    m_phase2 += (freq * pow(2.0, config.osc2Coarse / 12.0)) / sampleRate;
    m_phase3 += (freq * pow(2.0, config.osc3Coarse / 12.0)) / sampleRate;

    double out = (getWave(config.osc1Wave, m_phase1) * config.osc1Vol) +
                 (getWave(config.osc2Wave, m_phase2) * config.osc2Vol) +
                 (getWave(config.osc3Wave, m_phase3) * config.osc3Vol);
    return (out / 3.0) * config.vol;
}

double Lb302Processor::process(double t, double freq, const Lb302Config& config, double sampleRate, bool noteTrigger) {
    if (noteTrigger) m_envState = 1.0;
    m_phase += freq / sampleRate;
    double p = fmod(m_phase, 1.0);
    double raw = (config.waveForm == 0) ? (2.0 * p - 1.0) : ((p < 0.5) ? 1.0 : -1.0);
    m_envState *= exp(-1.0 / (sampleRate * config.vcfDecay));
    double cutoff = std::max(0.01, std::min(1.0, config.vcfCut + m_envState * 0.5));
    m_filterState += cutoff * (raw - m_filterState);
    return m_filterState * config.vol;
}

void setupAdsrSpinboxes(QFormLayout* form, QDoubleSpinBox*& a, QDoubleSpinBox*& d, QDoubleSpinBox*& s, QDoubleSpinBox*& r, double av, double dv, double sv, double rv) {
    a = new QDoubleSpinBox(); a->setRange(0, 5); a->setSingleStep(0.01); a->setValue(av);
    d = new QDoubleSpinBox(); d->setRange(0, 5); d->setSingleStep(0.01); d->setValue(dv);
    s = new QDoubleSpinBox(); s->setRange(0, 1); s->setSingleStep(0.1);  s->setValue(sv);
    r = new QDoubleSpinBox(); r->setRange(0, 5); r->setSingleStep(0.01); r->setValue(rv);
    form->addRow("Attack(s):", a); form->addRow("Decay(s):", d); form->addRow("Sustain:", s); form->addRow("Release(s):", r);
}

ChannelConfigDialog::ChannelConfigDialog(ChannelConfig& config, QWidget *parent) : QDialog(parent) {
    setWindowTitle("Configure " + config.name);
    setMinimumWidth(350);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QTabWidget* tabs = new QTabWidget();

    QWidget* tabOsc = new QWidget(); QFormLayout* formOsc = new QFormLayout(tabOsc);
    txtW1 = new QTextEdit(config.w1); txtW1->setMaximumHeight(40); formOsc->addRow("W1:", txtW1);
    txtO1 = new QTextEdit(config.o1); txtO1->setMaximumHeight(40); formOsc->addRow("O1:", txtO1);
    setupAdsrSpinboxes(formOsc, spinA, spinD, spinS, spinR, config.attack, config.decay, config.sustain, config.release);
    spinVol = new QDoubleSpinBox(); spinVol->setRange(0, 2); spinVol->setValue(config.vol); formOsc->addRow("Volume:", spinVol);
    tabs->addTab(tabOsc, "Oscillator / Amp");

    QWidget* tabFilt = new QWidget(); QFormLayout* formFilt = new QFormLayout(tabFilt);

    chkFilt = new QCheckBox("Enable Filter"); chkFilt->setChecked(config.filter.enabled); formFilt->addRow(chkFilt);
    cmbFiltType = new QComboBox(); cmbFiltType->addItems({"Lowpass", "Highpass", "Bandpass", "Notch"});
    cmbFiltType->setCurrentIndex(config.filter.type); formFilt->addRow("Topology:", cmbFiltType);

    fCutBase = new QDoubleSpinBox(); fCutBase->setRange(20, 20000); fCutBase->setValue(config.filter.cutBase); formFilt->addRow("Cutoff Base (Hz):", fCutBase);
    fCutAmt = new QDoubleSpinBox(); fCutAmt->setRange(-10000, 10000); fCutAmt->setValue(config.filter.cutEnvAmt); formFilt->addRow("Cutoff Env Amt:", fCutAmt);
    setupAdsrSpinboxes(formFilt, fCutA, fCutD, fCutS, fCutR, config.filter.cutA, config.filter.cutD, config.filter.cutS, config.filter.cutR);
    fResBase = new QDoubleSpinBox(); fResBase->setRange(0, 0.99); fResBase->setSingleStep(0.05); fResBase->setValue(config.filter.resBase); formFilt->addRow("Resonance Base:", fResBase);
    fResAmt = new QDoubleSpinBox(); fResAmt->setRange(-0.99, 0.99); fResAmt->setSingleStep(0.05); fResAmt->setValue(config.filter.resEnvAmt); formFilt->addRow("Resonance Env Amt:", fResAmt);
    setupAdsrSpinboxes(formFilt, fResA, fResD, fResS, fResR, config.filter.resA, config.filter.resD, config.filter.resS, config.filter.resR);
    tabs->addTab(tabFilt, "Filter & ENV");

    mainLayout->addWidget(tabs);
    QPushButton* btnSave = new QPushButton("Apply & Close");
    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnSave);
    setStyleSheet("QDialog { background-color: #2b2b2b; color: #ffffff; font-size: 11px; } QTextEdit, QDoubleSpinBox, QCheckBox, QComboBox { background-color: #1e1e1e; color: #fff; border: 1px solid #555; } QPushButton { background-color: #007acc; padding: 8px; color: white; font-weight: bold; border-radius: 4px; }");
}

void ChannelConfigDialog::saveToConfig(ChannelConfig& config) {
    config.w1 = txtW1->toPlainText(); config.o1 = txtO1->toPlainText();
    config.attack = spinA->value(); config.decay = spinD->value(); config.sustain = spinS->value(); config.release = spinR->value(); config.vol = spinVol->value();

    config.filter.enabled = chkFilt->isChecked();
    config.filter.type = cmbFiltType->currentIndex();
    config.filter.cutBase = fCutBase->value(); config.filter.cutEnvAmt = fCutAmt->value();
    config.filter.cutA = fCutA->value(); config.filter.cutD = fCutD->value(); config.filter.cutS = fCutS->value(); config.filter.cutR = fCutR->value();
    config.filter.resBase = fResBase->value(); config.filter.resEnvAmt = fResAmt->value();
    config.filter.resA = fResA->value(); config.filter.resD = fResD->value(); config.filter.resS = fResS->value(); config.filter.resR = fResR->value();
}

MelodicConfigDialog::MelodicConfigDialog(MelodicConfig& config, QWidget *parent) : QDialog(parent) {
    setWindowTitle("Configure " + config.name);
    setMinimumWidth(350);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QTabWidget* tabs = new QTabWidget();

    QWidget* tabOsc = new QWidget(); QFormLayout* formOsc = new QFormLayout(tabOsc);
    txtW1 = new QTextEdit(config.w1); txtW1->setMaximumHeight(40); formOsc->addRow("W1:", txtW1);
    txtW2 = new QTextEdit(config.w2); txtW2->setMaximumHeight(40); formOsc->addRow("W2:", txtW2);
    txtW3 = new QTextEdit(config.w3); txtW3->setMaximumHeight(40); formOsc->addRow("W3:", txtW3);
    txtO1 = new QTextEdit(config.o1); txtO1->setMaximumHeight(40); formOsc->addRow("O1:", txtO1);
    txtO2 = new QTextEdit(config.o2); txtO2->setMaximumHeight(40); formOsc->addRow("O2:", txtO2);
    chkUseAdsr = new QCheckBox("Enable Master ADSR"); chkUseAdsr->setChecked(config.useAdsr); formOsc->addRow(chkUseAdsr);
    setupAdsrSpinboxes(formOsc, spinA, spinD, spinS, spinR, config.attack, config.decay, config.sustain, config.release);
    spinVol = new QDoubleSpinBox(); spinVol->setRange(0, 2); spinVol->setValue(config.vol); formOsc->addRow("Volume:", spinVol);
    tabs->addTab(tabOsc, "Oscillator / Amp");

    QWidget* tabFilt = new QWidget(); QFormLayout* formFilt = new QFormLayout(tabFilt);

    chkFilt = new QCheckBox("Enable Filter"); chkFilt->setChecked(config.filter.enabled); formFilt->addRow(chkFilt);
    cmbFiltType = new QComboBox(); cmbFiltType->addItems({"Lowpass", "Highpass", "Bandpass", "Notch"});
    cmbFiltType->setCurrentIndex(config.filter.type); formFilt->addRow("Topology:", cmbFiltType);

    fCutBase = new QDoubleSpinBox(); fCutBase->setRange(20, 20000); fCutBase->setValue(config.filter.cutBase); formFilt->addRow("Cutoff Base (Hz):", fCutBase);
    fCutAmt = new QDoubleSpinBox(); fCutAmt->setRange(-10000, 10000); fCutAmt->setValue(config.filter.cutEnvAmt); formFilt->addRow("Cutoff Env Amt:", fCutAmt);
    setupAdsrSpinboxes(formFilt, fCutA, fCutD, fCutS, fCutR, config.filter.cutA, config.filter.cutD, config.filter.cutS, config.filter.cutR);
    fResBase = new QDoubleSpinBox(); fResBase->setRange(0, 0.99); fResBase->setSingleStep(0.05); fResBase->setValue(config.filter.resBase); formFilt->addRow("Resonance Base:", fResBase);
    fResAmt = new QDoubleSpinBox(); fResAmt->setRange(-0.99, 0.99); fResAmt->setSingleStep(0.05); fResAmt->setValue(config.filter.resEnvAmt); formFilt->addRow("Resonance Env Amt:", fResAmt);
    setupAdsrSpinboxes(formFilt, fResA, fResD, fResS, fResR, config.filter.resA, config.filter.resD, config.filter.resS, config.filter.resR);
    tabs->addTab(tabFilt, "Filter & ENV");

    mainLayout->addWidget(tabs);
    QPushButton* btnSave = new QPushButton("Apply & Close");
    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnSave);
    setStyleSheet("QDialog { background-color: #2b2b2b; color: #ffffff; font-size: 11px; } QTextEdit, QDoubleSpinBox, QCheckBox, QComboBox { background-color: #1e1e1e; color: #fff; border: 1px solid #555; } QPushButton { background-color: #7a00cc; padding: 8px; color: white; font-weight: bold; border-radius: 4px; }");
}

void MelodicConfigDialog::saveToConfig(MelodicConfig& config) {
    config.w1 = txtW1->toPlainText(); config.w2 = txtW2->toPlainText(); config.w3 = txtW3->toPlainText();
    config.o1 = txtO1->toPlainText(); config.o2 = txtO2->toPlainText(); config.useAdsr = chkUseAdsr->isChecked();
    config.attack = spinA->value(); config.decay = spinD->value(); config.sustain = spinS->value(); config.release = spinR->value(); config.vol = spinVol->value();

    config.filter.enabled = chkFilt->isChecked();
    config.filter.type = cmbFiltType->currentIndex();
    config.filter.cutBase = fCutBase->value(); config.filter.cutEnvAmt = fCutAmt->value();
    config.filter.cutA = fCutA->value(); config.filter.cutD = fCutD->value(); config.filter.cutS = fCutS->value(); config.filter.cutR = fCutR->value();
    config.filter.resBase = fResBase->value(); config.filter.resEnvAmt = fResAmt->value();
    config.filter.resA = fResA->value(); config.filter.resD = fResD->value(); config.filter.resS = fResS->value(); config.filter.resR = fResR->value();
}