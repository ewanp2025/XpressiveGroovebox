#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QString>
#include <QScrollArea>

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
    bool grid[96][16] = {{false}};
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
    explicit PianoRollDialog(MelodicConfig& config, QWidget *parent = nullptr);
    void saveToConfig(MelodicConfig& config);
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
    void onExportMmpClicked();

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

    QPushButton* m_btnPlay;
    QPushButton* m_btnExport;
    QSpinBox* m_spinBpm;
    QTextEdit* m_txtPreview;
};

#endif // MAINWINDOW_H