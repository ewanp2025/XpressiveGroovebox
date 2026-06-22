#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <QString>
#include <QDialog>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include "pianoroll.h" // We need the NoteEvent struct

struct ChannelConfig {
    QString name = "Channel";
    QString w1 = "sinew(t)";
    QString o1 = "W1(integrate(f))";
    double attack = 0.0;
    double decay = 0.0;
    double sustain = 1.0;
    double release = 0.0;
    double vol = 0.8;
};

struct MelodicConfig {
    QString name = "Synth";
    QString w1 = "sinew(t)";
    QString w2 = "saww(t)";
    QString w3 = "";
    QString o1 = "(W1(integrate(f)) + W2(integrate(f*1.01))) * 0.5";
    QString o2 = ""; // NEW
    
    bool useAdsr = true;
    
    double attack = 0.05;
    double decay = 0.3;
    double sustain = 0.5;
    double release = 0.2;
    double vol = 0.6;
    
    NoteEvent grid[8][96][16];
};


class ChannelConfigDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChannelConfigDialog(ChannelConfig& config, QWidget *parent = nullptr);
    void saveToConfig(ChannelConfig& config);
private:
    QTextEdit *txtW1, *txtO1;
    QDoubleSpinBox *spinA, *spinD, *spinS, *spinR, *spinVol;
};

class MelodicConfigDialog : public QDialog {
    Q_OBJECT
public:
    explicit MelodicConfigDialog(MelodicConfig& config, QWidget *parent = nullptr);
    void saveToConfig(MelodicConfig& config);
private:
    QTextEdit *txtW1, *txtW2, *txtW3, *txtO1, *txtO2;
    QCheckBox *chkUseAdsr;
    QDoubleSpinBox *spinA, *spinD, *spinS, *spinR, *spinVol;
};

#endif // INSTRUMENT_H