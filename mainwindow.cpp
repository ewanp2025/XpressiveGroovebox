#include "mainwindow.h"
#include "synthengine.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QtXml/QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>
#include <QTimer>
#include <QFrame>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


ChannelConfigDialog::ChannelConfigDialog(ChannelConfig& config, QWidget *parent) : QDialog(parent) {
    setWindowTitle("Configure " + config.name);
    setMinimumWidth(300); // Narrower for mobile
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* form = new QFormLayout();

    txtW1 = new QTextEdit(config.w1); txtW1->setMaximumHeight(40); form->addRow("W1:", txtW1);
    txtO1 = new QTextEdit(config.o1); txtO1->setMaximumHeight(40); form->addRow("O1:", txtO1);

    spinA = new QDoubleSpinBox(); spinA->setRange(0, 5); spinA->setValue(config.attack); spinA->setSingleStep(0.01);
    spinD = new QDoubleSpinBox(); spinD->setRange(0, 5); spinD->setValue(config.decay); spinD->setSingleStep(0.01);
    spinS = new QDoubleSpinBox(); spinS->setRange(0, 1); spinS->setValue(config.sustain); spinS->setSingleStep(0.1);
    spinR = new QDoubleSpinBox(); spinR->setRange(0, 5); spinR->setValue(config.release); spinR->setSingleStep(0.01);
    spinVol = new QDoubleSpinBox(); spinVol->setRange(0, 2); spinVol->setValue(config.vol); spinVol->setSingleStep(0.1);

    form->addRow("Attack(s):", spinA); form->addRow("Decay(s):", spinD);
    form->addRow("Sustain:", spinS); form->addRow("Release(s):", spinR);
    form->addRow("Volume:", spinVol);

    mainLayout->addLayout(form);
    QPushButton* btnSave = new QPushButton("Apply & Close");
    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnSave);

    setStyleSheet("QDialog { background-color: #2b2b2b; color: #ffffff; font-size: 11px; } QTextEdit, QDoubleSpinBox { background-color: #1e1e1e; color: #fff; border: 1px solid #555; } QPushButton { background-color: #007acc; padding: 8px; color: white; font-weight: bold; border-radius: 4px; }");
}

void ChannelConfigDialog::saveToConfig(ChannelConfig& config) {
    config.w1 = txtW1->toPlainText(); config.o1 = txtO1->toPlainText();
    config.attack = spinA->value(); config.decay = spinD->value(); config.sustain = spinS->value(); config.release = spinR->value(); config.vol = spinVol->value();
}


MelodicConfigDialog::MelodicConfigDialog(MelodicConfig& config, QWidget *parent) : QDialog(parent) {
    setWindowTitle("Configure " + config.name);
    setMinimumWidth(300);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* form = new QFormLayout();

    txtW1 = new QTextEdit(config.w1); txtW1->setMaximumHeight(40); form->addRow("W1:", txtW1);
    txtW2 = new QTextEdit(config.w2); txtW2->setMaximumHeight(40); form->addRow("W2:", txtW2);
    txtO1 = new QTextEdit(config.o1); txtO1->setMaximumHeight(40); form->addRow("O1:", txtO1);

    spinA = new QDoubleSpinBox(); spinA->setRange(0, 5); spinA->setValue(config.attack); spinA->setSingleStep(0.01);
    spinD = new QDoubleSpinBox(); spinD->setRange(0, 5); spinD->setValue(config.decay); spinD->setSingleStep(0.01);
    spinS = new QDoubleSpinBox(); spinS->setRange(0, 1); spinS->setValue(config.sustain); spinS->setSingleStep(0.1);
    spinR = new QDoubleSpinBox(); spinR->setRange(0, 5); spinR->setValue(config.release); spinR->setSingleStep(0.01);
    spinVol = new QDoubleSpinBox(); spinVol->setRange(0, 2); spinVol->setValue(config.vol); spinVol->setSingleStep(0.1);

    form->addRow("Attack(s):", spinA); form->addRow("Decay(s):", spinD);
    form->addRow("Sustain:", spinS); form->addRow("Release(s):", spinR);
    form->addRow("Volume:", spinVol);

    mainLayout->addLayout(form);
    QPushButton* btnSave = new QPushButton("Apply & Close");
    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnSave);

    setStyleSheet("QDialog { background-color: #2b2b2b; color: #ffffff; font-size: 11px; } QTextEdit, QDoubleSpinBox { background-color: #1e1e1e; color: #fff; border: 1px solid #555; } QPushButton { background-color: #7a00cc; padding: 8px; color: white; font-weight: bold; border-radius: 4px; }");
}

