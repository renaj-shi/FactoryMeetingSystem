/****************************************************************************
** Meta object code from reading C++ file 'meetingroomdialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../meetingroomdialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'meetingroomdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MeetingRoomDialog_t {
    QByteArrayData data[13];
    char stringdata0[244];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MeetingRoomDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MeetingRoomDialog_t qt_meta_stringdata_MeetingRoomDialog = {
    {
QT_MOC_LITERAL(0, 0, 17), // "MeetingRoomDialog"
QT_MOC_LITERAL(1, 18, 13), // "meetingClosed"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 16), // "addSystemMessage"
QT_MOC_LITERAL(4, 50, 7), // "message"
QT_MOC_LITERAL(5, 58, 15), // "onNewConnection"
QT_MOC_LITERAL(6, 74, 17), // "onClientReadyRead"
QT_MOC_LITERAL(7, 92, 20), // "onClientDisconnected"
QT_MOC_LITERAL(8, 113, 21), // "on_sendButton_clicked"
QT_MOC_LITERAL(9, 135, 26), // "on_broadcastButton_clicked"
QT_MOC_LITERAL(10, 162, 21), // "on_kickButton_clicked"
QT_MOC_LITERAL(11, 184, 29), // "on_closeMeetingButton_clicked"
QT_MOC_LITERAL(12, 214, 29) // "on_startMeetingButton_clicked"

    },
    "MeetingRoomDialog\0meetingClosed\0\0"
    "addSystemMessage\0message\0onNewConnection\0"
    "onClientReadyRead\0onClientDisconnected\0"
    "on_sendButton_clicked\0on_broadcastButton_clicked\0"
    "on_kickButton_clicked\0"
    "on_closeMeetingButton_clicked\0"
    "on_startMeetingButton_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MeetingRoomDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   65,    2, 0x0a /* Public */,
       5,    0,   68,    2, 0x08 /* Private */,
       6,    0,   69,    2, 0x08 /* Private */,
       7,    0,   70,    2, 0x08 /* Private */,
       8,    0,   71,    2, 0x08 /* Private */,
       9,    0,   72,    2, 0x08 /* Private */,
      10,    0,   73,    2, 0x08 /* Private */,
      11,    0,   74,    2, 0x08 /* Private */,
      12,    0,   75,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MeetingRoomDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MeetingRoomDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->meetingClosed(); break;
        case 1: _t->addSystemMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->onNewConnection(); break;
        case 3: _t->onClientReadyRead(); break;
        case 4: _t->onClientDisconnected(); break;
        case 5: _t->on_sendButton_clicked(); break;
        case 6: _t->on_broadcastButton_clicked(); break;
        case 7: _t->on_kickButton_clicked(); break;
        case 8: _t->on_closeMeetingButton_clicked(); break;
        case 9: _t->on_startMeetingButton_clicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MeetingRoomDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MeetingRoomDialog::meetingClosed)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MeetingRoomDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_MeetingRoomDialog.data,
    qt_meta_data_MeetingRoomDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MeetingRoomDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MeetingRoomDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MeetingRoomDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int MeetingRoomDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void MeetingRoomDialog::meetingClosed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
