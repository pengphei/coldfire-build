#
# Makefile for arch/m68k/coldfire/vdso with special rules
# for building the DSO lib
#
# Based on arch/sh/kernel/vsyscall/Makefile
#
# Kurt Mahan <kmahan@freescale.com>
#

obj-y := vdso.o vdso-bin.o

$(obj)/vdso-bin.o: $(obj)/vdso.lds $(obj)/vdso-lib.so

#
# The DSO Lib
#

# special linker script for building DSO images
quiet_cmd_vdso = VDSO $@
      cmd_vdso = $(LD) -nostdlib --eh-frame-hdr $(SYSCFLAGS_$(@F)) \
			-T $(obj)/vdso.lds $^ -o $@

vdso-flags = -shared -s -soname=linux-gate.so.1

SYSCFLAGS_vdso-lib.so = $(vdso-flags)

$(obj)/vdso-lib.so: $(obj)/vdso-lib.o $(obj)/vdso-note.o
	$(call if_changed,vdso)

$(obj)/vdso.lds: $(srctree)/arch/m68k/coldfire/vdso/vdso.lds.S
	cp $< $@

#
# Create a special relocatable object that should mirror the
# symbol table and layout of the linked DSO lib.  With ld -R
# these symbols can be refered to in the kernel code rather
# than as hand-coded addresses
#
# extra-y += vdso-syms.o
# $(obj)/built-in.o: $(obj)/vdso-syms.o
# $(obj)/built-in.o: ld_flags += -R (obj)/vdso-syms.o

# SYSCFLAGS_vdso-syms.o = -r
# $(obj)/vdso-syms.o: $(src)/vdso.lds \
# 	    $(obj)/vdso-lib.o $(obj)/vdso-note.o FORCE
# 	$(call if_changed,vdso)
