#ifndef ACHIEVEMENTSWINDOW_H
#define ACHIEVEMENTSWINDOW_H

#include "goal.h"

#include <QDialog>

namespace Ui {
class AchievementsWindow;
}

class AchievementsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AchievementsWindow(QWidget *parent = 0);
    ~AchievementsWindow();


private:
    Ui::AchievementsWindow *ui;
};

#endif // ACHIEVEMENTSWINDOW_H
