// ===================================================================
// [MOD] 文件名: chatmessagewidget.h
// [MOD] 内容: 无功能性改动，仅为保持代码同步。
// ===================================================================
#pragma once

#include <QWidget>

class QLabel;
class QFrame;

class ChatMessageWidget : public QWidget
{
    Q_OBJECT
public:
    enum Alignment {
        Self,
        Other,
        System
    };

    explicit ChatMessageWidget(QWidget *parent = nullptr);

    void setMessage(const QString &sender, const QString &text, const QString &timestamp, Alignment alignment);

private:
    QLabel *avatarLabel_{nullptr};
    QLabel *senderLabel_{nullptr};
    QLabel *messageLabel_{nullptr};
    QLabel *timestampLabel_{nullptr};
    QFrame *bubbleContainer_{nullptr};
};
