/****************************************************************************
** Meta object code from reading C++ file 'ParameterDialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ParameterDialog.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ParameterDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSParameterDialogENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSParameterDialogENDCLASS = QtMocHelpers::stringData(
    "ParameterDialog",
    "onSaveClicked",
    "",
    "onLoadClicked",
    "onResetClicked",
    "onApplyClicked",
    "onRefreshClicked",
    "onSpinBoxChanged",
    "value",
    "updateDisplay",
    "onDelayedUpdate"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSParameterDialogENDCLASS_t {
    uint offsetsAndSizes[22];
    char stringdata0[16];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[15];
    char stringdata5[15];
    char stringdata6[17];
    char stringdata7[17];
    char stringdata8[6];
    char stringdata9[14];
    char stringdata10[16];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSParameterDialogENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSParameterDialogENDCLASS_t qt_meta_stringdata_CLASSParameterDialogENDCLASS = {
    {
        QT_MOC_LITERAL(0, 15),  // "ParameterDialog"
        QT_MOC_LITERAL(16, 13),  // "onSaveClicked"
        QT_MOC_LITERAL(30, 0),  // ""
        QT_MOC_LITERAL(31, 13),  // "onLoadClicked"
        QT_MOC_LITERAL(45, 14),  // "onResetClicked"
        QT_MOC_LITERAL(60, 14),  // "onApplyClicked"
        QT_MOC_LITERAL(75, 16),  // "onRefreshClicked"
        QT_MOC_LITERAL(92, 16),  // "onSpinBoxChanged"
        QT_MOC_LITERAL(109, 5),  // "value"
        QT_MOC_LITERAL(115, 13),  // "updateDisplay"
        QT_MOC_LITERAL(129, 15)   // "onDelayedUpdate"
    },
    "ParameterDialog",
    "onSaveClicked",
    "",
    "onLoadClicked",
    "onResetClicked",
    "onApplyClicked",
    "onRefreshClicked",
    "onSpinBoxChanged",
    "value",
    "updateDisplay",
    "onDelayedUpdate"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSParameterDialogENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   62,    2, 0x08,    1 /* Private */,
       3,    0,   63,    2, 0x08,    2 /* Private */,
       4,    0,   64,    2, 0x08,    3 /* Private */,
       5,    0,   65,    2, 0x08,    4 /* Private */,
       6,    0,   66,    2, 0x08,    5 /* Private */,
       7,    1,   67,    2, 0x08,    6 /* Private */,
       9,    0,   70,    2, 0x08,    8 /* Private */,
      10,    0,   71,    2, 0x08,    9 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    8,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject ParameterDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSParameterDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSParameterDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSParameterDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ParameterDialog, std::true_type>,
        // method 'onSaveClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLoadClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onResetClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onApplyClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRefreshClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSpinBoxChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'updateDisplay'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDelayedUpdate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void ParameterDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ParameterDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onSaveClicked(); break;
        case 1: _t->onLoadClicked(); break;
        case 2: _t->onResetClicked(); break;
        case 3: _t->onApplyClicked(); break;
        case 4: _t->onRefreshClicked(); break;
        case 5: _t->onSpinBoxChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 6: _t->updateDisplay(); break;
        case 7: _t->onDelayedUpdate(); break;
        default: ;
        }
    }
}

const QMetaObject *ParameterDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ParameterDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSParameterDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int ParameterDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}
QT_WARNING_POP
