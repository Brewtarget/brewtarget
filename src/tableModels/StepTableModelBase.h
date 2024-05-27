/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/StepTableModelBase.h is part of Brewtarget, and is copyright the following authors 2024:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef TABLEMODELS_STEPTABLEMODELBASE_H
#define TABLEMODELS_STEPTABLEMODELBASE_H
#pragma once

#include <QDebug>

#include "utils/CuriouslyRecurringTemplateBase.h"

/**
 * \brief Extra functions used by \c MashStepTableModel, \c BoilStepTableModel, \c FermentationStepTableModel
 *
 *        Classes inheriting from this one need to include the STEP_TABLE_MODEL_COMMON_DECL macro in their header file
 *        and the STEP_TABLE_MODEL_COMMON_CODE macro in their .cpp file.
 */
template<class Derived> class StepTableModelPhantom;
template<class Derived, class StepClass, class StepOwnerClass>
class StepTableModelBase : public CuriouslyRecurringTemplateBase<StepTableModelPhantom, Derived> {

protected:
   StepTableModelBase() :
      m_stepOwnerObs{nullptr} {
      return;
   }
   ~StepTableModelBase() = default;

   /**
    * \brief Set the step owner (eg Mash) whose steps (eg MashStep objects) we want to model, or reload steps from an
    *        existing step owner after they were changed.
    */
   void setStepOwner(std::shared_ptr<StepOwnerClass> stepOwner) {
      if (this->m_stepOwnerObs && this->derived().rows.size() > 0) {
         qDebug() <<
            Q_FUNC_INFO << "Removing" << this->derived().rows.size() << StepClass::staticMetaObject.className() <<
            "rows for old" << StepOwnerClass::staticMetaObject.className() << "#" << this->m_stepOwnerObs->key();
         this->derived().beginRemoveRows(QModelIndex(), 0, this->derived().rows.size() - 1);

         for (auto step : this->derived().rows) {
            this->derived().disconnect(step.get(), nullptr, &this->derived(), nullptr);
         }
         this->derived().rows.clear();
         this->derived().endRemoveRows();
      }

      // Disconnect old signals if any were connected and we're changing step owner (eg Mash)
      if (this->m_stepOwnerObs && this->m_stepOwnerObs != stepOwner) {
         // Remove m_stepOwnerObs and all steps.
         this->derived().disconnect(this->m_stepOwnerObs.get(), nullptr, &this->derived(), nullptr);
      }

      // Connect new signals, unless there is no new Mash/Boil/etc or we're not changing step owner (eg Mash)
      if (stepOwner && this->m_stepOwnerObs != stepOwner) {
         this->derived().connect(stepOwner.get(), &StepOwnerClass::stepsChanged, &this->derived(), &Derived::stepOwnerChanged);
      }

      this->m_stepOwnerObs = stepOwner;
      if (this->m_stepOwnerObs) {
         qDebug() <<
            Q_FUNC_INFO << "Now watching" << StepOwnerClass::staticMetaObject.className() << "#" <<
            this->m_stepOwnerObs->key();

         auto tmpSteps = this->m_stepOwnerObs->steps();
         if (tmpSteps.size() > 0) {
            qDebug() <<
               Q_FUNC_INFO << "Inserting" << tmpSteps.size() << " " << StepClass::staticMetaObject.className() <<
               "rows";
            this->derived().beginInsertRows(QModelIndex(), 0, tmpSteps.size() - 1);
            this->derived().rows = tmpSteps;
            for (auto step : this->derived().rows) {
               this->derived().connect(step.get(), &NamedEntity::changed, &this->derived(), &Derived::stepChanged);
            }
            this->derived().endInsertRows();
         }
      }

      if (this->derived().m_parentTableWidget) {
         this->derived().m_parentTableWidget->resizeColumnsToContents();
         this->derived().m_parentTableWidget->resizeRowsToContents();
      }
      return;

   }

   std::shared_ptr<StepOwnerClass> getStepOwner() const {
      return this->m_stepOwnerObs;
   }

