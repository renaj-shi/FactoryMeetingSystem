#include "deviceinfo.h"
#include <QRandomGenerator>
#include <QTime>
#include <QtMath>

// 设备参数类实现
DeviceParameter::DeviceParameter(const QString &name, double currentValue, const QString &unit,
                               double minValue, double maxValue)
    : m_name(name),
      m_currentValue(currentValue),
      m_unit(unit),
      m_minValue(minValue),
      m_maxValue(maxValue)
{}

// 设备信息类实现
DeviceInfo::DeviceInfo(const QString &deviceId, const QString &deviceName, DeviceStatus status)
    : m_deviceId(deviceId),
      m_deviceName(deviceName),
      m_status(status)
{}

void DeviceInfo::addParameter(const DeviceParameter &parameter)
{
    m_parameters.append(parameter);
}

void DeviceInfo::updateAllParameters()
{
    // 设备参数现在由服务端提供，此函数保留但不执行任何操作
}

void DeviceInfo::updateParameter(const QString &parameterName, double value)
{
    for (int i = 0; i < m_parameters.size(); ++i) {
        if (m_parameters[i].name() == parameterName) {
            // 创建一个新的参数对象来替换旧的
            DeviceParameter updatedParam(
                parameterName,
                value,
                m_parameters[i].unit(),
                m_parameters[i].minValue(),
                m_parameters[i].maxValue()
            );
            m_parameters[i] = updatedParam;
            break;
        }
    }
}

// parameters() 方法已在内联中定义，此处不再重复实现

// 设备管理器类实现
DeviceManager::DeviceManager()
{
    // 创建一个默认设备作为占位符，确保UI上始终有设备显示
    DeviceInfo defaultDevice("device_001", "设备001", DeviceStatus::NORMAL);
    
    // 添加默认参数
    defaultDevice.addParameter(DeviceParameter("温度", 0.0, "°C", 20.0, 70.0));
    defaultDevice.addParameter(DeviceParameter("压力", 0.0, "kPa", 80.0, 130.0));
    defaultDevice.addParameter(DeviceParameter("振动", 0.0, "mm/s", 0.1, 4.0));
    defaultDevice.addParameter(DeviceParameter("电流", 0.0, "A", 3.0, 20.0));
    defaultDevice.addParameter(DeviceParameter("电压", 0.0, "V", 200.0, 240.0));
    defaultDevice.addParameter(DeviceParameter("转速", 0.0, "RPM", 1300.0, 1700.0));
    
    m_devices.append(std::move(defaultDevice));
}

void DeviceManager::createDefaultDevices()
{
    // 设备列表现在由服务端提供，此函数保留但不执行任何操作
}

void DeviceManager::updateAllDevices()
{
    // 设备参数现在由服务端提供，此函数保留但不执行任何操作
}

// 注意：devices()方法已经在头文件中以内联方式定义为返回引用
// 这里不需要再实现，避免重定义错误

DeviceInfo* DeviceManager::getDevice(const QString &deviceId)
{
    for (auto& device : m_devices) {
        if (device.deviceId() == deviceId) {
            return &device;
        }
    }
    return nullptr;
}

void DeviceManager::addDevice(DeviceInfo device)
{
    // 检查设备是否已存在
    DeviceInfo* existingDevice = getDevice(device.deviceId());
    if (existingDevice) {
        // 如果设备已存在，更新其信息
        existingDevice->setStatus(device.status());
        return;
    }
    
    // 使用移动语义添加新设备到列表，避免不必要的拷贝
    m_devices.append(std::move(device));
}
