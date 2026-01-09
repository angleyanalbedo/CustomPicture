#ifndef CMERAWINDOWS_H
#define CMERAWINDOWS_H

#include <QMainWindow>

namespace Ui {
class cmeraWindows;
}

class cmeraWindows : public QMainWindow
{
    Q_OBJECT

public:
    explicit cmeraWindows(QWidget *parent = nullptr);
    ~cmeraWindows();

private:
    Ui::cmeraWindows *ui;
};

#endif // CMERAWINDOWS_H
