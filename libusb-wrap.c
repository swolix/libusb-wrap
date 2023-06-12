#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <libusb-1.0/libusb.h>

bool keep(libusb_device *device) {
    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(device, &desc);
    uint8_t port = libusb_get_port_number(device); 
    uint8_t bus = libusb_get_bus_number(device);
    uint8_t address = libusb_get_device_address(device);
    // if (desc->idVendor == 0x0c45 && desc->idProduct == 0x670c) {
    //     fprintf(stderr, "REMOVING DEVICE: %04x:%04x\n", desc->idVendor, desc->idProduct);
    //     return false;
    // }
    fprintf(stderr, "DEVICE: idVendor=%04x, idProduct=%04x SerialNumber=%d, Port=%d, Bus=%d, Address=%d\n", 
            desc.idVendor, desc.idProduct, desc.iSerialNumber, port, bus, address);
    return true;
}

ssize_t libusb_get_device_list(libusb_context *ctx,	libusb_device ***list) {
    void *libusb = dlopen("/usr/lib/libusb-1.0.so", RTLD_NOW | RTLD_LOCAL);
    if (!libusb) {
        fprintf(stderr, "[libusb-wrap] Could not open libusb\n");
        return 0;
    }
    ssize_t (*orig)(libusb_context *ctx,	libusb_device ***list) = dlsym(libusb, "libusb_get_device_list");
    if (!orig) {
        fprintf(stderr, "[libusb-wrap] Could not find `libusb_get_device_list`\n");
        dlclose(libusb);
        return 0;
    }

    ssize_t count = orig(ctx, list);

    dlclose(libusb);

    int w = 0;
    for (int r = 0; r < count; r++) {
        if (keep((*list)[r])) (*list)[w++] = (*list)[r];
    }

    return w;
}
