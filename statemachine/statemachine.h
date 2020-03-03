#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

#include <QObject>
#include <QString>
#include <QMap>

class StateMachine : public QObject
{
    Q_OBJECT
public:
    static StateMachine& getInstance();

    void addState(QString name);
    void removeState(QString name);

    void addTargetState(QString initialState, QString targetState);

    QString getCurrentState(void);

public slots:
    void triggerState(QString name);
    void triggerNextState(QString name);
    void triggerNextState(void);
signals:
    void performStateChange(QString name);
private:
    Q_DISABLE_COPY(StateMachine);
    StateMachine();
    ~StateMachine();
    QString mCurrentState;
    QMap<QString, QString> mNextStateMapping;
    QStringList mStates;
};

#endif //_STATEMACHINE_H
