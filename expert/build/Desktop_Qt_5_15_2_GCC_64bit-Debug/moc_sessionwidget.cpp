/****************************************************************************
** Meta object code from reading C++ file 'sessionwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../sessionwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sessionwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SessionWidget_t {
    QByteArrayData data[20];
    char stringdata0[170];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SessionWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SessionWidget_t qt_meta_stringdata_SessionWidget = {
    {
QT_MOC_LITERAL(0, 0, 13), // "SessionWidget"
QT_MOC_LITERAL(1, 14, 10), // "setOrderId"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 2), // "id"
QT_MOC_LITERAL(4, 29, 13), // "onJoinClicked"
QT_MOC_LITERAL(5, 43, 14), // "onLeaveClicked"
QT_MOC_LITERAL(6, 58, 8), // "onJoined"
QT_MOC_LITERAL(7, 67, 7), // "orderId"
QT_MOC_LITERAL(8, 75, 12), // "onJoinFailed"
QT_MOC_LITERAL(9, 88, 6), // "reason"
QT_MOC_LITERAL(10, 95, 6), // "onLeft"
QT_MOC_LITERAL(11, 102, 9), // "onMetrics"
QT_MOC_LITERAL(12, 112, 7), // "metrics"
QT_MOC_LITERAL(13, 120, 9), // "onLogLine"
QT_MOC_LITERAL(14, 130, 4), // "line"
QT_MOC_LITERAL(15, 135, 7), // "onFault"
QT_MOC_LITERAL(16, 143, 4), // "code"
QT_MOC_LITERAL(17, 148, 4), // "text"
QT_MOC_LITERAL(18, 153, 5), // "level"
QT_MOC_LITERAL(19, 159, 10) // "onSendChat"

    },
    "SessionWidget\0setOrderId\0\0id\0onJoinClicked\0"
    "onLeaveClicked\0onJoined\0orderId\0"
    "onJoinFailed\0reason\0onLeft\0onMetrics\0"
    "metrics\0onLogLine\0line\0onFault\0code\0"
    "text\0level\0onSendChat"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SessionWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   64,    2, 0x0a /* Public */,
       4,    0,   67,    2, 0x08 /* Private */,
       5,    0,   68,    2, 0x08 /* Private */,
       6,    1,   69,    2, 0x08 /* Private */,
       8,    1,   72,    2, 0x08 /* Private */,
      10,    1,   75,    2, 0x08 /* Private */,
      11,    2,   78,    2, 0x08 /* Private */,
      13,    2,   83,    2, 0x08 /* Private */,
      15,    4,   88,    2, 0x08 /* Private */,
      19,    0,   97,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString, QMetaType::QJsonObject,    7,   12,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    7,   14,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString,    7,   16,   17,   18,
    QMetaType::Void,

       0        // eod
};

void SessionWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SessionWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setOrderId((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->onJoinClicked(); break;
        case 2: _t->onLeaveClicked(); break;
        case 3: _t->onJoined((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->onJoinFailed((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->onLeft((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->onMetrics((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QJsonObject(*)>(_a[2]))); break;
        case 7: _t->onLogLine((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 8: _t->onFault((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4]))); break;
        case 9: _t->onSendChat(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SessionWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_SessionWidget.data,
    qt_meta_data_SessionWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SessionWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SessionWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SessionWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SessionWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
