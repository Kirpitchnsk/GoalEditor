#include "editorderdialog.h"
#include "ui_editorderdialog.h"
#include <QMessageBox>
#include "repairstatus.h"
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QBuffer>

EditOrderDialog::EditOrderDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::EditOrderDialog),
      orderMarkedForDeletion(false)
{
    ui->setupUi(this);

    QFile styleFile(":/styles/stylesgoal.css");
    styleFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleFile.readAll());
    this->setStyleSheet(styleSheet);

    connect(ui->saveButton, &QPushButton::clicked, this, &EditOrderDialog::on_saveButton_clicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &EditOrderDialog::on_deleteButton_clicked);
    connect(ui->loadPhotoButton, &QPushButton::clicked, this, &EditOrderDialog::loadPhoto);

    imagePath = "";
    photoPixmap = QPixmap();
    ui->photoLabel->clear();
    imageByteArray.clear();
}

EditOrderDialog::~EditOrderDialog()
{
    delete ui;
}

void EditOrderDialog::setGoalData(const QString &name, const QString &description, const RepairStatus &status, const QDateTime &startDate, const QDateTime &endDate)
{
    ui->nameEdit->setText(name);
    ui->descriptionEdit->setText(description);
    ui->statusComboBox->setCurrentText(status.getStatusString());
    ui->startDateEdit->setDateTime(startDate);
    ui->endDateEdit->setDateTime(endDate);


    QSqlQuery query;
    query.prepare("SELECT photo FROM goals WHERE name = :name");
    query.bindValue(":name", name);

    if (query.exec() && query.first()) {
        QByteArray imageByteArray = query.value(0).toByteArray();
        QPixmap photoPixmap;
        photoPixmap.loadFromData(imageByteArray);
        ui->photoLabel->setPixmap(photoPixmap);
        ui->photoLabel->setScaledContents(true);
    } else {

    }
}
QString EditOrderDialog::getName() const
{
    return ui->nameEdit->text();
}

QString EditOrderDialog::getDescription() const
{
    return ui->descriptionEdit->toPlainText();
}

RepairStatus EditOrderDialog::getStatus() const
{
    QString statusString = ui->statusComboBox->currentText();
    RepairStatus::Status status;

    if (statusString == "Ожидается") {
        status = RepairStatus::Pending;
    } else if (statusString == "В процессе") {
        status = RepairStatus::InProgress;
    } else if (statusString == "Завершено") {
        status = RepairStatus::Completed;
    } else {
        status = RepairStatus::Pending;  // По умолчанию
    }

    return RepairStatus(status);
}

QDateTime EditOrderDialog::getStartDate() const
{
    return ui->startDateEdit->dateTime();
}

QDateTime EditOrderDialog::getEndDate() const
{
    return ui->endDateEdit->dateTime();
}

bool EditOrderDialog::isOrderMarkedForDeletion() const
{
    return orderMarkedForDeletion;
}

void EditOrderDialog::on_deleteButton_clicked()
{
    orderMarkedForDeletion = true;
    accept();
}

// Функция загрузки фотографии
void EditOrderDialog::loadPhoto()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Выберите фото"), "", tr("Изображения (*.png *.jpg *.jpeg *.bmp *.gif)"));

    if (!fileName.isEmpty()) {
        imagePath = fileName;

        if (photoPixmap.load(imagePath)) {
            ui->photoLabel->setPixmap(photoPixmap);
            ui->photoLabel->setScaledContents(true);

            // Преобразование изображения в массив байт для сохранения в базе данных
            QBuffer buffer(&imageByteArray);
            buffer.open(QIODevice::WriteOnly);
            photoPixmap.save(&buffer, "PNG");
        } else {

        }
    }
}

// Метод для получения данных изображения в виде массива байт
QByteArray EditOrderDialog::getImageByteArray() const
{
    return imageByteArray;
}

void EditOrderDialog::on_saveButton_clicked()
{
    QString name = ui->nameEdit->text();
    QString description = ui->descriptionEdit->toPlainText();

    if (name.isEmpty() || description.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, заполните все поля корректно.");
        return;
    }

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDateTime startDate = ui->startDateEdit->dateTime();
    QDateTime endDate = ui->endDateEdit->dateTime();

    // Проверка на корректность введенных дат
    if (startDate > endDate) {
        QMessageBox::warning(this, "Ошибка", "Дата начала не может быть позже даты завершения.");
        return;
    }

    // Проверка на просроченную дату начала
    if (endDate < currentDateTime) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение", "Вы ввели дату начала, которая уже прошла. Вы уверены в своем выборе?",
                                                                  QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }

    // Закрытие диалогового окна после успешного сохранения
    accept();
}
