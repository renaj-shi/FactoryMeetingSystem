// ===================================================================
// [MOD] 文件名: chatmessagewidget.cpp
// [MOD] 内容: 1. [FIX] 彻底重构了内部布局，使用一个 bubbleContainer 来包裹文本内容。
// [MOD]       2. [FIX] 通过动态添加和移除 Spacer 来实现左右对齐。
// [MOD]       3. [FIX] 移除了所有不必要的尺寸计算代码，让布局系统自动工作。
// ===================================================================
#include "chatmessagewidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QVariant>
#include <QStyle>
#include <QFrame>

ChatMessageWidget::ChatMessageWidget(QWidget *parent) : QWidget(parent)
{
    avatarLabel_ = new QLabel(this);
    senderLabel_ = new QLabel(this);
    messageLabel_ = new QLabel(this);
    timestampLabel_ = new QLabel(this);
    bubbleContainer_ = new QFrame(this);
    bubbleContainer_->setFrameShape(QFrame::NoFrame);
    bubbleContainer_->setAttribute(Qt::WA_StyledBackground, true);

    avatarLabel_->setObjectName("avatarLabel");
    senderLabel_->setObjectName("senderLabel");
    messageLabel_->setObjectName("messageLabel");
    timestampLabel_->setObjectName("timestampLabel");
    bubbleContainer_->setObjectName("bubbleContainer");

    messageLabel_->setWordWrap(true);
    messageLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    messageLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // --- 新布局 ---
    auto *textLayout = new QVBoxLayout(bubbleContainer_);
    textLayout->addWidget(senderLabel_);
    textLayout->addWidget(messageLabel_);
    textLayout->addWidget(timestampLabel_);
    textLayout->setSpacing(2);
    textLayout->setContentsMargins(10, 8, 10, 8);

    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(10, 5, 10, 5);
    rootLayout->setSpacing(10);
}

void ChatMessageWidget::setMessage(const QString &sender, const QString &text, const QString &timestamp, Alignment alignment)
{
    senderLabel_->setText(sender);
    messageLabel_->setText(text);
    timestampLabel_->setText(timestamp);

    // 新增：兜底让标签背景透明，避免覆盖气泡背景
    messageLabel_->setStyleSheet("background-color: transparent;");
    senderLabel_->setStyleSheet("background-color: transparent;");
    timestampLabel_->setStyleSheet("background-color: transparent;");

    // 清理旧的 spacer（你原有代码）
    QHBoxLayout* layout = static_cast<QHBoxLayout*>(this->layout());
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->spacerItem()) delete item;
    }

    if (alignment == Self || alignment == Other) {
        avatarLabel_->setText(sender.isEmpty() ? "?" : sender.left(1));

        this->setProperty("alignment", QString(alignment == Self ? "self" : "other"));

        // 新增：Self 时不显示 senderLabel，Other 时显示
        senderLabel_->setVisible(alignment != Self);

        if (alignment == Self) {
            senderLabel_->setAlignment(Qt::AlignRight);
            timestampLabel_->setAlignment(Qt::AlignRight);
            layout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
            layout->addWidget(bubbleContainer_);
            layout->addWidget(avatarLabel_);
        } else {
            senderLabel_->setAlignment(Qt::AlignLeft);
            timestampLabel_->setAlignment(Qt::AlignLeft);
            layout->addWidget(avatarLabel_);
            layout->addWidget(bubbleContainer_);
            layout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        }
    } else {
        this->setProperty("alignment", QString("system"));
        layout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        layout->addWidget(messageLabel_);
        layout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    }

    this->style()->unpolish(this);
    this->style()->polish(this);

}
