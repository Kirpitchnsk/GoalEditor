#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "goalmanager.h"
#include <QTableWidgetItem>
#include <QPixmap>
#include <QBuffer>
#include <QHelpEngine>
#include <QTimer>
#include "editorderdialog.h"
#include "repairstatus.h"
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void showHelpWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_submitButton_clicked();
    void on_orderTable_itemDoubleClicked(QTableWidgetItem *item);
    void on_statusComboBox_currentIndexChanged(int index);
    void on_searchLineEdit_textChanged(const QString &text);

private:
    Ui::MainWindow *ui;
    GoalManager orderManager;

    void updateOrderTable();
    void openEditOrderDialog(int row);

    void setGoalStatus(int row, const RepairStatus &status, const QDateTime &startDate, const QDateTime &endDate);
    RepairStatus getGoalStatus(int row) const;
    QDateTime getGoalStartDate(int row) const;
    QDateTime getGoalEndDate(int row) const;

    void saveWindowState();
    void loadWindowState();

    enum class TableColumn {
        Name,
        Description,
        Status,
        StartDate,
        EndDate
    };
    
    RepairStatus::Status currentFilterStatus;
    
    void setFilterStatus(RepairStatus::Status status);
    void filterTableBySearch(const QString &text);

    QTimer* deadlineCheckTimer;

    void checkDeadline();
    void updateGoalStatusInDatabase(const QString& goalName, int status);

};

#endif // MAINWINDOW_H

