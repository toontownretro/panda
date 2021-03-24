/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspLumps.h
 * @author lachbr
 * @date 2021-01-01
 */

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DFlagsLump::
get_size(int version) {
  return sizeof(DFlagsLump);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DGameLump::
get_size(int version) {
  return sizeof(DGameLump);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DModel::
get_size(int version) {
  return sizeof(DModel);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DPhysModel::
get_size(int version) {
  return sizeof(DPhysModel);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DPhysDisp::
get_size(int version) {
  return sizeof(DPhysDisp);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DVertex::
get_size(int version) {
  return sizeof(DVertex);
}

/**
 *
 */
INLINE DPlane::
DPlane() {
  dist = 0.0f;
  type = 0;
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DPlane::
get_size(int version) {
  return sizeof(DPlane);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DNode::
get_size(int version) {
  return sizeof(DNode);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t TexInfo::
get_size(int version) {
  return sizeof(TexInfo);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DTexData::
get_size(int version) {
  return sizeof(DTexData);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DOccluderData::
get_size(int version) {
  size_t size = sizeof(DOccluderData);

  if (version < 2) {
    // Version 0 doesn't have area.
    size -= sizeof(area);
  }

  return size;
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DOccluderPolyData::
get_size(int version) {
  return sizeof(DOccluderPolyData);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DispSubNeighbor::
get_size(int version) {
  return sizeof(DispSubNeighbor);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DispNeighbor::
get_size(int version) {
  return sizeof(DispNeighbor);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DispCornerNeighbors::
get_size(int version) {
  return sizeof(DispCornerNeighbors);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DispVert::
get_size(int version) {
  return sizeof(DispVert);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DispTri::
get_size(int version) {
  return sizeof(DispTri);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DispMultiBlend::
get_size(int version) {
  return sizeof(DispMultiBlend);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DDispInfo::
get_size(int version) {
  return sizeof(DDispInfo);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DEdge::
get_size(int version) {
  return sizeof(DEdge);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DPrimitive::
get_size(int version) {
  return sizeof(DPrimitive);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DPrimVert::
get_size(int version) {
  return sizeof(DPrimVert);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DFace::
get_size(int version) {
  return sizeof(DFace);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DFaceID::
get_size(int version) {
  return sizeof(DFaceID);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DFaceBrushList::
get_size(int version) {
  return sizeof(DFaceBrushList);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DLeaf::
get_size(int version) {
  size_t size = sizeof(DLeaf);
  if (version >= 1) {
    // This was moved to it's own lump in version 1.
    size -= sizeof(ambient_lighting);
  }

  return size;
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DLeafAmbientLighting::
get_size(int version) {
  return sizeof(DLeafAmbientLighting);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DLeafAmbientIndex::
get_size(int version) {
  return sizeof(DLeafAmbientIndex);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DBrushSide::
get_size(int version) {
  return sizeof(DBrushSide);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DBrush::
get_size(int version) {
  return sizeof(DBrush);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DVis::
get_size(int version) {
  return sizeof(DVis);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DAreaPortal::
get_size(int version) {
  return sizeof(DAreaPortal);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DArea::
get_size(int version) {
  return sizeof(DArea);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DLeafWaterData::
get_size(int version) {
  return sizeof(DLeafWaterData);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t FaceMacroTextureInfo::
get_size(int version) {
  return sizeof(FaceMacroTextureInfo);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DWorldlight::
get_size(int version) {
  size_t size = sizeof(origin) +
    sizeof(intensity) +
    sizeof(normal);

  if (version >= 1) {
    // Version 0 doesn't have this.
    size += sizeof(shadow_cast_offset);
  }

  size += sizeof(cluster) +
    sizeof(type) +
    sizeof(style) +
    sizeof(stopdot) +
    sizeof(stopdot2) +
    sizeof(exponent) +
    sizeof(radius) +
    sizeof(constant_attn) +
    sizeof(linear_attn) +
    sizeof(quadratic_attn) +
    sizeof(flags) +
    sizeof(texinfo) +
    sizeof(owner);

  return size;
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DCubeMapSample::
get_size(int version) {
  return sizeof(DCubeMapSample);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DOverlay::
get_size(int version) {
  return sizeof(DOverlay);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DOverlayFade::
get_size(int version) {
  return sizeof(DOverlayFade);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DOverlaySystemLevel::
get_size(int version) {
  return sizeof(DOverlaySystemLevel);
}

/**
 * Returns the number of bytes in the structure for the given lump version.
 */
INLINE size_t DWaterOverlay::
get_size(int version) {
  return sizeof(DWaterOverlay);
}

/**
 * Returns the number of primitives in the face.
 */
INLINE unsigned short DFace::
get_num_prims() const {
  return num_prims & 0x7FFF;
}

/**
 * Sets the number of primitives in the face.
 */
INLINE void DFace::
set_num_prims(unsigned short count) {
  nassertv((count & 0x8000) == 0);
	num_prims &= ~0x7FFF;
	num_prims |= (count & 0x7FFF);
}

/**
 * Returns whether or not dynamic shadows are enabled for this face.
 */
INLINE bool DFace::
get_dynamic_shadows_enabled() const {
  return (num_prims & 0x8000) == 0;
}

/**
 * Enables or disables dynamic shadows on this face.
 */
INLINE void DFace::
set_dynamic_shadows_enabled(bool enabled) {
  if (enabled) {
		num_prims &= ~0x8000;
  } else {
		num_prims |= 0x8000;
  }
}

/**
 * Sets the face count of the overlay.
 */
INLINE void DOverlay::
set_face_count(unsigned short count) {
  nassertv((count & OVERLAY_RENDER_ORDER_MASK) == 0);
  _face_count_render_order &= OVERLAY_RENDER_ORDER_MASK;
  _face_count_render_order |= (count & ~OVERLAY_RENDER_ORDER_MASK);
}

/**
 * Returns the face count of the overlay.
 */
INLINE unsigned short DOverlay::
get_face_count() const {
  return _face_count_render_order & ~OVERLAY_RENDER_ORDER_MASK;
}

/**
 * Sets the render order of the overlay.
 */
INLINE void DOverlay::
set_render_order(unsigned short order) {
  nassertv(order < OVERLAY_NUM_RENDER_ORDERS);
  _face_count_render_order &= ~OVERLAY_RENDER_ORDER_MASK;
  _face_count_render_order |= (order << (16 - OVERLAY_RENDER_ORDER_NUM_BITS));
}

/**
 * Returns the render order of the overlay.
 */
INLINE unsigned short DOverlay::
get_render_order() const {
  return (_face_count_render_order >> (16 - OVERLAY_RENDER_ORDER_NUM_BITS));
}

/**
 * Sets the face count of the overlay.
 */
INLINE void DWaterOverlay::
set_face_count(unsigned short count) {
  nassertv((count & WATEROVERLAY_RENDER_ORDER_MASK) == 0);
  _face_count_render_order &= WATEROVERLAY_RENDER_ORDER_MASK;
  _face_count_render_order |= (count & ~WATEROVERLAY_RENDER_ORDER_MASK);
}

/**
 * Returns the face count of the overlay.
 */
INLINE unsigned short DWaterOverlay::
get_face_count() const {
  return _face_count_render_order & ~WATEROVERLAY_RENDER_ORDER_MASK;
}

/**
 * Sets the render order of the overlay.
 */
INLINE void DWaterOverlay::
set_render_order(unsigned short order) {
  nassertv(order < WATEROVERLAY_NUM_RENDER_ORDERS);
  _face_count_render_order &= ~WATEROVERLAY_RENDER_ORDER_MASK;
  _face_count_render_order |= (order << (16 - WATEROVERLAY_RENDER_ORDER_NUM_BITS));
}

/**
 * Returns the render order of the overlay.
 */
INLINE unsigned short DWaterOverlay::
get_render_order() const {
  return (_face_count_render_order >> (16 - WATEROVERLAY_RENDER_ORDER_NUM_BITS));
}

/**
 *
 */
INLINE Entity::
Entity() {
  first_brush = 0;
  num_brushes = 0;
  epairs = nullptr;

  area_portal_num = 0;
  portal_areas[0] = portal_areas[1] = 0;
  portals_leading_into_areas[0] = portals_leading_into_areas[1] = nullptr;
}