   //! \returns true if \c step is successfully found and removed.
   bool doRemoveStep(std::shared_ptr<StepClass> step) {
      int ii {this->derived().rows.indexOf(step)};
      if (ii >= 0) {
         qDebug() <<
            Q_FUNC_INFO << "Removing" << StepClass::staticMetaObject.className() << step->name() << "(#" <<
            step->key() << ")";
         this->derived().beginRemoveRows(QModelIndex(), ii, ii);
         this->derived().disconnect(step.get(), nullptr, &this->derived(), nullptr);
         this->derived().rows.removeAt(ii);
         //reset(); // Tell everybody the table has changed.
         this->derived().endRemoveRows();

         return true;
      }

      return false;
   }

   void reorderStep(std::shared_ptr<StepClass> step, int current) {
      // doSomething will be -1 if we are moving up and 1 if we are moving down
      // and 0 if nothing is to be done (see next comment)
      int destChild   = step->stepNumber();
      int doSomething = destChild - current - 1;

      qDebug() << Q_FUNC_INFO << "Swapping" << destChild << "with" << current << ", so doSomething=" << doSomething;

      //
      // Moving a step up or down generates two signals, one for each row impacted. If we move row B above row A:
      //    1. The first signal is to move B above A, which will result in A being below B
      //    2. The second signal is to move A below B, which we just did.
      // Therefore, the second signal mostly needs to be ignored. In those circumstances, A->stepNumber() will be the
      // same as its position in the steps list, modulo some indexing.
      //
      if (doSomething == 0) {
         return;
      }

      // beginMoveRows is a little odd. When moving rows within the same parent,
      // destChild points one beyond where you want to insert the row. Think of
      // it as saying "insert before destChild". If we are moving something up,
      // we need to be one less than stepNumber. If we are moving down, it just
      // works.
      if (doSomething < 0) {
         destChild--;
      }

      // We assert that we are swapping valid locations on the list as, to do otherwise implies a coding error
      qDebug() <<
         Q_FUNC_INFO << "Swap" << current + doSomething << "with" << current << ", in list of " <<
         this->derived().rows.size();
      Q_ASSERT(current >= 0);
      Q_ASSERT(current + doSomething >= 0);
      Q_ASSERT(current < this->derived().rows.size());
      Q_ASSERT(current + doSomething < this->derived().rows.size());

      this->derived().beginMoveRows(QModelIndex(), current, current, QModelIndex(), destChild);

      // doSomething is -1 if moving up and 1 if moving down. swap current with
      // current -1 when moving up, and swap current with current+1 when moving
      // down
#if QT_VERSION < QT_VERSION_CHECK(5,13,0)
      this->derived().rows.swap(current, current + doSomething);
#else
      this->derived().rows.swapItemsAt(current, current + doSomething);
#endif
      this->derived().endMoveRows();
      return;
   }

   void doMoveStepUp(int stepNum) {
      if (!this->m_stepOwnerObs || stepNum == 0 || stepNum >= this->derived().rows.size()) {
         return;
      }

      this->m_stepOwnerObs->swapSteps(*this->derived().rows[stepNum], *this->derived().rows[stepNum - 1]);
      return;
   }

   void doMoveStepDown(int stepNum) {
      if (!this->m_stepOwnerObs || stepNum + 1 >= this->derived().rows.size()) {
         return;
      }

      this->m_stepOwnerObs->swapSteps(*this->derived().rows[stepNum], *this->derived().rows[stepNum + 1]);
      return;
   }

