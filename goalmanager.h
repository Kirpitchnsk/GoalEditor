#ifndef GOALMANAGER_H
#define GOALMANAGER_H

#include "goal.h"
#include <QVector>

class GoalManager
{
public:
    void addGoal(const Goal &goal);
    const QVector<Goal>& getGoals() const;
     static GoalManager& getInstance();
     QVector<Goal>& getCompletedGoals() const;

private:
    QVector<Goal> goals;

};

#endif // GOALMANAGER_H
