/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * database/ObjectStoreTyped.cpp is part of Brewtarget, and is copyright the following authors 2021-2024:
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
#include "database/ObjectStoreTyped.h"

#include  <mutex> // for std::once_flag

#include "database/DbTransaction.h"
#include "measurement/Unit.h"
#include "model/Boil.h"
#include "model/BoilStep.h"
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/FermentationStep.h"
#include "model/Hop.h"
#include "model/Instruction.h"
#include "model/Inventory.h"
#include "model/InventoryFermentable.h"
#include "model/InventoryHop.h"
#include "model/InventoryMisc.h"
#include "model/InventorySalt.h"
#include "model/InventoryYeast.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionMisc.h"
#include "model/RecipeAdditionYeast.h"
#include "model/RecipeAdjustmentSalt.h"
#include "model/RecipeUseOfWater.h"
#include "model/Recipe.h"
#include "model/Salt.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"

namespace {
   //
   // By the magic of templated variables and template specialisation, we have below all the constructor parameters for
   // each type of ObjectStoreTyped.
   //
   // The only wrinkle here is that the order of definitions matters, eg the definition of PRIMARY_TABLE<BrewNote>
   // needs to appear after that of PRIMARY_TABLE<Recipe>, as the address of the latter is used in the former (to show
   // foreign key references).  However, as long as we don't want circular foreign key references in the database,
   // there should always be an order that works!
   //
   // We only use specialisations of these templates (PRIMARY_TABLE and JUNCTION_TABLES)
   //
   // It would be nice to be able to write here:
   //    template<class NE> ObjectStore::TableDefinition          const PRIMARY_TABLE   = delete;
   //    template<class NE> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES = delete;
   // However, this is not currently valid C++.  There was a proposal that would have made it so (see
   // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2041r1.html) however, AFAICT this never made it into the
   // standard.
   //
   template<class NE> ObjectStore::TableDefinition          const PRIMARY_TABLE  {"", {}};
   template<class NE> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES{};

