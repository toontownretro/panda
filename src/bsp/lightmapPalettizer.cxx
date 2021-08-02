/**
 * PANDA3D BSP LIBRARY
 *
 * Copyright (c) Brian Lach <brianlach72@gmail.com>
 * All rights reserved.
 *
 * @file lightmapPalettizer.cpp
 * @author Brian Lach
 * @date August 01, 2018
 */

#include "lightmapPalettizer.h"
#include "bspData.h"
#include "bspFlags.h"

NotifyCategoryDef(lightmapPalettizer, "");

LightmapPalettizer::
LightmapPalettizer(const BSPData *data) :
  _data(data) {
}

int
get_byte_offset(int xel_size, int row_size, int face_size, int page, int x, int y) {
  int ofs = face_size * page;
  ofs += y * row_size;
  ofs += x * xel_size;
  return ofs;
}

void
put_xel(PTA_uchar &img, int pos, LVector3f xel, bool hdr) {
  if (!hdr) {
    unsigned char r = std::clamp((int)(xel[0] + 0.5), 0, 255);
    unsigned char g = std::clamp((int)(xel[1] + 0.5), 0, 255);
    unsigned char b = std::clamp((int)(xel[2] + 0.5), 0, 255);
    img.set_element(pos++, b);
    img.set_element(pos++, g);
    img.set_element(pos++, r);
  } else {
    xel /= 255.0f;
    // Write each byte of each component.
    for (int i = 2; i >= 0; i--) {
      const unsigned char *data = (const unsigned char *)(xel.get_data() + i);
      img.set_element(pos++, data[0]);
      img.set_element(pos++, data[1]);
      img.set_element(pos++, data[2]);
      img.set_element(pos++, data[3]);
    }
  }
}

void
blit_lightmap_bits(const BSPData *data, LightmapPalette::Entry *entry,
                   PTA_uchar &img, int lmnum = 0, bool bounced = false, int style = 0) {
  const DFace *face = &data->dfaces[entry->facenum];
  int width = face->lightmap_size[0] + 1;
  int height = face->lightmap_size[1] + 1;

  int border = 2;

  int width_border = width + (border * 2);
  int height_border = height + (border * 2);

  bool hdr = (data->pdlightdata) == &(data->dlightdata_hdr);
  int xel_size = hdr ? sizeof(float) * 3 : 3;
  int row_size = xel_size * entry->palette->size[0];
  int face_size = xel_size * entry->palette->size[0] * entry->palette->size[1];

  // Loop through each pixel that will be in the resulting palette, *including*
  // pixels part of the border.
  for (int y = 0; y < height_border; y++) {
    for (int x = 0; x < width_border; x++) {
      int pos = get_byte_offset(xel_size, row_size, face_size, lmnum,
                                entry->offset[0] - border + x, entry->offset[1] - border + y);

      // Now determine the luxel to sample.  Clamp the X and Y to the lightmap
      // size not including the border.
      int luxel_x = std::clamp(x - border, 0, width - 1);
      int luxel_y = std::clamp(y - border, 0, height - 1);
      int luxel = (luxel_y * width) + luxel_x;

      const ColorRGBExp32 *sample;
      //if ( !bounced )
      //        sample = SampleLightmap( level->get_bspdata(), face, luxel, style, lmnum );
      //else
      //        sample = SampleBouncedLightmap( level->get_bspdata(), face, luxel );
      sample = data->sample_lightmap(face, luxel, style, lmnum);

      // Luxel is in linear-space.
      LVector3 luxel_col = sample->as_linear_color();

      put_xel(img, pos, luxel_col, hdr);
    }
  }
}

