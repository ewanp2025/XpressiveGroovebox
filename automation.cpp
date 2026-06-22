#include "automation.h"
#include <QPainterPath>

AutomationGrid::AutomationGrid(AutomationTrack& track, QWidget *parent)
    : QWidget(parent), m_track(track) {
    setMinimumHeight(60);
    setMouseTracking(true);
}

QSize AutomationGrid::sizeHint() const {
    return QSize(16 * 30, 80);
}

void AutomationGrid::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int stepW = width() / 16;
    int h = height();


    p.fillRect(rect(), QColor(25, 25, 25));


    for (int i = 0; i < 16; ++i) {
        QRect stepRect(i * stepW, 0, stepW, h);


        p.setPen((i % 4 == 0) ? QColor(80, 80, 80) : QColor(45, 45, 45));
        p.drawLine(stepRect.topLeft(), stepRect.bottomLeft());


        double val = m_track.steps[i];
        int barH = static_cast<int>(val * h);
        QRect barRect(i * stepW + 1, h - barH, stepW - 2, barH);


        p.fillRect(barRect, QColor(0, 200, 100, 180));


        p.setBrush(QColor(255, 255, 255));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPoint(i * stepW + stepW / 2, h - barH), 3, 3);
    }
}

void AutomationGrid::updateValueFromMouse(QMouseEvent* event) {
    int stepW = width() / 16;
    if (stepW <= 0) return;

    int step = event->pos().x() / stepW;
    step = std::max(0, std::min(step, 15));


    double val = 1.0 - (static_cast<double>(event->pos().y()) / height());
    val = std::max(0.0, std::min(val, 1.0));

    m_track.steps[step] = val;
    update();
    emit automationChanged();
}

void AutomationGrid::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        updateValueFromMouse(event);
    }
}

void AutomationGrid::mouseMoveEvent(QMouseEvent *event) {
    if (m_isDragging) {
        updateValueFromMouse(event);
    }
}

void AutomationGrid::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
    }
}