#include "pianoroll.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>
#include <QPainterPath>
#include <QPushButton>



PianoRollGrid::PianoRollGrid(NoteEvent (&pattern)[96][16], QWidget *parent)
    : QWidget(parent), m_pattern(pattern) {
    setMouseTracking(true);
}

QSize PianoRollGrid::sizeHint() const {
    return QSize(16 * m_cellW, 96 * m_cellH);
}

void PianoRollGrid::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);


    for (int r = 0; r < 96; ++r) {
        int noteIndex = (107 - r) % 12;
        bool isBlackKey = (noteIndex == 1 || noteIndex == 3 || noteIndex == 6 || noteIndex == 8 || noteIndex == 10);
        p.fillRect(0, r * m_cellH, width(), m_cellH, isBlackKey ? QColor(30, 30, 30) : QColor(45, 45, 45));

        p.setPen(QColor(60, 60, 60));
        p.drawLine(0, r * m_cellH, width(), r * m_cellH);
    }

    for (int c = 0; c < 16; ++c) {
        p.setPen((c % 4 == 0) ? QColor(100, 100, 100) : QColor(60, 60, 60));
        p.drawLine(c * m_cellW, 0, c * m_cellW, height());
    }


    for (int r = 0; r < 96; ++r) {
        for (int c = 0; c < 16; ++c) {
            if (m_pattern[r][c].active) {
                int len = m_pattern[r][c].length;
                QRect rect(c * m_cellW, r * m_cellH, len * m_cellW, m_cellH);

                QPainterPath path;
                path.addRoundedRect(rect.adjusted(1, 1, -1, -1), 3, 3);
                p.fillPath(path, QColor("#9933ff"));

                p.fillRect(rect.right() - 6, rect.top() + 4, 3, rect.height() - 8, QColor(255, 255, 255, 120));
            }
        }
    }
}

void PianoRollGrid::mousePressEvent(QMouseEvent *event) {
    int c = event->pos().x() / m_cellW;
    int r = event->pos().y() / m_cellH;
    if (r < 0 || r >= 96 || c < 0 || c >= 16) return;

    int noteStartCol = -1;
    for (int i = c; i >= 0; --i) {
        if (m_pattern[r][i].active && i + m_pattern[r][i].length > c) {
            noteStartCol = i;
            break;
        }
    }

    if (noteStartCol != -1) {
        int noteRightEdge = (noteStartCol + m_pattern[r][noteStartCol].length) * m_cellW;
        if (event->pos().x() > noteRightEdge - 15) {
            m_action = Resize;
            m_editRow = r;
            m_editCol = noteStartCol;
        } else {
            m_pattern[r][noteStartCol].active = false;
            m_action = Erase;
            update();
        }
    } else {
        m_pattern[r][c].active = true;
        m_pattern[r][c].length = 1;
        m_action = Resize;
        m_editRow = r;
        m_editCol = c;
        update();
    }
}

void PianoRollGrid::mouseMoveEvent(QMouseEvent *event) {
    if (m_action == Resize && m_editRow != -1 && m_editCol != -1) {
        int currentC = event->pos().x() / m_cellW;
        int newLen = currentC - m_editCol + 1;

        if (m_editCol + newLen > 16) newLen = 16 - m_editCol;
        if (newLen < 1) newLen = 1;

        m_pattern[m_editRow][m_editCol].length = newLen;

        for (int i = m_editCol + 1; i < m_editCol + newLen; ++i) {
            m_pattern[m_editRow][i].active = false;
        }
        update();
    }
}

void PianoRollGrid::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    m_action = None;
    m_editRow = -1;
    m_editCol = -1;
}



PianoRollDialog::PianoRollDialog(NoteEvent (&pattern)[96][16], const QString& title, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(title);

    std::copy(&pattern[0][0], &pattern[0][0] + 96 * 16, &m_localPattern[0][0]);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(2);

    QPushButton* btnSave = new QPushButton("💾 Save Pattern & Close");
    btnSave->setStyleSheet("background-color: #7a00cc; color: white; padding: 10px; font-weight: bold; border-radius: 4px;");
    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnSave);

    QScrollArea* scrollArea = new QScrollArea();
    QWidget* container = new QWidget();
    QHBoxLayout* hLayout = new QHBoxLayout(container);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);

    QWidget* keyboardWidget = new QWidget();
    keyboardWidget->setFixedWidth(35);
    QVBoxLayout* keyLayout = new QVBoxLayout(keyboardWidget);
    keyLayout->setContentsMargins(0, 0, 0, 0);
    keyLayout->setSpacing(0);

    const QString noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    for (int r = 0; r < 96; ++r) {
        int midiNote = 107 - r;
        int octave = (midiNote / 12) - 1;
        int noteIndex = midiNote % 12;
        bool isBlackKey = (noteIndex == 1 || noteIndex == 3 || noteIndex == 6 || noteIndex == 8 || noteIndex == 10);

        QLabel* keyLabel = new QLabel(noteNames[noteIndex] + QString::number(octave));
        keyLabel->setFixedSize(35, 20);
        keyLabel->setAlignment(Qt::AlignCenter);

        if (isBlackKey) {
            keyLabel->setStyleSheet("background-color: #111; color: white; border-bottom: 1px solid #000; font-size: 9px;");
        } else {
            keyLabel->setStyleSheet("background-color: #eee; color: black; border-bottom: 1px solid #ccc; font-weight: bold; font-size: 9px;");
        }
        keyLayout->addWidget(keyLabel);
    }

    hLayout->addWidget(keyboardWidget);
    hLayout->addWidget(new PianoRollGrid(m_localPattern, this));

    scrollArea->setWidget(container);
    mainLayout->addWidget(scrollArea);

    setStyleSheet("QDialog { background-color: #1e1e1e; }");
    setWindowState(Qt::WindowMaximized);

    QTimer::singleShot(50, [scrollArea, container]() {
        int targetY = (47 * 20) - (scrollArea->viewport()->height() / 2);
        scrollArea->verticalScrollBar()->setValue(std::max(0, targetY));
    });
}

void PianoRollDialog::saveToGrid(NoteEvent (&pattern)[96][16]) {
    std::copy(&m_localPattern[0][0], &m_localPattern[0][0] + 96 * 16, &pattern[0][0]);
}