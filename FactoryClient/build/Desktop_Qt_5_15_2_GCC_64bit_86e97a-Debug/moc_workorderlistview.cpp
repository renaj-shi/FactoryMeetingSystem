/****************************************************************************
** Meta object code from reading C++ file 'workorderlistview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../workorderlistview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'workorderlistview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_WorkOrderListView_t {
    QByteArrayData data[18];
    char stringdata0[310];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WorkOrderListView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WorkOrderListView_t qt_meta_stringdata_WorkOrderListView = {
    {
QT_MOC_LITERAL(0, 0, 17), // "WorkOrderListView"
QT_MOC_LITERAL(1, 18, 18), // "createNewWorkOrder"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 13), // "editWorkOrder"
QT_MOC_LITERAL(4, 52, 10), // "WorkOrder*"
QT_MOC_LITERAL(5, 63, 9), // "workOrder"
QT_MOC_LITERAL(6, 73, 20), // "viewWorkOrderDetails"
QT_MOC_LITERAL(7, 94, 26), // "on_createNewButton_clicked"
QT_MOC_LITERAL(8, 121, 16), // "on_editWorkOrder"
QT_MOC_LITERAL(9, 138, 23), // "on_viewWorkOrderDetails"
QT_MOC_LITERAL(10, 162, 3), // "row"
QT_MOC_LITERAL(11, 166, 6), // "column"
QT_MOC_LITERAL(12, 173, 18), // "onWorkOrderCreated"
QT_MOC_LITERAL(13, 192, 18), // "onWorkOrderUpdated"
QT_MOC_LITERAL(14, 211, 43), // "on_filterStatusComboBox_curre..."
QT_MOC_LITERAL(15, 255, 5), // "index"
QT_MOC_LITERAL(16, 261, 23), // "on_searchButton_clicked"
QT_MOC_LITERAL(17, 285, 24) // "on_refreshButton_clicked"

    },
    "WorkOrderListView\0createNewWorkOrder\0"
    "\0editWorkOrder\0WorkOrder*\0workOrder\0"
    "viewWorkOrderDetails\0on_createNewButton_clicked\0"
    "on_editWorkOrder\0on_viewWorkOrderDetails\0"
    "row\0column\0onWorkOrderCreated\0"
    "onWorkOrderUpdated\0"
    "on_filterStatusComboBox_currentIndexChanged\0"
    "index\0on_searchButton_clicked\0"
    "on_refreshButton_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WorkOrderListView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x06 /* Public */,
       3,    1,   70,    2, 0x06 /* Public */,
       6,    1,   73,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,   76,    2, 0x08 /* Private */,
       8,    0,   77,    2, 0x08 /* Private */,
       9,    2,   78,    2, 0x08 /* Private */,
      12,    1,   83,    2, 0x08 /* Private */,
      13,    1,   86,    2, 0x08 /* Private */,
      14,    1,   89,    2, 0x08 /* Private */,
      16,    0,   92,    2, 0x08 /* Private */,
      17,    0,   93,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 4,    5,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   10,   11,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void WorkOrderListView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<WorkOrderListView *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->createNewWorkOrder(); break;
        case 1: _t->editWorkOrder((*reinterpret_cast< WorkOrder*(*)>(_a[1]))); break;
        case 2: _t->viewWorkOrderDetails((*reinterpret_cast< WorkOrder*(*)>(_a[1]))); break;
        case 3: _t->on_createNewButton_clicked(); break;
        case 4: _t->on_editWorkOrder(); break;
        case 5: _t->on_viewWorkOrderDetails((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->onWorkOrderCreated((*reinterpret_cast< WorkOrder*(*)>(_a[1]))); break;
        case 7: _t->onWorkOrderUpdated((*reinterpret_cast< WorkOrder*(*)>(_a[1]))); break;
        case 8: _t->on_filterStatusComboBox_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->on_searchButton_clicked(); break;
        case 10: _t->on_refreshButton_clicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< WorkOrder* >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< WorkOrder* >(); break;
            }
            break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< WorkOrder* >(); break;
            }
            break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< WorkOrder* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (WorkOrderListView::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WorkOrderListView::createNewWorkOrder)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (WorkOrderListView::*)(WorkOrder * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WorkOrderListView::editWorkOrder)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (WorkOrderListView::*)(WorkOrder * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WorkOrderListView::viewWorkOrderDetails)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject WorkOrderListView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_WorkOrderListView.data,
    qt_meta_data_WorkOrderListView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *WorkOrderListView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WorkOrderListView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_WorkOrderListView.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int WorkOrderListView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void WorkOrderListView::createNewWorkOrder()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void WorkOrderListView::editWorkOrder(WorkOrder * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void WorkOrderListView::viewWorkOrderDetails(WorkOrder * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
