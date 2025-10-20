#include "./QRCodeManager.h"

void QRCodeManager::init(LGFX_Sprite* sprite) {
  this->sprite = sprite;
}

void QRCodeManager::create(const char* content) {
  uint8_t qrcodeData[qrcode_getBufferSize(6)];
  qrcode_initText(&qrcode, qrcodeData, 6, 0, content);

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        sprite->fillRect(offsetX + x * pixelSide, offsetY + y * pixelSide, pixelSide, pixelSide, TFT_BLACK);
      } else {
        sprite->fillRect(offsetX + x * pixelSide, offsetY + y * pixelSide, pixelSide, pixelSide, TFT_WHITE);
      }
    }
  }
}

void QRCodeManager::setOffset(int x, int y) {
  offsetX = x;
  offsetY = y;
};
void QRCodeManager::setPixelSide(int width) {
  pixelSide = width;
};