   void doStepOwnerChanged() {
      // A step was added, removed or change order.  Remove and re-add all steps.
      qDebug() << Q_FUNC_INFO << "Re-reading" << StepClass::staticMetaObject.className() << "steps for" << this->m_stepOwnerObs;
      this->setStepOwner(this->m_stepOwnerObs);
      return;

   }
   void doStepChanged(QMetaProperty prop, [[maybe_unused]] QVariant val) {
      qDebug() << Q_FUNC_INFO;

      StepClass * stepSender = qobject_cast<StepClass *>(this->derived().sender());
      if (stepSender) {
         if (stepSender->ownerId() != this->m_stepOwnerObs->key()) {
            // It really shouldn't happen that we get a notification for a step (eg MashStep) that's not in the step
            // owner (eg Mash) we're watching, but, if we do, then stop trying to process the update.
            qCritical() <<
               Q_FUNC_INFO << "Instance @" << static_cast<void *>(this) << "received update for" <<
               StepClass::staticMetaObject.className() << "#" << stepSender->key() << "of" <<
               StepOwnerClass::staticMetaObject.className() << "#" << stepSender->ownerId() << "but we are watching" <<
               StepOwnerClass::staticMetaObject.className() << "#" << this->m_stepOwnerObs->key();
            return;
         }

         int ii = this->derived().findIndexOf(stepSender);
         if (ii >= 0) {
            if (prop.name() == PropertyNames::Step::stepNumber) {
               this->reorderStep(this->derived().rows.at(ii), ii);
            }

            emit this->derived().dataChanged(
               this->derived().QAbstractItemModel::createIndex(ii, 0),
               this->derived().QAbstractItemModel::createIndex(ii, this->derived().columnCount() - 1)
            );
         }

      }

      if (this->derived().m_parentTableWidget) {
         this->derived().m_parentTableWidget->resizeColumnsToContents();
         this->derived().m_parentTableWidget->resizeRowsToContents();
      }
      return;
   }

   //================================================ Member Variables =================================================
   std::shared_ptr<StepOwnerClass> m_stepOwnerObs;
};

/**
 * \brief Derived classes should include this in their header file, right after TABLE_MODEL_COMMON_DECL
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define STEP_TABLE_MODEL_COMMON_DECL(StepOwnerName)                                                    \
   /* This allows StepTableModelBase to call protected and private members of Derived */               \
   friend class StepTableModelBase<StepOwnerName##StepTableModel, StepOwnerName##Step, StepOwnerName>; \
                                                                                                       \
   public:                                                                                             \
      void set##StepOwnerName(std::shared_ptr<StepOwnerName> stepOwner);                               \
      std::shared_ptr<StepOwnerName> get##StepOwnerName() const;                                       \
      bool removeStep(std::shared_ptr<StepOwnerName##Step> step);                                      \
                                                                                                       \
   public slots:                                                                                       \
      void moveStepUp(int stepNum);                                                                    \
      void moveStepDown(int stepNum);                                                                  \
      void stepOwnerChanged();                                                                         \
      void stepChanged(QMetaProperty prop, QVariant val);                                              \


/**
 * \brief Derived classes should include this in their .cpp file
 *
 *        Note we have to be careful about comment formats in macro definitions
 *
 *        NB: Mostly I have tried to make these macro-included function bodies trivial.  Macros are a bit clunky, so we
 *            only really want to use them for the things that are hard to do other ways.
 */
#define STEP_TABLE_MODEL_COMMON_CODE(StepOwnerName)                                                              \
   void StepOwnerName##StepTableModel::set##StepOwnerName(std::shared_ptr<StepOwnerName> stepOwner) {            \
      this->setStepOwner(stepOwner);                                                                             \
      return;                                                                                                    \
   }                                                                                                             \
   std::shared_ptr<StepOwnerName> StepOwnerName##StepTableModel::get##StepOwnerName() const {                    \
      return this->getStepOwner();                                                                               \
   }                                                                                                             \
   bool StepOwnerName##StepTableModel::removeStep(std::shared_ptr<StepOwnerName##Step> step) {                   \
      return this->doRemoveStep(step);                                                                           \
   }                                                                                                             \
   void StepOwnerName##StepTableModel::moveStepUp(int stepNum) {                                                 \
      this->doMoveStepUp(stepNum);                                                                               \
      return;                                                                                                    \
   }                                                                                                             \
   void StepOwnerName##StepTableModel::moveStepDown(int stepNum) {                                               \
      this->doMoveStepDown(stepNum);                                                                             \
      return;                                                                                                    \
   }                                                                                                             \
   void StepOwnerName##StepTableModel::stepOwnerChanged() {                                                      \
      this->doStepOwnerChanged();                                                                                \
      return;                                                                                                    \
   }                                                                                                             \
   void StepOwnerName##StepTableModel::stepChanged(QMetaProperty prop, QVariant val) {                           \
      this->doStepChanged(prop, val);                                                                            \
      return;                                                                                                    \
   }                                                                                                             \

#endif
