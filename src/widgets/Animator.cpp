/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/Animator.cpp is is part of Brewtarget, and is copyright the following authors 2018-2021:
 *   • Iman Ahmadvand <iman72411@gmail.com>
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
#include "widgets/Animator.h"

#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QVariant>

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Animator.cpp"
#endif

Animator::Animator(QObject * target, QObject * parent) : QVariantAnimation(parent) {
   setTargetObject(target);
   return;
}

Animator::~Animator() {
   stop();
   return;
}

QObject * Animator::targetObject() const {
   return target.data();
}

void Animator::setTargetObject(QObject * _target) {
   if (target.data() == _target) {
      return;
   }

   if (isRunning()) {
      qWarning() << Q_FUNC_INFO << "You can't change the target of a running animation";
      return;
   }

   target = _target;
   return;
}

bool Animator::isRunning() const {
   return state() == Running;
}

void Animator::updateCurrentValue(const QVariant & value) {
   Q_UNUSED(value);

   if (!target.isNull()) {
      auto update = QEvent(QEvent::StyleAnimationUpdate);
      update.setAccepted(false);
      QCoreApplication::sendEvent(target.data(), &update);
      if (!update.isAccepted()) {
         stop();
      }
   }
   return;
}

void Animator::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) {
   if (target.isNull() && oldState == Stopped) {
      qWarning() << Q_FUNC_INFO << "Changing state of an animation without target";
      return;
   }

   QVariantAnimation::updateState(newState, oldState);

   if (!endValue().isValid() && direction() == Forward) {
      qWarning() <<
         Q_FUNC_INFO << targetObject()->metaObject()->className() << ": starting an animation without end value";
   }
   return;
}

void Animator::setup(int duration, QEasingCurve easing) {
   setDuration(duration);
   setEasingCurve(easing);
   return;
}

void Animator::interpolate(const QVariant & _start, const QVariant & end) {
   setStartValue(_start);
   setEndValue(end);
   start();
   return;
}

void Animator::setCurrentValue(const QVariant & value) {
   setStartValue(value);
   setEndValue(value);
   updateCurrentValue(currentValue());
   return;
}
