#include "mainwindow.h"
#include "synthengine.h"
#include "xpressive_parser.h"
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
#include <QMutexLocker>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_synthEngine = new SynthEngine(this);

    setWindowTitle("Xpressive Groovebox (Mobile)");
    resize(850, 500);

    m_channels[0].name = "Kick";
    m_channels[0].w1 = "sinew(t)";
    m_channels[0].o1 = "clamp(-1.0, sinew(40 * t) * exp(-t * 60) + sinew(40 * t) * exp(-t * 3) * 0.3, 1.0)";
    m_channels[0].attack = 0.0; m_channels[0].decay = 0.44; m_channels[0].sustain = 0.328; m_channels[0].release = 0.856; m_channels[0].vol = 1.0;

    m_channels[1].name = "Snare";
    m_channels[1].w1 = "sinew(t)";
    m_channels[1].o1 = "W1(integrate(f * (180.0 / 440.0))) * exp(-t * 6.0) + randv(t * srate) * exp(-t * 12.0)";
    m_channels[1].attack = 0.0; m_channels[1].decay = 0.5; m_channels[1].sustain = 0.5; m_channels[1].release = 0.1; m_channels[1].vol = 0.8;

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
        QSpinBox, QComboBox, QLineEdit { background-color: #333; color: white; border: 1px solid #555; padding: 2px; min-height: 25px; }
    )");

    QHBoxLayout* topLayout = new QHBoxLayout();
    m_spinBpm = new QSpinBox();
    m_spinBpm->setRange(60, 300);
    m_spinBpm->setValue(120);
    connect(m_spinBpm, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updatePlaybackState);

    topLayout->addWidget(new QLabel("BPM:"));
    topLayout->addWidget(m_spinBpm);

    m_btnPlay = new QPushButton("▶ Play Sequence");
    m_btnPlay->setStyleSheet("background-color: #005577; color: white; padding: 5px 15px;");

    m_txtTestString = new QLineEdit();
    m_txtTestString->setPlaceholderText("Paste Custom Formula Here...");

    m_btnTestString = new QPushButton("🧪 Test Expr");
    m_btnTestString->setStyleSheet("background-color: #5500aa; color: white; padding: 5px 15px;");

    m_btnImport = new QPushButton("📂 Import");
    m_btnImport->setStyleSheet("background-color: #775500; color: white;");

    m_btnExport = new QPushButton("💾 Export");
    m_btnExport->setStyleSheet("background-color: #005533; color: white;");

    topLayout->addWidget(m_btnPlay);
    topLayout->addWidget(m_txtTestString);
    topLayout->addWidget(m_btnTestString);
    topLayout->addWidget(m_btnImport);
    topLayout->addWidget(m_btnExport);
    mainLayout->addLayout(topLayout);

    QHBoxLayout* middleLayout = new QHBoxLayout();

    QGroupBox* drumBox = new QGroupBox("Percussion (Global)");
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
    gridLayout->setAlignment(Qt::AlignLeft);

    for (int r = 0; r < NUM_CHANNELS; ++r) {
        QPushButton* btnCfg = new QPushButton("⚙️" + m_channels[r].name);
        btnCfg->setFixedWidth(50);
        connect(btnCfg, &QPushButton::clicked, this, [this, r]() { onConfigClicked(r); });
        gridLayout->addWidget(btnCfg, r, 0);

        for (int c = 0; c < NUM_STEPS; ++c) {
            m_grid[r][c] = new QCheckBox();
            connect(m_grid[r][c], &QCheckBox::clicked, this, &MainWindow::updatePlaybackState);
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
    middleLayout->addWidget(drumBox);

    QGroupBox* arrangerBox = new QGroupBox("Song Arranger (Click to Cycle Patterns)");
    arrangerBox->setStyleSheet("QGroupBox::title { color: #ffff00; }");
    QGridLayout* arrangerLayout = new QGridLayout(arrangerBox);
    arrangerLayout->setSpacing(4);
    arrangerLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QStringList trackNames = {"Drums", "Bass", "Synth", "Pad"};
    for (int row = 0; row < 4; ++row) {
        QLabel* trackLabel = new QLabel(trackNames[row]);
        trackLabel->setFixedWidth(40);
        arrangerLayout->addWidget(trackLabel, row, 0);

        for (int bar = 0; bar < 8; ++bar) {
            m_arrangerGrid[row][bar] = new QPushButton("");
            m_arrangerGrid[row][bar]->setFixedSize(25, 25);
            m_arrangerGrid[row][bar]->setStyleSheet("QPushButton { background-color: #333; border: 1px solid #555; border-radius: 3px; }");

            connect(m_arrangerGrid[row][bar], &QPushButton::clicked, this, [this, row, bar]() {
                onArrangerClicked(row, bar);
            });
            arrangerLayout->addWidget(m_arrangerGrid[row][bar], row, bar + 1);
        }
    }
    onArrangerClicked(0, 0); onArrangerClicked(1, 0); onArrangerClicked(2, 0); onArrangerClicked(3, 0);

    middleLayout->addWidget(arrangerBox);
    mainLayout->addLayout(middleLayout);

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

        QPushButton* btnCfg = new QPushButton("⚙️");
        btnCfg->setFixedWidth(30);
        connect(btnCfg, &QPushButton::clicked, this, [this, m]() { onMelodicConfigClicked(m); });

        m_comboEditBar[m] = new QComboBox();
        m_comboEditBar[m]->setFixedWidth(85);
        for(int b=1; b<=8; ++b) m_comboEditBar[m]->addItem(QString("Pattern %1").arg(b));

        QPushButton* btnRoll = new QPushButton("🎹 Roll");
        btnRoll->setStyleSheet("background-color: #4a0077; font-weight: bold;");
        connect(btnRoll, &QPushButton::clicked, this, [this, m]() { onPianoRollClicked(m); });

        row->addWidget(lblName);
        row->addWidget(btnCfg);
        row->addWidget(m_comboEditBar[m]);
        row->addWidget(btnRoll);
        melodicLayout->addLayout(row);
    }
    mainLayout->addWidget(melodicBox);
    mainLayout->addStretch();

    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::onPlayClicked);
    connect(m_btnTestString, &QPushButton::clicked, this, &MainWindow::onTestStringClicked);
    connect(m_btnExport, &QPushButton::clicked, this, &MainWindow::onExportMmpClicked);
    connect(m_btnImport, &QPushButton::clicked, this, &MainWindow::onImportMmpClicked);

    updatePlaybackState();
}

