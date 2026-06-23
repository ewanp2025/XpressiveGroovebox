#include "mainwindow.h"
#include "synthengine.h"
#include "xpressive_parser.h"
#include "projectio.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QTimer>
#include <QFrame>
#include <QMutexLocker>
#include <memory>
#include <array>

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
    m_channels[4].name = "Prc1";  m_channels[4].w1 = "squarew(t)"; m_channels[4].o1 = "W1(integrate(f)) * exp(-t*20)"; m_channels[4].decay = 0.1; m_channels[4].vol = 0.4;
    m_channels[5].name = "Prc2";  m_channels[5].w1 = "saww(t)"; m_channels[5].o1 = "W1(integrate(f*0.5))"; m_channels[5].decay = 0.4; m_channels[5].sustain = 0.5; m_channels[5].vol = 0.6;

    m_melodic[0].name = "Bass";
    m_melodic[0].w1 = "saww(t)";
    m_melodic[0].w2 = "squarew(t)";
    m_melodic[0].o1 = "(W1(integrate(f)) + W2(integrate(f*1.005)))*0.5";
    m_melodic[0].decay = 0.4; m_melodic[0].sustain = 0.1;

    m_melodic[1].name = "Synth";
    m_melodic[1].w1 = "sinew(t)";
    m_melodic[1].w2 = "saww(t)";
    m_melodic[1].o1 = "(W1(integrate(f)) + W2(integrate(f*2.0)))*0.5";
    m_melodic[1].decay = 0.8; m_melodic[1].sustain = 0.6;

    m_melodic[2].name = "Pad";
    m_melodic[2].w1 = "saww(t)";
    m_melodic[2].w2 = "saww(t)";
    m_melodic[2].o1 = "(W1(integrate(f*0.99)) + W2(integrate(f*1.01)))*0.5";
    m_melodic[2].attack = 0.5; m_melodic[2].decay = 1.0; m_melodic[2].sustain = 0.8; m_melodic[2].release = 1.0;

    setupUI();
    recompileASTs();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updatePlaybackState);
    timer->start(50);
}

MainWindow::~MainWindow() {
    m_synthEngine->stop();
}

