#ifndef PIANOROLL_H
#define PIANOROLL_H

#include <QDialog>
#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>

struct NoteEvent {
    bool active;
    int length;
    float velocity;

    NoteEvent() : active(false), length(1), velocity(1.0f) {}
};

class PianoRollGrid : public QWidget {
    Q_OBJECT
public:
    explicit PianoRollGrid(NoteEvent (&pattern)[96][16], QWidget *parent = nullptr);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    NoteEvent (&m_pattern)[96][16];
    int m_cellW = 40;
    int m_cellH = 20;

    enum Action { None, Draw, Resize, Erase } m_action = None;
    int m_editRow = -1;
    int m_editCol = -1;
};

class PianoRollDialog : public QDialog {
    Q_OBJECT
public:
    explicit PianoRollDialog(NoteEvent (&pattern)[96][16], const QString& title, QWidget *parent = nullptr);
    void saveToGrid(NoteEvent (&pattern)[96][16]);
private:
    NoteEvent m_localPattern[96][16];
};

#endif // PIANOROLL_H