# PrintPreviewDialog
This dialog is an attempt to get the printing to be *cross-platform-WYSIWYG* in that it uses the QT standard way of generating the printout and preview.
I also aimed to make a unified print/export dialog instead of having multiple menu items saying print-this and print-that.

## BtPage namespace
I created this namespace to keep everything in one bubble of code and separate from the rest of Brewtarget. this was to minimize the risk of running into duplicate naming.
This namespace includes all that is necessary for generating the printout and Html-export to file, while relying on Brewtargets RecipeFormatter- and BrewNoteFormatter-class to get the data for generating the pages. I'm not sure this is the way it should be in the future but will leave it there for now.

### Page class
This class is used to store all child elements/classes and placing them on the page. this class also contains pointer to the printer in use and is responsible for calling all page-items ::render() method to get everything printed onto the page.
This is generated every you change your selection for printout or the preview needs to refresh for any reason.
#### Functions:
- **template \<class T> auto addChildObject(T \*obj, QPoint position = QPoint()) -> decltype(obj)**\
You add content to the page by creating and adding PageChildObject-subclasses, see below. this returns a pointer to the added object if you need to make more setting to is, i.e. adding a style to a table
- **renderPage()**\
Renders the page on the set printer/painter
- **template \<class T, class S> void placeRelationalTo(T \*targetObj, S \*sourceObj, RelationalPlacingFlags place, int xOffset = 0 /\* pixels \*/, int yOffset = 0 /\* pixels \*/)**\
Sets the postition of targetObj relational to the sourceObj according to the RelationalPlacingFlags. you can also offset the postion using the Offset arguments.
- **template \<class T, class S> void placeRelationalToMM(T \*targetObj, S \*sourceObj, RelationalPlacingFlags place, int xOffset = 0 /\* Millimeter \*/ , int yOffset = 0 /\* Millimeter \*/)**\
Same as placeRelationalTo(..) but offset is in millimeters and this will calculate the distance in pixels according to the set printer and then call the original function with those values.
- **template \<class T> void placeOnPage(T \*targetObj, FixedPlacingFlags place, int xOffset = 0, int yOffset = 0)**\
This will set the position on the supplied targetObj according to the FixedPlacingFlags provided. this funtion is a convinient way of placing anyting to, for example, the TOP-RIGHT corner of the page or why not the center of the page. more on [PlacingFlags](#placingflags) below.
### PageChildObject
 This is a abstract / Interface class for all child object to put on the page. It holds some basic informations about the object and makes sure that all derived classes implement a few methods, i.e. a render method, because all children need to know how to draw it self onto a QPainter and ultimately onto a page.\
This class also contains information on the bounding box and position that is the same for all derived classes.

#### Derived Classes
These are the derived classes for now, in the future I'll implement more children to draw more kind of objects on the page, for example arcs or circles or lines. I'm contemplating to have the MainWindows rangeWidgets print out on page, I think that would look good, Not there yet though. :)

##### PageText object
This holds a text to be generated onto the page, this is used for all text to be generated onto a page whether it be a single header added directly to BtPage object or added to a [PageTable object](#pagetable-object). This object has properties like Font to display correctly onto the page.

##### PageTable object
This object takes a QList\<QStringList> to generate a table on a BtPage. For now this will auto-size it self to the content of the table, but this is subject to change in the future to be able to set in more detail all columns and rows sizes to better "style" the table so to speak.\
This also contains a QList\<[PageTableStyle](#pagetablestyle-object)> which is a **small** implementation on styling the table, read more below.
You define columns by adding a [PageTableColumn](#pagetablecolumn-object) to the PageTable::columnHeaders pointer. these contain the formatting for the column and the header text.


##### PageImage object
Draws an image on the page (aspiring MUC-Award winner! :) )\
Contains the size of the image and can resize when needed.

##### PageBreak object
This small little class does one thing, and one thing only....\
Signals the page-renderer that there should be a page break before drawing the next object.

### Helper classes
#### PageTableStyle object
***THIS IS NOT IMPLEMENTED YET***
Create this style as you wish( read 'can') and then add it to the PageTable object to style the tables output on screen.
*NOTE: this does nothing to the output on screen/paper/file yet, it's an idea of implementing style to printout/pdfs but need more thought.

#### PageTableColumn object
Defines a column with attributes ina table. see [PageTable](#pagetable-object).


### Enums and Flags
#### PlacingFlags
Places an object on the page or relational to another object according to the flags below.\
they can be concatinated by using the | operator.\
Example:
`page.placeOnPage(myChildObject, PlacingFlags::TOP | PlacingFlags::RIGHT, 0, -10)`\

Definitions:

            enum struct RelationalPlacingFlags
            {
               /* These four are used with Page::placeRelationalto(..) and Page::placeRelationalToMM(..) functions */
               BELOW        = 1, // (1<<0) = 1! 00000000 00000000 00000000 00000001 (1)
               ABOVE        = (1<<1),         //00000000 00000000 00000000 00000010 (2)
               RIGHTOF      = (1<<2),         //00000000 00000000 00000000 00000100 (4)
               LEFTOF       = (1<<3),         //00000000 00000000 00000000 00001000 (8)
            };

            enum struct FixedPlacingFlags
            {
               CUSTOM       = 0,
               /* These six are used with Page::placeOnPage(...) function */
               LEFT         = 1,      // (1<<0) = 1, 00000000 00000000 00000000 00010000 (16)
               RIGHT        = (1<<1),              //00000000 00000000 00000000 00100000 (32)
               TOP          = (1<<2),              //00000000 00000000 00000000 01000000 (64)
               BOTTOM       = (1<<3),              //00000000 00000000 00000000 10000000 (128)
               VCENTER      = (1<<4),              //00000000 00000000 00000001 00000000 (256)
               HCENTER      = (1<<5)               //00000000 00000000 00000010 00000000 (512)
            };

### Example:
This will create a page, add the recipe and Brewers name as well as Brewtargets logo to it and then render it to the printer.


```
   using namespace nBtPage;
   QPrinter *printer;
   BtPage page(printer);
   // adding the Recipe name as a title.
   PageText *recipeText = page.addChildObject(
      new PageText (
         &page,
         _parent->currentRecipe()->name(),
         QFont("Arial", 18, QFont::Bold)
      ),
      QPoint(0,0)
      );
   // adding Brewers name
   PageText *brewerText = page.addChildObject(
      new PageText (
         &page,
         _parent->currentRecipe()->brewer(),
         QFont("Arial", 10)
      ));
   page.placeRelationalToMM(brewerText, recipeText, BtPage::BELOW, 2, 0);

   //Adding the Brewtarget logo.
   PageImage *img = page.addChildObject(
      new PageImage (
         &page,
         QPoint(),
         QImage(":/images/title.svg")
      ));
   img->setImageSizeMM(100, 20);
   //Since we are using the 'using namespace BtPage' above the 'BtPage::TOP | BtPage::RIGHT' can be written as 'TOP | RIGHT' as well.
   //But for clarity I'm being explicit here.
   page.placeOnPage(img, BtPage::TOP | BtPage::RIGHT);

   page.renderPage();

```



***Trivia:***\
*MUC Awards = Most Useless Comment Awards"*
