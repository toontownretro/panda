#include "physx_utils.h"
#include "config_pphysics.h"

/**
 * Converts a measurement from the configured Panda units to PhysX units
 * (meters).
 */
float
panda_length_to_physx(float distance) {
  switch (phys_panda_length_unit.get_value()) {
  case PPLU_meters:
    return distance;
  case PPLU_feet:
    return feet_to_meters(distance);
  case PPLU_inches:
    return inches_to_meters(distance);
  case PPLU_millimeters:
    return mm_to_meters(distance);
  case PPLU_centimeters:
    return cm_to_meters(distance);
  }
}

/**
 * Converts a measurement from PhysX units (meters) to the configured Panda
 * units.
 */
float
physx_length_to_panda(float distance) {
  switch(phys_panda_length_unit.get_value()) {
  case PPLU_meters:
    return distance;
  case PPLU_feet:
    return meters_to_feet(distance);
  case PPLU_inches:
    return meters_to_inches(distance);
  case PPLU_millimeters:
    return meters_to_mm(distance);
  case PPLU_centimeters:
    return meters_to_cm(distance);
  }
}

/**
 * Converts a mass value from the configured Panda units to PhysX units
 * (kilograms).
 */
float
panda_mass_to_physx(float mass) {
  switch (phys_panda_mass_unit.get_value()) {
  case PPMU_kilograms:
    return mass;
  case PPMU_grams:
    return g_to_kg(mass);
  case PPMU_milligrams:
    return mg_to_kg(mass);
  case PPMU_pounds:
    return lb_to_kg(mass);
  case PPMU_ounces:
    return oz_to_kg(mass);
  }
}

/**
 * Converts a mass value from PhysX units (kilograms) to the configured Panda
 * units.
 */
float
physx_mass_to_panda(float mass) {
  switch (phys_panda_mass_unit.get_value()) {
  case PPMU_kilograms:
    return mass;
  case PPMU_grams:
    return kg_to_g(mass);
  case PPMU_milligrams:
    return kg_to_mg(mass);
  case PPMU_pounds:
    return kg_to_lb(mass);
  case PPMU_ounces:
    return kg_to_oz(mass);
  }
}
