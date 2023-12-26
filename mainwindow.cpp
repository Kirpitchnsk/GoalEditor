#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "helpinfo.h"
#include "ui_helpinfo.h"
#include "goalmanager.h"
#include "achievementswindow.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QTextBrowser>
#include <QToolTip>
#include <QLabel>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentFilterStatus(RepairStatus::All)
{
    ui->setupUi(this);

    QFile styleFile(":/styles/stylesmain.css");
    styleFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleFile.readAll());
    this->setStyleSheet(styleSheet);

    loadWindowState();

    // Инициализация базы данных
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("goals.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть базу данных: " + db.lastError().text());
    } else {
        QSqlQuery query;
        if (!query.exec("CREATE TABLE IF NOT EXISTS goals (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, description TEXT, status INTEGER, start_date DATETIME, end_date DATETIME, photo BLOB)")) {
            qDebug() << "Ошибка при создании таблицы: " << query.lastError().text();
        }
    }

    // Добавление статусов в выпадающий список
    ui->comboBox->addItem("Все", RepairStatus::All);
    ui->comboBox->addItem("Ожидается", RepairStatus::Pending);
    ui->comboBox->addItem("В процессе", RepairStatus::InProgress);
    ui->comboBox->addItem("Завершено", RepairStatus::Completed);
    ui->comboBox->addItem("Просрочено", RepairStatus::Overdue);

    // Установка значения по умолчанию
    ui->comboBox->setCurrentIndex(0);

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_statusComboBox_currentIndexChanged);

    setFilterStatus(RepairStatus::All);

    updateOrderTable();

    QAction* helpAction = ui->menu->addAction("Справка");
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelpWindow);

    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::on_searchLineEdit_textChanged);

    // В конструкторе MainWindow:
    deadlineCheckTimer = new QTimer(this);
    connect(deadlineCheckTimer, &QTimer::timeout, this, &MainWindow::checkDeadline);
    deadlineCheckTimer->start(1000);  // Таймер каждую минуту (60000 миллисекунд)

}

MainWindow::~MainWindow()
{
    //saveWindowState();
    delete ui;
}

void MainWindow::on_submitButton_clicked()
{
    QString name = ui->nameEdit->text();
    QString description = ui->descriptionEdit->toPlainText();

    if (name.isEmpty() || description.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, заполните все поля корректно.");
        return;
    }


    QSqlQuery query;
    query.prepare("INSERT INTO goals (name, description, status, start_date, end_date) VALUES (:name, :description, :status, :start_date, :end_date)");
    query.bindValue(":name", name);
    query.bindValue(":description", description);
    query.bindValue(":status", RepairStatus::Pending);
    query.bindValue(":start_date", QDateTime::currentDateTime());
    query.bindValue(":end_date", QDateTime::currentDateTime());

    if (query.exec()) {
        QMessageBox::information(this, "Успех", "Цель добавлена успешно.");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить цель: " + query.lastError().text());
    }

    Goal newGoal(ui->nameEdit->text(), ui->descriptionEdit->toPlainText(), RepairStatus::Pending, QDateTime::currentDateTime(), QDateTime::currentDateTime());
    orderManager.addGoal(newGoal);

    updateOrderTable();

    ui->nameEdit->clear();
    ui->descriptionEdit->clear();
}

void MainWindow::updateOrderTable()
{
    ui->orderTable->clearContents();
    ui->orderTable->setRowCount(0);

    ui->orderTable->setSortingEnabled(true);

    QSqlQuery query;
    QString queryString = "SELECT * FROM goals";
    if (currentFilterStatus != RepairStatus::All) {
        queryString += QString(" WHERE status = %1").arg(currentFilterStatus);
    }
    
    if (query.exec(queryString)) {
        while (query.next()) {
            int rowPosition = ui->orderTable->rowCount();
            ui->orderTable->insertRow(rowPosition);

            QString name = query.value(1).toString();
            QString description = query.value(2).toString();
            int statusValue = query.value(3).toInt();
            QDateTime startDate = query.value(4).toDateTime();
            QDateTime endDate = query.value(5).toDateTime();

            QTableWidgetItem* itemName = new QTableWidgetItem(name);
            itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
            ui->orderTable->setItem(rowPosition, 0, itemName);

            QTableWidgetItem* itemDescription = new QTableWidgetItem(description);
            itemDescription->setFlags(itemDescription->flags() & ~Qt::ItemIsEditable);
            ui->orderTable->setItem(rowPosition, 1, itemDescription);

            setGoalStatus(rowPosition, RepairStatus::fromInt(statusValue), startDate, endDate);

            QTableWidgetItem* itemStartDate = new QTableWidgetItem(startDate.toString(Qt::ISODate));
            itemStartDate->setFlags(itemStartDate->flags() & ~Qt::ItemIsEditable);
            ui->orderTable->setItem(rowPosition, 3, itemStartDate);

            QTableWidgetItem* itemEndDate = new QTableWidgetItem(endDate.toString(Qt::ISODate));
            itemEndDate->setFlags(itemEndDate->flags() & ~Qt::ItemIsEditable);
            ui->orderTable->setItem(rowPosition, 4, itemEndDate);
        }
    } else {
        QMessageBox::critical(this, "Ошибка", "Ошибка при выполнении SQL-запроса: " + query.lastError().text());
    }
}

