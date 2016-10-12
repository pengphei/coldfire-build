#
# Makefile for the linux kernel.
#

ifdef CONFIG_SUN3
  extra-y := sun3-head.o vmlinux.lds
  obj-y := entry.o signal.o ints.o time.o
else
ifndef CONFIG_COLDFIRE
  extra-y := head.o vmlinux.lds
  obj-y := entry.o signal.o traps.o ints.o time.o
else   # CONFIG_COLDFIRE
  extra-y := vmlinux.lds
  ifdef CONFIG_M547X_8X
  obj-$(CONFIG_PCI) += bios32_mcf548x.o
  endif
endif
endif

obj-y	+= process.o ptrace.o module.o \
	   sys_m68k.o setup.o m68k_ksyms.o devres.o# semaphore.o

devres-y = ../../../kernel/irq/devres.o

obj-y$(CONFIG_MMU_SUN3) += dma.o	# no, it's not a typo

EXTRA_AFLAGS := -traditional
