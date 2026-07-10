#pragma once

#include "lgfx/v1/panel/Panel_SharpLCD.hpp"


/*\
 *

 Generic SharpDisplay Memory LCD configless example

 See https://github.com/lovyan03/LovyanGFX/issues/694

 Usage:

    static SharpDisplay tft(SHARP_SCK, SHARP_MOSI, SHARP_SS, 400, 240);

    void setup()
    {
      // Either start without clearing:
      tft.init_without_reset(false);
      // .. or:
      tft.begin();
      tft.clearDisplay();
    }



 *
\*/

class SharpDisplay : public lgfx::LGFX_Device
{
  protected:
    lgfx::Panel_SharpLCD _panel_sharp;
    int16_t _pin_sck = -1;
    int16_t _pin_mosi = -1;
    int16_t _pin_ss = -1;
    int16_t _pin_miso = -1;
    uint16_t _width = 400;
    uint16_t _height = 240;
    uint32_t _freq_write = 4000000;

  public:

    // overwrite LGFX_Base::clearDisplay(), Panel_SharpLCD::clearDisplay() is faster
    void clearDisplay() { _panel_sharp.clearDisplay(); }

    // provide means to get/push the buffer, GFX functions aren't fast with 1bit colors
    uint8_t *getBuffer() { return _panel_sharp.getBuffer(); }
    uint32_t getBufferSize() { return _panel_sharp.getBufferSize(); }
    void pushBuffer(uint8_t*buf) { _panel_sharp.display(buf,0,0,0,0); }

    void setDithering(bool enable)
    {
      auto cfg = _panel_sharp.config_detail();
      cfg.enable_dithering = enable;
      _panel_sharp.config_detail(cfg);
    }

    // Must be called before init()
    // Pass pin_miso (-1) if SD card not used
    void define(int16_t pin_sck, int16_t pin_mosi, int16_t pin_ss, int16_t pin_miso, uint16_t width, uint16_t height, uint32_t freq_write = 4000000)
    {
      _pin_sck = pin_sck;
      _pin_mosi = pin_mosi;
      _pin_ss = pin_ss;
      _pin_miso = pin_miso;
      _width = width;
      _height = height;
      _freq_write = freq_write;
      ESP_LOGI("SharpDisplay", "SharpDisplay pins are set");
    }

    bool init(lgfx::Bus_SPI* bus_spi)
    {

      if (_pin_ss == -1)
      {
        ESP_LOGE("SharpDisplay", "SharpDisplay pins not set!");
        return false;
      }

      {
        auto cfg = bus_spi->config();
        cfg.freq_write = _freq_write;
        cfg.pin_mosi   = _pin_mosi;
        cfg.pin_sclk   = _pin_sck;
        cfg.pin_miso   = _pin_miso;
        // No DMA: the SPI bus is shared with the ESP-IDF SDSPI driver, which
        // owns the GDMA channel. CPU-FIFO writes are sufficient for the small
        // monochrome framebuffer.
        cfg.dma_channel = 0;
        bus_spi->config(cfg);
        _panel_sharp.bus(bus_spi);
      }

      {
        auto cfg = _panel_sharp.config();
        cfg.pin_cs = _pin_ss;
        cfg.panel_width  = cfg.memory_width  = _width; // assuming width is a multiple of 8
        cfg.panel_height = cfg.memory_height = _height;
        _panel_sharp.config(cfg);
      }

      {
        auto cfg = _panel_sharp.config_detail();
        cfg.pin_dispon = -1;
        cfg.pin_extmod = -1;
        cfg.prefix_bytes = _height > 255 ? 2 : 1; // should be 2 when panel height > 255, otherwise 1
        cfg.suffix_bytes = 1; // your mileage may vary
        cfg.enable_dithering   = true; // lose 2-3 fps by enabling this
        cfg.enable_autodisplay = true; // disable this with tft.setAutoDisplay(false)
        _panel_sharp.config_detail(cfg);
      }

      setPanel(&_panel_sharp);

      bool success = bus_spi->init();
      if (success) {
        ESP_LOGI("SharpDisplay", "SharpDisplay::init OK");
      } else {
        ESP_LOGE("SharpDisplay", "SharpDisplay::init FAILED");
      }
      return success;
    }

};
