#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QScrollArea>
#include <QMutex>

// Include the new modules
#include "instrument.h"
#include "effect.h"
#include "pianoroll.h"
#include "automation.h"

class SynthEngine;

struct PlaybackState {
    QMutex mutex;
    bool drumSteps[6][16] = {{false}};
    NoteEvent synthGrid[3][8][96][16];
    int arranger[4][8] = {{0}};
    double bpm = 120.0;

    struct DrumLive { double a, d, s, vol, baseF; int wType; bool isK; };
    struct SynthLive { double a, d, s, r, vol; int w1Type; bool useAdsr; };

    DrumLive drums[6];
    SynthLive synths[3];

    struct AutoLive { double steps[16]; AutoTarget target; };
    AutoLive automations[4]; // Let's support up to 4 automation tracks for now

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

private:
    void setupUI();
    QString getAdsrString(double a, double d, double s, double r);

    static const int NUM_CHANNELS = 6;
    static const int NUM_MELODIC = 3;
    static const int NUM_STEPS = 16;

    SynthEngine* m_synthEngine;

    ChannelConfig m_channels[NUM_CHANNELS];
    QCheckBox* m_grid[NUM_CHANNELS][NUM_STEPS];

    MelodicConfig m_melodic[NUM_MELODIC];
    QComboBox* m_comboEditBar[NUM_MELODIC];

    QPushButton* m_btnPlay;
    QLineEdit* m_txtTestString;
    QPushButton* m_btnTestString;
    QPushButton* m_btnExport;
    QPushButton* m_btnImport;
    QSpinBox* m_spinBpm;
    QTextEdit* m_txtPreview;

    QPushButton* m_arrangerGrid[4][8];
    int m_arrangerState[4][8] = {{0}};
    PlaybackState m_playState;


    DummyDriveEffect m_masterEffect;

    AutomationTrack m_autoTracks[4];
    AutomationGrid* m_autoGrids[4];
};

#endif // MAINWINDOW_H