void MelodicConfigDialog::saveToConfig(MelodicConfig& config) {
    config.w1 = txtW1->toPlainText(); config.w2 = txtW2->toPlainText(); config.o1 = txtO1->toPlainText();
    config.attack = spinA->value(); config.decay = spinD->value(); config.sustain = spinS->value(); config.release = spinR->value(); config.vol = spinVol->value();
}


PianoRollDialog::PianoRollDialog(MelodicConfig& config, QWidget *parent) : QDialog(parent) {
    setWindowTitle(config.name + " - Piano Roll");
    resize(350, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(2);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);

    QWidget* gridWidget = new QWidget();
    QGridLayout* layout = new QGridLayout(gridWidget);
    layout->setSpacing(1);
    layout->setContentsMargins(0,0,0,0);

    const QString noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    for (int r = 0; r < 96; ++r) {
        int midiNote = 107 - r;
        int octave = (midiNote / 12) - 1;
        int noteIndex = midiNote % 12;
        bool isBlackKey = (noteIndex == 1 || noteIndex == 3 || noteIndex == 6 || noteIndex == 8 || noteIndex == 10);

        QLabel* keyLabel = new QLabel(noteNames[noteIndex] + QString::number(octave));
        keyLabel->setFixedSize(35, 30);
        keyLabel->setAlignment(Qt::AlignCenter);

        if (isBlackKey) {
            keyLabel->setStyleSheet("background-color: #111; color: white; border: 1px solid #000; font-size: 10px;");
        } else {
            keyLabel->setStyleSheet("background-color: #eee; color: black; border: 1px solid #ccc; font-weight: bold; font-size: 10px;");
        }
        layout->addWidget(keyLabel, r, 0);

        for (int c = 0; c < 16; ++c) {
            m_btnGrid[r][c] = new QPushButton();
            m_btnGrid[r][c]->setFixedSize(14, 30);
            m_btnGrid[r][c]->setCheckable(true);
            m_btnGrid[r][c]->setChecked(config.grid[r][c]);

            QString bg = (c / 4) % 2 == 0 ? "#2a2a2a" : "#333333";
            m_btnGrid[r][c]->setStyleSheet(QString(
                                               "QPushButton { background-color: %1; border: 1px solid #1a1a1a; border-radius: 2px; }"
                                               "QPushButton:checked { background-color: #7a00cc; border: 1px solid #ffffff; }"
                                               ).arg(bg));

            layout->addWidget(m_btnGrid[r][c], r, c + 1);
        }
    }

    scrollArea->setWidget(gridWidget);
    mainLayout->addWidget(scrollArea);

    QPushButton* btnSave = new QPushButton("Save Pattern & Close");
    btnSave->setStyleSheet("background-color: #7a00cc; color: white; padding: 10px; font-weight: bold; border-radius: 4px;");
    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnSave);

    setStyleSheet("QDialog { background-color: #1e1e1e; }");


    QTimer::singleShot(50, [scrollArea, gridWidget]() {
        int targetY = (47 * 32) - (scrollArea->viewport()->height() / 2);
        scrollArea->verticalScrollBar()->setValue(std::max(0, targetY));
    });
}

