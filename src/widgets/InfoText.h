/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/InfoText.h is part of Brewtarget, and is copyright the following authors 2023:
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
#ifndef WIDGETS_INFOTEXT_H
#define WIDGETS_INFOTEXT_H
#pragma once

#include <QLabel>

class InfoButton;

/**
 * \class InfoText
 *
 * \brief Partner class to \c InfoButton.  This holds the informational text that is shown when the corresponding
 *        \c InfoButton is clicked.  You can add \c InfoButton / \c InfoText pairs in a .ui file without needing any
 *        additional C++ code, provided you set the \c buddy property of the \c InfoText to point to the \c InfoButton,
 *        per the example below.  (An \c InfoButton without a corresponding \c InfoText will be displayed greyed-out.)
 *
 *        Specifically, the .ui file should contain something along the following lines:
 *
 *           <widget class="InfoButton" name="infoButton_foobar"></widget>
 *           ...
 *           <widget class="InfoText" name="infoText_foobar">
 *            <property name="buddy"><cstring>infoButton_foobar</cstring></property>
 *            <property name="text"><string>The info text we want to display...</string></property>
 *           </widget>
 *
 *        No additional code is required in our .cpp or .h files.
 *
 *        This is only possible because \c InfoText inherits from \c QLabel.  The "magic" that makes buddies work is not
 *        a generic Qt feature.  It is hard-coded in the MOC for \c QLabel and we can only use it by deriving from
 *        \c QLabel.  (If you try to code your own equivalent of the buddy property you'll founder on the rocks of the
 *        parameter being passed in as a string instead of magically converted to the address of a QWidget by the MOC.)
 *
 *        This is also why we cannot put the buddy property on the \c InfoButton -- because that does not inherit from
 *        \c QLabel.
 */
class InfoText : public QLabel {
   Q_OBJECT
public:
   explicit InfoText(QWidget * parent, Qt::WindowFlags flags = Qt::WindowFlags());
   virtual ~InfoText();

   /**
    * \brief We override \c QLabel::setText because we know this will get called \b after all the other widgets in our
    *        editor have been constructed.  In particular, our buddy will have been created and our
    *        \c QLabel::setBuddy member function will have been called.  Since, as noted above, we cannot override that
    *        buddy-setting function (as it would break the magic that allows it to be correctly called from code
    *        generated from the .ui file), we need to jump in at this point instead.
    *
    *        How do we know that \c setText will always be called after \c setBuddy?  It is because the former is called
    *        in \c retranslateUi, which is called at the end of \c setupUi (in which all the objects specified in the
    *        .ui file are created and \c setBuddy is called).  Things have to be done this way round because you need to
    *        be able to call \c retranslateUi directly later on in the program, whereas the rest of the \c setupUi
    *        functionality can only be done once when the parent widget (eg \c EquipmentEditor) is constructed.
    */
   void setText(const QString & text);

   /**
    * \brief We override \c QLabel::mouseReleaseEvent to make the \c InfoText clickable (so the user can dismiss it by
    *        clicking on it).
    */
   virtual void mouseReleaseEvent(QMouseEvent * event);

public slots:

   /**
    * \brief By default, when you show and hide a \c QLabel with a lot of text, there won't necessarily be space to
    *        show all the text.  These versions of \c show and \c hide call \c QWidget::adjustSize in an attempt to
    *        remedy this.
    *
    *        TBD: Not convinced this is 100% doing what we want, but it's better than nothing.
    */
   //! @{
   void show();
   void hide();
   //! @}

private:
   void resizeParent();

   InfoButton * m_infoButton;
};

#endif
