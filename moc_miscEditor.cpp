/****************************************************************************
** Meta object code from reading C++ file 'miscEditor.h'
**
** Created: Tue Jan 6 13:04:10 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "miscEditor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'miscEditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_miscEditor[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,
      19,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_miscEditor[] = {
    "miscEditor\0\0save()\0clearAndClose()\0"
};

const QMetaObject miscEditor::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_miscEditor,
      qt_meta_data_miscEditor, 0 }
};

const QMetaObject *miscEditor::metaObject() const
{
    return &staticMetaObject;
}

void *miscEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_miscEditor))
	return static_cast<void*>(const_cast< miscEditor*>(this));
    if (!strcmp(_clname, "Observer"))
	return static_cast< Observer*>(const_cast< miscEditor*>(this));
    return QDialog::qt_metacast(_clname);
}

int miscEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: save(); break;
        case 1: clearAndClose(); break;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
