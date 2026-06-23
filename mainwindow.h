#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QScrollArea>
#include <QMutex>
#include <QTabWidget>
#include <QStackedWidget>
#include <QListWidget>

#include "instrument.h"
#include "effect.h"
#include "pianoroll.h"
#include "automation.h"
#include "xpressive_parser.h"

class SynthEngine;

// NATIVE LMMS-STYLE STATE VARIABLE FILTER
struct SVFState {
    double ic1eq = 0.0, ic2eq = 0.0;

    double process(int type, double v0, double cutoff, double res, double sampleRate) {
        cutoff = std::max(20.0, std::min(cutoff, sampleRate / 2.1));
        res = std::max(0.0, std::min(res, 0.99));

        double g = std::tan(M_PI * cutoff / sampleRate);
        double k = 2.0 - 2.0 * res;

        double a1 = 1.0 / (1.0 + g * (g + k));
        double a2 = g * a1;
        double a3 = g * a2;

        double v3 = v0 - ic2eq;
        double v1 = a1 * ic1eq + a2 * v3;
        double v2 = ic2eq + a2 * ic1eq + a3 * v3;

        ic1eq = 2.0 * v1 - ic1eq;
        ic2eq = 2.0 * v2 - ic2eq;

        if (type == 0) return v2; // Lowpass
        if (type == 1) return v0 - k * v1 - v2; // Highpass
        if (type == 2) return v1; // Bandpass
        return v0 - k * v1; // Notch
    }
};

// --- NEW: Structures for Live Compilation ---
struct VoiceAST {
    std::shared_ptr<XpressiveParser::Node> o1;
    std::shared_ptr<XpressiveParser::Node> w1;
    std::shared_ptr<XpressiveParser::Node> w2;
    std::shared_ptr<XpressiveParser::Node> w3;
};

struct CompiledASTs {
    VoiceAST drums[6];
    VoiceAST synths[3][96];
};
// ---------------------------------------------

struct PlaybackState {
    QMutex mutex;
    bool drumSteps[6][16] = {{false}};
    NoteEvent synthGrid[3][8][96][16];
    int arranger[4][8] = {{0}};
    double bpm = 120.0;

    struct DrumLive { double a, d, s, vol, baseF; int wType; bool isK; FilterConfig filter; };
    struct SynthLive { double a, d, s, r, vol; int w1Type; bool useAdsr; FilterConfig filter; };

    DrumLive drums[6];
    SynthLive synths[3];

    struct AutoLive { double steps[16]; AutoTarget target; };
    AutoLive automations[4];

    SVFState drumFilters[6];
    SVFState synthFilters[3][96];

    // NEW: Safe live-pointer to the compiled mathematics
    std::shared_ptr<CompiledASTs> asts;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConfigClicked(int channelIndex);
    void onMelodicConfigClicked(int index);
    void onPianoRollClicked(int index);
    void onPlayClicked();
    void onTestStringClicked();
    void onArrangerClicked(int track, int bar);
    void onExportMmpClicked();
    void onImportMmpClicked();
    void updatePlaybackState();

    void onSynthTypeChanged(int index);
    void onHeadlessSynthParamChanged();
    void onAddEffectClicked();
    void onRemoveEffectClicked();

private:
    void setupUI();
    QString getAdsrString(double a, double d, double s, double r);


    void recompileASTs();

    static const int NUM_CHANNELS = 6;
    static const int NUM_MELODIC = 3;
    static const int NUM_STEPS = 16;

    SynthEngine* m_synthEngine;

    QSpinBox* m_spinBpm;
    QPushButton* m_arrangerGrid[4][8];
    int m_arrangerState[4][8] = {{0}};
    AutomationTrack m_autoTracks[4];
    AutomationGrid* m_autoGrids[4];
    PlaybackState m_playState;

    ChannelConfig m_channels[NUM_CHANNELS];
    QCheckBox* m_grid[NUM_CHANNELS][NUM_STEPS];
    MelodicConfig m_melodic[NUM_MELODIC];
    QComboBox* m_comboEditBar[NUM_MELODIC];

    QTabWidget* m_mainTabs;
    QPushButton* m_btnPlay, *m_btnTestString, *m_btnExport, *m_btnImport;
    QLineEdit* m_txtTestString;

    QComboBox* m_cmbSynthSelector;
    QStackedWidget* m_synthUIStack;

    QComboBox* m_lbWave, *m_toW1, *m_toW2, *m_toW3;
    QDoubleSpinBox *m_lbCut, *m_lbRes, *m_lbDec, *m_lbVol;
    QDoubleSpinBox *m_toC1, *m_toC2, *m_toC3, *m_toV1, *m_toV2, *m_toV3, *m_toVol;

    QListWidget* m_lstEffects;
    QComboBox* m_cmbAvailableEffects, *m_cmbFxTarget;
    QPushButton* m_btnAddEffect, *m_btnRemoveEffect;
};

#endif // MAINWINDOW_H