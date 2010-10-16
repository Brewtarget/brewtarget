#ifndef WATEREDITOR_H
#define WATEREDITOR_H

#include <QDialog>
#include "ui_waterEditor.h"
#include "observable.h"
#include "water.h"

class WaterEditor : public QDialog, public Ui::waterEditor, public Observer
{
    Q_OBJECT
public:
    WaterEditor(QWidget *parent = 0);
    ~WaterEditor();

    /*!
     * Sets the water we want to observe.
     */
    void setWater(Water* water);

    virtual void notify(Observable *notifier, QVariant info); // From Observer.

 public slots:
    void showChanges();
    void saveAndClose();

private:
    Water* obs; // Observed water.
};

#endif // WATEREDITOR_H
