#ifndef _BREWNOTEWIDGET_H
#define _BREWNOTEWIDGET_H

class BrewNoteWidget;

#include <QWidget>
#include <QDialog>
#include "ui_brewNoteWidget.h"

// Forward declarations.
class BrewNote;

class BrewNoteWidget : public QWidget, public Ui::brewNoteWidget
{
    Q_OBJECT

public:
   BrewNoteWidget(QWidget *parent = 0);
   virtual ~BrewNoteWidget() {}

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

   void changed(QMetaProperty,QVariant);
private:
   BrewNote* bNoteObs;

   void showChanges();

};

#endif // _BREWNOTESWIDGET_H
