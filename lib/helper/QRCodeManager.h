#ifndef QRCODEMANAGER_H
#define QRCODEMANAGER_H

#include "../hardware/LGFX_ILI9488.h"
#ifdef PLATFORMIO
#include <qrcode.h>
#else 
#include "QRCode.h"
#endif 

class QRCodeManager {
  public: 
    void init(LGFX_Sprite* sprite);

    void create(const char* content);

    void setOffset(int x, int y);
    void setPixelSide(int width);
  private:
    QRCode qrcode;
    LGFX_Sprite* sprite;

    int offsetX = 0;
    int offsetY = 0;
    int pixelSide = 5;
};

#endif
