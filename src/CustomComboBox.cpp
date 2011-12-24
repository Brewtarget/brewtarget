/*
* CustomComboBox.cpp is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2009-2011.
*
* Brewtarget is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* Brewtarget is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CustomComboBox.h"
#include <QListView>

#include <QStylePainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>

/*
CustomComboBox::CustomComboBox(QWidget* parent)
   : QPushButton(parent),
     listView(new QListView(parent)),
     listFrame(new QFrame(parent, Qt::Popup)),
     desktop(QApplication::desktop())
{
   setCheckable(true);
   
   QVBoxLayout* boxLayout = new QVBoxLayout();
   boxLayout->addWidget(listView);
   listFrame->setLayout(boxLayout);
   
   listFrame->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
   listFrame->setContentsMargins(2,2,2,2);
   
   connect( this, SIGNAL(toggled(bool)), this, SLOT(handleToggle(bool)) );
   connect( listView, SIGNAL(currentTextChanged(QString const&)), this, SLOT(handleTextChanged(QString const&)) );
   connect( listView, SIGNAL(currentRowChanged(int)), this, SLOT(handleRowChanged(int)) );
   setChecked(false);
}

void CustomComboBox::setModel( QAbstractItemModel* model )
{
   listView->setModel(model);
}

void CustomComboBox::handleToggle(bool checked)
{
   if(checked)
   {
      listFrame->setGeometry(computeListRect());
      //listFrame->move(point);
      listFrame->show();
   }
   else
   {
      listFrame->hide();
   }
}

void CustomComboBox::handleRowChanged(int row)
{
   listView->hide();
   emit activated(row);
}

void CustomComboBox::handleTextChanged(QString const& text)
{
   listView->hide();
   emit activated(text);
}

int CustomComboBox::currentIndex()
{
   return listView->currentIndex().row();
}

QRect CustomComboBox::computeListRect()
{
   // Place the list either directly above or below the button,
   // whichever gives more room.
   
   QRect screen = desktop->availableGeometry(desktop->screenNumber(this));
   QPoint buttonPoint = mapToGlobal(QPoint(0,0));
   
   int aboveSpace = buttonPoint.y() - screen.y();
   int belowSpace = (screen.y() + screen.height()) - (buttonPoint.y() + height());
   
   //int wwidth = qMin( 200, listView->sizeHint().width() );
   int wwidth = 240;
   int hheight = 0;
   int xx = buttonPoint.x();
   int yy = 0;
   if( aboveSpace > belowSpace )
   {
      //hheight = qMin( listView->sizeHint().height(), aboveSpace );
      hheight = aboveSpace;
      yy = buttonPoint.y() - hheight;
   }
   else
   {
      //hheight = qMin( listFrame->height(), belowSpace );
      hheight = belowSpace;
      yy = buttonPoint.y() + height();
   }
   
   return QRect(xx,yy,wwidth,hheight);
}

void CustomComboBox::paintEvent(QPaintEvent*)
{
   QStylePainter painter(this);
   
   // Set up painting options.
   QStyleOptionButton buttonOpts;
   initStyleOption(&buttonOpts); // Ask QPushButton to fill in :)
   
   //painter.setPen(palette().color(QPalette::Text));
   //painter.drawControl(QStyle::CE_PushButton, buttonOpts);
   
   QStyleOptionComboBox opts;
   // Copy QStyleOption stuff.
   opts.direction = buttonOpts.direction;
   opts.fontMetrics = buttonOpts.fontMetrics;
   opts.palette = buttonOpts.palette;
   opts.rect = buttonOpts.rect;
   opts.state = buttonOpts.state;
   opts.type = buttonOpts.type;
   opts.version = buttonOpts.version;
   
   // Set stuff specific to QStyleOptionComboBox
   opts.currentText = text();
   //opts.subControls = QStyle::SC_ComboBoxFrame | QStyle::SC_ComboBoxArrow;
   //opts.activeSubControls = QStyle::SC_ComboBoxFrame | QStyle::SC_ComboBoxArrow;
   opts.subControls = QStyle::SC_All;
   opts.activeSubControls = QStyle::SC_All;
   
   // Draw combobox.
   painter.setPen(palette().color(QPalette::Text));
   painter.drawComplexControl(QStyle::CC_ComboBox, opts);
}
*/

CustomComboBox::CustomComboBox(QWidget* parent)
   : QComboBox(parent)
{
}

void CustomComboBox::showPopup()
{
   view()->setFixedWidth(300);
   QComboBox::showPopup();
}

void CustomComboBox::paintEvent(QPaintEvent*)
{
   QStylePainter painter(this);
   
   QStyleOptionComboBox opts;
   initStyleOption(&opts);
   //opts.currentText = "Wasup";
   opts.currentText = "";
   
   // Draw combo box frame and shit.
   painter.drawComplexControl(QStyle::CC_ComboBox, opts);
   // Have to draw label separately? Stupid.
   //painter.drawControl(QStyle::CE_ComboBoxLabel, opts);
}
