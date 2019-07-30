
// Define tables names first, because I need them first
static const QString ktableMeta("bt_alltables");
static const QString ktableSettings("settings");
static const QString ktableEquipment("equipment");
static const QString ktableFermentable("fermentable");
static const QString ktableHop("hop");
static const QString ktableMisc("misc");
static const QString ktableYeast("yeast");
static const QString ktableWater("water");
static const QString ktableMash("mash");
static const QString ktableMashStep("mashstep");
static const QString ktableBrewnote("brewnote");
static const QString ktableInstruction("instruction");
static const QString ktableRecipe("recipe");

// BT default tables
static const QString ktableBtEquipment("bt_equipment");
static const QString ktableBtFermentable("bt_fermentable");
static const QString ktableBtHop("bt_hop");
static const QString ktableBtMisc("bt_misc");
static const QString ktableBtStyle("bt_style");
static const QString ktableBtYeast("bt_yeast");
static const QString ktableBtWater("bt_water");

// In recipe tables
static const QString ktableFermInRec("fermentable_in_recipe");
static const QString ktableHopInRec("hop_in_recipe");
static const QString ktableMiscInRec("misc_in_recipe");
static const QString ktableWaterInRec("water_in_recipe");
static const QString ktableYeastInRec("yeast_in_recipe");
static const QString ktableInsInRec("instruction_in_recipe");

// Children tables
static const QString ktableEquipChildren("equipment_children");
static const QString ktableFermChildren("fermentable_children");
static const QString ktableHopChildren("hop_children");
static const QString ktableMiscChildren("misc_children");
static const QString ktableRecChildren("recipe_children");
static const QString ktableStyleChildren("style_children");
static const QString ktableWaterChildren("water_children");
static const QString ktableYeastChildren("yeast_children");

// Inventory tables
static const QString ktableFermInventory("fermentable_in_inventory");
static const QString ktableHopInventory("hop_in_inventory");
static const QString ktableMiscInventory("misc_in_inventory");
static const QString ktableYeastInventory("yeast_in_inventory");

// These properties are pretty consistent over all objects, so defined them
// once and be done with it
static const QString kpropName("name");
static const QString kpropNotes("notes");
static const QString kpropType("type");
static const QString kpropDeleted("deleted");
static const QString kpropAmount("amount_kg");
static const QString kpropDisplay("display");
static const QString kpropFolder("folder");
static const QString kpropOrigin("origin");
static const QString kpropUse("use");
static const QString kpropInventory("inventory");
static const QString kpropTime("time_min");

// Same for these column names. Yes, but I think the consistency is better
// this way
static const QString kcolName("name");
static const QString kcolNotes("notes");
static const QString kcolDeleted("deleted");
static const QString kcolDisplay("display");
static const QString kcolFolder("folder");
static const QString kcolInventory("amount");
static const QString kcolAmount("amount");
static const QString kcolOrigin("origin");
static const QString kcolUse("use");
static const QString kcolSubstitutes("substitutes");
static const QString kcolTime("time");

// And things that are identical in beerxml
static const QString kxmlPropName("NAME");
static const QString kxmlPropNotes("NOTES");
static const QString kxmlPropAmount("AMOUNT");
static const QString kxmlPropSubstitutes("SUBSTITUTES");
static const QString kxmlPropTime("TIME");