void MainWindow::onArrangerClicked(int row, int bar) {
    if (row == 0) {
        m_arrangerState[row][bar] = (m_arrangerState[row][bar] == 0) ? 1 : 0;
    } else {
        m_arrangerState[row][bar] = (m_arrangerState[row][bar] + 1) % 9;
    }

    QPushButton* btn = m_arrangerGrid[row][bar];
    if (m_arrangerState[row][bar] == 0) {
        btn->setText("");
        btn->setStyleSheet("QPushButton { background-color: #333; border: 1px solid #555; border-radius: 3px; }");
    } else {
        if (row == 0) btn->setText("D");
        else btn->setText(QString::number(m_arrangerState[row][bar]));
        btn->setStyleSheet("QPushButton { background-color: #ffff00; color: black; font-weight: bold; border: 1px solid #fff; border-radius: 3px; }");
    }
    updatePlaybackState();
}

void MainWindow::updatePlaybackState() {
    QMutexLocker locker(&m_playState.mutex);
    m_playState.bpm = m_spinBpm->value();

    for (int r = 0; r < NUM_CHANNELS; ++r) {
        m_playState.drums[r].a = m_channels[r].attack;
        m_playState.drums[r].d = m_channels[r].decay;
        m_playState.drums[r].s = m_channels[r].sustain;
        m_playState.drums[r].vol = m_channels[r].vol;
        m_playState.drums[r].baseF = (r==0)?55.0 : (r==1)?200.0 : (r==2||r==3)?8000.0 : (r==4)?440.0 : 110.0;
        m_playState.drums[r].isK = (r==0);
        QString w1 = m_channels[r].w1.toLower();
        m_playState.drums[r].wType = w1.contains("sin") ? 0 : w1.contains("rand") ? 1 : w1.contains("saw") ? 2 : 3;

        for (int c = 0; c < NUM_STEPS; ++c) {
            m_playState.drumSteps[r][c] = m_grid[r][c]->isChecked();
        }
    }

    for (int m = 0; m < NUM_MELODIC; ++m) {
        m_playState.synths[m].a = m_melodic[m].attack;
        m_playState.synths[m].d = m_melodic[m].decay;
        m_playState.synths[m].s = m_melodic[m].sustain;
        m_playState.synths[m].vol = m_melodic[m].vol;
        m_playState.synths[m].useAdsr = m_melodic[m].useAdsr;
        QString w1 = m_melodic[m].w1.toLower();
        m_playState.synths[m].w1Type = w1.contains("sin") ? 0 : w1.contains("saw") ? 2 : 3;

        for (int b = 0; b < 8; ++b) {
            for (int key = 0; key < 96; ++key) {
                for (int c = 0; c < NUM_STEPS; ++c) {
                    m_playState.synthGrid[m][b][key][c] = m_melodic[m].grid[b][key][c];
                }
            }
        }
    }

    for (int t = 0; t < 4; ++t) {
        for (int b = 0; b < 8; ++b) {
            m_playState.arranger[t][b] = m_arrangerState[t][b];
        }
    }
}