void MainWindow::on_orderTable_itemDoubleClicked(QTableWidgetItem *item)
{
    int row = item->row();
    openEditOrderDialog(row);
}

void MainWindow::openEditOrderDialog(int row)
{
    if (row < 0 || row >= ui->orderTable->rowCount()) {
        return;
    }

    QString name = ui->orderTable->item(row, 0)->text();
    QString description = ui->orderTable->item(row, 1)->text();
    RepairStatus goalStatus = getGoalStatus(row);
    QDateTime startDate = getGoalStartDate(row);
    QDateTime endDate = getGoalEndDate(row);


    EditOrderDialog editDialog;
    editDialog.setGoalData(name, description, goalStatus, startDate, endDate);

    if (editDialog.exec() == QDialog::Accepted) {
        QString editedName = editDialog.getName();
        QString editedDescription = editDialog.getDescription();
        RepairStatus editedStatus = editDialog.getStatus();
        QDateTime editedStartDate = editDialog.getStartDate();
        QDateTime editedEndDate = editDialog.getEndDate();
        QByteArray editedImageByteArray = editDialog.getImageByteArray();

        if (editDialog.isOrderMarkedForDeletion()) {
            QSqlQuery deleteQuery;
            deleteQuery.prepare("DELETE FROM goals WHERE name = :name");
            deleteQuery.bindValue(":name", name);

            if (!deleteQuery.exec()) {
                QMessageBox::critical(this, "Ошибка", "Не удалось удалить цель: " + deleteQuery.lastError().text());
            }

            updateOrderTable();
        } else {
            ui->orderTable->item(row, 0)->setText(editedName);
            ui->orderTable->item(row, 1)->setText(editedDescription);

            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE goals SET name = :name, description = :description, status = :status, start_date = :start_date, end_date = :end_date, photo = :photo WHERE name = :oldName");
            updateQuery.bindValue(":name", editedName);
            updateQuery.bindValue(":description", editedDescription);
            updateQuery.bindValue(":oldName", name);
            updateQuery.bindValue(":status", editedStatus.toInt());
            updateQuery.bindValue(":start_date", editedStartDate);
            updateQuery.bindValue(":end_date", editedEndDate);
            updateQuery.bindValue(":photo", editedImageByteArray);

            if (!updateQuery.exec()) {
                QMessageBox::critical(this, "Ошибка", "Не удалось обновить данные цели: " + updateQuery.lastError().text());
            }

            setGoalStatus(row, editedStatus, editedStartDate, editedEndDate);
        }
    }
}

void MainWindow::setGoalStatus(int row, const RepairStatus &status, const QDateTime &startDate, const QDateTime &endDate)
{
    QTableWidgetItem *item = ui->orderTable->item(row, 2);
    if (!item) {
        item = new QTableWidgetItem();
        ui->orderTable->setItem(row, 2, item);
    }

    item->setText(status.getStatusString());

    // Проверка на просроченный статус
    if (status == RepairStatus::InProgress && QDateTime::currentDateTime() > endDate) {
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE goals SET status = :status WHERE name = :name");
        updateQuery.bindValue(":status", RepairStatus::Overdue);
        updateQuery.bindValue(":name", ui->orderTable->item(row, 0)->text());

        if (!updateQuery.exec()) {
            qDebug() << "Ошибка при установке статуса 'Просрочено': " << updateQuery.lastError().text();
        }

        RepairStatus overdueStatus(RepairStatus::Overdue);
            item->setText(overdueStatus.getStatusString());
    }
}

RepairStatus MainWindow::getGoalStatus(int row) const
{
    QTableWidgetItem *item = ui->orderTable->item(row, 2);
    if (item) {
        QString statusString = item->text();
        if (statusString == "Ожидается") {
            return RepairStatus(RepairStatus::Pending);
        } else if (statusString == "В процессе") {
            return RepairStatus(RepairStatus::InProgress);
        } else if (statusString == "Завершено") {
            return RepairStatus(RepairStatus::Completed);
        } else if (statusString == "Просрочено") {
            return RepairStatus(RepairStatus::Overdue);
        }
    }

    return RepairStatus(RepairStatus::Pending);
}

