#include "paypage.h"
#include <QLabel>
#include <QVBoxLayout>

PayPage::PayPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *label = new QLabel("付款页面（待接入支付）");
    label->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label);
}