void MainWindow::onConfigClicked(int index) {
    ChannelConfigDialog dlg(m_channels[index], this);
    if (dlg.exec() == QDialog::Accepted) { dlg.saveToConfig(m_channels[index]); updatePlaybackState(); }
}

void MainWindow::onMelodicConfigClicked(int index) {
    MelodicConfigDialog dlg(m_melodic[index], this);
    if (dlg.exec() == QDialog::Accepted) { dlg.saveToConfig(m_melodic[index]); updatePlaybackState(); }
}

void MainWindow::onPianoRollClicked(int index) {
    int pat = m_comboEditBar[index]->currentIndex();
    PianoRollDialog dlg(m_melodic[index].grid[pat], m_melodic[index].name + " - Pattern " + QString::number(pat+1), this);
    if (dlg.exec() == QDialog::Accepted) { dlg.saveToGrid(m_melodic[index].grid[pat]); updatePlaybackState(); }
}

QString MainWindow::getAdsrString(double a, double d, double s, double r) {
    if (a == 0 && d == 0 && r == 0) return "1.0";
    QString attackStr = (a > 0) ? QString("(lt / %1)").arg(a) : "1.0";
    QString decayStr = (d > 0) ? QString("(1.0 - (lt - %1) * ((1.0 - %2) / %3))").arg(a).arg(s).arg(d) : QString::number(s);
    return QString("((lt < %1) ? %2 : ((lt < %3) ? %4 : %5))").arg(a).arg(attackStr).arg(a + d).arg(decayStr).arg(s);
}

