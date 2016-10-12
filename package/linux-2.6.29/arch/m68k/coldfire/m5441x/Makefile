#
# Makefile for Linux arch/m68k/coldfire/m5441x source directory
#

obj-$(CONFIG_M5441X)	+= devices.o config.o

ifneq ($(strip $(CONFIG_USB) $(CONFIG_USB_GADGET_FSL_USB2)),)
	obj-y	+= usb.o
endif
ifneq ($(strip $(CONFIG_USB_GADGET_FSL_USB2) $(CONFIG_USB_EHCI_HCD)),)
	obj-y   += usb_dr.o
endif
ifneq ($(strip $(CONFIG_USB) $(CONFIG_USB_GADGET_FSL_USB2)),)
	obj-y   += xcvr.o
endif

ifneq ($(CONFIG_FEC),)
obj-y += fec.o
endif

ifneq ($(CONFIG_MODELO_SWITCH),)
obj-y += l2switch.o
endif
