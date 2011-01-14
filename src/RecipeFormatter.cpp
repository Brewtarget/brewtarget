/*
* RecipeFormatter.cpp is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2009.
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

#include "RecipeFormatter.h"
#include "style.h"
#include "fermentable.h"
#include "hop.h"
#include "misc.h"
#include "yeast.h"
#include "mash.h"
#include "unit.h"
#include "brewtarget.h"
#include <QClipboard>
#include <QObject>
#include <QPrinter>
#include <QPrintDialog>
#include "instruction.h"

RecipeFormatter::RecipeFormatter()
{
   textSeparator = 0;
   rec = 0;
   doc = new QWebView();
}

void RecipeFormatter::setRecipe(Recipe* recipe)
{
   rec = recipe;
}

QString RecipeFormatter::getTextFormat()
{
   QString ret = "";
   QString colorString;
   QString bitternessString;
   if( rec == 0 )
      return ret;
   
   unsigned int i, size;
   
   Style* style = rec->getStyle();
   Mash* mash = rec->getMash();
   
   // Vital statistics.
   ret += rec->getName().c_str();
   if( style != 0 && style->getName() != "" )
      ret += (" - " + style->getName()).c_str();
   ret += "\n";
   
   ret += getTextSeparator();
   
   ret += QObject::tr("Batch Size: %1\n").arg(Brewtarget::displayAmount(rec->getBatchSize_l(), Units::liters));
   ret += QObject::tr("Boil Size: %1\n").arg(Brewtarget::displayAmount(rec->getBoilSize_l(), Units::liters));
   ret += QObject::tr("Boil Time: %1\n").arg( (rec->getEquipment() == 0)?
                                              Brewtarget::displayAmount(0, Units::minutes)
                                            : Brewtarget::displayAmount( (rec->getEquipment())->getBoilTime_min(), Units::minutes));
   ret += QObject::tr("Efficiency: %1%%\n").arg(rec->getEfficiency_pct(), 0, 'f', 0);
   ret += QObject::tr("OG: %1\n").arg( Brewtarget::displayOG(rec->getOg(), true) );
   ret += QObject::tr("FG: %1\n").arg( Brewtarget::displayFG(rec->getFg(), rec->getOg(), true) );
   ret += QObject::tr("ABV: %1%%\n").arg( rec->getABV_pct(), 0, 'f', 1 );
   bitternessString = QObject::tr("Bitterness: %1 IBUs (%2)\n").arg( rec->getIBU(), 0, 'f', 1 );
   switch( Brewtarget::ibuFormula )
   {
      case Brewtarget::TINSETH:
         bitternessString = bitternessString.arg("Tinseth");
         break;
      case Brewtarget::RAGER:
         bitternessString = bitternessString.arg("Rager");
         break;
   }
   ret += bitternessString;
   colorString = QObject::tr("Color: %1 SRM (%2)\n").arg( rec->getColor_srm(), 0, 'f', 0 );
   switch( Brewtarget::colorFormula )
   {
      case Brewtarget::MOREY:
         colorString = colorString.arg("Morey");
         break;
      case Brewtarget::DANIEL:
         colorString = colorString.arg("Daniels");
         break;
      case Brewtarget::MOSHER:
         colorString = colorString.arg("Mosher");
         break;
   }
   ret += colorString;
   
   if( rec->getNumFermentables() > 0 )
   {
      QStringList names, types, amounts, masheds, lates, yields, colors;
      ret += "\n";
      ret += QObject::tr("Fermentables\n");
      ret += getTextSeparator();
      
      //ret += "Name\t\tType\t\tAmount\t\tMashed\t\tYield\t\tColor\n";
      names.append(QObject::tr("Name"));
      types.append(QObject::tr("Type"));
      amounts.append(QObject::tr("Amount"));
      masheds.append(QObject::tr("Mashed"));
      lates.append(QObject::tr("Late"));
      yields.append(QObject::tr("Yield"));
      colors.append(QObject::tr("Color"));
      
      size = rec->getNumFermentables();
      for( i = 0; i < size; ++i )
      {
         Fermentable* ferm =  rec->getFermentable(i);
         names.append( ferm->getName().c_str() );
         types.append( ferm->getTypeString() );
         amounts.append( Brewtarget::displayAmount(ferm->getAmount_kg(), Units::kilograms) );
         masheds.append( ferm->getIsMashed() ? QObject::tr("Yes") : QObject::tr("No") );
         lates.append( ferm->getAddAfterBoil() ? QObject::tr("Yes") : QObject::tr("No") );
         yields.append( QString("%1%%").arg(ferm->getYield_pct(), 0, 'f', 0) );
         colors.append( QString("%1 L").arg(ferm->getColor_srm(), 0, 'f', 0) );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&masheds);
      padAllToMaxLength(&lates);
      padAllToMaxLength(&yields);
      padAllToMaxLength(&colors);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + amounts.at(i) + masheds.at(i) + lates.at(i) + yields.at(i) + colors.at(i) + "\n";

      ret += QObject::tr("Total grain: %1\n").arg(Brewtarget::displayAmount(rec->getGrains_kg(), Units::kilograms));
   }
   
   if( rec->getNumHops() > 0 )
   {
      QStringList names, alphas, amounts, uses, times, forms, ibus;
      ret += "\n";
      ret += QObject::tr("Hops\n");
      ret += getTextSeparator();
      
      //ret += "Name\t\tAlpha\t\tAmount\t\tUse\t\tTime\t\tIBU\n";
      
      names.append( QObject::tr("Name") );
      alphas.append( QObject::tr("Alpha") );
      amounts.append( QObject::tr("Amount") );
      uses.append( QObject::tr("Use") );
      times.append( QObject::tr("Time") );
      forms.append( QObject::tr("Form") );
      ibus.append( QObject::tr("IBU") );
      
      size = rec->getNumHops();
      for( i = 0; i < size; ++i )
      {
         Hop* hop = rec->getHop(i);
         
         names.append( hop->getName().c_str() );
         alphas.append( QString("%1%%").arg(hop->getAlpha_pct(), 0, 'f', 1) );
         amounts.append( Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms) );
         uses.append( hop->getUseString() );
         times.append( Brewtarget::displayAmount(hop->getTime_min(), Units::minutes) );
         forms.append( hop->getFormString() );
         ibus.append( QString("%1").arg( rec->getIBUFromHop(i), 0, 'f', 1 ) );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&alphas);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&uses);
      padAllToMaxLength(&times);
      padAllToMaxLength(&forms);
      padAllToMaxLength(&ibus);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + alphas.at(i) + amounts.at(i) + uses.at(i) + times.at(i) + forms.at(i) + ibus.at(i) + "\n";
   }
   
   if( rec->getNumMiscs() > 0 )
   {
      QStringList names, types, uses, amounts, times;
      ret += "\n";
      ret += QObject::tr("Misc\n");
      ret += getTextSeparator();
      
      names.append(QObject::tr("Name"));
      types.append(QObject::tr("Type"));
      uses.append(QObject::tr("Use"));
      amounts.append(QObject::tr("Amount"));
      times.append(QObject::tr("Time"));
      
      size = rec->getNumMiscs();
      for( i = 0; i < size; ++i )
      {
         Misc* misc = rec->getMisc(i);
         names.append(misc->getName().c_str());
         types.append(misc->getTypeString());
         uses.append(misc->getUseString());
         amounts.append(Brewtarget::displayAmount(misc->getAmount(), misc->getAmountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters));
         times.append( Brewtarget::displayAmount(misc->getTime(), Units::minutes) );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&uses);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&times);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + uses.at(i) + amounts.at(i) + times.at(i) + "\n";
   }
   
   if( rec->getNumYeasts() > 0 )
   {
      QStringList names, types, forms, amounts, stages;
      ret += "\n";
      ret += QObject::tr("Yeast\n");
      ret += getTextSeparator();
      
      names.append(QObject::tr("Name"));
      types.append(QObject::tr("Type"));
      forms.append(QObject::tr("Form"));
      amounts.append(QObject::tr("Amount"));
      stages.append(QObject::tr("Stage"));
      
      size = rec->getNumYeasts();
      for( i = 0; i < size; ++i )
      {
         Yeast* y = rec->getYeast(i);
         names.append( y->getName().c_str() );
         types.append( y->getTypeString() );
         forms.append( y->getFormString() );
         amounts.append( Brewtarget::displayAmount( y->getAmount(), y->getAmountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters ) );
         stages.append( y->getAddToSecondary() ? QObject::tr("Secondary") : QObject::tr("Primary") );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&forms);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&stages);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + forms.at(i) + amounts.at(i) + stages.at(i) + "\n";
   }
   
   if( mash && mash->getNumMashSteps() > 0 )
   {
      QStringList names, types, amounts, targets, times;
      ret += "\n";
      ret += QObject::tr("Mash\n");
      ret += getTextSeparator();
      
      names.append(QObject::tr("Name"));
      types.append(QObject::tr("Type"));
      amounts.append(QObject::tr("Amount"));
      targets.append(QObject::tr("Target"));
      times.append(QObject::tr("Time"));
      
      size = mash->getNumMashSteps();
      for( i = 0; i < size; ++i )
      {
         MashStep* s = mash->getMashStep(i);
         names.append(s->getName().c_str());
         types.append(s->getTypeString());
         if( s->getType() == MashStep::TYPEINFUSION )
            amounts.append( Brewtarget::displayAmount( s->getInfuseAmount_l(), Units::liters ) );
         else if( s->getType() == MashStep::TYPEDECOCTION )
            amounts.append( Brewtarget::displayAmount( s->getDecoctionAmount_l(), Units::liters ) );
         else
            amounts.append( "---" );
         targets.append( Brewtarget::displayAmount( s->getStepTemp_c(), Units::celsius ) );
         times.append( Brewtarget::displayAmount( s->getStepTime_min(), Units::minutes ) );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&targets);
      padAllToMaxLength(&times);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + amounts.at(i) + targets.at(i) + times.at(i) + "\n";
   }
   
   if( rec->getNotes() != "" )
   {
      ret += "\n";
      ret += QObject::tr("Notes\n") + getTextSeparator() + "\n" + rec->getNotes();
   }
   
   if( (rec->instructions).size() > 0 )
   {
      ret += "\n";
      ret += QObject::tr("Instructions\n") + getTextSeparator();
      size = (rec->instructions).size();
      Instruction* ins;
      for( i = 0; i < size; ++i )
      {
         ins = rec->instructions[i];
         ret += QString("%1) %2\n\n").arg(i).arg(ins->getDirections());
      }
   }
   
   return ret;
}

QString RecipeFormatter::getTextSeparator()
{
   if( textSeparator != 0 )
      return *textSeparator;
   
   int i;
   textSeparator = new QString();
   for( i = 0; i < 80; ++i )
      textSeparator->append('=');
   
   textSeparator->append('\n');
   return *textSeparator;
}

QString RecipeFormatter::getHTMLFormat()
{
   return "";
}

QString RecipeFormatter::getBBCodeFormat()
{
   return "";
}

void RecipeFormatter::toTextClipboard()
{
   QApplication::clipboard()->setText(getTextFormat());
}

unsigned int RecipeFormatter::getMaxLength( QStringList* list )
{
   int i;
   int maxlen = 0;

   for( i = 0; i < list->count(); ++i )
   {
      if( list->at(i).size() > maxlen )
         maxlen = list->at(i).size();
   }
   
   return maxlen;
}

QString RecipeFormatter::padToLength( QString str, unsigned int length )
{
   return QString("%1").arg(str, length, ' ');
}

void RecipeFormatter::padAllToMaxLength( QStringList* list )
{
   unsigned int maxlen = getMaxLength( list );
   unsigned int i, size;
   
   size = list->count();
   for( i = 0; i < size; ++i )
      list->replace( i, padToLength( list->at(i), maxlen+1 ) ); // Add one so that text doesn't run together.
}

QString RecipeFormatter::getCSS()
{
   if ( cssName == NULL )
       cssName = tr(":/css/recipe.css");

   QFile cssInput(cssName);
   QString css;

   if (cssInput.open(QFile::ReadOnly)) {
      QTextStream inStream(&cssInput);
      while ( ! inStream.atEnd() )
      {
         css += inStream.readLine();
      }
   }
   return css;
}

QString RecipeFormatter::buildTitleTable()
{
   QString header;
   QString body;
   QString color;
   QString bitterness;

   if ( rec == 0 )
	   return "";

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += getCSS();
   header += "</style></head>";

   body   = "<body>";
   body += tr("<h1>%1</h1>").arg(rec->getName().c_str());

   // Build the top table
   // Build the first row: Batch Size and Boil Size
   body += "<table id=\"title\">";
   body += tr("<tr><td class=\"left\">Batch Size</td><td class=\"value\">%1</td>")
           .arg(Brewtarget::displayAmount(rec->getBatchSize_l(), Units::liters));
   body += tr("<td class=\"right\">Boil Size</td><td class=\"value\">%1</td></tr>")
           .arg(Brewtarget::displayAmount(rec->getBoilSize_l(), Units::liters));
   // Second row: Boil Time and Efficiency
   body += tr("<tr><td class=\"left\">Boil Time</td><td class=\"value\">%1</td>")
           .arg( (rec->getEquipment() == 0)?
                   Brewtarget::displayAmount(0, Units::minutes)
                 : Brewtarget::displayAmount( (rec->getEquipment())->getBoilTime_min(), Units::minutes));
   body += tr("<td class=\"right\">Efficiency</td><td class=\"value\">%1%</td></tr>")
           .arg(rec->getEfficiency_pct(), 0, 'f', 0);

   // Third row: OG and FG
   body += tr("<tr><td class=\"left\">OG</td><td class=\"value\">%1</td>")
           .arg(Brewtarget::displayOG(rec->getOg(), true ));
   body += tr("<td class=\"right\">FG</td><td class=\"value\">%1</td></tr>")
           .arg(Brewtarget::displayFG(rec->getFg(), rec->getOg(), true));

   // Fourth row: ABV and Bitterness.  We need to set the bitterness string up first
   bitterness = tr("%1 IBUs (%2)").arg( rec->getIBU(), 0, 'f', 1);
   switch ( Brewtarget::ibuFormula )
   {
	   case Brewtarget::TINSETH:
		   bitterness.arg("Tinseth");
		   break;
	   case Brewtarget::RAGER:
		   bitterness.arg("Rager");
		   break;
   }
   body += tr("<tr><td class=\"left\">ABV</td><td class=\"value\">%1%</td>")
           .arg(rec->getABV_pct(), 0, 'f', 1);
   body += tr("<td class=\"right\">Bitterness</td><td class=\"value\">%1</td></tr>")
           .arg(bitterness);
   // Fifth row: Color and calories.  Set up the color string first
   color = tr("%1 SRM (%2)").arg(rec->getColor_srm(), 0, 'f', 0);
   switch( Brewtarget::colorFormula )
   {
	   case Brewtarget::MOREY:
		   color = color.arg("Morey");
		   break;
	   case Brewtarget::DANIEL:
		   color = color.arg("Daniels");
		   break;
	   case Brewtarget::MOSHER:
		   color = color.arg("Mosher");
		   break;
   }
   //body += tr("<tr><td class=\"left\">Color</td><td class=\"value\">%1%</td>")
   body += tr("<tr><td class=\"left\">Color</td><td class=\"value\">%1</td>")
           .arg(color);
   body += tr("<td class=\"right\">Calories (per 12 ounces)</td><td class=\"value\">%1</td></tr>")
           .arg(rec->estimateCalories(), 0, 'f', 0);

   body += "</table>";

   return header + body;

}

QString RecipeFormatter::buildFermentableTable()
{
	QString ftable;
	if ( rec == 0 || rec->getNumFermentables() < 1 )
		return "";

	ftable = tr("<h3>Fermentables</h3>");
	ftable += tr("<table id=\"fermentables\"");
	ftable += tr("<caption>Total grain: %1</caption>")
			.arg(Brewtarget::displayAmount(rec->getGrains_kg(), Units::kilograms));
	// Set up the header row.
	ftable += tr("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th><th>%6</th><th>%7</th></tr>")
			.arg("Name")
			.arg("Type")
			.arg("Amount")
			.arg("Mashed")
			.arg("Late")
			.arg("Yield")
			.arg("Color");
	// Now add a row for each fermentable
	for(unsigned int i=0; i < rec->getNumFermentables(); ++i)
	{
		Fermentable* ferm = rec->getFermentable(i);
		ftable += "<tr>";
		ftable += tr("<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6%</td><td>%7 L</td>")
				.arg( ferm->getName().c_str())
				.arg( ferm->getTypeString())
				.arg( Brewtarget::displayAmount(ferm->getAmount_kg(), Units::kilograms))
				.arg( ferm->getIsMashed() ? "Yes" : "No" )
				.arg( ferm->getAddAfterBoil() ? "Yes" : "No")
				.arg( ferm->getYield_pct(), 0, 'f', 0)
				.arg( ferm->getColor_srm(), 0, 'f', 0);
		ftable += "</tr>";
	}
	ftable += "</table>";
	return ftable;
}

QString RecipeFormatter::buildHopsTable()
{
	QString hTable;

	if ( rec == 0 || rec->getNumHops() < 1 )
		return "";

	hTable = tr("<h3>Hops</h3>");
	hTable += tr("<table id=\"hops\"");
	// Set up the header row.
	hTable += tr("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th><th>%6</th><th>%7</th></tr>")
			.arg("Name")
			.arg("Alpha")
			.arg("Amount")
			.arg("Use")
			.arg("Time")
			.arg("Form")
			.arg("IBU");
	for( unsigned int i = 0; i < rec->getNumHops(); ++i)
	{
		Hop *hop = rec->getHop(i);
		hTable += tr("<td>%1</td><td>%2%</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7%</td>")
				.arg( hop->getName().c_str())
				.arg( hop->getAlpha_pct(), 0, 'f', 0)
				.arg( Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms))
				.arg( hop->getUseString())
				.arg( Brewtarget::displayAmount(hop->getTime_min(), Units::minutes) )
				.arg( hop->getFormString())
				.arg( rec->getIBUFromHop(i), 0, 'f', 1);
		hTable += "</tr>";
	}
	hTable += "</table>";
	return hTable;
}

QString RecipeFormatter::buildMiscTable()
{
	QString mtable;

	if ( rec == 0 || rec->getNumMiscs() < 1 )
		return "";

	mtable = tr("<h3>Misc</h3>");
	mtable += tr("<table id=\"misc\"");
	// Set up the header row.
	mtable += tr("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th></tr>")
			.arg("Name")
			.arg("Type")
			.arg("Use")
			.arg("Amount")
			.arg("Time");
	for( unsigned int i = 0; i < rec->getNumMiscs(); ++i)
	{
		Misc *misc = rec->getMisc(i);
		mtable += tr("<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td>")
				.arg( misc->getName().c_str())
				.arg( misc->getTypeString())
				.arg( misc->getUseString())
				.arg( Brewtarget::displayAmount(misc->getAmount(), misc->getAmountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters))
				.arg( Brewtarget::displayAmount(misc->getTime(), Units::minutes) );
		mtable += "</tr>";
	}
	mtable += "</table>";
	return mtable;

}

QString RecipeFormatter::buildYeastTable()
{
	QString ytable;

	if ( rec == 0 || rec->getNumYeasts() < 1 )
		return "";

	ytable = tr("<h3>Yeast</h3>");
	ytable += tr("<table id=\"yeast\"");
	// Set up the header row.
	ytable += tr("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th></tr>")
			.arg("Name")
			.arg("Type")
			.arg("Form")
			.arg("Amount")
			.arg("Stage");
	for( unsigned int i = 0; i < rec->getNumYeasts(); ++i)
	{
		Yeast *y = rec->getYeast(i);
		ytable += tr("<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td>")
				.arg( y->getName().c_str())
				.arg( y->getTypeString())
				.arg( y->getFormString())
				.arg( Brewtarget::displayAmount( y->getAmount(), y->getAmountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters ) )
				.arg( y->getAddToSecondary() ? "Secondary" : "Primary");
		ytable += "</tr>";
	}
	ytable += "</table>";
	return ytable;
}

QString RecipeFormatter::buildNotes()
{
	QString notes;

	if ( rec == 0 || rec->getNotes() == "" )
		return "";

	notes = "<h3>Notes</h3>";
	notes += tr("<pre>%1</pre>").arg( rec->getNotes());

	return notes;
}

QString RecipeFormatter::buildInstructionTable()
{
   QString itable;

   if ( rec == 0 || rec->getNumInstructions() < 1 )
	   return "";

   itable = "<h3>Instructions</h3>";
   itable += "<ol id=\"instruction\">";

   for(int i = 0; i < rec->getNumInstructions(); ++i )
   {
	   Instruction* ins = rec->getInstruction(i);
	   itable += tr("<li>%1</li>").arg( ins->getDirections());
   }

   itable += "</table>";

   return itable;
}

bool RecipeFormatter::loadComplete(bool ok)
{
   doc->print(printer);
   disconnect( doc, SIGNAL(loadFinished(bool)), this, SLOT(loadComplete(bool)) );
   return ok;
}

void RecipeFormatter::print(QPrinter* mainPrinter, QPrintDialog *dialog)
{
   QString pDoc;
//   QPrintDialog *dialog = new QPrintDialog(printer, parent);

   printer = mainPrinter;
   /* Instantiate the Webview and then connect its signal */
   connect( doc, SIGNAL(loadFinished(bool)), this, SLOT(loadComplete(bool)) );

   dialog->setWindowTitle(tr("Print Document"));
   if (dialog->exec() != QDialog::Accepted)
      return;

   if( rec == 0 )
      return;

   // Start building the document to be printed.  I think.
   pDoc = buildTitleTable();
   pDoc += buildFermentableTable();
   pDoc += buildHopsTable();
   pDoc += buildMiscTable();
   pDoc += buildYeastTable();
   pDoc += buildNotes();
   pDoc += buildInstructionTable();

   pDoc += "</body></html>";

   doc->setHtml(pDoc);
}

void RecipeFormatter::printPreview()
{
   QString pDoc;

   if( rec == 0 )
      return;

   // Start building the document to be printed.  I think.
   pDoc = buildTitleTable();
   pDoc += buildFermentableTable();
   pDoc += buildHopsTable();
   pDoc += buildMiscTable();
   pDoc += buildYeastTable();
   pDoc += buildNotes();
   pDoc += buildInstructionTable();
   pDoc += "</body></html>";

   doc->setHtml(pDoc);
   doc->show();
}
