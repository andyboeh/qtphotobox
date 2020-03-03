#include "statemachine.h"

StateMachine &
StateMachine::getInstance() {
    static StateMachine instance;
    return instance;
}

StateMachine::StateMachine()
{
    mStates.clear();
    mNextStateMapping.clear();
    mCurrentState = "";
}

StateMachine::~StateMachine()
{

}

void StateMachine::addState(QString name)
{
    if(!mStates.contains(name)) {
        mStates.append(name);
    }
}

void StateMachine::removeState(QString name)
{
    if(mStates.contains(name)) {
        mStates.removeAll(name);
    }
    if(mNextStateMapping.contains(name)) {
        mNextStateMapping.remove(name);
    }
    foreach(QString key, mNextStateMapping.keys()) {
        if(mNextStateMapping.value(key) == name) {
            mNextStateMapping.remove(key);
        }
    }
}

void StateMachine::addTargetState(QString initialState, QString targetState)
{
    if(mStates.contains(initialState) && mStates.contains(targetState)) {
        mNextStateMapping.insert(initialState, targetState);
    }
}

QString StateMachine::getCurrentState()
{
    return mCurrentState;
}

void StateMachine::triggerState(QString name)
{
    if(mStates.contains(name)) {
        mCurrentState = name;
        emit performStateChange(name);
    }
}

void StateMachine::triggerNextState(QString name)
{
    if(mStates.contains(name) && mNextStateMapping.contains(name)) {
        QString targetState = mNextStateMapping.value(name);
        mCurrentState = targetState;
        emit performStateChange(targetState);
    }
}

void StateMachine::triggerNextState()
{
    triggerNextState(mCurrentState);
}


