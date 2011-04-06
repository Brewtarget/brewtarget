/*
 * BrewTargetTreeView.h
 *
 *  Created on: Apr 3, 2011
 *      Author: mik
 */

#ifndef BREWTARGETTREEVIEW_H_
#define BREWTARGETTREEVIEW_H_

class BrewTargetTreeView;

#include <QTreeView>
#include <QWidget>
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

	int getType(const QModelIndex &index);

private:
	BrewTargetTreeModel* model;

};

#endif /* BREWTARGETTREEVIEW_H_ */
