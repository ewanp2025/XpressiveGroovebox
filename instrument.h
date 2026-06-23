#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <QString>
#include <QDialog>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTabWidget>
#include <QComboBox>
#include "pianoroll.h"

struct FilterConfig {
    bool enabled = false;
    int type = 0;
    double cutBase = 2000.0;
    double cutEnvAmt = 1000.0;
    double cutA = 0.0, cutD = 0.3, cutS = 0.5, cutR = 0.2;

    double resBase = 0.1;
    double resEnvAmt = 0.0;
    double resA = 0.0, resD = 0.3, resS = 0.5, resR = 0.2;
};

struct ChannelConfig {
    QString name = "Channel";
    QString w1 = "sinew(t)";
    QString o1 = "W1(integrate(f))";
    double attack = 0.0, decay = 0.0, sustain = 1.0, release = 0.0, vol = 0.8;
    FilterConfig filter;
};

struct MelodicConfig {
    QString name = "Synth";
    QString w1 = "sinew(t)", w2 = "saww(t)", w3 = "";
    QString o1 = "(W1(integrate(f)) + W2(integrate(f*1.01))) * 0.5", o2 = "";
    bool useAdsr = true;
    double attack = 0.05, decay = 0.3, sustain = 0.5, release = 0.2, vol = 0.6;
    NoteEvent grid[8][96][16];
    FilterConfig filter;
};

enum class InstrumentType { Xpressive, TripleOscillator, Lb302 };

struct TripleOscConfig {
    QString name = "TripleOsc";
    double vol = 0.8;
    int osc1Wave = 0, osc2Wave = 1, osc3Wave = 2;
    double osc1Coarse = 0.0, osc2Coarse = 12.0, osc3Coarse = -12.0;
    double osc1Vol = 1.0, osc2Vol = 0.5, osc3Vol = 0.5;
};

struct Lb302Config {
    QString name = "LB302";
    double vol = 0.8;
    int waveForm = 0;
    double vcfCut = 0.6, vcfRes = 0.5, vcfDecay = 0.4;
};

class TripleOscProcessor {
    double m_phase1 = 0.0, m_phase2 = 0.0, m_phase3 = 0.0;
public:
    double process(double t, double freq, const TripleOscConfig& config, double sampleRate);
};

class Lb302Processor {
    double m_phase = 0.0, m_filterState = 0.0, m_envState = 1.0;
public:
    double process(double t, double freq, const Lb302Config& config, double sampleRate, bool noteTrigger);
};

class ChannelConfigDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChannelConfigDialog(ChannelConfig& config, QWidget *parent = nullptr);
    void saveToConfig(ChannelConfig& config);
private:
    QTextEdit *txtW1, *txtO1;
    QDoubleSpinBox *spinA, *spinD, *spinS, *spinR, *spinVol;
    QCheckBox *chkFilt;
    QComboBox *cmbFiltType;
    QDoubleSpinBox *fCutBase, *fCutAmt, *fCutA, *fCutD, *fCutS, *fCutR;
    QDoubleSpinBox *fResBase, *fResAmt, *fResA, *fResD, *fResS, *fResR;
};

class MelodicConfigDialog : public QDialog {
    Q_OBJECT
public:
    explicit MelodicConfigDialog(MelodicConfig& config, QWidget *parent = nullptr);
    void saveToConfig(MelodicConfig& config);
private:
    QTextEdit *txtW1, *txtW2, *txtW3, *txtO1, *txtO2;
    QCheckBox *chkUseAdsr, *chkFilt;
    QComboBox *cmbFiltType;
    QDoubleSpinBox *spinA, *spinD, *spinS, *spinR, *spinVol;
    QDoubleSpinBox *fCutBase, *fCutAmt, *fCutA, *fCutD, *fCutS, *fCutR;
    QDoubleSpinBox *fResBase, *fResAmt, *fResA, *fResD, *fResS, *fResR;
};

#endif // INSTRUMENT_H