QDateTime MainWindow::getGoalStartDate(int row) const
{
    QTableWidgetItem *item = ui->orderTable->item(row, 3);
    if (item && !item->text().isEmpty()) {
        return QDateTime::fromString(item->text(), Qt::ISODate);
    }
    return QDateTime();
}

QDateTime MainWindow::getGoalEndDate(int row) const
{
    QTableWidgetItem *item = ui->orderTable->item(row, 4);
    if (item && !item->text().isEmpty()) {
        return QDateTime::fromString(item->text(), Qt::ISODate);
    }
    return QDateTime();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveWindowState();
    QMainWindow::closeEvent(event);
}

void MainWindow::saveWindowState()
{
    QSettings settings("ThisIsTheWay", "GoalEditor");

    // Сохранение размера и положения окна
    settings.setValue("MainWindow/Size", size());
    settings.setValue("MainWindow/Position", pos());

    // Сохранение текущего выбранного статуса фильтра
    settings.setValue("MainWindow/FilterStatus", static_cast<int>(currentFilterStatus));

    // Получаем текущие размеры столбцов
    for (int col = 0; col < ui->orderTable->columnCount(); ++col) {
            int width = ui->orderTable->columnWidth(col);
            settings.setValue(QString("MainWindow/ColumnWidth_%1").arg(col), width);
    }
}

void MainWindow::loadWindowState()
{
    QSettings settings("ThisIsTheWay", "GoalEditor");

    // Загрузка размера и положения окна
    resize(settings.value("MainWindow/Size", QSize(800, 454)).toSize());
    move(settings.value("MainWindow/Position", QPoint(200, 200)).toPoint());

    // Загрузка последнего выбранного статуса фильтра
    int filterStatus = settings.value("MainWindow/FilterStatus", static_cast<int>(RepairStatus::All)).toInt();
    currentFilterStatus = static_cast<RepairStatus::Status>(filterStatus);

    // Применение загруженного статуса фильтра
    setFilterStatus(currentFilterStatus);

    // Восстанавливаем размеры столбцов
    for (int col = 0; col < ui->orderTable->columnCount(); ++col) {
            int width = settings.value(QString("MainWindow/ColumnWidth_%1").arg(col), -1).toInt();
            if (width != -1) {
                ui->orderTable->setColumnWidth(col, width);
            }
    }
}


void MainWindow::setFilterStatus(RepairStatus::Status status) {
    currentFilterStatus = status;
    if (status != RepairStatus::All) {
        ui->comboBox->setCurrentIndex(static_cast<int>(status));
    } else {
        ui->comboBox->setCurrentIndex(0);
    }
    updateOrderTable();
}

void MainWindow::on_statusComboBox_currentIndexChanged(int index) {
    Q_UNUSED(index);
    RepairStatus::Status selectedStatus = static_cast<RepairStatus::Status>(index);
    if (selectedStatus == RepairStatus::All) {
        setFilterStatus(RepairStatus::All);
    } else {
        setFilterStatus(selectedStatus);
    }
}

void MainWindow::showHelpWindow() {
    HelpInfo *helpWindow = new HelpInfo(this);
    helpWindow->show();
}

void MainWindow::on_searchLineEdit_textChanged(const QString &text)
{
    // При изменении текста в поле поиска, обновляем таблицу с учетом фильтрации
    filterTableBySearch(text);
}

void MainWindow::filterTableBySearch(const QString &text)
{
    // Очищаем текущий фильтр статуса
    setFilterStatus(RepairStatus::All);

    // Проходим по всем строкам таблицы и скрываем те, которые не соответствуют тексту поиска
    for (int row = 0; row < ui->orderTable->rowCount(); ++row)
    {
        QString goalName = ui->orderTable->item(row, 0)->text();
        bool isVisible = goalName.contains(text, Qt::CaseInsensitive);
        ui->orderTable->setRowHidden(row, !isVisible);
    }
}

void MainWindow::checkDeadline()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    GoalManager& goalManager = GoalManager::getInstance();

    for (const Goal& goal : goalManager.getGoals()) {
        if (goal.getStatus() == RepairStatus::InProgress && goal.getEndDate() < currentDateTime) {
            // Обновляем базу данных
            updateGoalStatusInDatabase(goal.getName(), goal.getStatus().toInt());

            // Уведомление пользователя
            QMessageBox::information(this, "Уведомление", "Цель '" + goal.getName() + "' просрочена!");
        }
    }
}

// В MainWindow.cpp, добавьте функцию для обновления статуса цели в базе данных:
void MainWindow::updateGoalStatusInDatabase(const QString& goalName, int status)
{
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE goals SET status = :status WHERE name = :name");
    updateQuery.bindValue(":status", status);
    updateQuery.bindValue(":name", goalName);
    if (!updateQuery.exec()) {
        qDebug() << "Ошибка при обновлении статуса цели в базе данных: " << updateQuery.lastError().text();
    }
    updateOrderTable();
}
