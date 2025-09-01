#ifndef AUDIOPROCESSOR_H
#define AUDIOPROCESSOR_H

#include <QObject>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QIODevice>
#include <QByteArray>

class AudioProcessor : public QObject
{
    Q_OBJECT

public:
    explicit AudioProcessor(QObject *parent = nullptr);
    ~AudioProcessor();

    bool initialize();
    void startCapture();
    void stopCapture();
    void playAudio(const QByteArray &audioData);

signals:
    void audioDataReady(const QByteArray &audioData);

private slots:
    void onAudioDataAvailable();

private:
    QAudioFormat audioFormat;
    QAudioInput *audioInput;
    QAudioOutput *audioOutput;
    QIODevice *inputDevice;
    QIODevice *outputDevice;
    QByteArray audioBuffer;
};

#endif // AUDIOPROCESSOR_H