PT(LightmapPaletteDirectory) LightmapPalettizer::
palettize_lightmaps() {
  PT(LightmapPaletteDirectory) dir = new LightmapPaletteDirectory;
  dir->face_palette_entries.resize(_data->dfaces.size());
  memset(dir->face_palette_entries.data(), 0, sizeof(LightmapPalette::Entry *) * dir->face_palette_entries.size());

  // Put each face in one or more palettes
  for (int facenum = 0; facenum < _data->dfaces.size(); facenum++) {
    const DFace *face = &_data->dfaces[facenum];
    if ( face->lightofs == -1 ) {
      // Face does not have a lightmap.
      continue;
    }

    PT(LightmapPalette::Entry) entry = new LightmapPalette::Entry;
    entry->facenum = facenum;
    entry->palette = nullptr;

    // Find or create a palette to put this face's lightmap in

    bool added = false;
    size_t palcount = dir->palettes.size();
    for (size_t i = 0; i < palcount; i++) {
      LightmapPalette *pal = dir->palettes[i];
      if (pal->packer.add_block(face->lightmap_size[0] + 1, face->lightmap_size[1] + 1, &entry->offset[0], &entry->offset[1])) {
        pal->entries.push_back(entry);
        entry->palette = pal;
        added = true;
        break;
      }
    }

    if (!added) {
      PT(LightmapPalette) pal = new LightmapPalette;
      if (!pal->packer.add_block(face->lightmap_size[0] + 1, face->lightmap_size[1] + 1, &entry->offset[0], &entry->offset[1])) {
        lightmapPalettizer_cat.error()
          << "lightmap (" << face->lightmap_size[0] + 1 << "x" << face->lightmap_size[1] + 1
          << ") too big to fit in palette (" << max_palette << "x" << max_palette << ")\n";
      }
      pal->entries.push_back(entry);
      entry->palette = pal;
      dir->palettes.push_back(pal);
    }

    dir->face_palette_entries[facenum] = entry;
  }

  Texture::ComponentType lightmap_component_type;
  int lightmap_xel_size;

  // LDR and HDR lightmaps are already in linear color space.
  if (_data->pdlightdata == &(_data->dlightdata_hdr)) {
    lightmap_component_type = Texture::T_float;
    lightmap_xel_size = sizeof(float) * 3;
  } else {
    lightmap_component_type = Texture::T_unsigned_byte;
    lightmap_xel_size = 3;
  }

  // We've found a palette for each lightmap to fit in. Now generate the actual textures
  // for each palette that can be applied to geometry.
  for (size_t i = 0; i < dir->palettes.size(); i++) {
    LightmapPalette *pal = dir->palettes[i];
    pal->packer.get_minimum_dimensions(&pal->size[0], &pal->size[1]);
    int width = pal->size[0];
    int height = pal->size[1];

    // We will manually fill in the RAM image for the texture.
    PTA_uchar image;
    image.resize(((width * height) * NUM_LIGHTMAPS) * lightmap_xel_size);

    pal->texture = new Texture;
    pal->texture->setup_2d_texture_array(width, height, NUM_LIGHTMAPS,
                                         lightmap_component_type, Texture::F_rgb);
    pal->texture->set_minfilter(SamplerState::FT_linear_mipmap_linear);
    pal->texture->set_magfilter(SamplerState::FT_linear);

    for (size_t j = 0; j < pal->entries.size(); j++) {
      LightmapPalette::Entry *entry = pal->entries[j];
      const DFace *face = &_data->dfaces[entry->facenum];
      const TexInfo *texinfo = &_data->texinfo[face->texinfo];

      int count = (texinfo->flags & SURF_BUMPLIGHT) ? (NUM_BUMP_VECTS + 1) : 1;
      for (int n = 0; n < count; n++) {
        blit_lightmap_bits(_data, entry, image, n);
      }

#if 0
      // Bounced
      blit_lightmap_bits(_data, entry, images[0], 0, true);

      int direct_count = face->bumped_lightmap ? NUM_BUMP_VECTS + 1 : 1;

      for (int n = 0; n < direct_count; n++) {
        blit_lightmap_bits(_data, entry, images[n + 1], n);
      }
#endif
    }

    pal->texture->set_ram_image(image);

    for (int n = 0; n < NUM_LIGHTMAPS; n++) {
      std::ostringstream ss;
      ss << "palette_dump/palette_" << i << "_" << n << ".tga";
      pal->texture->write(ss.str(), n, 0, false, false);
    }
  }

  return dir;
}
