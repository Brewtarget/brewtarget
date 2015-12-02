#ifndef BOILTIME_H
#define BOILTIME_H

#include <QObject>

class BoilTime : public QObject
{
    Q_OBJECT
public:
    BoilTime(QObject * parent);
    BoilTime(QObject * parent, bool start, int boilTime);
    void setBoilStarted(bool start);
    void setBoilTime(int boilTime);
    int getTime();
    bool isStarted();
    void decrementTime();

signals:
    void BoilTimeChanged();

private:
    unsigned int time;
    bool started;
};

#endif // BOILTIME_H
