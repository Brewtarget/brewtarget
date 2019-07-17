
// Define tables names first, because I need them first
static QString ktableMeta("bt_alltables");
static QString ktableSettings("settings");
static QString ktableEquipment("equipment");
static QString ktableFermentable("fermentable");
static QString ktableHop("hop");
static QString ktableMisc("misc");
static QString ktableYeast("yeast");
static QString ktableWater("water");
static QString ktableMash("mash");
static QString ktableMashStep("mashstep");
static QString ktableBrewnote("brewnote");
static QString ktableInstruction("instruction");
static QString ktableRecipe("recipe");

// BT default tables
static QString ktableBtEquipment("bt_equipment");
static QString ktableBtFermentable("bt_fermentable");
static QString ktableBtHop("bt_hop");
static QString ktableBtMisc("bt_misc");
static QString ktableBtStyle("bt_style");
static QString ktableBtYeast("bt_yeast");
static QString ktableBtWater("bt_water");

// In recipe tables
static QString ktableFermInRec("fermentable_in_recipe");
static QString ktableHopInRec("hop_in_recipe");
static QString ktableMiscInRec("misc_in_recipe");
static QString ktableWaterInRec("water_in_recipe");
static QString ktableYeastInRec("yeast_in_recipe");
static QString ktableInsInRec("instruction_in_recipe");

// Children tables
static QString ktableEquipChildren("equipment_children");
static QString ktableFermChildren("fermentable_children");
static QString ktableHopChildren("hop_children");
static QString ktableMiscChildren("misc_children");
static QString ktableRecChildren("recipe_children");
static QString ktableStyleChildren("style_children");
static QString ktableWaterChildren("water_children");
static QString ktableYeastChildren("yeast_children");

// Inventory tables
static QString ktableFermInventory("fermentable_in_inventory");
static QString ktableHopInventory("hop_in_inventory");
static QString ktableMiscInventory("misc_in_inventory");
static QString ktableYeastInventory("yeast_in_inventory");

// These properties are pretty consistent over all objects, so defined them
// once and be done with it
const QString kpropName("name");
const QString kpropNotes("notes");
const QString kpropDeleted("deleted");
const QString kpropDisplay("display");
const QString kpropFolder("folder");

// Same for these column names. Yes, but I think the consistency is better
// this way
const QString kcolName("name");
const QString kcolNotes("notes");
const QString kcolDeleted("deleted");
const QString kcolDisplay("display");
const QString kcolFolder("folder");

const QString kxmlPropName("NAME");
const QString kxmlPropNotes("NOTES");

