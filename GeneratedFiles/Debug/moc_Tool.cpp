/****************************************************************************
** Meta object code from reading C++ file 'Tool.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Tool.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Tool.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Tool_t {
    QByteArrayData data[9];
    char stringdata0[102];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Tool_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Tool_t qt_meta_stringdata_Tool = {
    {
QT_MOC_LITERAL(0, 0, 4), // "Tool"
QT_MOC_LITERAL(1, 5, 8), // "FileOpen"
QT_MOC_LITERAL(2, 14, 0), // ""
QT_MOC_LITERAL(3, 15, 11), // "FileSaveDxd"
QT_MOC_LITERAL(4, 27, 10), // "FileSaveMt"
QT_MOC_LITERAL(5, 38, 17), // "FileSaveAnimation"
QT_MOC_LITERAL(6, 56, 11), // "FileSaveAll"
QT_MOC_LITERAL(7, 68, 14), // "OpenMeshWindow"
QT_MOC_LITERAL(8, 83, 18) // "OpenMaterialWindow"

    },
    "Tool\0FileOpen\0\0FileSaveDxd\0FileSaveMt\0"
    "FileSaveAnimation\0FileSaveAll\0"
    "OpenMeshWindow\0OpenMaterialWindow"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Tool[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08 /* Private */,
       3,    0,   50,    2, 0x08 /* Private */,
       4,    0,   51,    2, 0x08 /* Private */,
       5,    0,   52,    2, 0x08 /* Private */,
       6,    0,   53,    2, 0x08 /* Private */,
       7,    0,   54,    2, 0x08 /* Private */,
       8,    0,   55,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Tool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Tool *_t = static_cast<Tool *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->FileOpen(); break;
        case 1: _t->FileSaveDxd(); break;
        case 2: _t->FileSaveMt(); break;
        case 3: _t->FileSaveAnimation(); break;
        case 4: _t->FileSaveAll(); break;
        case 5: _t->OpenMeshWindow(); break;
        case 6: _t->OpenMaterialWindow(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject Tool::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_Tool.data,
      qt_meta_data_Tool,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *Tool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Tool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Tool.stringdata0))
        return static_cast<void*>(const_cast< Tool*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int Tool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
