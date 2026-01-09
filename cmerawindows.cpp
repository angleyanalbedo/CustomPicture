#include "cmerawindows.h"
#include "ui_cmerawindows.h"

cmeraWindows::cmeraWindows(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::cmeraWindows)
{
    ui->setupUi(this);
}

cmeraWindows::~cmeraWindows()
{
    delete ui;
}