void MainWindow::setupUI() {
    QWidget* baseWidget = new QWidget(this);
    setCentralWidget(baseWidget);
    QVBoxLayout* baseLayout = new QVBoxLayout(baseWidget);
    baseLayout->setContentsMargins(0, 0, 0, 0);

    m_mainTabs = new QTabWidget(this);
    m_mainTabs->setStyleSheet("QTabWidget::pane { border: 1px solid #444; } QTabBar::tab { background: #333; color: white; padding: 8px 20px; } QTabBar::tab:selected { background: #007acc; font-weight: bold; }");
    baseLayout->addWidget(m_mainTabs);

    QScrollArea* mainScroll = new QScrollArea();
    mainScroll->setWidgetResizable(true);
    mainScroll->setStyleSheet("border: none; background-color: transparent;");

    QWidget* centralWidget = new QWidget();
    mainScroll->setWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    centralWidget->setStyleSheet(R"(QWidget { background-color: #1e1e1e; color: #e0e0e0; font-family: "Segoe UI", sans-serif; font-size: 11px; } QGroupBox { border: 1px solid #444; border-radius: 4px; font-weight: bold; margin-top: 1ex; padding: 2px; } QGroupBox::title { subcontrol-origin: margin; left: 5px; padding: 0 3px; color: #00ffff; } QCheckBox::indicator { width: 14px; height: 35px; border: 1px solid #555; background: #2b2b2b; border-radius: 2px; } QCheckBox::indicator:checked { background: #00ffff; border: 1px solid #ffffff; } QPushButton { background-color: #333; border: 1px solid #555; padding: 6px 2px; color: #fff; border-radius: 3px; font-weight: bold; } QPushButton:hover, QPushButton:pressed { background-color: #444; border: 1px solid #00ffff; } QSpinBox, QComboBox, QLineEdit { background-color: #333; color: white; border: 1px solid #555; padding: 2px; min-height: 25px; })");

    QHBoxLayout* topLayout = new QHBoxLayout();
    m_spinBpm = new QSpinBox(); m_spinBpm->setRange(60, 300); m_spinBpm->setValue(120);
    connect(m_spinBpm, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updatePlaybackState);

    m_btnPlay = new QPushButton("▶ Play Sequence"); m_btnPlay->setStyleSheet("background-color: #005577; color: white; padding: 5px 15px;");
    m_txtTestString = new QLineEdit(); m_txtTestString->setPlaceholderText("Paste Custom Formula Here...");
    m_btnTestString = new QPushButton("🧪 Test Expr"); m_btnTestString->setStyleSheet("background-color: #5500aa; color: white; padding: 5px 15px;");
    m_btnImport = new QPushButton("📂 Import"); m_btnImport->setStyleSheet("background-color: #775500; color: white;");
    m_btnExport = new QPushButton("💾 Export"); m_btnExport->setStyleSheet("background-color: #005533; color: white;");

    topLayout->addWidget(new QLabel("BPM:")); topLayout->addWidget(m_spinBpm); topLayout->addWidget(m_btnPlay); topLayout->addWidget(m_txtTestString); topLayout->addWidget(m_btnTestString); topLayout->addWidget(m_btnImport); topLayout->addWidget(m_btnExport);
    mainLayout->addLayout(topLayout);

    QHBoxLayout* middleLayout = new QHBoxLayout();
    QGroupBox* drumBox = new QGroupBox("Percussion (Global)");
    QVBoxLayout* drumBoxLayout = new QVBoxLayout(drumBox);
    QScrollArea* drumScrollArea = new QScrollArea(); drumScrollArea->setWidgetResizable(true); drumScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QWidget* gridContainer = new QWidget(); QGridLayout* gridLayout = new QGridLayout(gridContainer);

    for (int r = 0; r < NUM_CHANNELS; ++r) {
        QPushButton* btnCfg = new QPushButton("⚙️" + m_channels[r].name); btnCfg->setFixedWidth(50);
        connect(btnCfg, &QPushButton::clicked, this, [this, r]() { onConfigClicked(r); });
        gridLayout->addWidget(btnCfg, r, 0);
        for (int c = 0; c < NUM_STEPS; ++c) {
            m_grid[r][c] = new QCheckBox(); connect(m_grid[r][c], &QCheckBox::clicked, this, &MainWindow::updatePlaybackState);
            gridLayout->addWidget(m_grid[r][c], r, c + 1);
        }
    }
    drumScrollArea->setWidget(gridContainer); drumBoxLayout->addWidget(drumScrollArea); middleLayout->addWidget(drumBox);

    QGroupBox* arrangerBox = new QGroupBox("Song Arranger");
    QGridLayout* arrangerLayout = new QGridLayout(arrangerBox);
    QStringList trackNames = {"Drums", "Bass", "Synth", "Pad"};
    for (int row = 0; row < 4; ++row) {
        QLabel* trackLabel = new QLabel(trackNames[row]); trackLabel->setFixedWidth(40); arrangerLayout->addWidget(trackLabel, row, 0);
        for (int bar = 0; bar < 8; ++bar) {
            m_arrangerGrid[row][bar] = new QPushButton(""); m_arrangerGrid[row][bar]->setFixedSize(25, 25);
            connect(m_arrangerGrid[row][bar], &QPushButton::clicked, this, [this, row, bar]() { onArrangerClicked(row, bar); });
            arrangerLayout->addWidget(m_arrangerGrid[row][bar], row, bar + 1);
        }
    }
    onArrangerClicked(0, 0); onArrangerClicked(1, 0); onArrangerClicked(2, 0); onArrangerClicked(3, 0);
    middleLayout->addWidget(arrangerBox); mainLayout->addLayout(middleLayout);

    QGroupBox* autoBox = new QGroupBox("Step Automations (Macros)");
    QVBoxLayout* autoLayout = new QVBoxLayout(autoBox);
    m_autoTracks[0].name = "Master Drive"; m_autoTracks[0].target = AutoTarget::MasterDrive;
    m_autoTracks[1].name = "Master Volume"; m_autoTracks[1].target = AutoTarget::MasterVolume;
    for (int i = 0; i < 2; ++i) {
        QHBoxLayout* row = new QHBoxLayout(); QLabel* lbl = new QLabel("<b>" + m_autoTracks[i].name + "</b>"); lbl->setFixedWidth(80);
        m_autoGrids[i] = new AutomationGrid(m_autoTracks[i], this); connect(m_autoGrids[i], &AutomationGrid::automationChanged, this, &MainWindow::updatePlaybackState);
        row->addWidget(lbl); row->addWidget(m_autoGrids[i]); autoLayout->addLayout(row);
    }
    mainLayout->addWidget(autoBox);

    QGroupBox* melodicBox = new QGroupBox("Melodic Synths");
    QVBoxLayout* melodicLayout = new QVBoxLayout(melodicBox);
    for (int m = 0; m < NUM_MELODIC; ++m) {
        QHBoxLayout* row = new QHBoxLayout(); QLabel* lblName = new QLabel("<b>" + m_melodic[m].name + "</b>"); lblName->setFixedWidth(45);
        QPushButton* btnCfg = new QPushButton("⚙️"); btnCfg->setFixedWidth(30); connect(btnCfg, &QPushButton::clicked, this, [this, m]() { onMelodicConfigClicked(m); });
        m_comboEditBar[m] = new QComboBox(); m_comboEditBar[m]->setFixedWidth(85); for(int b=1; b<=8; ++b) m_comboEditBar[m]->addItem(QString("Pattern %1").arg(b));
        QPushButton* btnRoll = new QPushButton("🎹 Roll"); btnRoll->setStyleSheet("background-color: #4a0077; font-weight: bold;"); connect(btnRoll, &QPushButton::clicked, this, [this, m]() { onPianoRollClicked(m); });
        row->addWidget(lblName); row->addWidget(btnCfg); row->addWidget(m_comboEditBar[m]); row->addWidget(btnRoll); melodicLayout->addLayout(row);
    }
    mainLayout->addWidget(melodicBox); mainLayout->addStretch(); m_mainTabs->addTab(mainScroll, "Grid Sequencer");

    QWidget* engineTab = new QWidget(); engineTab->setStyleSheet("QWidget { background-color: #1e1e1e; color: #e0e0e0; font-size: 11px; }");
    QHBoxLayout* engineLayout = new QHBoxLayout(engineTab);

    QGroupBox* grpSynth = new QGroupBox("Engine Selection (Headless)");
    QVBoxLayout* synthLayout = new QVBoxLayout(grpSynth);
    m_cmbSynthSelector = new QComboBox(); m_cmbSynthSelector->addItems({"Xpressive (Legacy)"});
    connect(m_cmbSynthSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSynthTypeChanged);
    synthLayout->addWidget(m_cmbSynthSelector);

    m_synthUIStack = new QStackedWidget(); m_synthUIStack->addWidget(new QLabel("Xpressive controlled via Grid Sequencer tab."));

    QWidget* hiddenContainer = new QWidget(this); hiddenContainer->hide();
    m_toVol = new QDoubleSpinBox(hiddenContainer); m_toW1 = new QComboBox(hiddenContainer); m_toC1 = new QDoubleSpinBox(hiddenContainer); m_toV1 = new QDoubleSpinBox(hiddenContainer);
    m_lbVol = new QDoubleSpinBox(hiddenContainer); m_lbWave = new QComboBox(hiddenContainer); m_lbCut = new QDoubleSpinBox(hiddenContainer); m_lbRes = new QDoubleSpinBox(hiddenContainer); m_lbDec = new QDoubleSpinBox(hiddenContainer);

    synthLayout->addWidget(m_synthUIStack); engineLayout->addWidget(grpSynth);

    QGroupBox* grpEffects = new QGroupBox("Multi-Track Effect Rack");
    QVBoxLayout* fxLayout = new QVBoxLayout(grpEffects);
    m_lstEffects = new QListWidget(); fxLayout->addWidget(m_lstEffects);
    QGridLayout* fxControls = new QGridLayout();
    m_cmbAvailableEffects = new QComboBox(); m_cmbAvailableEffects->addItems({"Dummy Drive (Test)", "LMMS Bitcrush", "LMMS Stereo Delay", "Lush Reverb (Freeverb Alg)"});
    m_cmbFxTarget = new QComboBox(); m_cmbFxTarget->addItem("Master Bus");
    for(int i=0; i<6; ++i) m_cmbFxTarget->addItem("Drum " + QString::number(i+1) + " (" + m_channels[i].name + ")");
    for(int i=0; i<3; ++i) m_cmbFxTarget->addItem("Synth " + QString::number(i+1) + " (" + m_melodic[i].name + ")");
    m_btnAddEffect = new QPushButton("Insert FX"); m_btnRemoveEffect = new QPushButton("Remove Selected");
    connect(m_btnAddEffect, &QPushButton::clicked, this, &MainWindow::onAddEffectClicked);
    connect(m_btnRemoveEffect, &QPushButton::clicked, this, &MainWindow::onRemoveEffectClicked);
    fxControls->addWidget(new QLabel("Effect:"), 0, 0); fxControls->addWidget(m_cmbAvailableEffects, 0, 1);
    fxControls->addWidget(new QLabel("Route To:"), 1, 0); fxControls->addWidget(m_cmbFxTarget, 1, 1);
    fxControls->addWidget(m_btnAddEffect, 2, 0); fxControls->addWidget(m_btnRemoveEffect, 2, 1);
    fxLayout->addLayout(fxControls); engineLayout->addWidget(grpEffects); m_mainTabs->addTab(engineTab, "DSP & FX Routing");

    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::onPlayClicked);
    connect(m_btnTestString, &QPushButton::clicked, this, &MainWindow::onTestStringClicked);
    connect(m_btnExport, &QPushButton::clicked, this, &MainWindow::onExportMmpClicked);
    connect(m_btnImport, &QPushButton::clicked, this, &MainWindow::onImportMmpClicked);
    updatePlaybackState();
}

