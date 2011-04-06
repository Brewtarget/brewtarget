/*
 * BrewTargetTreeView.cpp is part of Brewtarget and was written by Mik
 * Firestone (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
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

#include <QDebug>
#include "BrewTargetTreeView.h"
#include "BrewTargetTreeModel.h"

BrewTargetTreeView::BrewTargetTreeView(QWidget *parent) :
	QTreeView(parent)
{
	model = new BrewTargetTreeModel(this);
	model->startObservingDB();

	setModel(model);
}

BrewTargetTreeView::~BrewTargetTreeView()
{
	delete model;
}

BrewTargetTreeModel* BrewTargetTreeView::getModel()
{
	return model;
}

bool BrewTargetTreeView::removeRow(const QModelIndex &index)
{
	QModelIndex parent = model->parent(index);
	int position       = index.row();

	return model->removeRows(position,1,parent);
}

Recipe* BrewTargetTreeView::getRecipe(const QModelIndex &index) const
{
	return model->getRecipe(index);
}

QModelIndex BrewTargetTreeView::findRecipe(Recipe* rec)
{
	return model->findRecipe(rec);
}

Equipment* BrewTargetTreeView::getEquipment(const QModelIndex &index) const
{
	return model->getEquipment(index);
}

QModelIndex BrewTargetTreeView::findEquipment(Equipment* kit)
{
	return model->findEquipment(kit);
}

Fermentable* BrewTargetTreeView::getFermentable(const QModelIndex &index) const
{
	return model->getFermentable(index);
}

QModelIndex BrewTargetTreeView::findFermentable(Fermentable* ferm)
{
	return model->findFermentable(ferm);
}

Hop* BrewTargetTreeView::getHop(const QModelIndex &index) const
{
	return model->getHop(index);
}

QModelIndex BrewTargetTreeView::findHop(Hop* hop)
{
	return model->findHop(hop);
}

int BrewTargetTreeView::getType(const QModelIndex &index)
{
	return model->getType(index);
}
