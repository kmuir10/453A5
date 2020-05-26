#include "mintool.h"

int32_t liend32(int32_t i){
  return (((i&0xFF)<<24)|((i&0xFF00)<<8)|((i&0xFF0000)>>8)|((i&0xFF000000)>>24));
}

int16_t liend16(int16_t i){
  return (((i&0xFF)<<8)|((i&0xFF00)>>8));
}