void PianoRollDialog::saveToConfig(MelodicConfig& config) {
    for(int r=0; r<96; ++r) {
        for(int c=0; c<16; ++c) {
            config.grid[r][c] = m_btnGrid[r][c]->isChecked();
        }
    }
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_synthEngine = new SynthEngine(this);

    setWindowTitle("Xpressive Groovebox (Mobile)");
    resize(350, 700);


    m_channels[0].name = "Kick";  m_channels[0].w1 = "sinew(t)"; m_channels[0].o1 = "W1(integrate(f * exp(-t*15)))"; m_channels[0].decay = 0.3; m_channels[0].sustain = 0; m_channels[0].vol = 1.0;
    m_channels[1].name = "Snar"; m_channels[1].w1 = "randv(t*srate)"; m_channels[1].o1 = "W1(integrate(f)) * exp(-t*10)"; m_channels[1].decay = 0.2; m_channels[1].sustain = 0; m_channels[1].vol = 0.8;
    m_channels[2].name = "CHat";  m_channels[2].w1 = "randv(t*srate)"; m_channels[2].o1 = "W1(integrate(f))"; m_channels[2].decay = 0.05; m_channels[2].sustain = 0; m_channels[2].vol = 0.5;
    m_channels[3].name = "OHat";  m_channels[3].w1 = "randv(t*srate)"; m_channels[3].o1 = "W1(integrate(f))"; m_channels[3].decay = 0.3; m_channels[3].sustain = 0; m_channels[3].vol = 0.6;
    m_channels[4].name = "Prc1"; m_channels[4].w1 = "squarew(t)"; m_channels[4].o1 = "W1(integrate(f)) * exp(-t*20)"; m_channels[4].decay = 0.1; m_channels[4].vol = 0.4;
    m_channels[5].name = "Prc2";  m_channels[5].w1 = "saww(t)"; m_channels[5].o1 = "W1(integrate(f*0.5))"; m_channels[5].decay = 0.4; m_channels[5].sustain = 0.5; m_channels[5].vol = 0.6;


    m_melodic[0].name = "Bass"; m_melodic[0].w1 = "saww(t)"; m_melodic[0].w2 = "squarew(t)"; m_melodic[0].o1 = "(W1(integrate(f)) + W2(integrate(f*1.005)))*0.5"; m_melodic[0].decay = 0.4; m_melodic[0].sustain = 0.1;
    m_melodic[1].name = "Synth"; m_melodic[1].w1 = "sinew(t)"; m_melodic[1].w2 = "saww(t)"; m_melodic[1].o1 = "(W1(integrate(f)) + W2(integrate(f*2.0)))*0.5"; m_melodic[1].decay = 0.8; m_melodic[1].sustain = 0.6;
    m_melodic[2].name = "Pad"; m_melodic[2].w1 = "saww(t)"; m_melodic[2].w2 = "saww(t)"; m_melodic[2].o1 = "(W1(integrate(f*0.99)) + W2(integrate(f*1.01)))*0.5"; m_melodic[2].attack = 0.5; m_melodic[2].decay = 1.0; m_melodic[2].sustain = 0.8; m_melodic[2].release = 1.0;

    setupUI();
}

MainWindow::~MainWindow() {
    m_synthEngine->stop();
}

