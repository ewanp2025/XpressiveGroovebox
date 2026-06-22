#ifndef AUTOMATION_H
#define AUTOMATION_H

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QString>


enum class AutoTarget {
    None,
    MasterVolume,
    MasterDrive,
    Melodic1Volume,
    DrumKickVolume
};

struct AutomationTrack {
    QString name = "Macro 1";
    AutoTarget target = AutoTarget::MasterVolume;
    double steps[16];

    AutomationTrack() {
        for (int i = 0; i < 16; ++i) steps[i] = 0.5;
    }
};

class AutomationGrid : public QWidget {
    Q_OBJECT
public:
    explicit AutomationGrid(AutomationTrack& track, QWidget *parent = nullptr);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void automationChanged();

private:
    void updateValueFromMouse(QMouseEvent* event);
    AutomationTrack& m_track;
    bool m_isDragging = false;
};

#endif // AUTOMATION_H