void MainWindow::onSynthTypeChanged(int index) { m_synthUIStack->setCurrentIndex(index); m_synthEngine->setInstrumentType(index == 0 ? InstrumentType::Xpressive : index == 1 ? InstrumentType::TripleOscillator : InstrumentType::Lb302); }
void MainWindow::onHeadlessSynthParamChanged() {}

void MainWindow::onAddEffectClicked() {
    int fxType = m_cmbAvailableEffects->currentIndex(); int target = m_cmbFxTarget->currentIndex(); m_synthEngine->addEffect(target, fxType);
    QListWidgetItem* item = new QListWidgetItem(m_cmbAvailableEffects->currentText() + " -> " + m_cmbFxTarget->currentText());
    item->setData(Qt::UserRole, target); item->setData(Qt::UserRole + 1, fxType); m_lstEffects->addItem(item);
}

void MainWindow::onRemoveEffectClicked() {
    QListWidgetItem* item = m_lstEffects->takeItem(m_lstEffects->currentRow());
    if (item) {
        delete item; m_synthEngine->clearEffects();
        for(int i = 0; i < m_lstEffects->count(); ++i) { QListWidgetItem* itm = m_lstEffects->item(i); m_synthEngine->addEffect(itm->data(Qt::UserRole).toInt(), itm->data(Qt::UserRole + 1).toInt()); }
    }
}

