#include "goalmanager.h"
#include "repairstatus.h"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>

void GoalManager::addGoal(const Goal &goal)
{
    goals.push_back(goal);
}

const QVector<Goal>& GoalManager::getGoals() const
{
    return goals;
}

GoalManager& GoalManager::getInstance()
{
    static GoalManager instance;  // Создаем единственный экземпляр при первом вызове
    return instance;
}

QVector<Goal>& GoalManager::getCompletedGoals() const
{
    QVector<Goal> completedGoals;

    return completedGoals;
}
