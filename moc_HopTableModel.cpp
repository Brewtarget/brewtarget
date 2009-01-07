/****************************************************************************
** Meta object code from reading C++ file 'HopTableModel.h'
**
** Created: Tue Jan 6 22:38:14 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "HopTableModel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'HopTableModel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_HopTableModel[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_HopTableModel[] = {
    "HopTableModel\0"
};

const QMetaObject HopTableModel::staticMetaObject = {
    { &QAbstractTableModel::staticMetaObject, qt_meta_stringdata_HopTableModel,
      qt_meta_data_HopTableModel, 0 }
};

const QMetaObject *HopTableModel::metaObject() const
{
    return &staticMetaObject;
}

void *HopTableModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HopTableModel))
	return static_cast<void*>(const_cast< HopTableModel*>(this));
    if (!strcmp(_clname, "MultipleObserver"))
	return static_cast< MultipleObserver*>(const_cast< HopTableModel*>(this));
    return QAbstractTableModel::qt_metacast(_clname);
}

int HopTableModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