void MainWindow::onPlayClicked() {
    if (m_btnPlay->text().contains("Play")) {
        if (m_btnTestString->text().contains("Stop")) onTestStringClicked();

        std::vector<double> noteFreqs(96);
        for(int key=0; key<96; ++key) noteFreqs[key] = 440.0 * std::pow(2.0, ((107 - key) - 69) / 12.0);

        m_synthEngine->setAudioSource([this, noteFreqs](double t) mutable -> double {
            QMutexLocker locker(&m_playState.mutex);
            double stepRate = (m_playState.bpm / 60.0) * 4.0;
            long globalStep = (long)std::floor(t * stepRate);
            int seqStep = globalStep % 16;
            int currentBar = (globalStep / 16) % 8;
            double lt = std::fmod(t, 1.0 / stepRate);
            double mix = 0.0;

            static uint32_t rngSeed = 123456789;

            if (m_playState.arranger[0][currentBar] == 1) {
                for(int r=0; r<6; ++r) {
                    if (m_playState.drumSteps[r][seqStep]) {
                        const auto& trk = m_playState.drums[r];
                        double env = trk.s;
                        if (lt < trk.a && trk.a > 0.001) env = lt / trk.a;
                        else if (lt < trk.a + trk.d && trk.d > 0.001) env = 1.0 - ((lt - trk.a) / trk.d) * (1.0 - trk.s);
                        else if (trk.a == 0 && trk.d == 0) env = 1.0;

                        double osc = 0.0;
                        if (trk.isK) {
                            double osc1 = std::sin(2.0 * M_PI * t * 40.0) * std::exp(-lt * 60.0);
                            double osc2 = std::sin(2.0 * M_PI * t * 40.0) * std::exp(-lt * 3.0) * 0.3;
                            osc = std::max(-1.0, std::min(1.0, osc1 + osc2));
                        }
                        else if (r == 1) {
                            double oscTone = std::sin(2.0 * M_PI * t * trk.baseF * (180.0/440.0)) * std::exp(-lt * 6.0);
                            rngSeed = rngSeed * 1664525 + 1013904223;
                            double noise = ((static_cast<float>(rngSeed) / 4294967296.0f) * 2.0f - 1.0f) * std::exp(-lt * 12.0);
                            osc = oscTone + noise;
                        }
                        else {
                            double phase = t * trk.baseF;
                            if (trk.wType == 0) osc = std::sin(2.0 * M_PI * phase);
                            else if (trk.wType == 1) { rngSeed = rngSeed * 1664525 + 1013904223; osc = (static_cast<float>(rngSeed) / 4294967296.0f) * 2.0f - 1.0f; }
                            else if (trk.wType == 2) osc = 2.0 * std::fmod(phase, 1.0) - 1.0;
                            else osc = (std::fmod(phase, 1.0) < 0.5) ? 1.0 : -1.0;
                        }
                        mix += osc * env * trk.vol;
                    }
                }
            }

            for(int m=0; m<3; ++m) {
                int patIdx = m_playState.arranger[m+1][currentBar];
                if (patIdx > 0) {
                    int p = patIdx - 1;
                    for(int key=0; key<96; ++key) {
                        if (m_playState.synthGrid[m][p][key][seqStep].active) {
                            const auto& trk = m_playState.synths[m];
                            
                            double env = 1.0;
                            if (trk.useAdsr) {
                                env = trk.s;
                                if (lt < trk.a && trk.a > 0.001) env = lt / trk.a;
                                else if (lt < trk.a + trk.d && trk.d > 0.001) env = 1.0 - ((lt - trk.a) / trk.d) * (1.0 - trk.s);
                                else if (trk.a == 0 && trk.d == 0) env = 1.0;
                            }

                            double freq = noteFreqs[key];
                            double osc = 0.0, phase = t * freq;

                            if (trk.w1Type == 0) osc = std::sin(2.0 * M_PI * phase);
                            else if (trk.w1Type == 2) osc = 2.0 * std::fmod(phase, 1.0) - 1.0;
                            else osc = (std::fmod(phase, 1.0) < 0.5) ? 1.0 : -1.0;

                            mix += osc * env * trk.vol;
                        }
                    }
                }
            }
            double finalMix = std::max(-1.0, std::min(mix, 1.0));
            return m_masterEffect.process(finalMix);
        });

        m_synthEngine->start();
        m_btnPlay->setText("⏹ Stop Sequence");
        m_btnPlay->setStyleSheet("background-color: #880000; color: white; padding: 5px 15px;");
    } else {
        m_synthEngine->stop();
        m_btnPlay->setText("▶ Play Sequence");
        m_btnPlay->setStyleSheet("background-color: #005577; color: white; padding: 5px 15px;");
    }
}

void MainWindow::onTestStringClicked() {
    if (m_btnTestString->text().contains("Stop")) {
        m_synthEngine->stop();
        m_btnTestString->setText("🧪 Test Expr");
        m_btnPlay->setEnabled(true);
        return;
    }

    QString code = m_txtTestString->text();
    try {
        auto ast = XpressiveParser::parse(code.toStdString());
        std::shared_ptr<XpressiveParser::Node> sharedAst(ast.release());

        double testBpm = m_spinBpm->value();

        m_synthEngine->setAudioSource([sharedAst, testBpm](double t) mutable -> double {
            XpressiveParser::Env env;
            env.t = t;
            env.tempo = testBpm;
            env.srate = 44100.0;
            env.f = 440.0;

            double val = sharedAst->eval(env);
            return std::max(-1.0, std::min(val, 1.0));
        });

        m_synthEngine->start();
        m_btnTestString->setText("⏹ Stop Test");
        if (m_btnPlay->text().contains("Stop")) onPlayClicked();
        m_btnPlay->setEnabled(false);

    } catch (std::exception& e) {
        QMessageBox::warning(this, "Parse Error", "Check your syntax: " + QString(e.what()));
    }
}

