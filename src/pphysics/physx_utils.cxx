#include "physx_utils.h"
#include "config_pphysics.h"
#include "physShape.h"
#include "physMaterial.h"

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

/**
 *
 */
PhysMaterial *
phys_material_from_shape_and_face_index(PhysShape *shape, size_t face_index) {
  if (shape == nullptr) {
    return nullptr;
  }

  physx::PxShape *pxshape = shape->get_shape();

  int material_index = 0;

  switch (pxshape->getGeometryType()) {
  // For triangle meshes and height fields, materials can be assigned to each
  // triangle that index into the shape's list of materials.
  case physx::PxGeometryType::eTRIANGLEMESH: {
      if (face_index != 0xFFFFffff) {
        physx::PxTriangleMeshGeometry geom;
        pxshape->getTriangleMeshGeometry(geom);
        material_index = geom.triangleMesh->getTriangleMaterialIndex(face_index);
        if (material_index == 0xffff) {
          material_index = 0;
        }
      }
    }
    break;

  case physx::PxGeometryType::eHEIGHTFIELD: {
      if (face_index != 0xFFFFffff) {
        physx::PxHeightFieldGeometry geom;
        pxshape->getHeightFieldGeometry(geom);
        material_index = geom.heightField->getTriangleMaterialIndex(face_index);
        if (material_index == 0xffff) {
          material_index = 0;
        }
      }
    }
    break;

  default:
    break;
  }

  return shape->get_material(material_index);
}
