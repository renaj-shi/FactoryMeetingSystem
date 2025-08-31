#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QString>
#include <QList>

// 设备状态枚举
enum class DeviceStatus {
    NORMAL,
    ABNORMAL,
    DISCONNECTED
};

// 设备参数类
class DeviceParameter {
public:
    DeviceParameter(const QString &name, double currentValue, const QString &unit,
                   double minValue, double maxValue);
    
    // 获取参数信息
    QString name() const { return m_name; }
    double currentValue() const { return m_currentValue; }
    QString unit() const { return m_unit; }
    double minValue() const { return m_minValue; }
    double maxValue() const { return m_maxValue; }
    
    // 检查参数值是否在正常范围内（使用Qt的浮点数比较函数避免精度问题）
    bool isNormal() const {
        // 使用Qt的qFuzzyCompare函数进行浮点数比较，处理精度问题
        return (qFuzzyCompare(m_currentValue + 1.0, m_minValue + 1.0) || m_currentValue > m_minValue) &&
               (qFuzzyCompare(m_currentValue + 1.0, m_maxValue + 1.0) || m_currentValue < m_maxValue);
    }
    
    // 更新参数值
    void setCurrentValue(double value) { m_currentValue = value; }
    
private:
    QString m_name;            // 参数名称
    double m_currentValue;     // 当前值
    QString m_unit;            // 单位
    double m_minValue;         // 最小值（正常范围）
    double m_maxValue;         // 最大值（正常范围）
};

// 设备信息类
class DeviceInfo {
public:
    DeviceInfo(const QString &deviceId, const QString &deviceName, DeviceStatus status = DeviceStatus::NORMAL);
    
    // 添加参数
    void addParameter(const DeviceParameter &parameter);
    
    // 更新所有参数
    void updateAllParameters();
    
    // 获取设备信息
    QString deviceId() const { return m_deviceId; }
    QString deviceName() const { return m_deviceName; }
    DeviceStatus status() const { return m_status; }
    
    // 设置状态
    void setStatus(DeviceStatus status) { m_status = status; }
    
    // 获取参数列表（返回const引用避免拷贝）
    const QList<DeviceParameter>& parameters() const { return m_parameters; }
    
    // 获取参数列表（非常量版本，允许修改）
    QList<DeviceParameter>& parameters() { return m_parameters; }
    
    // 更新特定参数的值
    void updateParameter(const QString &parameterName, double value);
    
private:
    QString m_deviceId;                   // 设备ID
    QString m_deviceName;                 // 设备名称
    DeviceStatus m_status;                // 设备状态
    QList<DeviceParameter> m_parameters;  // 参数列表
};

// 设备管理器类
class DeviceManager {
public:
    DeviceManager();
    ~DeviceManager() = default; // 不需要手动释放内存，使用默认析构函数
    
    // 禁止拷贝和移动
    DeviceManager(const DeviceManager&) = delete;
    DeviceManager& operator=(const DeviceManager&) = delete;
    DeviceManager(DeviceManager&&) = delete;
    DeviceManager& operator=(DeviceManager&&) = delete;
    
    // 创建默认设备
    void createDefaultDevices();
    
    // 获取所有设备（返回const引用避免拷贝）
    const QList<DeviceInfo>& devices() const { return m_devices; }
    
    // 获取所有设备（非常量版本，允许修改）
    QList<DeviceInfo>& devices() { return m_devices; }
    
    // 更新所有设备参数
    void updateAllDevices();
    
    // 获取特定设备（返回引用）
    DeviceInfo* getDevice(const QString &deviceId);
    
    // 添加设备（使用移动语义避免拷贝）
    void addDevice(DeviceInfo device);
    
private:
    QList<DeviceInfo> m_devices; // 设备列表，使用值类型管理
};

#endif // DEVICEINFO_H