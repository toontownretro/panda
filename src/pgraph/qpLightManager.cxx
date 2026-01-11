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
#include "pStatTimer.h"
#include "pStatCollector.h"

static PStatCollector update_buffer_pcollector("LightManager:UpdateBuffer");

/**
 *
 */
qpLightManager::
qpLightManager() :
  _dynamic_buffer_index(0),
  _dynamic_lights_dirty(true)
{
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

  for (int i = 0; i < num_buffers; ++i) {
    PT(Texture) dynamic_light_buffer = new Texture("dynamic-light-buffer");
    dynamic_light_buffer->setup_buffer_texture(1, Texture::T_float, Texture::F_rgba32, GeomEnums::UH_dynamic);
    dynamic_light_buffer->set_compression(Texture::CM_off);
    dynamic_light_buffer->set_keep_ram_image(true);
    _dynamic_light_buffers[i] = dynamic_light_buffer;
  }
}

/**
 *
 */
void qpLightManager::
update_light_buffer(Texture *buffer, PT(qpLight) *lights, int num_lights) {
  PStatTimer timer(update_buffer_pcollector);

  if (buffer->get_x_size() < num_lights * 5) {
    buffer->set_x_size(num_lights * 5);
  }
  PTA_uchar img = buffer->modify_ram_image();
  float *fdata = (float *)img.p();

  for (int i = 0; i < num_lights; ++i) {
    qpLight *light = lights[i];

    PN_stdfloat stopdot = ccos(light->get_inner_cone());
    PN_stdfloat stopdot2 = ccos(light->get_outer_cone());
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
  light->set_manager(this);
  _static_lights.push_back(light);
}

/**
 *
 */
void qpLightManager::
clear_static_lights() {
  _static_lights.clear();
  for (qpLight *light : _static_lights) {
    light->set_manager(nullptr);
  }
}

/**
 *
 */
void qpLightManager::
add_dynamic_light(qpLight *light) {
  light->set_manager(this);
  _dynamic_lights.insert(light);
  _dynamic_lights_dirty = true;
}

/**
 *
 */
void qpLightManager::
remove_dynamic_light(qpLight *light) {
  light->set_manager(nullptr);
  _dynamic_lights.erase(light);
  _dynamic_lights_dirty = true;
}

/**
 *
 */
void qpLightManager::
clear_dynamic_lights() {
  for (qpLight *light : _dynamic_lights) {
    light->set_manager(nullptr);
  }
  _dynamic_lights.clear();
  _dynamic_lights_dirty = true;
}

/**
 *
 */
void qpLightManager::
update() {
  if (_dynamic_lights_dirty) {
    Texture *buffer = _dynamic_light_buffers[_dynamic_buffer_index];
    {
      CDWriter cdata(_cycler);
      cdata->_dynamic_light_buffer = buffer;
    }
    PT(qpLight) *lights = nullptr;
    if (!_dynamic_lights.empty()) {
      lights = &_dynamic_lights.front();
    }
    update_light_buffer(buffer, lights, _dynamic_lights.size());
    ++_dynamic_buffer_index;
    _dynamic_buffer_index %= num_buffers;
    _dynamic_lights_dirty = false;
  }
}

/**
 *
 */
qpLightManager::CData::
CData() :
  _dynamic_light_buffer(nullptr)
{
}

/**
 *
 */
qpLightManager::CData::
CData(const CData &copy) :
  _dynamic_light_buffer(copy._dynamic_light_buffer)
{
}

/**
 *
 */
CycleData *qpLightManager::CData::
make_copy() const {
  return new CData(*this);
}
