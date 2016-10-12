#
# Makefile for Linux arch/m68k/coldfire/m5445x source directory
#

obj-$(CONFIG_M5445X)	+= devices.o config.o

ifneq ($(strip $(CONFIG_USB) $(CONFIG_USB_GADGET_FSL_USB2)),)
	obj-y	+= usb.o
endif
ifneq ($(strip $(CONFIG_USB_GADGET_FSL_USB2) $(CONFIG_USB_EHCI_HCD)),)
	obj-y   += usb_dr.o
endif
ifneq ($(strip $(CONFIG_USB) $(CONFIG_USB_GADGET_FSL_USB2)),)
	obj-y   += xcvr.o
endif

obj-$(CONFIG_PCI)	+= pci.o mcf5445x-pci.o
ifneq ($(CONFIG_FEC),)
obj-y += fec.o
endif