void MainWindow::onImportMmpClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open LMMS Project", "", "LMMS Project (*.mmp)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QDomDocument doc;
    if (!doc.setContent(&file)) { QMessageBox::warning(this, "Parse Error", "Failed to parse .mmp file format."); return; }

    QDomElement root = doc.documentElement();
    QDomElement head = root.firstChildElement("head");
    m_spinBpm->setValue(head.attribute("bpm", "120").toInt());

    // Clear state
    for (int r = 0; r < NUM_CHANNELS; ++r) { for (int c = 0; c < NUM_STEPS; ++c) m_grid[r][c]->setChecked(false); }
    for (int t = 0; t < 4; ++t) {
        for (int b = 0; b < 8; ++b) {
            m_arrangerState[t][b] = 0;
            if (t > 0) {
                for (int k = 0; k < 96; ++k) { for (int c = 0; c < NUM_STEPS; ++c) m_melodic[t-1].grid[b][k][c].active = false; }
            }
        }
    }

    QDomElement song = root.firstChildElement("song");
    QDomElement trackContainer = song.firstChildElement("trackcontainer");

    QDomNode n = trackContainer.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull() && e.tagName() == "track") {
            if (e.attribute("type") == "1") {
                QDomNodeList bbtcos = e.elementsByTagName("bbtco");
                for (int i=0; i<bbtcos.size(); ++i) {
                    int bar = bbtcos.at(i).toElement().attribute("pos").toInt() / 192;
                    if (bar >= 0 && bar < 8) m_arrangerState[0][bar] = 1;
                }

                QDomElement bbTc = e.firstChildElement("bbtrack").firstChildElement("trackcontainer");
                QDomNodeList bbTracks = bbTc.elementsByTagName("track");
                for (int i=0; i<bbTracks.size(); ++i) {
                    QDomElement t = bbTracks.at(i).toElement();
                    QString name = t.attribute("name");
                    int dIdx = -1;
                    for(int r=0; r<NUM_CHANNELS; ++r) if(m_channels[r].name == name) dIdx = r;

                    if (dIdx != -1) {
                        QDomElement instTrack = t.firstChildElement("instrumenttrack");
                        QDomElement xpressive = instTrack.firstChildElement("instrument").firstChildElement("xpressive");
                        if (xpressive.isNull()) xpressive = instTrack.firstChildElement("instrument").firstChildElement("Xpressive");
                        if (!xpressive.isNull()) {
                            m_channels[dIdx].w1 = xpressive.hasAttribute("W1") ? xpressive.attribute("W1") : xpressive.attribute("w1");
                            m_channels[dIdx].o1 = xpressive.hasAttribute("O1") ? xpressive.attribute("O1") : xpressive.attribute("o1");
                            m_channels[dIdx].attack = xpressive.attribute("env_atk").toDouble();
                            m_channels[dIdx].decay = xpressive.attribute("env_dec").toDouble();
                            m_channels[dIdx].sustain = xpressive.attribute("env_sus").toDouble();
                            m_channels[dIdx].release = xpressive.attribute("env_rel").toDouble();
                        }
                        QDomElement elvol = instTrack.firstChildElement("eldata").firstChildElement("elvol");
                        if (!elvol.isNull()) {
                            m_channels[dIdx].attack = elvol.attribute("att", QString::number(m_channels[dIdx].attack)).toDouble();
                            m_channels[dIdx].decay = elvol.attribute("dec", QString::number(m_channels[dIdx].decay)).toDouble();
                            m_channels[dIdx].sustain = elvol.attribute("sustain", QString::number(m_channels[dIdx].sustain)).toDouble();
                            m_channels[dIdx].release = elvol.attribute("rel", QString::number(m_channels[dIdx].release)).toDouble();
                        }

                        QDomElement pat = t.firstChildElement("pattern");
                        if (!pat.isNull()) {
                            QDomNodeList notes = pat.elementsByTagName("note");
                            for (int nIdx=0; nIdx<notes.size(); ++nIdx) {
                                int step = notes.at(nIdx).toElement().attribute("pos").toInt() / 12;
                                if (step >= 0 && step < NUM_STEPS) m_grid[dIdx][step]->setChecked(true);
                            }
                        }
                    }
                }
            } else if (e.attribute("type") == "0") {
                QString name = e.attribute("name");
                int mIdx = -1; for(int m=0; m<NUM_MELODIC; ++m) if(m_melodic[m].name == name) mIdx = m;

                if (mIdx != -1) {
                    QDomElement instTrack = e.firstChildElement("instrumenttrack");
                    QDomElement xpressive = instTrack.firstChildElement("instrument").firstChildElement("xpressive");
                    if (xpressive.isNull()) xpressive = instTrack.firstChildElement("instrument").firstChildElement("Xpressive");

                    if (!xpressive.isNull()) {
                        m_melodic[mIdx].w1 = xpressive.hasAttribute("W1") ? xpressive.attribute("W1") : xpressive.attribute("w1");
                        m_melodic[mIdx].w2 = xpressive.hasAttribute("W2") ? xpressive.attribute("W2") : xpressive.attribute("w2");
                        m_melodic[mIdx].w3 = xpressive.hasAttribute("W3") ? xpressive.attribute("W3") : xpressive.attribute("w3");
                        m_melodic[mIdx].o1 = xpressive.hasAttribute("O1") ? xpressive.attribute("O1") : xpressive.attribute("o1");
                        m_melodic[mIdx].o2 = xpressive.hasAttribute("O2") ? xpressive.attribute("O2") : xpressive.attribute("o2");

                        m_melodic[mIdx].attack = xpressive.attribute("env_atk").toDouble();
                        m_melodic[mIdx].decay = xpressive.attribute("env_dec").toDouble();
                        m_melodic[mIdx].sustain = xpressive.attribute("env_sus").toDouble();
                        m_melodic[mIdx].release = xpressive.attribute("env_rel").toDouble();
                    }

                    QDomElement eldata = instTrack.firstChildElement("eldata");
                    QDomElement elvol = eldata.firstChildElement("elvol");
                    if (!elvol.isNull()) {
                        m_melodic[mIdx].attack = elvol.attribute("att", QString::number(m_melodic[mIdx].attack)).toDouble();
                        m_melodic[mIdx].decay = elvol.attribute("dec", QString::number(m_melodic[mIdx].decay)).toDouble();
                        m_melodic[mIdx].sustain = elvol.attribute("sustain", QString::number(m_melodic[mIdx].sustain)).toDouble();
                        m_melodic[mIdx].release = elvol.attribute("rel", QString::number(m_melodic[mIdx].release)).toDouble();
                    }

                    QDomNodeList patterns = e.elementsByTagName("pattern");
                    for (int p = 0; p < patterns.size(); ++p) {
                        QDomElement pat = patterns.at(p).toElement();
                        int bar = pat.attribute("pos").toInt() / (16 * 12);
                        if (bar >= 0 && bar < 8) {
                            m_arrangerState[mIdx+1][bar] = bar + 1;
                            QDomNodeList notes = pat.elementsByTagName("note");
                            for (int nIdx = 0; nIdx < notes.size(); ++nIdx) {
                                int step = notes.at(nIdx).toElement().attribute("pos").toInt() / 12;
                                int key = 107 - notes.at(nIdx).toElement().attribute("key").toInt();
                                int len = notes.at(nIdx).toElement().attribute("len", "12").toInt() / 12;

                                if (step >= 0 && step < NUM_STEPS && key >= 0 && key < 96) {
                                    m_melodic[mIdx].grid[bar][key][step].active = true;
                                    m_melodic[mIdx].grid[bar][key][step].length = len;
                                }
                            }
                        }
                    }
                }
            }
        }
        n = n.nextSibling();
    }

    for(int r=0; r<4; ++r) {
        for(int b=0; b<8; ++b) {
            int hold = m_arrangerState[r][b];
            m_arrangerState[r][b] = 0;
            if (hold > 0) {
                m_arrangerState[r][b] = hold - 1;
                onArrangerClicked(r, b);
            } else {
                onArrangerClicked(r, b); onArrangerClicked(r, b);
            }
        }
    }
    updatePlaybackState();
    QMessageBox::information(this, "Success", "Full project loaded successfully!");
}