void MainWindow::onArrangerClicked(int row, int bar) {
    if (row == 0) m_arrangerState[row][bar] = (m_arrangerState[row][bar] == 0) ? 1 : 0; else m_arrangerState[row][bar] = (m_arrangerState[row][bar] + 1) % 9;
    QPushButton* btn = m_arrangerGrid[row][bar];
    if (m_arrangerState[row][bar] == 0) { btn->setText(""); btn->setStyleSheet("background-color: #333; border: 1px solid #555; border-radius: 3px;"); }
    else { btn->setText(row == 0 ? "D" : QString::number(m_arrangerState[row][bar])); btn->setStyleSheet("background-color: #ffff00; color: black; font-weight: bold; border: 1px solid #fff; border-radius: 3px;"); }
    updatePlaybackState();
}

void MainWindow::updatePlaybackState() {
    QMutexLocker locker(&m_playState.mutex);
    m_playState.bpm = m_spinBpm->value();
    for (int r = 0; r < NUM_CHANNELS; ++r) {
        m_playState.drums[r].a = m_channels[r].attack; m_playState.drums[r].d = m_channels[r].decay; m_playState.drums[r].s = m_channels[r].sustain; m_playState.drums[r].vol = m_channels[r].vol; m_playState.drums[r].baseF = (r==0)?55.0 : (r==1)?200.0 : (r==2||r==3)?8000.0 : (r==4)?440.0 : 110.0; m_playState.drums[r].isK = (r==0); m_playState.drums[r].filter = m_channels[r].filter;
        for (int c = 0; c < NUM_STEPS; ++c) m_playState.drumSteps[r][c] = m_grid[r][c]->isChecked();
    }
    for (int m = 0; m < NUM_MELODIC; ++m) {
        m_playState.synths[m].a = m_melodic[m].attack; m_playState.synths[m].d = m_melodic[m].decay; m_playState.synths[m].s = m_melodic[m].sustain; m_playState.synths[m].r = m_melodic[m].release; m_playState.synths[m].vol = m_melodic[m].vol; m_playState.synths[m].useAdsr = m_melodic[m].useAdsr; m_playState.synths[m].filter = m_melodic[m].filter;
        for (int b = 0; b < 8; ++b) for (int key = 0; key < 96; ++key) for (int c = 0; c < NUM_STEPS; ++c) m_playState.synthGrid[m][b][key][c] = m_melodic[m].grid[b][key][c];
    }
    for (int t = 0; t < 4; ++t) for (int b = 0; b < 8; ++b) m_playState.arranger[t][b] = m_arrangerState[t][b];
    for (int i = 0; i < 2; ++i) { m_playState.automations[i].target = m_autoTracks[i].target; for (int step = 0; step < 16; ++step) m_playState.automations[i].steps[step] = m_autoTracks[i].steps[step]; }
}

