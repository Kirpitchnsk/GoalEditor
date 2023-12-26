#ifndef HELPINFO_H
#define HELPINFO_H

#include <QDialog>

namespace Ui {
class HelpInfo;
}

class HelpInfo : public QDialog
{
    Q_OBJECT

public:
    explicit HelpInfo(QWidget *parent = 0);
    ~HelpInfo();

private:
    Ui::HelpInfo *ui;
};

#endif // HELPINFO_H
