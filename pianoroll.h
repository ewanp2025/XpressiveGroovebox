#ifndef PIANOROLL_H
#define PIANOROLL_H

#include <QDialog>
#include <QPushButton>


struct NoteEvent {
    bool active;
    int length;
    float velocity;
    
    NoteEvent() : active(false), length(1), velocity(1.0f) {}
};

class PianoRollDialog : public QDialog {
    Q_OBJECT
public:
    explicit PianoRollDialog(NoteEvent (&pattern)[96][16], const QString& title, QWidget *parent = nullptr);
    void saveToGrid(NoteEvent (&pattern)[96][16]);
private:
    QPushButton* m_btnGrid[96][16];
};

#endif // PIANOROLL_H