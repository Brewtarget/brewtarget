/*
 * widgets/Animator.h is is part of Brewtarget, and is copyright the following
 * authors 2018-2021:
 * - Iman Ahmadvand <iman72411@gmail.com>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef WIDGETS_ANIMATOR_H
#define WIDGETS_ANIMATOR_H
#pragma once

#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QObject>
#include <QPointer>
#include <QVariantAnimation>

class QVariant;

class Animator final : public QVariantAnimation {
   Q_OBJECT
   Q_PROPERTY(QObject * targetObject READ targetObject WRITE setTargetObject)

public:
   Animator(QObject * target, QObject * parent = nullptr);
   ~Animator() override;

   QObject * targetObject() const;
   void setTargetObject(QObject * target);

   bool isRunning() const;

public slots:
   void setup(int duration, QEasingCurve easing = QEasingCurve::Linear);
   void interpolate(const QVariant & start, const QVariant & end);
   void setCurrentValue(const QVariant &);

protected:
   void updateCurrentValue(const QVariant & value) override final;
   void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) override final;

private:
   QPointer<QObject> target;
};

#endif