   //
   // NOTE: Unlike C++, SQL is generally case-insensitive, so we have slightly different naming conventions.
   //       Specifically, we use snake_case rather than camelCase for field and table names.  By convention, we also
   //       use upper case for SQL keywords and lower case for everything else.  This is in pursuit of making SQL
   //       slightly more readable.
   //

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Equipment
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Equipment> {
      "equipment",
      {
         {ObjectStore::FieldType::Int   , "id"                            , PropertyNames::NamedEntity::key                      },
         {ObjectStore::FieldType::String, "name"                          , PropertyNames::NamedEntity::name                     },
         {ObjectStore::FieldType::Bool  , "display"                       , PropertyNames::NamedEntity::display                  },
         {ObjectStore::FieldType::Bool  , "deleted"                       , PropertyNames::NamedEntity::deleted                  },
         {ObjectStore::FieldType::String, "folder"                        , PropertyNames::FolderBase::folder         },
         {ObjectStore::FieldType::Double, "fermenter_batch_size_l"        , PropertyNames::Equipment::fermenterBatchSize_l       },
         {ObjectStore::FieldType::Double, "boiling_point"                 , PropertyNames::Equipment::boilingPoint_c             },
         {ObjectStore::FieldType::Double, "kettle_boil_size_l"            , PropertyNames::Equipment::kettleBoilSize_l           },
         {ObjectStore::FieldType::Double, "boil_time"                     , PropertyNames::Equipment::boilTime_min               },
         {ObjectStore::FieldType::Bool  , "calc_boil_volume"              , PropertyNames::Equipment::calcBoilVolume             },
         {ObjectStore::FieldType::Double, "kettle_evaporation_per_hour_l" , PropertyNames::Equipment::kettleEvaporationPerHour_l },
         {ObjectStore::FieldType::Double, "evap_rate"                     , PropertyNames::Equipment::evapRate_pctHr             },
         {ObjectStore::FieldType::Double, "mash_tun_grain_absorption_lkg" , PropertyNames::Equipment::mashTunGrainAbsorption_LKg },
         {ObjectStore::FieldType::Double, "hop_utilization"               , PropertyNames::Equipment::hopUtilization_pct         },
         {ObjectStore::FieldType::Double, "lauter_tun_deadspace_loss_l"   , PropertyNames::Equipment::lauterTunDeadspaceLoss_l   },
         {ObjectStore::FieldType::String, "kettle_notes"                  , PropertyNames::Equipment::kettleNotes                },
         {ObjectStore::FieldType::Double, "top_up_kettle"                 , PropertyNames::Equipment::topUpKettle_l              },
         {ObjectStore::FieldType::Double, "top_up_water"                  , PropertyNames::Equipment::topUpWater_l               },
         {ObjectStore::FieldType::Double, "kettle_trub_chiller_loss_l"    , PropertyNames::Equipment::kettleTrubChillerLoss_l    },
         {ObjectStore::FieldType::Double, "mash_tun_specific_heat_calgc"  , PropertyNames::Equipment::mashTunSpecificHeat_calGC  },
         {ObjectStore::FieldType::Double, "mash_tun_volume_l"             , PropertyNames::Equipment::mashTunVolume_l            },
         {ObjectStore::FieldType::Double, "mash_tun_weight_kg"            , PropertyNames::Equipment::mashTunWeight_kg           },
         {ObjectStore::FieldType::Double, "kettleInternalDiameter_cm"     , PropertyNames::Equipment::kettleInternalDiameter_cm  },
         {ObjectStore::FieldType::Double, "kettleOpeningDiameter_cm"      , PropertyNames::Equipment::kettleOpeningDiameter_cm   },
         // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
         {ObjectStore::FieldType::String, "hlt_type"                      , PropertyNames::Equipment::hltType                    },
         {ObjectStore::FieldType::String, "mash_tun_type"                 , PropertyNames::Equipment::mashTunType                },
         {ObjectStore::FieldType::String, "lauter_tun_type"               , PropertyNames::Equipment::lauterTunType              },
         {ObjectStore::FieldType::String, "kettle_type"                   , PropertyNames::Equipment::kettleType                 },
         {ObjectStore::FieldType::String, "fermenter_type"                , PropertyNames::Equipment::fermenterType              },
         {ObjectStore::FieldType::String, "agingvessel_type"              , PropertyNames::Equipment::agingVesselType            },
         {ObjectStore::FieldType::String, "packaging_vessel_type"         , PropertyNames::Equipment::packagingVesselType        },
         {ObjectStore::FieldType::Double, "hlt_volume_l"                  , PropertyNames::Equipment::hltVolume_l                },
         {ObjectStore::FieldType::Double, "lauter_tun_volume_l"           , PropertyNames::Equipment::lauterTunVolume_l          },
         {ObjectStore::FieldType::Double, "aging_vessel_volume_l"         , PropertyNames::Equipment::agingVesselVolume_l        },
         {ObjectStore::FieldType::Double, "packaging_vessel_volume_l"     , PropertyNames::Equipment::packagingVesselVolume_l    },
         {ObjectStore::FieldType::Double, "hlt_loss_l"                    , PropertyNames::Equipment::hltLoss_l                  },
         {ObjectStore::FieldType::Double, "mash_tun_loss_l"               , PropertyNames::Equipment::mashTunLoss_l              },
         {ObjectStore::FieldType::Double, "fermenter_loss_l"              , PropertyNames::Equipment::fermenterLoss_l            },
         {ObjectStore::FieldType::Double, "aging_vessel_loss_l"           , PropertyNames::Equipment::agingVesselLoss_l          },
         {ObjectStore::FieldType::Double, "packaging_vessel_loss_l"       , PropertyNames::Equipment::packagingVesselLoss_l      },
         {ObjectStore::FieldType::Double, "kettle_outflow_per_minute_l"   , PropertyNames::Equipment::kettleOutflowPerMinute_l   },
         {ObjectStore::FieldType::Double, "hlt_weight_kg"                 , PropertyNames::Equipment::hltWeight_kg               },
         {ObjectStore::FieldType::Double, "lauter_tun_weight_kg"          , PropertyNames::Equipment::lauterTunWeight_kg         },
         {ObjectStore::FieldType::Double, "kettle_weight_kg"              , PropertyNames::Equipment::kettleWeight_kg            },
         {ObjectStore::FieldType::Double, "hlt_specific_heat_calgc"       , PropertyNames::Equipment::hltSpecificHeat_calGC      },
         {ObjectStore::FieldType::Double, "lauter_tun_specific_heat_calgc", PropertyNames::Equipment::lauterTunSpecificHeat_calGC},
         {ObjectStore::FieldType::Double, "kettle_specific_heat_calgc"    , PropertyNames::Equipment::kettleSpecificHeat_calGC   },
         {ObjectStore::FieldType::String, "hlt_notes"                     , PropertyNames::Equipment::hltNotes                   },
         {ObjectStore::FieldType::String, "mash_tun_notes"                , PropertyNames::Equipment::mashTunNotes               },
         {ObjectStore::FieldType::String, "lauter_tun_notes"              , PropertyNames::Equipment::lauterTunNotes             },
         {ObjectStore::FieldType::String, "fermenter_notes"               , PropertyNames::Equipment::fermenterNotes             },
         {ObjectStore::FieldType::String, "aging_vessel_notes"            , PropertyNames::Equipment::agingVesselNotes           },
         {ObjectStore::FieldType::String, "packaging_vessel_notes"        , PropertyNames::Equipment::packagingVesselNotes       },
      }
   };
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<Equipment> {
      // NamedEntity objects store their parents not their children, so this view of the junction table is from the child's point of view
      {
         "equipment_children",
         {
            {ObjectStore::FieldType::Int, "id"                                                                         },
            {ObjectStore::FieldType::Int, "child_id",  PropertyNames::NamedEntity::key,       &PRIMARY_TABLE<Equipment>},
            {ObjectStore::FieldType::Int, "parent_id", PropertyNames::NamedEntity::parentKey, &PRIMARY_TABLE<Equipment>},
         },
         ObjectStore::MAX_ONE_ENTRY
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Fermentable
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Fermentable> {
      "fermentable",
      {
         {ObjectStore::FieldType::Int   , "id"                            , PropertyNames::NamedEntity::key                      },
         {ObjectStore::FieldType::String, "name"                          , PropertyNames::NamedEntity::name                     },
         {ObjectStore::FieldType::Bool  , "deleted"                       , PropertyNames::NamedEntity::deleted                  },
         {ObjectStore::FieldType::Bool  , "display"                       , PropertyNames::NamedEntity::display                  },
         {ObjectStore::FieldType::String, "folder"                        , PropertyNames::FolderBase::folder                    },
         {ObjectStore::FieldType::Double, "coarse_fine_diff"              , PropertyNames::Fermentable::coarseFineDiff_pct       },
         {ObjectStore::FieldType::Double, "color"                         , PropertyNames::Fermentable::color_srm                },
         {ObjectStore::FieldType::Double, "diastatic_power"               , PropertyNames::Fermentable::diastaticPower_lintner   },
         {ObjectStore::FieldType::Enum  , "ftype"                         , PropertyNames::Fermentable::type                     , &Fermentable::typeStringMapping},
         {ObjectStore::FieldType::Double, "ibu_gal_per_lb"                , PropertyNames::Fermentable::ibuGalPerLb              },
         {ObjectStore::FieldType::Double, "max_in_batch"                  , PropertyNames::Fermentable::maxInBatch_pct           },
         {ObjectStore::FieldType::Double, "moisture"                      , PropertyNames::Fermentable::moisture_pct             },
         {ObjectStore::FieldType::String, "notes"                         , PropertyNames::Fermentable::notes                    },
         {ObjectStore::FieldType::String, "origin"                        , PropertyNames::Fermentable::origin                   },
         {ObjectStore::FieldType::String, "supplier"                      , PropertyNames::Fermentable::supplier                 },
         {ObjectStore::FieldType::Double, "protein"                       , PropertyNames::Fermentable::protein_pct              },
         {ObjectStore::FieldType::Bool  , "recommend_mash"                , PropertyNames::Fermentable::recommendMash            },
         // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
         {ObjectStore::FieldType::Enum  , "grain_group"                   , PropertyNames::Fermentable::grainGroup               , &Fermentable::grainGroupStringMapping},
         {ObjectStore::FieldType::String, "producer"                      , PropertyNames::Fermentable::producer                 },
         {ObjectStore::FieldType::String, "product_id"                    , PropertyNames::Fermentable::productId                },
         {ObjectStore::FieldType::Double, "fine_grind_yield_pct"          , PropertyNames::Fermentable::fineGrindYield_pct       }, // Replaces yield / yield_pct
         {ObjectStore::FieldType::Double, "coarse_grind_yield_pct"        , PropertyNames::Fermentable::coarseGrindYield_pct     },
         {ObjectStore::FieldType::Double, "potential_yield_sg"            , PropertyNames::Fermentable::potentialYield_sg        },
         {ObjectStore::FieldType::Double, "alpha_amylase_dext_units"      , PropertyNames::Fermentable::alphaAmylase_dextUnits   },
         {ObjectStore::FieldType::Double, "kolbach_index_pct"             , PropertyNames::Fermentable::kolbachIndex_pct         },
         {ObjectStore::FieldType::Double, "hardness_prp_glassy_pct"       , PropertyNames::Fermentable::hardnessPrpGlassy_pct    },
         {ObjectStore::FieldType::Double, "hardness_prp_half_pct"         , PropertyNames::Fermentable::hardnessPrpHalf_pct      },
         {ObjectStore::FieldType::Double, "hardness_prp_mealy_pct"        , PropertyNames::Fermentable::hardnessPrpMealy_pct     },
         {ObjectStore::FieldType::Double, "kernel_size_prp_plump_pct"     , PropertyNames::Fermentable::kernelSizePrpPlump_pct   },
         {ObjectStore::FieldType::Double, "kernel_size_prp_thin_pct"      , PropertyNames::Fermentable::kernelSizePrpThin_pct    },
         {ObjectStore::FieldType::Double, "friability_pct"                , PropertyNames::Fermentable::friability_pct           },
         {ObjectStore::FieldType::Double, "di_ph"                         , PropertyNames::Fermentable::di_ph                    },
         {ObjectStore::FieldType::Double, "viscosity_cp"                  , PropertyNames::Fermentable::viscosity_cP             },
         {ObjectStore::FieldType::Double, "dmsp_ppm"                      , PropertyNames::Fermentable::dmsP_ppm                 },
         {ObjectStore::FieldType::Double, "fan_ppm"                       , PropertyNames::Fermentable::fan_ppm                  },
         {ObjectStore::FieldType::Double, "fermentability_pct"            , PropertyNames::Fermentable::fermentability_pct       },
         {ObjectStore::FieldType::Double, "beta_glucan_ppm"               , PropertyNames::Fermentable::betaGlucan_ppm           },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for InventoryFermentable
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<InventoryFermentable> {
      "fermentable_in_inventory",
      {
         {ObjectStore::FieldType::Int   , "id"            , PropertyNames::NamedEntity::key                     },
         {ObjectStore::FieldType::Int   , "fermentable_id", PropertyNames::Inventory::ingredientId   , &PRIMARY_TABLE<Fermentable>},
         {ObjectStore::FieldType::Double, "quantity"      , PropertyNames::IngredientAmount::quantity},
         {ObjectStore::FieldType::Unit  , "unit"          , PropertyNames::IngredientAmount::unit    , &Measurement::Units::unitStringMapping},
      }
   };
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<InventoryFermentable> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Hop
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Hop> {
      "hop",
      {
         {ObjectStore::FieldType::Int   , "id"                   , PropertyNames::NamedEntity::key       },
         {ObjectStore::FieldType::String, "name"                 , PropertyNames::NamedEntity::name      },
         {ObjectStore::FieldType::Bool  , "display"              , PropertyNames::NamedEntity::display   },
         {ObjectStore::FieldType::Bool  , "deleted"              , PropertyNames::NamedEntity::deleted   },
         {ObjectStore::FieldType::String, "folder"               , PropertyNames::FolderBase::folder     },
         {ObjectStore::FieldType::Double, "alpha"                , PropertyNames::Hop::alpha_pct         },
         {ObjectStore::FieldType::Double, "beta"                 , PropertyNames::Hop::beta_pct          },
         {ObjectStore::FieldType::Double, "caryophyllene"        , PropertyNames::Hop::caryophyllene_pct },
         {ObjectStore::FieldType::Double, "cohumulone"           , PropertyNames::Hop::cohumulone_pct    },
         {ObjectStore::FieldType::Enum  , "form"                 , PropertyNames::Hop::form              , &Hop::formStringMapping},
         {ObjectStore::FieldType::Double, "hsi"                  , PropertyNames::Hop::hsi_pct           },
         {ObjectStore::FieldType::Double, "humulene"             , PropertyNames::Hop::humulene_pct      },
         {ObjectStore::FieldType::Double, "myrcene"              , PropertyNames::Hop::myrcene_pct       },
         {ObjectStore::FieldType::String, "notes"                , PropertyNames::Hop::notes             },
         {ObjectStore::FieldType::String, "origin"               , PropertyNames::Hop::origin            },
         {ObjectStore::FieldType::String, "substitutes"          , PropertyNames::Hop::substitutes       },
         {ObjectStore::FieldType::Enum  , "htype"                , PropertyNames::Hop::type              , &Hop::typeStringMapping},
         // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
         {ObjectStore::FieldType::String, "producer"             , PropertyNames::Hop::producer          },
         {ObjectStore::FieldType::String, "product_id"           , PropertyNames::Hop::productId         },
         {ObjectStore::FieldType::String, "year"                 , PropertyNames::Hop::year              },
         {ObjectStore::FieldType::Double, "total_oil_ml_per_100g", PropertyNames::Hop::totalOil_mlPer100g},
         {ObjectStore::FieldType::Double, "farnesene_pct"        , PropertyNames::Hop::farnesene_pct     },
         {ObjectStore::FieldType::Double, "geraniol_pct"         , PropertyNames::Hop::geraniol_pct      },
         {ObjectStore::FieldType::Double, "b_pinene_pct"         , PropertyNames::Hop::bPinene_pct       },
         {ObjectStore::FieldType::Double, "linalool_pct"         , PropertyNames::Hop::linalool_pct      },
         {ObjectStore::FieldType::Double, "limonene_pct"         , PropertyNames::Hop::limonene_pct      },
         {ObjectStore::FieldType::Double, "nerol_pct"            , PropertyNames::Hop::nerol_pct         },
         {ObjectStore::FieldType::Double, "pinene_pct"           , PropertyNames::Hop::pinene_pct        },
         {ObjectStore::FieldType::Double, "polyphenols_pct"      , PropertyNames::Hop::polyphenols_pct   },
         {ObjectStore::FieldType::Double, "xanthohumol_pct"      , PropertyNames::Hop::xanthohumol_pct   },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for InventoryHop
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<InventoryHop> {
      "hop_in_inventory",
      {
         {ObjectStore::FieldType::Int   , "id"      , PropertyNames::NamedEntity::key                     },
         {ObjectStore::FieldType::Int   , "hop_id"  , PropertyNames::Inventory::ingredientId   , &PRIMARY_TABLE<Hop>},
         {ObjectStore::FieldType::Double, "quantity", PropertyNames::IngredientAmount::quantity},
         {ObjectStore::FieldType::Unit  , "unit"    , PropertyNames::IngredientAmount::unit    , &Measurement::Units::unitStringMapping},
      }
   };
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<InventoryHop> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Instruction
   // NB: instructions aren't displayed in trees, and get no folder
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Instruction> {
      "instruction",
      {
         {ObjectStore::FieldType::Int   , "id"        , PropertyNames::NamedEntity::key       },
         {ObjectStore::FieldType::String, "name"      , PropertyNames::NamedEntity::name      },
         {ObjectStore::FieldType::Bool  , "display"   , PropertyNames::NamedEntity::display   },
         {ObjectStore::FieldType::Bool  , "deleted"   , PropertyNames::NamedEntity::deleted   },
         {ObjectStore::FieldType::String, "directions", PropertyNames::Instruction::directions},
         {ObjectStore::FieldType::Bool  , "hasTimer"  , PropertyNames::Instruction::hasTimer  },
         {ObjectStore::FieldType::String, "timervalue", PropertyNames::Instruction::timerValue},
         {ObjectStore::FieldType::Bool  , "completed" , PropertyNames::Instruction::completed },
         {ObjectStore::FieldType::Double, "interval"  , PropertyNames::Instruction::interval  },
      }
   };
   // Instructions don't have children
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<Instruction> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Mash
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Mash> {
      "mash",
      {
         {ObjectStore::FieldType::Int   , "id"               , PropertyNames::NamedEntity::key               },
         {ObjectStore::FieldType::String, "name"             , PropertyNames::NamedEntity::name              },
         {ObjectStore::FieldType::Bool  , "deleted"          , PropertyNames::NamedEntity::deleted           },
         {ObjectStore::FieldType::Bool  , "display"          , PropertyNames::NamedEntity::display           },
         {ObjectStore::FieldType::String, "folder"           , PropertyNames::FolderBase::folder            },
         {ObjectStore::FieldType::Bool  , "equip_adjust"     , PropertyNames::Mash::equipAdjust              },
         {ObjectStore::FieldType::Double, "grain_temp"       , PropertyNames::Mash::grainTemp_c              },
         {ObjectStore::FieldType::String, "notes"            , PropertyNames::Mash::notes                    },
         {ObjectStore::FieldType::Double, "ph"               , PropertyNames::Mash::ph                       },
         {ObjectStore::FieldType::Double, "sparge_temp"      , PropertyNames::Mash::spargeTemp_c             },
         {ObjectStore::FieldType::Double, "tun_specific_heat", PropertyNames::Mash::mashTunSpecificHeat_calGC},
         {ObjectStore::FieldType::Double, "tun_temp"         , PropertyNames::Mash::tunTemp_c                },
         {ObjectStore::FieldType::Double, "tun_weight"       , PropertyNames::Mash::mashTunWeight_kg         },
      }
   };
   // Mashes don't have children, and the link with their MashSteps is stored in the MashStep (as between Recipe and BrewNotes)
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<Mash> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for MashStep
   // NB: MashSteps don't get folders, because they don't separate from their Mash
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<MashStep> {
      "mash_step",
      {
         {ObjectStore::FieldType::Int   , "id"                       , PropertyNames::NamedEntity::key                },
         {ObjectStore::FieldType::String, "name"                     , PropertyNames::NamedEntity::name               },
         {ObjectStore::FieldType::Bool  , "deleted"                  , PropertyNames::NamedEntity::deleted            },
         {ObjectStore::FieldType::Bool  , "display"                  , PropertyNames::NamedEntity::display            },
         // NB: MashSteps don't have folders, as each one is owned by a Mash
         {ObjectStore::FieldType::Double, "end_temp_c"               , PropertyNames::    Step::endTemp_c             },
         {ObjectStore::FieldType::Double, "infuse_temp_c"            , PropertyNames::MashStep::infuseTemp_c          },
         {ObjectStore::FieldType::Int   , "mash_id"                  , PropertyNames::    Step::ownerId               , &PRIMARY_TABLE<Mash>},
         {ObjectStore::FieldType::Enum  , "mstype"                   , PropertyNames::MashStep::type                  , &MashStep::typeStringMapping},
         {ObjectStore::FieldType::Double, "ramp_time_mins"           , PropertyNames::    Step::rampTime_mins         },
         {ObjectStore::FieldType::Int   , "step_number"              , PropertyNames::    Step::stepNumber            },
         {ObjectStore::FieldType::Double, "step_temp_c"              , PropertyNames::    Step::startTemp_c           },
         {ObjectStore::FieldType::Double, "step_time_mins"           , PropertyNames::    Step::stepTime_mins         },
         // Now we support BeerJSON, amount_l unifies and replaces infuseAmount_l and decoctionAmount_l
         // See comment in model/MashStep.h for more info
         {ObjectStore::FieldType::Double, "amount_l"                 , PropertyNames::MashStep::amount_l              },
         // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
         {ObjectStore::FieldType::String, "description"              , PropertyNames::    Step::description           },
         {ObjectStore::FieldType::Double, "liquor_to_grist_ratio_lkg", PropertyNames::MashStep::liquorToGristRatio_lKg},
         {ObjectStore::FieldType::Double, "start_acidity_ph"         , PropertyNames::    Step::startAcidity_pH       },
         {ObjectStore::FieldType::Double, "end_acidity_ph"           , PropertyNames::    Step::endAcidity_pH         },
      }
   };
   // MashSteps don't have children
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<MashStep> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Boil
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Boil> {
      "boil",
      {
         {ObjectStore::FieldType::Int   , "id"             , PropertyNames::NamedEntity::key    },
         {ObjectStore::FieldType::String, "name"           , PropertyNames::NamedEntity::name   },
         {ObjectStore::FieldType::Bool  , "deleted"        , PropertyNames::NamedEntity::deleted},
         {ObjectStore::FieldType::Bool  , "display"        , PropertyNames::NamedEntity::display},
         {ObjectStore::FieldType::String, "folder"         , PropertyNames::FolderBase::folder },
         {ObjectStore::FieldType::String, "description"    , PropertyNames::Boil::description   },
         {ObjectStore::FieldType::String, "notes"          , PropertyNames::Boil::notes         },
         {ObjectStore::FieldType::Double, "pre_boil_size_l", PropertyNames::Boil::preBoilSize_l },
      }
   };
   // Boils don't have children, and the link with their BoilSteps is stored in the BoilStep (as between Recipe and BrewNotes)
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<Boil> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for BoilStep
   // NB: BoilSteps don't get folders, because they don't separate from their Boil
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<BoilStep> {
      "boil_step",
      {
         {ObjectStore::FieldType::Int   , "id"              , PropertyNames::NamedEntity::key             },
         {ObjectStore::FieldType::String, "name"            , PropertyNames::NamedEntity::name            },
         {ObjectStore::FieldType::Bool  , "deleted"         , PropertyNames::NamedEntity::deleted         },
         {ObjectStore::FieldType::Bool  , "display"         , PropertyNames::NamedEntity::display         },
         // NB: BoilSteps don't have folders, as each one is owned by a Boil
         {ObjectStore::FieldType::Double, "step_time_mins"  , PropertyNames::Step::stepTime_mins          },
         {ObjectStore::FieldType::Double, "start_temp_c"    , PropertyNames::Step::startTemp_c            },
         {ObjectStore::FieldType::Double, "end_temp_c"      , PropertyNames::Step::endTemp_c              },
         {ObjectStore::FieldType::Double, "ramp_time_mins"  , PropertyNames::Step::rampTime_mins          },
         {ObjectStore::FieldType::Int   , "step_number"     , PropertyNames::Step::stepNumber             },
         {ObjectStore::FieldType::Int   , "boil_id"         , PropertyNames::Step::ownerId                , &PRIMARY_TABLE<Boil>},
         {ObjectStore::FieldType::String, "description"     , PropertyNames::Step::description            },
         {ObjectStore::FieldType::Double, "start_acidity_ph", PropertyNames::Step::startAcidity_pH        },
         {ObjectStore::FieldType::Double, "end_acidity_ph"  , PropertyNames::Step::endAcidity_pH          },
         {ObjectStore::FieldType::Double, "start_gravity_sg", PropertyNames::StepExtended::startGravity_sg},
         {ObjectStore::FieldType::Double, "end_gravity_sg"  , PropertyNames::StepExtended::  endGravity_sg},
         {ObjectStore::FieldType::Enum  , "chilling_type"   , PropertyNames::BoilStep::chillingType       , &BoilStep::chillingTypeStringMapping},
      }
   };
   // BoilSteps don't have children
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<BoilStep> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Fermentation
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Fermentation> {
      "fermentation",
      {
         {ObjectStore::FieldType::Int   , "id"               , PropertyNames::NamedEntity::key               },
         {ObjectStore::FieldType::String, "name"             , PropertyNames::NamedEntity::name              },
         {ObjectStore::FieldType::Bool  , "deleted"          , PropertyNames::NamedEntity::deleted           },
         {ObjectStore::FieldType::Bool  , "display"          , PropertyNames::NamedEntity::display           },
         {ObjectStore::FieldType::String, "folder"           , PropertyNames::FolderBase::folder            },
         {ObjectStore::FieldType::String, "description"      , PropertyNames::Fermentation::description      },
         {ObjectStore::FieldType::String, "notes"            , PropertyNames::Fermentation::notes            },
      }
   };
   // Fermentations don't have children, and the link with their FermentationSteps is stored in the FermentationStep (as between Recipe and BrewNotes)
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<Fermentation> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for FermentationStep
   //
   // NB: FermentationSteps don't get folders, because they don't separate from their Fermentation
   //
   // NB: Although FermentationStep inherits (via StepExtended) from Step, the rampTime_mins field is not used and
   //     should not be stored in the DB or serialised.  See comment in model/Step.h.
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<FermentationStep> {
      "fermentation_step",
      {
         {ObjectStore::FieldType::Int   , "id"              , PropertyNames::NamedEntity::key             },
         {ObjectStore::FieldType::String, "name"            , PropertyNames::NamedEntity::name            },
         {ObjectStore::FieldType::Bool  , "deleted"         , PropertyNames::NamedEntity::deleted         },
         {ObjectStore::FieldType::Bool  , "display"         , PropertyNames::NamedEntity::display         },
         // NB: FermentationSteps don't have folders, as each one is owned by a Fermentation
         {ObjectStore::FieldType::Double, "step_time_mins"  , PropertyNames::Step::stepTime_mins          },
         {ObjectStore::FieldType::Double, "start_temp_c"    , PropertyNames::Step::startTemp_c            },
         {ObjectStore::FieldType::Double, "end_temp_c"      , PropertyNames::Step::endTemp_c              },
         {ObjectStore::FieldType::Int   , "step_number"     , PropertyNames::Step::stepNumber             },
         {ObjectStore::FieldType::Int   , "fermentation_id" , PropertyNames::Step::ownerId                , &PRIMARY_TABLE<Fermentation>},
         {ObjectStore::FieldType::String, "description"     , PropertyNames::Step::description            },
         {ObjectStore::FieldType::Double, "start_acidity_ph", PropertyNames::Step::startAcidity_pH        },
         {ObjectStore::FieldType::Double, "end_acidity_ph"  , PropertyNames::Step::  endAcidity_pH        },
         {ObjectStore::FieldType::Double, "start_gravity_sg", PropertyNames::StepExtended::startGravity_sg},
         {ObjectStore::FieldType::Double, "end_gravity_sg"  , PropertyNames::StepExtended::  endGravity_sg},
         {ObjectStore::FieldType::Bool  , "free_rise"       , PropertyNames::FermentationStep::freeRise   },
         {ObjectStore::FieldType::String, "vessel"          , PropertyNames::FermentationStep::vessel     },
      }
   };
   // FermentationSteps don't have children
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<FermentationStep> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Misc
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Misc> {
      "misc",
      {
         {ObjectStore::FieldType::Int   , "id"              , PropertyNames::NamedEntity::key                     },
         {ObjectStore::FieldType::String, "name"            , PropertyNames::NamedEntity::name                    },
         {ObjectStore::FieldType::Bool  , "deleted"         , PropertyNames::NamedEntity::deleted                 },
         {ObjectStore::FieldType::Bool  , "display"         , PropertyNames::NamedEntity::display                 },
         {ObjectStore::FieldType::String, "folder"          , PropertyNames::FolderBase::folder                   },
         {ObjectStore::FieldType::Enum  , "mtype"           , PropertyNames::Misc::type                           , &Misc::typeStringMapping},
         {ObjectStore::FieldType::String, "use_for"         , PropertyNames::Misc::useFor                         },
         {ObjectStore::FieldType::String, "notes"           , PropertyNames::Misc::notes                          },
         // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
         {ObjectStore::FieldType::String, "producer"        , PropertyNames::Misc::producer                       },
         {ObjectStore::FieldType::String, "product_id"      , PropertyNames::Misc::productId                      },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for InventoryMisc
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<InventoryMisc> {
      "misc_in_inventory",
      {
         {ObjectStore::FieldType::Int   , "id"      , PropertyNames::NamedEntity::key                     },
         {ObjectStore::FieldType::Int   , "misc_id" , PropertyNames::Inventory::ingredientId   , &PRIMARY_TABLE<Misc>},
         {ObjectStore::FieldType::Double, "quantity", PropertyNames::IngredientAmount::quantity},
         {ObjectStore::FieldType::Unit  , "unit"    , PropertyNames::IngredientAmount::unit    , &Measurement::Units::unitStringMapping},
      }
   };
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<InventoryMisc> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Salt
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Salt> {
      "salt",
      {
         {ObjectStore::FieldType::Int   , "id"              , PropertyNames::NamedEntity::key     },
         {ObjectStore::FieldType::String, "name"            , PropertyNames::NamedEntity::name    },
         {ObjectStore::FieldType::Bool  , "deleted"         , PropertyNames::NamedEntity::deleted },
         {ObjectStore::FieldType::Bool  , "display"         , PropertyNames::NamedEntity::display },
         {ObjectStore::FieldType::String, "folder"          , PropertyNames::FolderBase::folder  },
///         {ObjectStore::FieldType::Bool  , "is_acid"         , PropertyNames::Salt::isAcid         },
         {ObjectStore::FieldType::Double, "percent_acid"    , PropertyNames::Salt::percentAcid    },
         {ObjectStore::FieldType::Enum  , "stype"           , PropertyNames::Salt::type           , &Salt::typeStringMapping},
      }
   };
   // Salts don't have children
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<Salt> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for InventorySalt
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<InventorySalt> {
      "salt_in_inventory",
      {
         {ObjectStore::FieldType::Int   , "id"      , PropertyNames::NamedEntity::key                     },
         {ObjectStore::FieldType::Int   , "salt_id" , PropertyNames::Inventory::ingredientId   , &PRIMARY_TABLE<Salt>},
         {ObjectStore::FieldType::Double, "quantity", PropertyNames::IngredientAmount::quantity},
         {ObjectStore::FieldType::Unit  , "unit"    , PropertyNames::IngredientAmount::unit    , &Measurement::Units::unitStringMapping},
      }
   };
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<InventorySalt> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Style
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Style> {
      "style",
      {
         {ObjectStore::FieldType::Int   , "id"                , PropertyNames::NamedEntity::key        },
         {ObjectStore::FieldType::String, "name"              , PropertyNames::NamedEntity::name       },
         {ObjectStore::FieldType::Bool  , "display"           , PropertyNames::NamedEntity::display    },
         {ObjectStore::FieldType::Bool  , "deleted"           , PropertyNames::NamedEntity::deleted    },
         {ObjectStore::FieldType::String, "folder"            , PropertyNames::FolderBase::folder     },
         {ObjectStore::FieldType::Double, "abv_max"           , PropertyNames::Style::abvMax_pct       },
         {ObjectStore::FieldType::Double, "abv_min"           , PropertyNames::Style::abvMin_pct       },
         {ObjectStore::FieldType::Double, "carb_max"          , PropertyNames::Style::carbMax_vol      },
         {ObjectStore::FieldType::Double, "carb_min"          , PropertyNames::Style::carbMin_vol      },
         {ObjectStore::FieldType::String, "category"          , PropertyNames::Style::category         },
         {ObjectStore::FieldType::String, "category_number"   , PropertyNames::Style::categoryNumber   },
         {ObjectStore::FieldType::Double, "color_max"         , PropertyNames::Style::colorMax_srm     },
         {ObjectStore::FieldType::Double, "color_min"         , PropertyNames::Style::colorMin_srm     },
         {ObjectStore::FieldType::String, "examples"          , PropertyNames::Style::examples         },
         {ObjectStore::FieldType::Double, "fg_max"            , PropertyNames::Style::fgMax            },
         {ObjectStore::FieldType::Double, "fg_min"            , PropertyNames::Style::fgMin            },
         {ObjectStore::FieldType::Double, "ibu_max"           , PropertyNames::Style::ibuMax           },
         {ObjectStore::FieldType::Double, "ibu_min"           , PropertyNames::Style::ibuMin           },
         {ObjectStore::FieldType::String, "ingredients"       , PropertyNames::Style::ingredients      },
         {ObjectStore::FieldType::String, "notes"             , PropertyNames::Style::notes            },
         {ObjectStore::FieldType::Double, "og_max"            , PropertyNames::Style::ogMax            },
         {ObjectStore::FieldType::Double, "og_min"            , PropertyNames::Style::ogMin            },
         {ObjectStore::FieldType::String, "style_guide"       , PropertyNames::Style::styleGuide       },
         {ObjectStore::FieldType::String, "style_letter"      , PropertyNames::Style::styleLetter      },
         {ObjectStore::FieldType::Enum  , "stype"             , PropertyNames::Style::type             , &Style::typeStringMapping},
         // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
         {ObjectStore::FieldType::String, "aroma"             , PropertyNames::Style::aroma            },
         {ObjectStore::FieldType::String, "appearance"        , PropertyNames::Style::appearance       },
         {ObjectStore::FieldType::String, "flavor"            , PropertyNames::Style::flavor           },
         {ObjectStore::FieldType::String, "mouthfeel"         , PropertyNames::Style::mouthfeel        },
         {ObjectStore::FieldType::String, "overall_impression", PropertyNames::Style::overallImpression},
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Water
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Water> {
      "water",
      {
         {ObjectStore::FieldType::Int   , "id"           , PropertyNames::NamedEntity::key       },
         {ObjectStore::FieldType::String, "name"         , PropertyNames::NamedEntity::name      },
         {ObjectStore::FieldType::Bool  , "display"      , PropertyNames::NamedEntity::display   },
         {ObjectStore::FieldType::Bool  , "deleted"      , PropertyNames::NamedEntity::deleted   },
         {ObjectStore::FieldType::String, "folder"       , PropertyNames::FolderBase::folder    },
         {ObjectStore::FieldType::String, "notes"        , PropertyNames::Water::notes           },
         {ObjectStore::FieldType::Double, "calcium"      , PropertyNames::Water::calcium_ppm     },
         {ObjectStore::FieldType::Double, "bicarbonate"  , PropertyNames::Water::bicarbonate_ppm },
         {ObjectStore::FieldType::Double, "sulfate"      , PropertyNames::Water::sulfate_ppm     },
         {ObjectStore::FieldType::Double, "sodium"       , PropertyNames::Water::sodium_ppm      },
         {ObjectStore::FieldType::Double, "chloride"     , PropertyNames::Water::chloride_ppm    },
         {ObjectStore::FieldType::Double, "magnesium"    , PropertyNames::Water::magnesium_ppm   },
         {ObjectStore::FieldType::Double, "ph"           , PropertyNames::Water::ph              },
         {ObjectStore::FieldType::Double, "alkalinity"   , PropertyNames::Water::alkalinity_ppm  },
         {ObjectStore::FieldType::Int   , "wtype"        , PropertyNames::Water::type            }, // TODO: Would be less fragile to store this as text than a number
         {ObjectStore::FieldType::Double, "mash_ro"      , PropertyNames::Water::mashRo_pct      },
         {ObjectStore::FieldType::Double, "sparge_ro"    , PropertyNames::Water::spargeRo_pct    },
         {ObjectStore::FieldType::Bool  , "as_hco3"      , PropertyNames::Water::alkalinityAsHCO3},
         // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
         {ObjectStore::FieldType::Double, "carbonate_ppm", PropertyNames::Water::carbonate_ppm   },
         {ObjectStore::FieldType::Double, "potassium_ppm", PropertyNames::Water::potassium_ppm   },
         {ObjectStore::FieldType::Double, "iron_ppm"     , PropertyNames::Water::iron_ppm        },
         {ObjectStore::FieldType::Double, "nitrate_ppm"  , PropertyNames::Water::nitrate_ppm     },
         {ObjectStore::FieldType::Double, "nitrite_ppm"  , PropertyNames::Water::nitrite_ppm     },
         // .:TODO:. We should correct the typo in this column name (copy-and-paste from BeerJSON
         {ObjectStore::FieldType::Double, "flouride_ppm" , PropertyNames::Water::fluoride_ppm    },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Yeast
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Yeast> {
      "yeast",
      {
         {ObjectStore::FieldType::Int   , "id"                          , PropertyNames::NamedEntity::key                     },
         {ObjectStore::FieldType::String, "name"                        , PropertyNames::NamedEntity::name                    },
         {ObjectStore::FieldType::Bool  , "display"                     , PropertyNames::NamedEntity::display                 },
         {ObjectStore::FieldType::Bool  , "deleted"                     , PropertyNames::NamedEntity::deleted                 },
         {ObjectStore::FieldType::String, "folder"                      , PropertyNames::FolderBase::folder                  },
         {ObjectStore::FieldType::Double, "max_temperature"             , PropertyNames::Yeast::maxTemperature_c              },
         {ObjectStore::FieldType::Double, "min_temperature"             , PropertyNames::Yeast::minTemperature_c              },
         {ObjectStore::FieldType::Enum  , "flocculation"                , PropertyNames::Yeast::flocculation                  , &Yeast::flocculationStringMapping},
         {ObjectStore::FieldType::Enum  , "form"                        , PropertyNames::Yeast::form                          , &Yeast::formStringMapping        },
         {ObjectStore::FieldType::Enum  , "ytype"                       , PropertyNames::Yeast::type                          , &Yeast::typeStringMapping        },
         {ObjectStore::FieldType::Int   , "max_reuse"                   , PropertyNames::Yeast::maxReuse                      },
         {ObjectStore::FieldType::String, "best_for"                    , PropertyNames::Yeast::bestFor                       },
         {ObjectStore::FieldType::String, "laboratory"                  , PropertyNames::Yeast::laboratory                    },
         {ObjectStore::FieldType::String, "notes"                       , PropertyNames::Yeast::notes                         },
         {ObjectStore::FieldType::String, "product_id"                  , PropertyNames::Yeast::productId                     }, // Manufacturer's product ID, so, unlike other blah_id fields, not a foreign key!
         // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
         {ObjectStore::FieldType::Double, "alcohol_tolerance_pct"       , PropertyNames::Yeast::alcoholTolerance_pct          },
         {ObjectStore::FieldType::Double, "attenuation_min_pct"         , PropertyNames::Yeast::attenuationMin_pct            },
         {ObjectStore::FieldType::Double, "attenuation_max_pct"         , PropertyNames::Yeast::attenuationMax_pct            },
         {ObjectStore::FieldType::Bool  , "phenolic_off_flavor_positive", PropertyNames::Yeast::phenolicOffFlavorPositive     },
         {ObjectStore::FieldType::Bool  , "glucoamylase_positive"       , PropertyNames::Yeast::glucoamylasePositive          },
         {ObjectStore::FieldType::Bool  , "killer_producing_k1_toxin"   , PropertyNames::Yeast::killerProducingK1Toxin        },
         {ObjectStore::FieldType::Bool  , "killer_producing_k2_toxin"   , PropertyNames::Yeast::killerProducingK2Toxin        },
         {ObjectStore::FieldType::Bool  , "killer_producing_k28_toxin"  , PropertyNames::Yeast::killerProducingK28Toxin       },
         {ObjectStore::FieldType::Bool  , "killer_producing_klus_toxin" , PropertyNames::Yeast::killerProducingKlusToxin      },
         {ObjectStore::FieldType::Bool  , "killer_neutral"              , PropertyNames::Yeast::killerNeutral                 },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for InventoryYeast
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<InventoryYeast> {
      "yeast_in_inventory",
      {
         {ObjectStore::FieldType::Int   , "id"            , PropertyNames::NamedEntity::key                     },
         {ObjectStore::FieldType::Int   , "yeast_id"      , PropertyNames::Inventory::ingredientId   , &PRIMARY_TABLE<Yeast>},
         {ObjectStore::FieldType::Double, "quantity"      , PropertyNames::IngredientAmount::quantity},
         {ObjectStore::FieldType::Unit  , "unit"          , PropertyNames::IngredientAmount::unit    , &Measurement::Units::unitStringMapping},
      }
   };
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<InventoryYeast> {};

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for Recipe
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<Recipe> {
      "recipe",
      {
         {ObjectStore::FieldType::Int   , "id"                 , PropertyNames::NamedEntity::key          },
         {ObjectStore::FieldType::String, "name"               , PropertyNames::NamedEntity::name         },
         {ObjectStore::FieldType::Bool  , "deleted"            , PropertyNames::NamedEntity::deleted      },
         {ObjectStore::FieldType::Bool  , "display"            , PropertyNames::NamedEntity::display      },
         {ObjectStore::FieldType::String, "folder"             , PropertyNames::FolderBase::folder        },
         {ObjectStore::FieldType::Double, "age"                , PropertyNames::Recipe::age_days          },
         {ObjectStore::FieldType::Double, "age_temp"           , PropertyNames::Recipe::ageTemp_c         },
         {ObjectStore::FieldType::String, "assistant_brewer"   , PropertyNames::Recipe::asstBrewer        },
         {ObjectStore::FieldType::Double, "batch_size"         , PropertyNames::Recipe::batchSize_l       },
         {ObjectStore::FieldType::String, "brewer"             , PropertyNames::Recipe::brewer            },
         {ObjectStore::FieldType::Double, "carb_volume"        , PropertyNames::Recipe::carbonation_vols  },
         {ObjectStore::FieldType::Double, "carbonationtemp_c"  , PropertyNames::Recipe::carbonationTemp_c },
         {ObjectStore::FieldType::Date  , "date"               , PropertyNames::Recipe::date              },
         {ObjectStore::FieldType::Double, "efficiency"         , PropertyNames::Recipe::efficiency_pct    },
         {ObjectStore::FieldType::Int   , "equipment_id"       , PropertyNames::Recipe::equipmentId       , &PRIMARY_TABLE<Equipment>},
         {ObjectStore::FieldType::Double, "fg"                 , PropertyNames::Recipe::fg                },
         {ObjectStore::FieldType::Bool  , "forced_carb"        , PropertyNames::Recipe::forcedCarbonation },
         {ObjectStore::FieldType::Double, "keg_priming_factor" , PropertyNames::Recipe::kegPrimingFactor  },
         {ObjectStore::FieldType::Int   , "mash_id"            , PropertyNames::Recipe::mashId            , &PRIMARY_TABLE<Mash>},
         {ObjectStore::FieldType::String, "notes"              , PropertyNames::Recipe::notes             },
         {ObjectStore::FieldType::Double, "og"                 , PropertyNames::Recipe::og                },
         {ObjectStore::FieldType::Double, "priming_sugar_equiv", PropertyNames::Recipe::primingSugarEquiv },
         {ObjectStore::FieldType::String, "priming_sugar_name" , PropertyNames::Recipe::primingSugarName  },
         {ObjectStore::FieldType::Int   , "style_id"           , PropertyNames::Recipe::styleId           , &PRIMARY_TABLE<Style>},
         {ObjectStore::FieldType::String, "taste_notes"        , PropertyNames::Recipe::tasteNotes        },
         {ObjectStore::FieldType::Double, "taste_rating"       , PropertyNames::Recipe::tasteRating       },
         {ObjectStore::FieldType::Enum  , "type"               , PropertyNames::Recipe::type              , &Recipe::typeStringMapping},
         {ObjectStore::FieldType::Int   , "ancestor_id"        , PropertyNames::Recipe::ancestorId        , &PRIMARY_TABLE<Recipe>},
         {ObjectStore::FieldType::Bool  , "locked"             , PropertyNames::Recipe::locked            },
         {ObjectStore::FieldType::Int   , "boil_id"            , PropertyNames::Recipe::boilId            , &PRIMARY_TABLE<Boil>},
         {ObjectStore::FieldType::Int   , "fermentation_id"    , PropertyNames::Recipe::fermentationId    , &PRIMARY_TABLE<Fermentation>},
         // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
         {ObjectStore::FieldType::Double, "beer_acidity_ph"         , PropertyNames::Recipe::beerAcidity_pH         },
         {ObjectStore::FieldType::Double, "apparent_attenuation_pct", PropertyNames::Recipe::apparentAttenuation_pct},

      }
   };
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<Recipe> {
      //
      // .:TODO:. This is the wrong way to model Instructions.  We should treat them more like BrewNotes.  In both case
      //          the objects cannot meaningfully exist without a corresponding Recipe.
      //
      //          Having the "instruction" table (see PRIMARY_TABLE<Instruction>) and this instruction_in_recipe
      //          junction table implies the possibility for an existence of Instruction objects independently of Recipe
      //          ones, which is incorrect.
      //
      // There is a lot of boiler-plate here, and we could have gone for a much more compact representation of junction
      // tables, but this keeps the definition format relatively closely aligned with that of primary tables.
      //
      // NOTE that the tables for ingredient additions (such as fermentable_in_recipe for RecipeAdditionFermentable)
      //      could, in principle, be treated as a junction tables, because they really are modelling a many-to-many
      //      relationship between, eg, Recipe and Fermentable.  However, because they are also storing other properties
      //      (to do with the timing and amount of the addition), and because the Recipe class is already quite large,
      //      we choose to model the ingredient addition tables as freestanding entities.
      //
      {
         "instruction_in_recipe",
         {
            {ObjectStore::FieldType::Int, "id"                                                                                   },
            {ObjectStore::FieldType::Int, "recipe_id",         PropertyNames::NamedEntity::key,       &PRIMARY_TABLE<Recipe>     },
            {ObjectStore::FieldType::Int, "instruction_id",    PropertyNames::Recipe::instructionIds, &PRIMARY_TABLE<Instruction>},
            {ObjectStore::FieldType::Int, "instruction_number"                                                                   },
         }
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for RecipeAdditionFermentable
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<RecipeAdditionFermentable> {
      "fermentable_in_recipe",
      {
         {ObjectStore::FieldType::Int   , "id"               , PropertyNames::NamedEntity::key                },
         {ObjectStore::FieldType::String, "name"             , PropertyNames::NamedEntity::name               },
         {ObjectStore::FieldType::Bool  , "display"          , PropertyNames::NamedEntity::display            },
         {ObjectStore::FieldType::Bool  , "deleted"          , PropertyNames::NamedEntity::deleted            },
         {ObjectStore::FieldType::Int   , "recipe_id"        , PropertyNames::OwnedByRecipe::recipeId         , &PRIMARY_TABLE<Recipe>     },
         {ObjectStore::FieldType::Int   , "fermentable_id"   , PropertyNames::IngredientInRecipe::ingredientId, &PRIMARY_TABLE<Fermentable>},
         {ObjectStore::FieldType::Enum  , "stage"            , PropertyNames::RecipeAddition::stage           , &RecipeAddition::stageStringMapping},
         {ObjectStore::FieldType::Double, "quantity"         , PropertyNames::IngredientAmount::quantity      },
         {ObjectStore::FieldType::Unit  , "unit"             , PropertyNames::IngredientAmount::unit          , &Measurement::Units::unitStringMapping},
         {ObjectStore::FieldType::Int   , "step"             , PropertyNames::RecipeAddition::step            },
         {ObjectStore::FieldType::Double, "add_at_time_mins" , PropertyNames::RecipeAddition::addAtTime_mins  },
         {ObjectStore::FieldType::Double, "add_at_gravity_sg", PropertyNames::RecipeAddition::addAtGravity_sg },
         {ObjectStore::FieldType::Double, "add_at_acidity_ph", PropertyNames::RecipeAddition::addAtAcidity_pH },
         {ObjectStore::FieldType::Double, "duration_mins"    , PropertyNames::RecipeAddition::duration_mins   },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for RecipeAdditionHop
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<RecipeAdditionHop> {
      "hop_in_recipe",
      {
         {ObjectStore::FieldType::Int   , "id"               , PropertyNames::NamedEntity::key                },
         {ObjectStore::FieldType::String, "name"             , PropertyNames::NamedEntity::name               },
         {ObjectStore::FieldType::Bool  , "display"          , PropertyNames::NamedEntity::display            },
         {ObjectStore::FieldType::Bool  , "deleted"          , PropertyNames::NamedEntity::deleted            },
         {ObjectStore::FieldType::Int   , "recipe_id"        , PropertyNames::OwnedByRecipe::recipeId         , &PRIMARY_TABLE<Recipe>},
         {ObjectStore::FieldType::Int   , "hop_id"           , PropertyNames::IngredientInRecipe::ingredientId, &PRIMARY_TABLE<Hop>   },
         {ObjectStore::FieldType::Enum  , "stage"            , PropertyNames::RecipeAddition::stage           , &RecipeAddition::stageStringMapping},
         {ObjectStore::FieldType::Double, "quantity"         , PropertyNames::IngredientAmount::quantity      },
         {ObjectStore::FieldType::Unit  , "unit"             , PropertyNames::IngredientAmount::unit          , &Measurement::Units::unitStringMapping},
         {ObjectStore::FieldType::Int   , "step"             , PropertyNames::RecipeAddition::step            },
         {ObjectStore::FieldType::Double, "add_at_time_mins" , PropertyNames::RecipeAddition::addAtTime_mins  },
         {ObjectStore::FieldType::Double, "add_at_gravity_sg", PropertyNames::RecipeAddition::addAtGravity_sg },
         {ObjectStore::FieldType::Double, "add_at_acidity_ph", PropertyNames::RecipeAddition::addAtAcidity_pH },
         {ObjectStore::FieldType::Double, "duration_mins"    , PropertyNames::RecipeAddition::duration_mins   },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for RecipeAdditionMisc
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<RecipeAdditionMisc> {
      "misc_in_recipe",
      {
         {ObjectStore::FieldType::Int   , "id"               , PropertyNames::NamedEntity::key                },
         {ObjectStore::FieldType::String, "name"             , PropertyNames::NamedEntity::name               },
         {ObjectStore::FieldType::Bool  , "display"          , PropertyNames::NamedEntity::display            },
         {ObjectStore::FieldType::Bool  , "deleted"          , PropertyNames::NamedEntity::deleted            },
         {ObjectStore::FieldType::Int   , "recipe_id"        , PropertyNames::OwnedByRecipe::recipeId         , &PRIMARY_TABLE<Recipe>},
         {ObjectStore::FieldType::Int   , "misc_id"          , PropertyNames::IngredientInRecipe::ingredientId, &PRIMARY_TABLE<Misc>   },
         {ObjectStore::FieldType::Enum  , "stage"            , PropertyNames::RecipeAddition::stage           , &RecipeAddition::stageStringMapping},
         {ObjectStore::FieldType::Double, "quantity"         , PropertyNames::IngredientAmount::quantity      },
         {ObjectStore::FieldType::Unit  , "unit"             , PropertyNames::IngredientAmount::unit          , &Measurement::Units::unitStringMapping},
         {ObjectStore::FieldType::Int   , "step"             , PropertyNames::RecipeAddition::step            },
         {ObjectStore::FieldType::Double, "add_at_time_mins" , PropertyNames::RecipeAddition::addAtTime_mins  },
         {ObjectStore::FieldType::Double, "add_at_gravity_sg", PropertyNames::RecipeAddition::addAtGravity_sg },
         {ObjectStore::FieldType::Double, "add_at_acidity_ph", PropertyNames::RecipeAddition::addAtAcidity_pH },
         {ObjectStore::FieldType::Double, "duration_mins"    , PropertyNames::RecipeAddition::duration_mins   },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for RecipeAdditionYeast
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<RecipeAdditionYeast> {
      "yeast_in_recipe",
      {
         {ObjectStore::FieldType::Int   , "id"                 , PropertyNames::NamedEntity::key                      },
         {ObjectStore::FieldType::String, "name"               , PropertyNames::NamedEntity::name                     },
         {ObjectStore::FieldType::Bool  , "display"            , PropertyNames::NamedEntity::display                  },
         {ObjectStore::FieldType::Bool  , "deleted"            , PropertyNames::NamedEntity::deleted                  },
         {ObjectStore::FieldType::Int   , "recipe_id"          , PropertyNames::OwnedByRecipe::recipeId               , &PRIMARY_TABLE<Recipe>},
         {ObjectStore::FieldType::Int   , "yeast_id"           , PropertyNames::IngredientInRecipe::ingredientId      , &PRIMARY_TABLE<Yeast>   },
         {ObjectStore::FieldType::Enum  , "stage"              , PropertyNames::RecipeAddition::stage                 , &RecipeAddition::stageStringMapping},
         {ObjectStore::FieldType::Double, "quantity"           , PropertyNames::IngredientAmount::quantity            },
         {ObjectStore::FieldType::Unit  , "unit"               , PropertyNames::IngredientAmount::unit                , &Measurement::Units::unitStringMapping},
         {ObjectStore::FieldType::Int   , "step"               , PropertyNames::RecipeAddition::step                  },
         {ObjectStore::FieldType::Double, "add_at_time_mins"   , PropertyNames::RecipeAddition::addAtTime_mins        },
         {ObjectStore::FieldType::Double, "add_at_gravity_sg"  , PropertyNames::RecipeAddition::addAtGravity_sg       },
         {ObjectStore::FieldType::Double, "add_at_acidity_ph"  , PropertyNames::RecipeAddition::addAtAcidity_pH       },
         {ObjectStore::FieldType::Double, "duration_mins"      , PropertyNames::RecipeAddition::duration_mins         },
         {ObjectStore::FieldType::Double, "attenuation_pct"    , PropertyNames::RecipeAdditionYeast::attenuation_pct  },
         {ObjectStore::FieldType::Int   , "times_cultured"     , PropertyNames::RecipeAdditionYeast::timesCultured    },
         {ObjectStore::FieldType::Int   , "cell_count_billions", PropertyNames::RecipeAdditionYeast::cellCountBillions},
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for RecipeAdjustmentSalt
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<RecipeAdjustmentSalt> {
      "salt_in_recipe",
      {
         {ObjectStore::FieldType::Int   , "id"         , PropertyNames::NamedEntity::key                },
         {ObjectStore::FieldType::String, "name"       , PropertyNames::NamedEntity::name               },
         {ObjectStore::FieldType::Bool  , "display"    , PropertyNames::NamedEntity::display            },
         {ObjectStore::FieldType::Bool  , "deleted"    , PropertyNames::NamedEntity::deleted            },
         {ObjectStore::FieldType::Int   , "recipe_id"  , PropertyNames::OwnedByRecipe::recipeId         , &PRIMARY_TABLE<Recipe>},
         {ObjectStore::FieldType::Int   , "salt_id"    , PropertyNames::IngredientInRecipe::ingredientId, &PRIMARY_TABLE<Salt>   },
         {ObjectStore::FieldType::Double, "quantity"   , PropertyNames::IngredientAmount::quantity      },
         {ObjectStore::FieldType::Unit  , "unit"       , PropertyNames::IngredientAmount::unit          , &Measurement::Units::unitStringMapping},
         {ObjectStore::FieldType::Enum  , "when_to_add", PropertyNames::RecipeAdjustmentSalt::whenToAdd , &RecipeAdjustmentSalt::whenToAddStringMapping},
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for RecipeUseOfWater
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<RecipeUseOfWater> {
      "water_in_recipe",
      {
         {ObjectStore::FieldType::Int   , "id"       , PropertyNames::NamedEntity::key                },
         {ObjectStore::FieldType::String, "name"     , PropertyNames::NamedEntity::name               },
         {ObjectStore::FieldType::Bool  , "display"  , PropertyNames::NamedEntity::display            },
         {ObjectStore::FieldType::Bool  , "deleted"  , PropertyNames::NamedEntity::deleted            },
         {ObjectStore::FieldType::Int   , "recipe_id", PropertyNames::OwnedByRecipe::recipeId         , &PRIMARY_TABLE<Recipe>},
         {ObjectStore::FieldType::Int   , "water_id" , PropertyNames::IngredientInRecipe::ingredientId, &PRIMARY_TABLE<Water>   },
         {ObjectStore::FieldType::Double, "volume_l" , PropertyNames::RecipeUseOfWater::volume_l      },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Database field mappings for BrewNote
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> ObjectStore::TableDefinition const PRIMARY_TABLE<BrewNote> {
      "brewnote",
      {
         {ObjectStore::FieldType::Int   , "id"                     , PropertyNames::NamedEntity::key           },
         // NB: BrewNotes don't have names in DB
         {ObjectStore::FieldType::Bool  , "display"                , PropertyNames::NamedEntity::display       },
         {ObjectStore::FieldType::Bool  , "deleted"                , PropertyNames::NamedEntity::deleted       },
         // NB: BrewNotes don't have folders, as each one is owned by a Recipe
         {ObjectStore::FieldType::Double, "abv"                    , PropertyNames::BrewNote::abv              },
         {ObjectStore::FieldType::Double, "attenuation"            , PropertyNames::BrewNote::attenuation      },
         {ObjectStore::FieldType::Double, "boil_off"               , PropertyNames::BrewNote::boilOff_l        },
         {ObjectStore::FieldType::Date  , "brewdate"               , PropertyNames::BrewNote::brewDate         },
         {ObjectStore::FieldType::Double, "brewhouse_eff"          , PropertyNames::BrewNote::brewhouseEff_pct },
         {ObjectStore::FieldType::Double, "eff_into_bk"            , PropertyNames::BrewNote::effIntoBK_pct    },
         {ObjectStore::FieldType::Date  , "fermentdate"            , PropertyNames::BrewNote::fermentDate      },
         {ObjectStore::FieldType::Double, "fg"                     , PropertyNames::BrewNote::fg               },
         {ObjectStore::FieldType::Double, "final_volume"           , PropertyNames::BrewNote::finalVolume_l    },
         {ObjectStore::FieldType::Double, "mash_final_temp"        , PropertyNames::BrewNote::mashFinTemp_c    },
         {ObjectStore::FieldType::String, "notes"                  , PropertyNames::BrewNote::notes            },
         {ObjectStore::FieldType::Double, "og"                     , PropertyNames::BrewNote::og               },
         {ObjectStore::FieldType::Double, "pitch_temp"             , PropertyNames::BrewNote::pitchTemp_c      },
         {ObjectStore::FieldType::Double, "post_boil_volume"       , PropertyNames::BrewNote::postBoilVolume_l },
         {ObjectStore::FieldType::Double, "projected_abv"          , PropertyNames::BrewNote::projABV_pct      },
         {ObjectStore::FieldType::Double, "projected_atten"        , PropertyNames::BrewNote::projAtten        },
         {ObjectStore::FieldType::Double, "projected_boil_grav"    , PropertyNames::BrewNote::projBoilGrav     },
         {ObjectStore::FieldType::Double, "projected_eff"          , PropertyNames::BrewNote::projEff_pct      },
         {ObjectStore::FieldType::Double, "projected_ferm_points"  , PropertyNames::BrewNote::projFermPoints   },
         {ObjectStore::FieldType::Double, "projected_fg"           , PropertyNames::BrewNote::projFg           },
         {ObjectStore::FieldType::Double, "projected_mash_fin_temp", PropertyNames::BrewNote::projMashFinTemp_c},
         {ObjectStore::FieldType::Double, "projected_og"           , PropertyNames::BrewNote::projOg           },
         {ObjectStore::FieldType::Double, "projected_points"       , PropertyNames::BrewNote::projPoints       },
         {ObjectStore::FieldType::Double, "projected_strike_temp"  , PropertyNames::BrewNote::projStrikeTemp_c },
         {ObjectStore::FieldType::Double, "projected_vol_into_bk"  , PropertyNames::BrewNote::projVolIntoBK_l  },
         {ObjectStore::FieldType::Double, "projected_vol_into_ferm", PropertyNames::BrewNote::projVolIntoFerm_l},
         {ObjectStore::FieldType::Double, "sg"                     , PropertyNames::BrewNote::sg               },
         {ObjectStore::FieldType::Double, "strike_temp"            , PropertyNames::BrewNote::strikeTemp_c     },
         {ObjectStore::FieldType::Double, "volume_into_bk"         , PropertyNames::BrewNote::volumeIntoBK_l   },
         {ObjectStore::FieldType::Double, "volume_into_fermenter"  , PropertyNames::BrewNote::volumeIntoFerm_l },
         {ObjectStore::FieldType::Int   , "recipe_id"              , PropertyNames::OwnedByRecipe::recipeId    , &PRIMARY_TABLE<Recipe>},
      }
   };
   // BrewNotes don't have children
   template<> ObjectStore::JunctionTableDefinitions const JUNCTION_TABLES<BrewNote> {};

}


template<class NE>
ObjectStoreTyped<NE> & ObjectStoreTyped<NE>::getInstance() {
   //
   // As of C++11, simple "Meyers singleton" is now thread-safe -- see
   // https://www.modernescpp.com/index.php/thread-safe-initialization-of-a-singleton#h3-guarantees-of-the-c-runtime
   //
   static ObjectStoreTyped<NE> ostSingleton{NE::typeLookup, PRIMARY_TABLE<NE>, JUNCTION_TABLES<NE>};

   // C++11 provides a thread-safe way to ensure singleton.loadAll() is called exactly once
   //
   // NB: It's easier to just pass in nullptr to ObjectStoreTyped<NE>::loadAll than to do all the magic casting to
   //     allow std::call_once to invoke it with the default parameter (which is nullptr).
   static std::once_flag initFlag;
   std::call_once(initFlag, &ObjectStoreTyped<NE>::loadAll, &ostSingleton, nullptr);

   return ostSingleton;
}

//
// We have to make sure that each version of the above function gets instantiated.  NOTE: This is the 1st of 3 places we
// need to add any new ObjectStoreTyped
//
// You might think the use in InitialiseAllObjectStores below is sufficient for this, but the GCC linker says
// otherwise.
//
template ObjectStoreTyped<Boil                     > & ObjectStoreTyped<Boil                     >::getInstance();
template ObjectStoreTyped<BoilStep                 > & ObjectStoreTyped<BoilStep                 >::getInstance();
template ObjectStoreTyped<BrewNote                 > & ObjectStoreTyped<BrewNote                 >::getInstance();
template ObjectStoreTyped<Equipment                > & ObjectStoreTyped<Equipment                >::getInstance();
template ObjectStoreTyped<Fermentable              > & ObjectStoreTyped<Fermentable              >::getInstance();
template ObjectStoreTyped<Fermentation             > & ObjectStoreTyped<Fermentation             >::getInstance();
template ObjectStoreTyped<FermentationStep         > & ObjectStoreTyped<FermentationStep         >::getInstance();
template ObjectStoreTyped<Hop                      > & ObjectStoreTyped<Hop                      >::getInstance();
template ObjectStoreTyped<Instruction              > & ObjectStoreTyped<Instruction              >::getInstance();
template ObjectStoreTyped<InventoryFermentable     > & ObjectStoreTyped<InventoryFermentable     >::getInstance();
template ObjectStoreTyped<InventoryHop             > & ObjectStoreTyped<InventoryHop             >::getInstance();
template ObjectStoreTyped<InventoryMisc            > & ObjectStoreTyped<InventoryMisc            >::getInstance();
template ObjectStoreTyped<InventorySalt            > & ObjectStoreTyped<InventorySalt            >::getInstance();
template ObjectStoreTyped<InventoryYeast           > & ObjectStoreTyped<InventoryYeast           >::getInstance();
template ObjectStoreTyped<Mash                     > & ObjectStoreTyped<Mash                     >::getInstance();
template ObjectStoreTyped<MashStep                 > & ObjectStoreTyped<MashStep                 >::getInstance();
template ObjectStoreTyped<Misc                     > & ObjectStoreTyped<Misc                     >::getInstance();
template ObjectStoreTyped<Recipe                   > & ObjectStoreTyped<Recipe                   >::getInstance();
template ObjectStoreTyped<RecipeAdditionFermentable> & ObjectStoreTyped<RecipeAdditionFermentable>::getInstance();
template ObjectStoreTyped<RecipeAdditionHop        > & ObjectStoreTyped<RecipeAdditionHop        >::getInstance();
template ObjectStoreTyped<RecipeAdditionMisc       > & ObjectStoreTyped<RecipeAdditionMisc       >::getInstance();
template ObjectStoreTyped<RecipeAdditionYeast      > & ObjectStoreTyped<RecipeAdditionYeast      >::getInstance();
template ObjectStoreTyped<RecipeAdjustmentSalt     > & ObjectStoreTyped<RecipeAdjustmentSalt     >::getInstance();
template ObjectStoreTyped<RecipeUseOfWater         > & ObjectStoreTyped<RecipeUseOfWater         >::getInstance();
template ObjectStoreTyped<Salt                     > & ObjectStoreTyped<Salt                     >::getInstance();
template ObjectStoreTyped<Style                    > & ObjectStoreTyped<Style                    >::getInstance();
template ObjectStoreTyped<Water                    > & ObjectStoreTyped<Water                    >::getInstance();
template ObjectStoreTyped<Yeast                    > & ObjectStoreTyped<Yeast                    >::getInstance();

bool InitialiseAllObjectStores(QString & errorMessage) {
   // It's deliberate that we don't stop after the first error.  If there is a problem, it's quite useful to know how
   // extensive it is.
   QStringList errors;
   // NOTE: This is the 2nd of 3 places we need to add any new ObjectStoreTyped
   if (ObjectStoreTyped<Boil                     >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Boil"                     ; }
   if (ObjectStoreTyped<BoilStep                 >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "BoilStep"                 ; }
   if (ObjectStoreTyped<BrewNote                 >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "BrewNote"                 ; }
   if (ObjectStoreTyped<Equipment                >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Equipment"                ; }
   if (ObjectStoreTyped<Fermentable              >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Fermentable"              ; }
   if (ObjectStoreTyped<Fermentation             >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Fermentation"             ; }
   if (ObjectStoreTyped<FermentationStep         >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "FermentationStep"         ; }
   if (ObjectStoreTyped<Hop                      >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Hop"                      ; }
   if (ObjectStoreTyped<Instruction              >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Instruction"              ; }
   if (ObjectStoreTyped<InventoryFermentable     >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "InventoryFermentable"     ; }
   if (ObjectStoreTyped<InventoryHop             >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "InventoryHop"             ; }
   if (ObjectStoreTyped<InventoryMisc            >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "InventoryMisc"            ; }
   if (ObjectStoreTyped<InventorySalt            >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "InventorySalt"            ; }
   if (ObjectStoreTyped<InventoryYeast           >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "InventoryYeast"           ; }
   if (ObjectStoreTyped<Mash                     >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Mash"                     ; }
   if (ObjectStoreTyped<MashStep                 >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "MashStep"                 ; }
   if (ObjectStoreTyped<Misc                     >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Misc"                     ; }
   if (ObjectStoreTyped<Recipe                   >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Recipe"                   ; }
   if (ObjectStoreTyped<RecipeAdditionFermentable>::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "RecipeAdditionFermentable"; }
   if (ObjectStoreTyped<RecipeAdditionHop        >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "RecipeAdditionHop"        ; }
   if (ObjectStoreTyped<RecipeAdditionMisc       >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "RecipeAdditionMisc"       ; }
   if (ObjectStoreTyped<RecipeAdditionYeast      >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "RecipeAdditionYeast"      ; }
   if (ObjectStoreTyped<RecipeAdjustmentSalt     >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "RecipeAdjustmentSalt"     ; }
   if (ObjectStoreTyped<RecipeUseOfWater         >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "RecipeUseOfWater"         ; }
   if (ObjectStoreTyped<Salt                     >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Salt"                     ; }
   if (ObjectStoreTyped<Style                    >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Style"                    ; }
   if (ObjectStoreTyped<Water                    >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Water"                    ; }
   if (ObjectStoreTyped<Yeast                    >::getInstance().state() == ObjectStore::State::ErrorInitialising) { errors << "Yeast"                    ; }

   if (errors.size() > 0) {
      errorMessage = QObject::tr("There were errors loading the following object store(s): %1").arg(errors.join(", "));
      return false;
   }

   return true;
}

namespace {
   QVector<ObjectStore const *> getAllObjectStores() {
      // NOTE: This is the 3rd of 3 places we need to add any new ObjectStoreTyped
      static QVector<ObjectStore const *> allObjectStores {
         &ObjectStoreTyped<Boil                     >::getInstance(),
         &ObjectStoreTyped<BoilStep                 >::getInstance(),
         &ObjectStoreTyped<BrewNote                 >::getInstance(),
         &ObjectStoreTyped<Equipment                >::getInstance(),
         &ObjectStoreTyped<Fermentable              >::getInstance(),
         &ObjectStoreTyped<Fermentation             >::getInstance(),
         &ObjectStoreTyped<FermentationStep         >::getInstance(),
         &ObjectStoreTyped<Hop                      >::getInstance(),
         &ObjectStoreTyped<Instruction              >::getInstance(),
         &ObjectStoreTyped<InventoryFermentable     >::getInstance(),
         &ObjectStoreTyped<InventoryHop             >::getInstance(),
         &ObjectStoreTyped<InventoryMisc            >::getInstance(),
         &ObjectStoreTyped<InventorySalt            >::getInstance(),
         &ObjectStoreTyped<InventoryYeast           >::getInstance(),
         &ObjectStoreTyped<Mash                     >::getInstance(),
         &ObjectStoreTyped<MashStep                 >::getInstance(),
         &ObjectStoreTyped<Misc                     >::getInstance(),
         &ObjectStoreTyped<Recipe                   >::getInstance(),
         &ObjectStoreTyped<RecipeAdditionFermentable>::getInstance(),
         &ObjectStoreTyped<RecipeAdditionHop        >::getInstance(),
         &ObjectStoreTyped<RecipeAdditionMisc       >::getInstance(),
         &ObjectStoreTyped<RecipeAdditionYeast      >::getInstance(),
         &ObjectStoreTyped<RecipeAdjustmentSalt     >::getInstance(),
         &ObjectStoreTyped<RecipeUseOfWater         >::getInstance(),
         &ObjectStoreTyped<Salt                     >::getInstance(),
         &ObjectStoreTyped<Style                    >::getInstance(),
         &ObjectStoreTyped<Water                    >::getInstance(),
         &ObjectStoreTyped<Yeast                    >::getInstance(),
      };
      return allObjectStores;
   }
}

bool CreateAllDatabaseTables(Database & database, QSqlDatabase & connection) {
   qDebug() << Q_FUNC_INFO;
   for (auto ii : getAllObjectStores()) {
      if (!ii->createTables(database, connection)) {
         return false;
      }
   }
   for (auto jj : getAllObjectStores()) {
      if (!jj->addTableConstraints(database, connection)) {
         return false;
      }
   }
   return true;
}

bool WriteAllObjectStoresToNewDb(Database & newDatabase, QSqlDatabase & connectionNew) {
   //
   // Start transaction
   // By the magic of RAII, this will abort if we exit this function (including by throwing an exception) without
   // having called dbTransaction.commit().  (It will also turn foreign keys back on either way -- whether the
   // transaction is committed or rolled back.)
   //
   DbTransaction dbTransaction{newDatabase, connectionNew, "Write All", DbTransaction::DISABLE_FOREIGN_KEYS};

   for (ObjectStore const * objectStore : getAllObjectStores()) {
      if (!objectStore->writeAllToNewDb(newDatabase, connectionNew)) {
         return false;
      }
   }

   dbTransaction.commit();
   return true;
}
