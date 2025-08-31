#pragma once
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>

class TcpClient;

class KnowledgeClient : public QObject {
    Q_OBJECT
public:
    explicit KnowledgeClient(TcpClient* tcp, QObject* parent=nullptr);
    void list(int page=1, int pageSize=50);
    void get(int id);

signals:
    void listed(const QJsonArray& rows);
    void listFailed(const QString& reason);
    void got(int id, const QJsonObject& item);
    void getFailed(const QString& reason);

private slots:
    void onLine(const QString& line);

private:
    TcpClient* tcp_{nullptr};
};
