#include "BulkUSB.h"

#define BULK_ENDPOINT_OUT pluggedEndpoint
#define BULK_ENDPOINT_IN  (uint8_t)(pluggedEndpoint + 1)

BulkUSB_ BulkUSB;

BulkUSB_::BulkUSB_(void)
  : PluggableUSBModule(2, 1, epType)   // 2 endpoints, 1 interface
{
  epType[0] = EP_TYPE_BULK_OUT;        // OUT
  epType[1] = EP_TYPE_BULK_IN;         // IN
  PluggableUSB().plug(this);
}

int BulkUSB_::getInterface(uint8_t* interfaceCount) {
  *interfaceCount += 1;

  BulkInterfaceDescriptor desc = {
    // bInterfaceNumber, bNumEndpoints, bInterfaceClass, bInterfaceSubClass, bInterfaceProtocol
    D_INTERFACE(pluggedInterface, 2, 0xFF, 0x00, 0x00),
    D_ENDPOINT(USB_ENDPOINT_OUT(BULK_ENDPOINT_OUT), USB_ENDPOINT_TYPE_BULK, USB_EP_SIZE, 0),
    D_ENDPOINT(USB_ENDPOINT_IN(BULK_ENDPOINT_IN),  USB_ENDPOINT_TYPE_BULK, USB_EP_SIZE, 0),
  };

  return USB_SendControl(0, &desc, sizeof(desc));
}

int BulkUSB_::getDescriptor(USBSetup& setup) {
  return 0;
}

bool BulkUSB_::setup(USBSetup& setup) {
  return false;
}

uint8_t BulkUSB_::getShortName(char* name) {
  name[0] = 'B';
  name[1] = 'U';
  name[2] = 'L';
  name[3] = 'K';
  return 4;
}

int BulkUSB_::write(const void* buffer, uint16_t len) {
  int send = USB_Send(BULK_ENDPOINT_IN, buffer, len);
  USB_Flush(BULK_ENDPOINT_IN);
  return send;
}

int BulkUSB_::read(void* buffer, uint16_t len) {
  if (USB_Available(BULK_ENDPOINT_OUT) >= 1) {
    return USB_Recv(BULK_ENDPOINT_OUT, buffer, len);
  }
  return 0;
}
