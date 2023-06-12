/*
    Copyright (c) 2023, Sijmen Woutersen

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

        1. Redistributions of source code must retain the above copyright
           notice, this list of conditions and the following disclaimer.
        2. Redistributions in binary form must reproduce the above copyright
           notice, this list of conditions and the following disclaimer in the
           documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <libusb-1.0/libusb.h>

static bool keep(libusb_device *device) {
    bool ret = true;
    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(device, &desc);
    uint8_t port = libusb_get_port_number(device); 
    uint8_t bus = libusb_get_bus_number(device);
    uint8_t address = libusb_get_device_address(device);

    char *include_bus = getenv("LIBUSB_WRAP_INCLUDE_BUS");
    if (include_bus && bus != strtoul(include_bus, NULL, 0)) ret = false;

    char *include_address = getenv("LIBUSB_WRAP_INCLUDE_ADDRESS");
    if (include_address && address != strtoul(include_address, NULL, 0)) ret = false;

    char *include_port = getenv("LIBUSB_WRAP_INCLUDE_PORT");
    if (include_port && port != strtoul(include_port, NULL, 0)) ret = false;

    char *include_vendor_id = getenv("LIBUSB_WRAP_INCLUDE_VENDOR_ID");
    if (include_vendor_id && desc.idVendor != strtoul(include_vendor_id, NULL, 0)) ret = false;

    char *include_product_id = getenv("LIBUSB_WRAP_INCLUDE_PRODUCT_ID");
    if (include_product_id && desc.idProduct != strtoul(include_product_id, NULL, 0)) ret = false;

    char *include_serial = getenv("LIBUSB_WRAP_INCLUDE_SERIAL");
    if (include_serial && desc.iSerialNumber != strtoul(include_serial, NULL, 0)) ret = false;

    if (getenv("LIBUSB_WRAP_DEBUG")) {
        fprintf(stderr, "[libusb-wrap] idVendor=0x%04x, idProduct=0x%04x SerialNumber=%d, Port=%d, Bus=%d, Address=%d",
                desc.idVendor, desc.idProduct, desc.iSerialNumber, port, bus, address);
        if (ret) {
            fprintf(stderr, " => keep\n");
        } else {
            fprintf(stderr, " => drop\n");
        }
    }

    return ret;
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
