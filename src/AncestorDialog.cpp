/*
 * AncestorDialog.cpp is part of Brewtarget, and is Copyright the following
 * authors 2016 - 2019
 * - Mik Firestone <mikfire@fastmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDebug>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <algorithm>

#include "AncestorDialog.h"
#include "MainWindow.h"
#include "database.h"

#include "model/NamedEntity.h"
#include "model/Recipe.h"

AncestorDialog::AncestorDialog(QWidget *parent)
   : QDialog(parent)
{

   setupUi(this);

   pushButton_apply->setEnabled(false);
   comboBox_descendant->setEnabled(false);

   buildAncestorBox();

   // this does the dirty
   connect(pushButton_apply, SIGNAL(clicked()), this, SLOT(connectDescendant()));
   connect(pushButton_close, SIGNAL(clicked()), this, SLOT(reject()));

   // just some nice things
   connect( comboBox_ancestor,   SIGNAL(activated(int)), this, SLOT(ancestorSelected(int)));
   // connect( comboBox_descendant, SIGNAL(activated(int)), this, SLOT(activateButton()));
}

bool AncestorDialog::recipeLessThan(Recipe *right, Recipe *left)
{
   if ( right->name() == left->name() )
      return right->key() < left->key();

   return right->name() < left->name();
}

void AncestorDialog::buildAncestorBox()
{
   QList<Recipe*> recipes = Database::instance().recipes();
   std::sort(recipes.begin(),recipes.end(),AncestorDialog::recipeLessThan);

   foreach (Recipe* recipe, recipes) {
      if ( recipe->display() ) {
         comboBox_ancestor->addItem(recipe->name(), recipe->key());
      }
   }
   comboBox_ancestor->setCurrentIndex(-1);
}

void AncestorDialog::buildDescendantBox(Recipe *ignore)
{
   QList<Recipe*> recipes = Database::instance().recipes();
   std::sort(recipes.begin(),recipes.end(),recipeLessThan);

   //  The rules of what can be a target are complex
   foreach (Recipe* recipe, recipes) {
      // if we are ignoring the recipe, skip
      if ( recipe == ignore )
         continue;
      // if the recipe is not being displayed, skip
      if ( ! recipe->display())
         continue;
      // if the recipe already has ancestors, skip
      if ( recipe->hasAncestors() )
         continue;
      comboBox_descendant->addItem(recipe->name(), recipe->key());
   }
}

void AncestorDialog::connectDescendant()
{
   Recipe *ancestor, *descendant;

   ancestor   = Database::instance().recipe( comboBox_ancestor->currentData().toInt() );
   descendant = Database::instance().recipe( comboBox_descendant->currentData().toInt() );

   // No loops in the inheritance
   if ( ! descendant->isMyAncestor(ancestor) ) {
      descendant->setAncestor(ancestor);

      emit ancestoryChanged(ancestor,descendant);
   }

   // disable the apply button
   pushButton_apply->setEnabled(false);

   // reset the descendant box
   comboBox_descendant->setEnabled(false);
   comboBox_descendant->clear();

   // and rebuild the ancestors box
   comboBox_ancestor->clear();
   buildAncestorBox();
}

void AncestorDialog::setAncestor(Recipe* anc)
{
   comboBox_ancestor->setCurrentText( anc->name() );
   buildDescendantBox(anc);

   comboBox_descendant->setEnabled(true);
   activateButton();
}

void AncestorDialog::ancestorSelected(int ndx)
{
   Recipe *ancestor = Database::instance().recipe( comboBox_ancestor->currentData().toInt() );
   comboBox_descendant->setEnabled(true);

   buildDescendantBox(ancestor);

   activateButton();
}

void AncestorDialog::activateButton()
{
   if ( ! pushButton_apply->isEnabled() )
      pushButton_apply->setEnabled(true);
}


