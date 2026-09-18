#pragma once
//
// Portable I2C / MCP23017 diagnostics for ESPHome.
// Drop this file next to your YAML and add:
//
//   esphome:
//     includes:
//       - mcp_tools.h
//
// Then call from any lambda:
//
//   mcp_tools::dump_mcp23017(id(bus_i2c), 0x20);
//   mcp_tools::i2c_scan(id(bus_i2c));
//
// Nothing here is project specific — the same file is meant to be copied
// (or git-shared) across boards.
//
#include "esphome/core/log.h"
#include "esphome/components/i2c/i2c.h"

namespace mcp_tools {

static const char *const TAG_DUMP = "mcp_dump";
static const char *const TAG_SCAN = "i2c_scan";

// Render a byte MSB..LSB into buf[9].
inline void byte_to_bin(uint8_t v, char *buf) {
  for (int b = 0; b < 8; b++)
    buf[7 - b] = ((v >> b) & 1) ? '1' : '0';
  buf[8] = '\0';
}

// Full MCP23017 register dump, A/B side by side.
// Assumes IOCON.BANK = 0 (power-on default, and what ESPHome uses).
inline void dump_mcp23017(esphome::i2c::I2CBus *bus, uint8_t addr) {
  struct Reg {
    const char *name;
    uint8_t a;
    uint8_t b;
  };
  static const Reg REGS[] = {
      {"IODIR", 0x00, 0x01},    // 1 = input, 0 = output
      {"IPOL", 0x02, 0x03},     // 1 = value inverted in GPIO reg
      {"GPINTEN", 0x04, 0x05},  // interrupt-on-change enable
      {"DEFVAL", 0x06, 0x07},
      {"INTCON", 0x08, 0x09},
      {"IOCON", 0x0A, 0x0B},  // BANK, MIRROR, SEQOP, ODR, INTPOL
      {"GPPU", 0x0C, 0x0D},   // 1 = 100k pull-up on
      {"INTF", 0x0E, 0x0F},
      {"INTCAP", 0x10, 0x11},
      {"GPIO", 0x12, 0x13},  // raw level seen at the pin
      {"OLAT", 0x14, 0x15},
  };
  const uint8_t n = sizeof(REGS) / sizeof(REGS[0]);

  esphome::i2c::I2CDevice dev;
  dev.set_i2c_bus(bus);
  dev.set_i2c_address(addr);
  auto rd = [&dev](uint8_t reg, uint8_t &out) -> bool {
    return dev.read_register(reg, &out, 1) == esphome::i2c::ERROR_OK;
  };

  ESP_LOGI(TAG_DUMP, "########## MCP23017 @ 0x%02X ##########", addr);

  uint8_t gpio_a = 0, gpio_b = 0, ipol_a = 0, ipol_b = 0;
  uint8_t iodir_a = 0, iodir_b = 0, gppu_a = 0, gppu_b = 0;
  bool any_fail = false;

  for (uint8_t r = 0; r < n; r++) {
    uint8_t va = 0, vb = 0;
    const bool oka = rd(REGS[r].a, va);
    const bool okb = rd(REGS[r].b, vb);
    if (!oka || !okb) {
      ESP_LOGW(TAG_DUMP, "  %-8s READ FAILED (A:%d B:%d)", REGS[r].name, oka, okb);
      any_fail = true;
      continue;
    }
    char ba[9], bb[9];
    byte_to_bin(va, ba);
    byte_to_bin(vb, bb);
    ESP_LOGI(TAG_DUMP, "  %-8s A=0x%02X (%s)  B=0x%02X (%s)", REGS[r].name, va, ba, vb, bb);
    switch (REGS[r].a) {
      case 0x00: iodir_a = va; iodir_b = vb; break;
      case 0x02: ipol_a = va;  ipol_b = vb;  break;
      case 0x0C: gppu_a = va;  gppu_b = vb;  break;
      case 0x12: gpio_a = va;  gpio_b = vb;  break;
      default: break;
    }
  }

  if (any_fail) {
    ESP_LOGW(TAG_DUMP, "  one or more registers did not answer — check wiring/address");
  }

  ESP_LOGI(TAG_DUMP, "  --- per pin: raw GPIO / IPOL / direction / pull-up ---");
  for (uint8_t pin = 0; pin < 16; pin++) {
    const bool hi = pin >= 8;
    const uint8_t sh = pin % 8;
    const uint8_t g = ((hi ? gpio_b : gpio_a) >> sh) & 1;
    const uint8_t i = ((hi ? ipol_b : ipol_a) >> sh) & 1;
    const uint8_t d = ((hi ? iodir_b : iodir_a) >> sh) & 1;
    const uint8_t p = ((hi ? gppu_b : gppu_a) >> sh) & 1;
    ESP_LOGI(TAG_DUMP, "  %s%u (pin %2u): GPIOraw=%u  IPOL=%u  DIR=%-3s  PU=%u",
             hi ? "B" : "A", sh, pin, g, i, d ? "in" : "out", p);
  }
  ESP_LOGI(TAG_DUMP, "########## END 0x%02X ##########", addr);
}

// Probe every 7-bit address with a zero-length write: an ACK means something
// is there. ESPHome's own boot scan uses writev(), but that is not public on
// I2CBus, so we go through write() -- same bus transaction (address + stop).
// Returns the number of devices found.
inline int i2c_scan(esphome::i2c::I2CBus *bus) {
  ESP_LOGI(TAG_SCAN, "--- I2C scan ---");
  int found = 0;
  for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
    if (bus->write(addr, nullptr, 0) != esphome::i2c::ERROR_OK)
      continue;
    const char *hint = "";
    switch (addr) {
      case 0x20 ... 0x27: hint = "  (MCP23017 / PCF8574 range)"; break;
      case 0x50 ... 0x57: hint = "  (EEPROM range)"; break;
      case 0x68: hint = "  (DS1307/DS3231 RTC, MPU6050)"; break;
      case 0x6F: hint = "  (MCP7940M RTC)"; break;
      case 0x3C: case 0x3D: hint = "  (SSD1306 OLED)"; break;
      case 0x76: case 0x77: hint = "  (BMP/BME280)"; break;
      default: break;
    }
    ESP_LOGI(TAG_SCAN, "  found 0x%02X%s", addr, hint);
    found++;
  }
  ESP_LOGI(TAG_SCAN, "--- scan done: %d device(s) ---", found);
  return found;
}

}  // namespace mcp_tools