void MainWindow::onExportMmpClicked() {
    QString filePath = QFileDialog::getSaveFileName(this, "Save LMMS Project", "", "LMMS Project (*.mmp)");
    if (filePath.isEmpty()) return;

    QDomDocument doc;
    QDomProcessingInstruction instr = doc.createProcessingInstruction("xml", "version=\"1.0\"");
    doc.appendChild(instr);

    QDomElement root = doc.createElement("lmms-project");
    root.setAttribute("type", "song");
    root.setAttribute("version", "20");
    doc.appendChild(root);

    QDomElement head = doc.createElement("head");
    head.setAttribute("bpm", QString::number(m_spinBpm->value()));
    root.appendChild(head);

    QDomElement song = doc.createElement("song");
    root.appendChild(song);

    QDomElement trackContainer = doc.createElement("trackcontainer");
    trackContainer.setAttribute("type", "song");
    song.appendChild(trackContainer);

    const int ticksPerStep = 12;

    QDomElement bbMasterTrack = doc.createElement("track");
    bbMasterTrack.setAttribute("name", "Beat/Bassline 0");
    bbMasterTrack.setAttribute("type", "1");
    trackContainer.appendChild(bbMasterTrack);

    QDomElement bbTrack = doc.createElement("bbtrack");
    bbMasterTrack.appendChild(bbTrack);

    QDomElement bbTrackContainer = doc.createElement("trackcontainer");
    bbTrackContainer.setAttribute("type", "bbtrackcontainer");
    bbTrack.appendChild(bbTrackContainer);

    for (int r = 0; r < NUM_CHANNELS; ++r) {
        ChannelConfig& ch = m_channels[r];
        QDomElement track = doc.createElement("track");
        track.setAttribute("name", ch.name);
        track.setAttribute("type", "0");
        bbTrackContainer.appendChild(track);

        QDomElement instTrack = doc.createElement("instrumenttrack");
        instTrack.setAttribute("vol", "100");
        instTrack.setAttribute("pan", "0");
        track.appendChild(instTrack);

        QDomElement instrument = doc.createElement("instrument"); instrument.setAttribute("name", "xpressive");

        QDomElement xpressive = doc.createElement("xpressive");
        xpressive.setAttribute("W1", ch.w1); xpressive.setAttribute("W2", ""); xpressive.setAttribute("W3", "");
        xpressive.setAttribute("O1", ch.o1); xpressive.setAttribute("O2", "");

        xpressive.setAttribute("env_atk", QString::number(ch.attack)); xpressive.setAttribute("env_dec", QString::number(ch.decay));
        xpressive.setAttribute("env_sus", QString::number(ch.sustain)); xpressive.setAttribute("env_rel", QString::number(ch.release));

        QDomElement keyNode = doc.createElement("key");
        xpressive.appendChild(keyNode);

        instrument.appendChild(xpressive); instTrack.appendChild(instrument);

        QDomElement eldata = doc.createElement("eldata");
        QDomElement elvol = doc.createElement("elvol");
        elvol.setAttribute("att", QString::number(ch.attack));
        elvol.setAttribute("dec", QString::number(ch.decay));
        elvol.setAttribute("sustain", QString::number(ch.sustain));
        elvol.setAttribute("rel", QString::number(ch.release));

        eldata.appendChild(elvol); instTrack.appendChild(eldata);

        QDomElement pattern = doc.createElement("pattern");
        pattern.setAttribute("type", "0");
        pattern.setAttribute("pos", "0");
        pattern.setAttribute("steps", "16");
        track.appendChild(pattern);

        for (int c = 0; c < NUM_STEPS; ++c) {
            if (m_grid[r][c]->isChecked()) {
                QDomElement note = doc.createElement("note");
                note.setAttribute("pos", QString::number(c * ticksPerStep));
                note.setAttribute("len", "12");
                note.setAttribute("key", "60");
                note.setAttribute("vol", QString::number(int(ch.vol * 100)));
                pattern.appendChild(note);
            }
        }
    }

    for (int b = 0; b < 8; ++b) {
        if (m_arrangerState[0][b] == 1) {
            QDomElement bbtco = doc.createElement("bbtco");
            bbtco.setAttribute("pos", QString::number(b * 16 * ticksPerStep));
            bbtco.setAttribute("len", "192");
            bbMasterTrack.appendChild(bbtco);
        }
    }

    for (int m = 0; m < NUM_MELODIC; ++m) {
        MelodicConfig& ch = m_melodic[m];
        QDomElement track = doc.createElement("track"); track.setAttribute("name", ch.name); track.setAttribute("type", "0");
        trackContainer.appendChild(track);

        QDomElement instTrack = doc.createElement("instrumenttrack");
        instTrack.setAttribute("vol", "100");
        track.appendChild(instTrack);

        QDomElement instrument = doc.createElement("instrument"); instrument.setAttribute("name", "xpressive");

        QDomElement xpressive = doc.createElement("xpressive");
        xpressive.setAttribute("W1", ch.w1); xpressive.setAttribute("W2", ch.w2); xpressive.setAttribute("W3", ch.w3);
        xpressive.setAttribute("O1", ch.o1); xpressive.setAttribute("O2", ch.o2);

        xpressive.setAttribute("env_atk", QString::number(ch.attack)); xpressive.setAttribute("env_dec", QString::number(ch.decay));
        xpressive.setAttribute("env_sus", QString::number(ch.sustain)); xpressive.setAttribute("env_rel", QString::number(ch.release));

        QDomElement keyNode = doc.createElement("key");
        xpressive.appendChild(keyNode);

        instrument.appendChild(xpressive); instTrack.appendChild(instrument);

        QDomElement eldata = doc.createElement("eldata");
        QDomElement elvol = doc.createElement("elvol");
        elvol.setAttribute("att", QString::number(ch.attack));
        elvol.setAttribute("dec", QString::number(ch.decay));
        elvol.setAttribute("sustain", QString::number(ch.sustain));
        elvol.setAttribute("rel", QString::number(ch.release));

        eldata.appendChild(elvol); instTrack.appendChild(eldata);

        for (int b = 0; b < 8; ++b) {
            int patIdx = m_arrangerState[m+1][b];
            if (patIdx == 0) continue;
            int p = patIdx - 1;

            bool hasNotes = false;
            for (int key = 0; key < 96; ++key) { for (int c = 0; c < 16; ++c) { if (ch.grid[p][key][c].active) hasNotes = true; } }
            if (!hasNotes) continue;

            QDomElement pattern = doc.createElement("pattern");
            pattern.setAttribute("type", "1");
            pattern.setAttribute("pos", QString::number(b * 16 * ticksPerStep));
            pattern.setAttribute("steps", "16");
            pattern.setAttribute("len", QString::number(16 * ticksPerStep));
            track.appendChild(pattern);

            for (int key = 0; key < 96; ++key) {
                for (int c = 0; c < 16; ++c) {
                    if (ch.grid[p][key][c].active) {
                        QDomElement note = doc.createElement("note");
                        note.setAttribute("pos", QString::number(c * ticksPerStep));
                        note.setAttribute("len", QString::number(ch.grid[p][key][c].length * ticksPerStep));
                        note.setAttribute("key", QString::number(107 - key));
                        note.setAttribute("vol", QString::number(int(ch.vol * 100)));
                        pattern.appendChild(note);
                    }
                }
            }
        }
    }

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file); out << doc.toString(4); file.close();
        QMessageBox::information(this, "Export Complete", "Full project saved to:\n" + filePath);
    }
}