void MainWindow::setupUI() {

    QScrollArea* mainScroll = new QScrollArea(this);
    mainScroll->setWidgetResizable(true);
    mainScroll->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");
    setCentralWidget(mainScroll);

    QWidget* centralWidget = new QWidget();
    mainScroll->setWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);


    centralWidget->setStyleSheet(R"(
        QWidget { background-color: #1e1e1e; color: #e0e0e0; font-family: "Segoe UI", sans-serif; font-size: 11px; }
        QGroupBox { border: 1px solid #444; border-radius: 4px; font-weight: bold; margin-top: 1ex; padding: 2px; }
        QGroupBox::title { subcontrol-origin: margin; left: 5px; padding: 0 3px; color: #00ffff; }
        QCheckBox::indicator { width: 14px; height: 35px; border: 1px solid #555; background: #2b2b2b; border-radius: 2px; }
        QCheckBox::indicator:checked { background: #00ffff; border: 1px solid #ffffff; }
        QPushButton { background-color: #333; border: 1px solid #555; padding: 6px 2px; color: #fff; border-radius: 3px; font-weight: bold; }
        QPushButton:hover, QPushButton:pressed { background-color: #444; border: 1px solid #00ffff; }
        QSpinBox { background-color: #333; color: white; border: 1px solid #555; padding: 2px; min-width: 45px; min-height: 25px; }
    )");


    QHBoxLayout* topLayout = new QHBoxLayout();
    m_spinBpm = new QSpinBox();
    m_spinBpm->setRange(60, 300);
    m_spinBpm->setValue(120);
    topLayout->addWidget(new QLabel("BPM:"));
    topLayout->addWidget(m_spinBpm);

    m_btnPlay = new QPushButton("▶ Play");
    m_btnPlay->setStyleSheet("background-color: #005577; color: white;");


    m_btnExport = new QPushButton("Export .mmp");
    m_btnExport->setStyleSheet("background-color: #005533; color: white;");

    topLayout->addWidget(m_btnPlay);
    topLayout->addWidget(m_btnExport);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);


    QGroupBox* drumBox = new QGroupBox("Percussion");
    QVBoxLayout* drumBoxLayout = new QVBoxLayout(drumBox);
    drumBoxLayout->setContentsMargins(2, 2, 2, 2);

    QScrollArea* drumScrollArea = new QScrollArea();
    drumScrollArea->setWidgetResizable(true);
    drumScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    drumScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget* gridContainer = new QWidget();
    QGridLayout* gridLayout = new QGridLayout(gridContainer);
    gridLayout->setSpacing(1);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    for (int r = 0; r < NUM_CHANNELS; ++r) {
        QPushButton* btnCfg = new QPushButton("⚙️" + m_channels[r].name);
        btnCfg->setFixedWidth(50);
        connect(btnCfg, &QPushButton::clicked, this, [this, r]() { onConfigClicked(r); });
        gridLayout->addWidget(btnCfg, r, 0);

        for (int c = 0; c < NUM_STEPS; ++c) {
            m_grid[r][c] = new QCheckBox();
            gridLayout->addWidget(m_grid[r][c], r, c + 1);

            if ((c + 1) % 4 == 0 && c != 15) {
                QFrame* line = new QFrame();
                line->setFrameShape(QFrame::VLine);
                line->setStyleSheet("border-left: 1px solid #444; margin-left: 0px; margin-right: 0px;");
                gridLayout->addWidget(line, r, c + 2);
            }
        }
    }
    drumScrollArea->setWidget(gridContainer);
    drumBoxLayout->addWidget(drumScrollArea);
    mainLayout->addWidget(drumBox);


    QGroupBox* melodicBox = new QGroupBox("Melodic Synths");
    melodicBox->setStyleSheet("QGroupBox::title { color: #cc55ff; }");
    QVBoxLayout* melodicLayout = new QVBoxLayout(melodicBox);
    melodicLayout->setContentsMargins(2, 2, 2, 2);
    melodicLayout->setSpacing(2);

    for (int m = 0; m < NUM_MELODIC; ++m) {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(2);

        QLabel* lblName = new QLabel("<b>" + m_melodic[m].name + "</b>");
        lblName->setFixedWidth(45);

        QPushButton* btnCfg = new QPushButton("⚙️"); // Just icon to save space
        btnCfg->setFixedWidth(30);
        connect(btnCfg, &QPushButton::clicked, this, [this, m]() { onMelodicConfigClicked(m); });

        QPushButton* btnRoll = new QPushButton("🎹 Roll"); // Short text
        btnRoll->setStyleSheet("background-color: #4a0077; font-weight: bold;");
        connect(btnRoll, &QPushButton::clicked, this, [this, m]() { onPianoRollClicked(m); });

        row->addWidget(lblName);
        row->addWidget(btnCfg);
        row->addWidget(btnRoll);
        melodicLayout->addLayout(row);
    }
    mainLayout->addWidget(melodicBox);
    mainLayout->addStretch();


    m_txtPreview = new QTextEdit();
    m_txtPreview->hide();
    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::onPlayClicked);
    connect(m_btnExport, &QPushButton::clicked, this, &MainWindow::onExportMmpClicked);
}

void MainWindow::onConfigClicked(int index) {
    ChannelConfigDialog dlg(m_channels[index], this);
    if (dlg.exec() == QDialog::Accepted) dlg.saveToConfig(m_channels[index]);
}

void MainWindow::onMelodicConfigClicked(int index) {
    MelodicConfigDialog dlg(m_melodic[index], this);
    if (dlg.exec() == QDialog::Accepted) dlg.saveToConfig(m_melodic[index]);
}

void MainWindow::onPianoRollClicked(int index) {
    PianoRollDialog dlg(m_melodic[index], this);
    if (dlg.exec() == QDialog::Accepted) dlg.saveToConfig(m_melodic[index]);
}

QString MainWindow::getAdsrString(double a, double d, double s, double r) {
    if (a == 0 && d == 0 && r == 0) return "1.0";
    QString attackStr = (a > 0) ? QString("(lt / %1)").arg(a) : "1.0";
    QString decayStr = (d > 0) ? QString("(1.0 - (lt - %1) * ((1.0 - %2) / %3))").arg(a).arg(s).arg(d) : QString::number(s);
    return QString("((lt < %1) ? %2 : ((lt < %3) ? %4 : %5))").arg(a).arg(attackStr).arg(a + d).arg(decayStr).arg(s);
}

QString MainWindow::generateMathExpression() {
    double bps = m_spinBpm->value() / 60.0;
    double stepsPerSec = bps * 4.0;

    QString finalMath = QString("var stepRate = %1;\nvar globalStep = floor(t * stepRate);\nvar seqStep = mod(globalStep, 16);\nvar lt = mod(t, 1.0 / stepRate);\n\n").arg(stepsPerSec);
    QString mix = "0";


    for (int r = 0; r < NUM_CHANNELS; ++r) {
        QString stepGate = ""; bool hasSteps = false;
        for (int c = 0; c < NUM_STEPS; ++c) {
            if (m_grid[r][c]->isChecked()) {
                if (hasSteps) stepGate += " + ";
                stepGate += QString("(seqStep == %1)").arg(c);
                hasSteps = true;
            }
        }
        if (hasSteps) {
            ChannelConfig& ch = m_channels[r];
            QString chW1 = ch.w1; chW1.replace("t", "lt");
            QString chO1 = ch.o1; chO1.replace("W1", "(" + chW1 + ")"); chO1.replace("t", "lt");
            QString adsr = getAdsrString(ch.attack, ch.decay, ch.sustain, ch.release);
            finalMath += QString("// Drum %1 (%2)\nvar gate_d%1 = (%3);\nvar out_d%1 = (%4) * %5 * %6 * gate_d%1;\n\n")
                             .arg(r).arg(ch.name).arg(stepGate).arg(chO1).arg(adsr).arg(ch.vol);
            mix += QString(" + out_d%1").arg(r);
        }
    }


    for (int m = 0; m < NUM_MELODIC; ++m) {
        MelodicConfig& ch = m_melodic[m];
        bool trackUsed = false;

        for (int key = 0; key < 96; ++key) {
            QString stepGate = ""; bool hasSteps = false;
            for (int c = 0; c < NUM_STEPS; ++c) {
                if (ch.grid[key][c]) {
                    if (hasSteps) stepGate += " + ";
                    stepGate += QString("(seqStep == %1)").arg(c);
                    hasSteps = true;
                }
            }

            if (hasSteps) {
                if(!trackUsed) { finalMath += QString("// Synth %1 (%2)\n").arg(m).arg(ch.name); trackUsed = true; }

                double freq = 440.0 * std::pow(2.0, ((107 - key) - 69) / 12.0);

                QString chW1 = ch.w1; chW1.replace("t", "lt");
                QString chW2 = ch.w2; chW2.replace("t", "lt");
                QString chO1 = ch.o1;
                chO1.replace("W1", "(" + chW1 + ")");
                chO1.replace("W2", "(" + chW2 + ")");
                chO1.replace("f", QString::number(freq, 'f', 2));
                chO1.replace("t", "lt");

                QString adsr = getAdsrString(ch.attack, ch.decay, ch.sustain, ch.release);

                finalMath += QString("var gate_m%1_k%2 = (%3);\n").arg(m).arg(key).arg(stepGate);
                finalMath += QString("var out_m%1_k%2 = (%4) * %5 * %6 * gate_m%1_k%2;\n").arg(m).arg(key).arg(chO1).arg(adsr).arg(ch.vol);
                mix += QString(" + out_m%1_k%2").arg(m).arg(key);
            }
        }
        if(trackUsed) finalMath += "\n";
    }

    finalMath += QString("clamp(-1.0, %1, 1.0)\n").arg(mix);
    return finalMath;
}

void MainWindow::onPlayClicked() {
    if (m_btnPlay->text().contains("Play")) {
        m_txtPreview->setText(generateMathExpression());


        struct LiveDrum { bool steps[16]={false}; double a,d,s,vol; int wType; double baseF; bool isK; };
        struct LiveMel { bool grid[96][16]={{false}}; double a,d,s,vol; int w1Type; };

        std::vector<LiveDrum> drums(NUM_CHANNELS);
        std::vector<LiveMel> synths(NUM_MELODIC);

        for (int r=0; r<NUM_CHANNELS; ++r) {
            drums[r].a = m_channels[r].attack; drums[r].d = m_channels[r].decay; drums[r].s = m_channels[r].sustain; drums[r].vol = m_channels[r].vol;
            for (int c=0; c<16; ++c) drums[r].steps[c] = m_grid[r][c]->isChecked();
            QString w1 = m_channels[r].w1.toLower();
            drums[r].wType = w1.contains("sin") ? 0 : w1.contains("rand") ? 1 : w1.contains("saw") ? 2 : 3;
            drums[r].baseF = (r==0)?55.0 : (r==1)?200.0 : (r==2||r==3)?8000.0 : (r==4)?440.0 : 110.0;
            drums[r].isK = (r==0);
        }
        for (int m=0; m<NUM_MELODIC; ++m) {
            synths[m].a = m_melodic[m].attack; synths[m].d = m_melodic[m].decay; synths[m].s = m_melodic[m].sustain; synths[m].vol = m_melodic[m].vol;
            for(int r=0; r<96; ++r) for (int c=0; c<16; ++c) synths[m].grid[r][c] = m_melodic[m].grid[r][c];
            QString w1 = m_melodic[m].w1.toLower();
            synths[m].w1Type = w1.contains("sin") ? 0 : w1.contains("saw") ? 2 : 3;
        }

        double stepRate = (m_spinBpm->value() / 60.0) * 4.0;

        m_synthEngine->setAudioSource([drums, synths, stepRate](double t) -> double {
            int seqStep = (long)std::floor(t * stepRate) % 16;
            double lt = std::fmod(t, 1.0 / stepRate);
            double mix = 0.0;


            for(int r=0; r<6; ++r) {
                if (drums[r].steps[seqStep]) {
                    const auto& trk = drums[r];
                    double env = trk.s;
                    if (lt < trk.a && trk.a > 0.001) env = lt / trk.a;
                    else if (lt < trk.a + trk.d && trk.d > 0.001) env = 1.0 - ((lt - trk.a) / trk.d) * (1.0 - trk.s);
                    else if (trk.a == 0 && trk.d == 0) env = 1.0;

                    double osc = 0.0, phase = t * trk.baseF;
                    if (trk.isK) phase *= std::exp(-lt * 15.0);
                    if (trk.wType == 0) osc = std::sin(2.0 * M_PI * phase);
                    else if (trk.wType == 1) osc = ((rand() % 2000) / 1000.0) - 1.0;
                    else if (trk.wType == 2) osc = 2.0 * std::fmod(phase, 1.0) - 1.0;
                    else osc = (std::fmod(phase, 1.0) < 0.5) ? 1.0 : -1.0;
                    mix += osc * env * trk.vol;
                }
            }


            for(int m=0; m<3; ++m) {
                for(int key=0; key<96; ++key) {
                    if (synths[m].grid[key][seqStep]) {
                        const auto& trk = synths[m];
                        double env = trk.s;
                        if (lt < trk.a && trk.a > 0.001) env = lt / trk.a;
                        else if (lt < trk.a + trk.d && trk.d > 0.001) env = 1.0 - ((lt - trk.a) / trk.d) * (1.0 - trk.s);
                        else if (trk.a == 0 && trk.d == 0) env = 1.0;

                        double freq = 440.0 * std::pow(2.0, ((107 - key) - 69) / 12.0);
                        double osc = 0.0, phase = t * freq;
                        if (trk.w1Type == 0) osc = std::sin(2.0 * M_PI * phase);
                        else if (trk.w1Type == 2) osc = 2.0 * std::fmod(phase, 1.0) - 1.0;
                        else osc = (std::fmod(phase, 1.0) < 0.5) ? 1.0 : -1.0;
                        mix += osc * env * trk.vol;
                    }
                }
            }
            return std::max(-1.0, std::min(mix, 1.0));
        });

        m_synthEngine->start();
        m_btnPlay->setText("⏹ Stop");
        m_btnPlay->setStyleSheet("background-color: #880000; color: white;");
    } else {
        m_synthEngine->stop();
        m_btnPlay->setText("▶ Play");
        m_btnPlay->setStyleSheet("background-color: #005577; color: white;");
    }
}

void MainWindow::onExportMmpClicked() {
    QString filePath = QFileDialog::getSaveFileName(this, "Save LMMS Project", "", "LMMS Project (*.mmp)");
    if (filePath.isEmpty()) return;

    QDomDocument doc("lmms-project");
    QDomElement root = doc.createElement("lmms-project");
    root.setAttribute("version", "1.2.2");
    doc.appendChild(root);
    QDomElement song = doc.createElement("song");
    song.setAttribute("bpm", QString::number(m_spinBpm->value()));
    root.appendChild(song);

    const int ticksPerStep = 48;


    for (int r = 0; r < NUM_CHANNELS; ++r) {
        bool hasNotes = false;
        for (int c = 0; c < NUM_STEPS; ++c) { if (m_grid[r][c]->isChecked()) hasNotes = true; }
        if (!hasNotes) continue;

        ChannelConfig& ch = m_channels[r];
        QDomElement track = doc.createElement("track"); track.setAttribute("name", ch.name); track.setAttribute("type", "0"); song.appendChild(track);
        QDomElement instTrack = doc.createElement("instrumenttrack"); track.appendChild(instTrack);
        QDomElement instrument = doc.createElement("instrument"); instrument.setAttribute("name", "Xpressive");

        QDomElement xpressive = doc.createElement("Xpressive");
        xpressive.setAttribute("w1", ch.w1); xpressive.setAttribute("o1", ch.o1);
        xpressive.setAttribute("env_atk", QString::number(ch.attack)); xpressive.setAttribute("env_dec", QString::number(ch.decay));
        xpressive.setAttribute("env_sus", QString::number(ch.sustain)); xpressive.setAttribute("env_rel", QString::number(ch.release));

        instrument.appendChild(xpressive); instTrack.appendChild(instrument);
        QDomElement pattern = doc.createElement("pattern"); pattern.setAttribute("type", "1"); pattern.setAttribute("pos", "0"); pattern.setAttribute("steps", "16"); pattern.setAttribute("len", QString::number(16 * ticksPerStep)); track.appendChild(pattern);

        for (int c = 0; c < NUM_STEPS; ++c) {
            if (m_grid[r][c]->isChecked()) {
                QDomElement note = doc.createElement("note"); note.setAttribute("pos", QString::number(c * ticksPerStep)); note.setAttribute("len", QString::number(ticksPerStep)); note.setAttribute("key", "60"); note.setAttribute("vol", QString::number(int(ch.vol * 100))); pattern.appendChild(note);
            }
        }
    }


    for (int m = 0; m < NUM_MELODIC; ++m) {
        bool hasNotes = false;
        for (int key = 0; key < 96; ++key) { for (int c = 0; c < 16; ++c) { if (m_melodic[m].grid[key][c]) hasNotes = true; } }
        if (!hasNotes) continue;

        MelodicConfig& ch = m_melodic[m];
        QDomElement track = doc.createElement("track"); track.setAttribute("name", ch.name); track.setAttribute("type", "0"); song.appendChild(track);
        QDomElement instTrack = doc.createElement("instrumenttrack"); track.appendChild(instTrack);
        QDomElement instrument = doc.createElement("instrument"); instrument.setAttribute("name", "Xpressive");

        QDomElement xpressive = doc.createElement("Xpressive");
        xpressive.setAttribute("w1", ch.w1); xpressive.setAttribute("w2", ch.w2); xpressive.setAttribute("o1", ch.o1);
        xpressive.setAttribute("env_atk", QString::number(ch.attack)); xpressive.setAttribute("env_dec", QString::number(ch.decay));
        xpressive.setAttribute("env_sus", QString::number(ch.sustain)); xpressive.setAttribute("env_rel", QString::number(ch.release));

        instrument.appendChild(xpressive); instTrack.appendChild(instrument);
        QDomElement pattern = doc.createElement("pattern"); pattern.setAttribute("type", "1"); pattern.setAttribute("pos", "0"); pattern.setAttribute("steps", "16"); pattern.setAttribute("len", QString::number(16 * ticksPerStep)); track.appendChild(pattern);

        for (int key = 0; key < 96; ++key) {
            for (int c = 0; c < 16; ++c) {
                if (ch.grid[key][c]) {
                    QDomElement note = doc.createElement("note");
                    note.setAttribute("pos", QString::number(c * ticksPerStep)); note.setAttribute("len", QString::number(ticksPerStep));
                    note.setAttribute("key", QString::number(107 - key)); // Match exact LMMS Midi mapping
                    note.setAttribute("vol", QString::number(int(ch.vol * 100)));
                    pattern.appendChild(note);
                }
            }
        }
    }

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file); out << doc.toString(4); file.close();
        QMessageBox::information(this, "Export Complete", "Groove sequence + Melodies saved to:\n" + filePath);
    }
}