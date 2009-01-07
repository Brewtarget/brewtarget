/****************************************************************************
** Meta object code from reading C++ file 'MiscTableModel.h'
**
** Created: Tue Jan 6 13:04:11 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "MiscTableModel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MiscTableModel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MiscTableModel[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_MiscTableModel[] = {
    "MiscTableModel\0"
};

const QMetaObject MiscTableModel::staticMetaObject = {
    { &QAbstractTableModel::staticMetaObject, qt_meta_stringdata_MiscTableModel,
      qt_meta_data_MiscTableModel, 0 }
};

const QMetaObject *MiscTableModel::metaObject() const
{
    return &staticMetaObject;
}

void *MiscTableModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MiscTableModel))
	return static_cast<void*>(const_cast< MiscTableModel*>(this));
    if (!strcmp(_clname, "MultipleObserver"))
	return static_cast< MultipleObserver*>(const_cast< MiscTableModel*>(this));
    return QAbstractTableModel::qt_metacast(_clname);
}

int MiscTableModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
