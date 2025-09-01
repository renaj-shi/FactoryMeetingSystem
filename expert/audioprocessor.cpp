#include "audioprocessor.h"
#include <QDebug>

AudioProcessor::AudioProcessor(QObject *parent)
    : QObject(parent)
    , audioInput(nullptr)
    , audioOutput(nullptr)
    , inputDevice(nullptr)
    , outputDevice(nullptr)
{
    // 设置音频格式
    audioFormat.setSampleRate(16000);          // 采样率
    audioFormat.setChannelCount(1);            // 单声道
    audioFormat.setSampleSize(16);             // 16位采样
    audioFormat.setCodec("audio/pcm");
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::SignedInt);
}

AudioProcessor::~AudioProcessor()
{
    stopCapture();
    if (audioInput) {
        delete audioInput;
    }
    if (audioOutput) {
        delete audioOutput;
    }
}

bool AudioProcessor::initialize()
{
    // 检查音频格式是否支持
    QAudioDeviceInfo inputDeviceInfo(QAudioDeviceInfo::defaultInputDevice());
    if (!inputDeviceInfo.isFormatSupported(audioFormat)) {
        qDebug() << "Default input format not supported, trying to use nearest format.";
        audioFormat = inputDeviceInfo.nearestFormat(audioFormat);
    }

    QAudioDeviceInfo outputDeviceInfo(QAudioDeviceInfo::defaultOutputDevice());
    if (!outputDeviceInfo.isFormatSupported(audioFormat)) {
        qDebug() << "Default output format not supported, trying to use nearest format.";
        audioFormat = outputDeviceInfo.nearestFormat(audioFormat);
    }

    // 创建音频输入输出对象
    audioInput = new QAudioInput(inputDeviceInfo, audioFormat, this);
    audioOutput = new QAudioOutput(outputDeviceInfo, audioFormat, this);

    return true;
}

void AudioProcessor::startCapture()
{
    if (!audioInput) return;

    inputDevice = audioInput->start();
    connect(inputDevice, &QIODevice::readyRead, this, &AudioProcessor::onAudioDataAvailable);
}

void AudioProcessor::stopCapture()
{
    if (audioInput) {
        audioInput->stop();
        inputDevice = nullptr;
    }
}

void AudioProcessor::onAudioDataAvailable()
{
    if (!inputDevice) return;

    QByteArray data = inputDevice->readAll();
    if (!data.isEmpty()) {
        emit audioDataReady(data);
    }
}

void AudioProcessor::playAudio(const QByteArray &audioData)
{
    if (!audioOutput) return;

    if (!outputDevice) {
        outputDevice = audioOutput->start();
    }

    if (outputDevice) {
        outputDevice->write(audioData);
    }
}