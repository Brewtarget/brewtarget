/*
 * BrewTargetTreeView.h is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
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

#ifndef BREWTARGETTREEVIEW_H_
#define BREWTARGETTREEVIEW_H_

class BrewTargetTreeView;

#include <QTreeView>
#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
#include "database.h"
#include "BrewTargetTreeModel.h"

class BrewTargetTreeView : public QTreeView
{
	Q_OBJECT
public:
	BrewTargetTreeView(QWidget *parent = 0);
	virtual ~BrewTargetTreeView();
	void startObservingDB();
	BrewTargetTreeModel* getModel();

	bool removeRow(const QModelIndex &index);

	// Ugh
	Recipe* getRecipe(const QModelIndex &index) const;
	QModelIndex findRecipe(Recipe* rec);

	Equipment* getEquipment(const QModelIndex &index) const;
	QModelIndex findEquipment(Equipment* kit);

	Fermentable* getFermentable(const QModelIndex &index) const;
	QModelIndex findFermentable(Fermentable* ferm);

	Hop* getHop(const QModelIndex &index) const;
	QModelIndex findHop(Hop* hop);

	Misc* getMisc(const QModelIndex &index) const;
	QModelIndex findMisc(Misc* misc);

	Yeast* getYeast(const QModelIndex &index) const;
	QModelIndex findYeast(Yeast* yeast);

	int getType(const QModelIndex &index);

    // Another try at drag and drop
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
	BrewTargetTreeModel* model;
    QPoint dragStart;
    bool doubleClick;

    QMimeData *mimeData(QModelIndexList indexes);
};

#endif /* BREWTARGETTREEVIEW_H_ */