void MainWindow::recompileASTs() {
    auto newAsts = std::make_shared<CompiledASTs>();
    for (int r = 0; r < NUM_CHANNELS; ++r) {
        try { newAsts->drums[r].o1 = std::shared_ptr<XpressiveParser::Node>(XpressiveParser::parse(m_channels[r].o1.toStdString()).release()); } catch (...) {}
        try { newAsts->drums[r].w1 = std::shared_ptr<XpressiveParser::Node>(XpressiveParser::parse(m_channels[r].w1.toStdString()).release()); } catch (...) {}
    }
    for (int m = 0; m < NUM_MELODIC; ++m) {
        for (int key = 0; key < 96; ++key) {
            try { newAsts->synths[m][key].o1 = std::shared_ptr<XpressiveParser::Node>(XpressiveParser::parse(m_melodic[m].o1.toStdString()).release()); } catch (...) {}
            try { newAsts->synths[m][key].w1 = std::shared_ptr<XpressiveParser::Node>(XpressiveParser::parse(m_melodic[m].w1.toStdString()).release()); } catch (...) {}
            try { newAsts->synths[m][key].w2 = std::shared_ptr<XpressiveParser::Node>(XpressiveParser::parse(m_melodic[m].w2.toStdString()).release()); } catch (...) {}
            try { newAsts->synths[m][key].w3 = std::shared_ptr<XpressiveParser::Node>(XpressiveParser::parse(m_melodic[m].w3.toStdString()).release()); } catch (...) {}
        }
    }
    QMutexLocker locker(&m_playState.mutex);
    m_playState.asts = newAsts;
}

void MainWindow::onConfigClicked(int index) { ChannelConfigDialog dlg(m_channels[index], this); if (dlg.exec() == QDialog::Accepted) { dlg.saveToConfig(m_channels[index]); updatePlaybackState(); recompileASTs(); } }
void MainWindow::onMelodicConfigClicked(int index) { MelodicConfigDialog dlg(m_melodic[index], this); if (dlg.exec() == QDialog::Accepted) { dlg.saveToConfig(m_melodic[index]); updatePlaybackState(); recompileASTs(); } }
void MainWindow::onPianoRollClicked(int index) { int pat = m_comboEditBar[index]->currentIndex(); PianoRollDialog dlg(m_melodic[index].grid[pat], m_melodic[index].name + " - Pattern " + QString::number(pat+1), this); if (dlg.exec() == QDialog::Accepted) { dlg.saveToGrid(m_melodic[index].grid[pat]); updatePlaybackState(); } }

