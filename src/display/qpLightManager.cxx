/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qpLightManager.cxx
 * @author brian
 * @date 2022-12-21
 */

#include "qpLightManager.h"

/**
 *
 */
qpLightManager::
qpLightManager() {
}

/**
 *
 */
void qpLightManager::
initialize() {
  _static_light_buffer = new Texture("static-light-buffer");
  _static_light_buffer->setup_buffer_texture(1, Texture::T_float, Texture::F_rgba32, GeomEnums::UH_static);
  _static_light_buffer->set_compression(Texture::CM_off);
  _static_light_buffer->set_keep_ram_image(false);

  _dynamic_light_buffer = new Texture("dynamic-light-buffer");
  _dynamic_light_buffer->setup_buffer_texture(1, Texture::T_float, Texture::F_rgba32, GeomEnums::UH_dynamic);
  _dynamic_light_buffer->set_compression(Texture::CM_off);
  _dynamic_light_buffer->set_keep_ram_image(true);
}

/**
 *
 */
void qpLightManager::
update_light_buffer(Texture *buffer, PT(qpLight) *lights, int num_lights) {
  if (buffer->get_x_size() < num_lights * 5) {
    buffer->set_x_size(num_lights * 5);
  }
  PTA_uchar img = buffer->modify_ram_image();
  float *fdata = (float *)img.p();

  for (int i = 0; i < num_lights; ++i) {
    qpLight *light = lights[i];

    PN_stdfloat stopdot = ccos(light->get_outer_cone());
    PN_stdfloat stopdot2 = ccos(light->get_inner_cone());
    PN_stdfloat oodot = (stopdot > stopdot2) ? 1.0f / (stopdot - stopdot2) : 0.0f;

    // Texel 0: light type, atten coefficients
    *fdata++ = light->get_light_type();
    *fdata++ = light->get_constant_atten();
    *fdata++ = light->get_linear_atten();
    *fdata++ = light->get_quadratic_atten();

    // Texel 1: color, atten radius
    *fdata++ = light->get_color_linear()[0];
    *fdata++ = light->get_color_linear()[1];
    *fdata++ = light->get_color_linear()[2];
    *fdata++ = light->get_attenuation_radius();

    // Texel 2: pos
    *fdata++ = light->get_pos()[0];
    *fdata++ = light->get_pos()[1];
    *fdata++ = light->get_pos()[2];
    *fdata++ = 0.0f;

    // Texel 3: dir
    *fdata++ = light->get_direction()[0];
    *fdata++ = light->get_direction()[1];
    *fdata++ = light->get_direction()[2];
    *fdata++ = 0.0f;

    // Texel 4: spotlight params
    *fdata++ = light->get_exponent();
    *fdata++ = stopdot;
    *fdata++ = stopdot2;
    *fdata++ = oodot;
  }
}

/**
 *
 */
void qpLightManager::
add_static_light(qpLight *light) {
  _static_lights.push_back(light);
}

/**
 *
 */
void qpLightManager::
clear_static_lights() {
  _static_lights.clear();
}

/**
 *
 */
void qpLightManager::
add_dynamic_light(qpLight *light) {
  _dynamic_lights.insert(light);
}

/**
 *
 */
void qpLightManager::
remove_dynamic_light(qpLight *light) {
  _dynamic_lights.erase(light);
}

/**
 *
 */
void qpLightManager::
clear_dynamic_lights() {
  _dynamic_lights.clear();
}

/**
 *
 */
void qpLightManager::
update() {
  update_light_buffer(_dynamic_light_buffer, &_dynamic_lights.front(), _dynamic_lights.size());
}
