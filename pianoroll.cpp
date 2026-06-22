#include "pianoroll.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>

PianoRollDialog::PianoRollDialog(NoteEvent (&pattern)[96][16], const QString& title, QWidget *parent) : QDialog(parent) {
    setWindowTitle(title);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(2);

    QPushButton* btnSave = new QPushButton("💾 Save Pattern & Close");
    btnSave->setStyleSheet("background-color: #7a00cc; color: white; padding: 10px; font-weight: bold; border-radius: 4px;");
    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnSave);

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
            m_btnGrid[r][c]->setChecked(pattern[r][c].active);

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

    setStyleSheet("QDialog { background-color: #1e1e1e; }");

    setWindowState(Qt::WindowMaximized);

    QTimer::singleShot(50, [scrollArea, gridWidget]() {
        int targetY = (47 * 32) - (scrollArea->viewport()->height() / 2);
        scrollArea->verticalScrollBar()->setValue(std::max(0, targetY));
    });
}

void PianoRollDialog::saveToGrid(NoteEvent (&pattern)[96][16]) {
    for(int r=0; r<96; ++r) {
        for(int c=0; c<16; ++c) {
            pattern[r][c].active = m_btnGrid[r][c]->isChecked();

        }
    }
}