void MainWindow::onPlayClicked() {
    if (m_btnPlay->text().contains("Play")) {
        if (m_btnTestString->text().contains("Stop")) onTestStringClicked();

        std::vector<double> noteFreqs(96);
        for(int key=0; key<96; ++key) noteFreqs[key] = 440.0 * std::pow(2.0, ((107 - key) - 69) / 12.0);

        m_synthEngine->setAudioSource([this, noteFreqs,
                                       cachedAsts = std::shared_ptr<CompiledASTs>(),
                                       updateCounter = 0,
                                       lastGlobalStep = -1L,
                                       drumActive = std::array<bool, 6>{},
                                       drumNoteTimes = std::array<double, 6>{},
                                       synthActive = std::array<std::array<bool, 96>, 3>{},
                                       synthNoteTimes = std::array<std::array<double, 96>, 3>{},
                                       synthLenSteps = std::array<std::array<int, 96>, 3>{}
        ](double t, double outDrums[6], double outSynths[3], double& masterVol, double& masterDrive) mutable {


            if (updateCounter++ % 256 == 0) {
                if (m_playState.mutex.tryLock()) {
                    cachedAsts = m_playState.asts;
                    m_playState.mutex.unlock();
                }
            }
            if (!cachedAsts) return;

            double stepRate = (m_playState.bpm / 60.0) * 4.0;
            long globalStep = (long)std::floor(t * stepRate);
            int seqStep = globalStep % 16;
            int currentBar = (globalStep / 16) % 8;
            double lt = std::fmod(t, 1.0 / stepRate);


            if (globalStep != lastGlobalStep) {
                lastGlobalStep = globalStep;

                if (m_playState.arranger[0][currentBar] == 1) {
                    for(int r=0; r<6; ++r) {
                        drumActive[r] = false;
                        for (int c = seqStep; c >= 0; --c) {
                            if (m_playState.drumSteps[r][c]) {
                                drumActive[r] = true; drumNoteTimes[r] = (seqStep - c) / stepRate; break;
                            }
                        }
                        if (!drumActive[r]) {
                            int prevBar = (currentBar - 1 + 8) % 8;
                            if (m_playState.arranger[0][prevBar] == 1) {
                                for (int c = 15; c > seqStep; --c) {
                                    if (m_playState.drumSteps[r][c]) {
                                        drumActive[r] = true; drumNoteTimes[r] = ((seqStep + 16) - c) / stepRate; break;
                                    }
                                }
                            }
                        }
                    }
                }

                for(int m = 0; m < 3; ++m) {
                    int patIdx = m_playState.arranger[m+1][currentBar];
                    if (patIdx > 0) {
                        for(int key = 0; key < 96; ++key) {
                            synthActive[m][key] = false;
                            for (int c = seqStep; c >= 0; --c) {
                                if (m_playState.synthGrid[m][patIdx-1][key][c].active) {
                                    int endStep = c + m_playState.synthGrid[m][patIdx-1][key][c].length;
                                    if (endStep > seqStep || (m_playState.synths[m].useAdsr && ((seqStep - endStep) / stepRate) < m_playState.synths[m].r)) {
                                        synthActive[m][key] = true; synthNoteTimes[m][key] = (seqStep - c) / stepRate; synthLenSteps[m][key] = m_playState.synthGrid[m][patIdx-1][key][c].length; break;
                                    }
                                }
                            }
                            if (!synthActive[m][key]) {
                                int prevBar = (currentBar - 1 + 8) % 8; int prevPatIdx = m_playState.arranger[m+1][prevBar];
                                if (prevPatIdx > 0) {
                                    for (int c = 15; c >= 0; --c) {
                                        if (m_playState.synthGrid[m][prevPatIdx-1][key][c].active) {
                                            int endStep = c + m_playState.synthGrid[m][prevPatIdx-1][key][c].length;
                                            if (endStep > (seqStep + 16) || (m_playState.synths[m].useAdsr && (((seqStep + 16) - endStep) / stepRate) < m_playState.synths[m].r)) {
                                                synthActive[m][key] = true; synthNoteTimes[m][key] = ((seqStep + 16) - c) / stepRate; synthLenSteps[m][key] = m_playState.synthGrid[m][prevPatIdx-1][key][c].length; break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }


            if (m_playState.arranger[0][currentBar] == 1) {
                for(int r=0; r<6; ++r) {
                    if (drumActive[r] && cachedAsts->drums[r].o1) {
                        const auto& trk = m_playState.drums[r];
                        double noteTime = drumNoteTimes[r] + lt;
                        double env = trk.s;

                        if (noteTime < trk.a && trk.a > 0.001) env = noteTime / trk.a;
                        else if (noteTime < trk.a + trk.d && trk.d > 0.001) env = 1.0 - ((noteTime - trk.a) / trk.d) * (1.0 - trk.s);
                        else if (trk.a == 0 && trk.d == 0) env = 1.0;


                        XpressiveParser::Env xEnv; xEnv.t = noteTime; xEnv.tempo = m_playState.bpm; xEnv.srate = 44100.0; xEnv.f = trk.baseF;
                        xEnv.w1 = cachedAsts->drums[r].w1.get();

                        double osc = cachedAsts->drums[r].o1->eval(xEnv);

                        if (trk.filter.enabled) {
                            double eC = trk.filter.cutS;
                            if (noteTime < trk.filter.cutA && trk.filter.cutA > 0) eC = noteTime / trk.filter.cutA;
                            else if (noteTime < trk.filter.cutA + trk.filter.cutD && trk.filter.cutD > 0) eC = 1.0 - ((noteTime - trk.filter.cutA) / trk.filter.cutD) * (1.0 - trk.filter.cutS);

                            double eR = trk.filter.resS;
                            if (noteTime < trk.filter.resA && trk.filter.resA > 0) eR = noteTime / trk.filter.resA;
                            else if (noteTime < trk.filter.resA + trk.filter.resD && trk.filter.resD > 0) eR = 1.0 - ((noteTime - trk.filter.resA) / trk.filter.resD) * (1.0 - trk.filter.resS);

                            osc = m_playState.drumFilters[r].process(trk.filter.type, osc, trk.filter.cutBase + eC * trk.filter.cutEnvAmt, trk.filter.resBase + eR * trk.filter.resEnvAmt, 44100.0);
                        }
                        outDrums[r] += osc * env * trk.vol;
                    }
                }
            }

            for(int m = 0; m < 3; ++m) {
                for(int key = 0; key < 96; ++key) {
                    if (synthActive[m][key] && cachedAsts->synths[m][key].o1) {
                        const auto& trk = m_playState.synths[m];
                        double noteTime = synthNoteTimes[m][key] + lt;
                        double noteDur = synthLenSteps[m][key] / stepRate;
                        double env = 1.0;

                        if (trk.useAdsr) {
                            env = trk.s;
                            if (noteTime < trk.a && trk.a > 0.001) env = noteTime / trk.a;
                            else if (noteTime < trk.a + trk.d && trk.d > 0.001) env = 1.0 - ((noteTime - trk.a) / trk.d) * (1.0 - trk.s);

                            if (noteTime >= noteDur) {
                                double rT = noteTime - noteDur;
                                if (rT < trk.r && trk.r > 0.001) env = env * (1.0 - (rT / trk.r));
                                else env = 0.0;
                            }
                        }

                        if (env > 0.0001) {

                            XpressiveParser::Env xEnv; xEnv.t = noteTime; xEnv.tempo = m_playState.bpm; xEnv.srate = 44100.0; xEnv.f = noteFreqs[key];
                            xEnv.w1 = cachedAsts->synths[m][key].w1.get();
                            xEnv.w2 = cachedAsts->synths[m][key].w2.get();
                            xEnv.w3 = cachedAsts->synths[m][key].w3.get();

                            double osc = cachedAsts->synths[m][key].o1->eval(xEnv);

                            if (trk.filter.enabled) {
                                double eC = trk.filter.cutS;
                                if (noteTime < trk.filter.cutA && trk.filter.cutA > 0) eC = noteTime / trk.filter.cutA;
                                else if (noteTime < trk.filter.cutA + trk.filter.cutD && trk.filter.cutD > 0) eC = 1.0 - ((noteTime - trk.filter.cutA) / trk.filter.cutD) * (1.0 - trk.filter.cutS);

                                double eR = trk.filter.resS;
                                if (noteTime < trk.filter.resA && trk.filter.resA > 0) eR = noteTime / trk.filter.resA;
                                else if (noteTime < trk.filter.resA + trk.filter.resD && trk.filter.resD > 0) eR = 1.0 - ((noteTime - trk.filter.resA) / trk.filter.resD) * (1.0 - trk.filter.resS);

                                osc = m_playState.synthFilters[m][key].process(trk.filter.type, osc, trk.filter.cutBase + eC * trk.filter.cutEnvAmt, trk.filter.resBase + eR * trk.filter.resEnvAmt, 44100.0);
                            }
                            outSynths[m] += osc * env * trk.vol;
                        }
                    }
                }
            }

            for (int i = 0; i < 4; ++i) {
                double aV = m_playState.automations[i].steps[seqStep];
                if (m_playState.automations[i].target == AutoTarget::MasterVolume) masterVol = aV * 2.0;
                else if (m_playState.automations[i].target == AutoTarget::MasterDrive) masterDrive = 1.0 + (aV * 9.0);
            }
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
    if (m_btnTestString->text().contains("Stop")) { m_synthEngine->stop(); m_btnTestString->setText("🧪 Test Expr"); m_btnPlay->setEnabled(true); return; }
    QString code = m_txtTestString->text();
    try {
        auto ast = XpressiveParser::parse(code.toStdString()); std::shared_ptr<XpressiveParser::Node> sA(ast.release()); double bpm = m_spinBpm->value();
        m_synthEngine->setAudioSource([sA, bpm](double t, double d[6], double s[3], double& mV, double& mD) mutable {
            XpressiveParser::Env env; env.t = t; env.tempo = bpm; env.srate = 44100.0; env.f = 440.0; mV = 1.0; mD = 1.0; d[0] = std::max(-1.0, std::min(sA->eval(env), 1.0));
        });
        m_synthEngine->start(); m_btnTestString->setText("⏹ Stop Test"); if (m_btnPlay->text().contains("Stop")) onPlayClicked(); m_btnPlay->setEnabled(false);
    } catch (std::exception& e) { QMessageBox::warning(this, "Parse Error", "Syntax: " + QString(e.what())); }
}

void MainWindow::onImportMmpClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open LMMS Project", "", "LMMS Project (*.mmp)"); if (filePath.isEmpty()) return;
    int bpm = 120; bool drumSteps[6][16] = {{false}};
    if (ProjectIO::loadMmp(filePath, bpm, m_channels, drumSteps, m_arrangerState, m_melodic)) {
        m_spinBpm->setValue(bpm);
        for (int r = 0; r < 6; ++r) { for (int c = 0; c < 16; ++c) m_grid[r][c]->setChecked(drumSteps[r][c]); }
        for(int r = 0; r < 4; ++r) {
            for(int b = 0; b < 8; ++b) {
                int hold = m_arrangerState[r][b]; m_arrangerState[r][b] = 0;
                if (hold > 0) { m_arrangerState[r][b] = hold - 1; onArrangerClicked(r, b); }
                else { onArrangerClicked(r, b); onArrangerClicked(r, b); }
            }
        }
        updatePlaybackState(); recompileASTs(); QMessageBox::information(this, "Success", "Full project loaded!");
    } else QMessageBox::warning(this, "Error", "Failed to load project.");
}

void MainWindow::onExportMmpClicked() {
    QString filePath = QFileDialog::getSaveFileName(this, "Save LMMS Project", "", "LMMS Project (*.mmp)"); if (filePath.isEmpty()) return;
    bool drumSteps[6][16];
    for (int r = 0; r < 6; ++r) { for (int c = 0; c < 16; ++c) drumSteps[r][c] = m_grid[r][c]->isChecked(); }
    if (ProjectIO::saveMmp(filePath, m_spinBpm->value(), m_channels, drumSteps, m_arrangerState, m_melodic)) QMessageBox::information(this, "Export Complete", "LMMS Project Saved Successfully!");
    else QMessageBox::warning(this, "Error", "Failed to write file.");
}