#ifndef _BREWNOTEWIDGET_H
#define _BREWNOTEWIDGET_H

class BrewNoteWidget;

#include <QWidget>
#include <QDialog>
#include "ui_brewNoteWidget.h"
#include "observable.h"
#include "brewnote.h"

class BrewNoteWidget : public QWidget, public Ui::brewNoteWidget, public Observer
{
    Q_OBJECT

public:
   BrewNoteWidget(QWidget *parent = 0);
   ~BrewNoteWidget();

   void setBrewNote(BrewNote* bNote);

public slots:
   void updateBrewDate();
   void updateSG();
   void updateVolumeIntoBK_l();
   void updateStrikeTemp_c();
   void updateMashFinTemp_c();

   void updateOG();
   void updatePostBoilVolume_l();
   void updateVolumeIntoFerm_l();
   void updatePitchTemp_c();

   void updateFG();
   void updateFinalVolume_l();
   void updateFermentDate();

   void updateNotes();
   void saveAll();

private:
   BrewNote* bNoteObs;

   void showChanges();
   virtual void notify(Observable* notifier, QVariant info = QVariant());

};

#endif // _BREWNOTESWIDGET_H
