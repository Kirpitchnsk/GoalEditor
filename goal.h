#ifndef GOAL_H
#define GOAL_H

#include <QString>
#include <QDateTime>
#include "repairstatus.h"

class Goal
{
public:
    Goal();
    Goal(const QString &name, const QString &description, const RepairStatus &status, const QDateTime &startDate = QDateTime(), const QDateTime &endDate = QDateTime());

    QString getName() const;
    QString getDescription() const;
    RepairStatus getStatus() const;
    QDateTime getStartDate() const;
    QDateTime getEndDate() const;

    void setStartDate(const QDateTime &startDate);
    void setEndDate(const QDateTime &endDate);

    QString getMedalImagePath() const;

private:
    QString name;
    QString description;
    RepairStatus status;
    QDateTime startDate;
    QDateTime endDate;
};

#endif // GOAL_H
