#include "goal.h"

Goal::Goal() {}

Goal::Goal(const QString &name, const QString &description, const RepairStatus &status, const QDateTime &startDate, const QDateTime &endDate)
    : name(name), description(description), status(status), startDate(startDate), endDate(endDate) {}

QString Goal::getName() const
{
    return name;
}

QString Goal::getDescription() const
{
    return description;
}

RepairStatus Goal::getStatus() const
{
    return status;
}

QDateTime Goal::getStartDate() const
{
    return startDate;
}

QDateTime Goal::getEndDate() const
{
    return endDate;
}

void Goal::setStartDate(const QDateTime &startDate)
{
    this->startDate = startDate;
}

void Goal::setEndDate(const QDateTime &endDate)
{
    this->endDate = endDate;
}

QString Goal::getMedalImagePath() const
{
    // Замените на ваш путь к изображению медали для завершенных целей
    return ":/medal.png";
}
