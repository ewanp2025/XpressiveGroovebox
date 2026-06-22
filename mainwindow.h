#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QString>
#include <QScrollArea>
#include <QMutex>

class SynthEngine;

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
    QString o1 = "(W1(integrate(f)) + W2(integrate(f*1.01))) * 0.5";
    double attack = 0.05;
    double decay = 0.3;
    double sustain = 0.5;
    double release = 0.2;
    double vol = 0.6;
    bool grid[8][96][16] = {{{false}}};
};

struct PlaybackState {
    QMutex mutex;
    bool drumSteps[6][16] = {{false}};
    bool synthGrid[3][8][96][16] = {{{{false}}}};
    int arranger[4][8] = {{0}};
    double bpm = 120.0;

    struct DrumLive { double a, d, s, vol, baseF; int wType; bool isK; };
    struct SynthLive { double a, d, s, vol; int w1Type; };

    DrumLive drums[6];
    SynthLive synths[3];
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
    QTextEdit *txtW1, *txtW2, *txtO1;
    QDoubleSpinBox *spinA, *spinD, *spinS, *spinR, *spinVol;
};

class PianoRollDialog : public QDialog {
    Q_OBJECT
public:
    explicit PianoRollDialog(bool (&pattern)[96][16], const QString& title, QWidget *parent = nullptr);
    void saveToGrid(bool (&pattern)[96][16]);
private:
    QPushButton* m_btnGrid[96][16];
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
    QString generateMathExpression();
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
};

#endif // MAINWINDOW_H