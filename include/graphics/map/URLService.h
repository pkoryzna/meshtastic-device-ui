#pragma once
#include "graphics/map/TileService.h"
#include <functional>

#ifdef ARDUINO_ARCH_ESP32
#ifndef WHY_BADGE // P4 doesn't have a wifi built in so we get compilation errors

class URLService : public ITileService
{
  public:
    using Callback = std::function<bool(const char *name, void *img, size_t len)>;

    URLService(Callback cb = nullptr);
    bool load(const char *name, void *img) override;
    virtual ~URLService();

  private:
    Callback saveCB = nullptr;
};

#endif
#endif