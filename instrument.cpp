#include "instrument.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>



ChannelConfigDialog::ChannelConfigDialog(ChannelConfig& config, QWidget *parent) : QDialog(parent) {
    setWindowTitle("Configure " + config.name);
    setMinimumWidth(300);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* form = new QFormLayout();

    txtW1 = new QTextEdit(config.w1); 
    txtW1->setMaximumHeight(40); 
    form->addRow("W1:", txtW1);
    
    txtO1 = new QTextEdit(config.o1); 
    txtO1->setMaximumHeight(40); 
    form->addRow("O1:", txtO1);

    spinA = new QDoubleSpinBox(); spinA->setRange(0, 5); spinA->setValue(config.attack); spinA->setSingleStep(0.01);
    spinD = new QDoubleSpinBox(); spinD->setRange(0, 5); spinD->setValue(config.decay); spinD->setSingleStep(0.01);
    spinS = new QDoubleSpinBox(); spinS->setRange(0, 1); spinS->setValue(config.sustain); spinS->setSingleStep(0.1);
    spinR = new QDoubleSpinBox(); spinR->setRange(0, 5); spinR->setValue(config.release); spinR->setSingleStep(0.01);
    spinVol = new QDoubleSpinBox(); spinVol->setRange(0, 2); spinVol->setValue(config.vol); spinVol->setSingleStep(0.1);

    form->addRow("Attack(s):", spinA); 
    form->addRow("Decay(s):", spinD);
    form->addRow("Sustain:", spinS); 
    form->addRow("Release(s):", spinR);
    form->addRow("Volume:", spinVol);

    mainLayout->addLayout(form);
    
    QPushButton* btnSave = new QPushButton("Apply & Close");
    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnSave);


    setStyleSheet("QDialog { background-color: #2b2b2b; color: #ffffff; font-size: 11px; } "
                  "QTextEdit, QDoubleSpinBox { background-color: #1e1e1e; color: #fff; border: 1px solid #555; } "
                  "QPushButton { background-color: #007acc; padding: 8px; color: white; font-weight: bold; border-radius: 4px; }");
}

void ChannelConfigDialog::saveToConfig(ChannelConfig& config) {
    config.w1 = txtW1->toPlainText(); 
    config.o1 = txtO1->toPlainText();
    config.attack = spinA->value(); 
    config.decay = spinD->value(); 
    config.sustain = spinS->value(); 
    config.release = spinR->value(); 
    config.vol = spinVol->value();
}




MelodicConfigDialog::MelodicConfigDialog(MelodicConfig& config, QWidget *parent) : QDialog(parent) {
    setWindowTitle("Configure " + config.name);
    setMinimumWidth(320);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* form = new QFormLayout();

    txtW1 = new QTextEdit(config.w1); txtW1->setMaximumHeight(40); form->addRow("W1:", txtW1);
    txtW2 = new QTextEdit(config.w2); txtW2->setMaximumHeight(40); form->addRow("W2:", txtW2);
    txtW3 = new QTextEdit(config.w3); txtW3->setMaximumHeight(40); form->addRow("W3:", txtW3);
    
    txtO1 = new QTextEdit(config.o1); txtO1->setMaximumHeight(40); form->addRow("O1:", txtO1);
    txtO2 = new QTextEdit(config.o2); txtO2->setMaximumHeight(40); form->addRow("O2:", txtO2);

    chkUseAdsr = new QCheckBox("Enable ADSR (Uncheck for Formula Time/AMT)");
    chkUseAdsr->setChecked(config.useAdsr);
    form->addRow(chkUseAdsr);

    spinA = new QDoubleSpinBox(); spinA->setRange(0, 5); spinA->setValue(config.attack); spinA->setSingleStep(0.01);
    spinD = new QDoubleSpinBox(); spinD->setRange(0, 5); spinD->setValue(config.decay); spinD->setSingleStep(0.01);
    spinS = new QDoubleSpinBox(); spinS->setRange(0, 1); spinS->setValue(config.sustain); spinS->setSingleStep(0.1);
    spinR = new QDoubleSpinBox(); spinR->setRange(0, 5); spinR->setValue(config.release); spinR->setSingleStep(0.01);
    spinVol = new QDoubleSpinBox(); spinVol->setRange(0, 2); spinVol->setValue(config.vol); spinVol->setSingleStep(0.1);

    form->addRow("Attack(s):", spinA); 
    form->addRow("Decay(s):", spinD);
    form->addRow("Sustain:", spinS); 
    form->addRow("Release(s):", spinR);
    form->addRow("Volume:", spinVol);

    mainLayout->addLayout(form);
    
    QPushButton* btnSave = new QPushButton("Apply & Close");
    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnSave);


    setStyleSheet("QDialog { background-color: #2b2b2b; color: #ffffff; font-size: 11px; } "
                  "QTextEdit, QDoubleSpinBox, QCheckBox { background-color: #1e1e1e; color: #fff; border: 1px solid #555; } "
                  "QPushButton { background-color: #7a00cc; padding: 8px; color: white; font-weight: bold; border-radius: 4px; }");
}

void MelodicConfigDialog::saveToConfig(MelodicConfig& config) {
    config.w1 = txtW1->toPlainText(); 
    config.w2 = txtW2->toPlainText(); 
    config.w3 = txtW3->toPlainText();
    
    config.o1 = txtO1->toPlainText(); 
    config.o2 = txtO2->toPlainText();
    
    config.useAdsr = chkUseAdsr->isChecked();
    
    config.attack = spinA->value(); 
    config.decay = spinD->value(); 
    config.sustain = spinS->value(); 
    config.release = spinR->value(); 
    config.vol = spinVol->value();
}