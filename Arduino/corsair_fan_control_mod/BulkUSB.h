#pragma once
#include <Arduino.h>
#include "PluggableUSB.h"

#if !defined(USBCON)
#error "Only for native USB MCUs (ATmega32U4, etc.)"
#endif

typedef struct {
  InterfaceDescriptor iface;
  EndpointDescriptor  epOut;
  EndpointDescriptor  epIn;
} BulkInterfaceDescriptor;

class BulkUSB_ : public PluggableUSBModule {
public:
  BulkUSB_(void);

  int  getInterface(uint8_t* interfaceCount) override;
  int  getDescriptor(USBSetup& setup) override;
  bool setup(USBSetup& setup) override;
  uint8_t getShortName(char* name) override;

  int  write(const void* buffer, uint16_t len);
  int  read(void* buffer, uint16_t len);

private:
  uint8_t epType[2];
};

extern BulkUSB_ BulkUSB;
