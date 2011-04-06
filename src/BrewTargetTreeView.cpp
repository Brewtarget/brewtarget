/*
 * BrewTargetTreeView.cpp
 *
 *  Created on: Apr 3, 2011
 *      Author: mik
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
