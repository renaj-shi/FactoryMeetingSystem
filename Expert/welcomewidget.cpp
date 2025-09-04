#include "welcomewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QVariant> // 新增：包含 QVariant 类的完整定义

WelcomeWidget::WelcomeWidget(QWidget* parent) : QWidget(parent) {
    // [ADD] 统一浅色样式（不使用图片）
    setStyleSheet(
        "QWidget{background:#f5f7fb;}"
        "QFrame#card{background:#ffffff;border:1px solid #e6e9ef;border-radius:10px;}"
        "QPushButton{padding:8px 18px;border-radius:6px;background:#2f80ed;color:white;}"
        "QPushButton:hover{background:#3b86f6;}"
        "QLabel.subtitle{color:#555;}"
        "QLabel.conn{color:#777;}"
        );

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(36, 36, 36, 36);

    auto* card = new QFrame(this);
    card->setObjectName("card");
    auto* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 35));
    card->setGraphicsEffect(shadow);

    auto* cardLy = new QVBoxLayout(card);
    cardLy->setContentsMargins(32, 28, 32, 28);
    cardLy->setSpacing(12);

    title_ = new QLabel(tr("工业现场远程专家支持系统 · 专家端"), card);
    QFont tf = title_->font(); tf.setPointSize(tf.pointSize()+6); tf.setBold(true);
    title_->setFont(tf);
    cardLy->addWidget(title_);

    subtitle_ = new QLabel(tr("远程协作 | 实时设备状态 | 全程安全审计"), card);
    subtitle_->setObjectName("subtitle");
    subtitle_->setProperty("class", QString("subtitle"));
    QFont sf = subtitle_->font(); sf.setPointSize(sf.pointSize()+1);
    subtitle_->setFont(sf);
    cardLy->addWidget(subtitle_);

    connLabel_ = new QLabel(tr("连接状态：未连接服务器"), card);
    connLabel_->setObjectName("conn");
    connLabel_->setProperty("class", QString("conn"));
    cardLy->addWidget(connLabel_);

    cardLy->addSpacing(6);

    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    loginBtn_ = new QPushButton(tr("立即登录"), card);
    btnRow->addWidget(loginBtn_);
    cardLy->addLayout(btnRow);

    root->addStretch();
    root->addWidget(card);
    root->addStretch();

    connect(loginBtn_, &QPushButton::clicked, this, &WelcomeWidget::loginClicked);
}

void WelcomeWidget::setConnectionText(const QString& text) {
    connLabel_->setText(text);
}
