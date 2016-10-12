
#Create configuration options
addcfg family
addcfg family_member
addcfg nexus_accesses
addcfg nexus_pin_count
addcfg nexus_capture
addcfg nexus_buffer_full_action
addcfg cpuscr_length_hack
addcfg endian
addcfg single_step_hack
addcfg slow_memory
addcfg nexus_buffer_action
addcfg nexus_start_read_frame
addcfg ocr_mask_hack
addcfg corelis_rom_address
addcfg corelis_rom_size

#Create registers
addreg r0
addreg r1
addreg r2
addreg r3
addreg r4
addreg r5
addreg r6
addreg r7
addreg r8
addreg r9
addreg r10
addreg r11
addreg r12
addreg r13
addreg r14
addreg r15
addreg ar0
addreg ar1
addreg ar2
addreg ar3
addreg ar4
addreg ar5
addreg ar6
addreg ar7
addreg ar8
addreg ar9
addreg ar10
addreg ar11
addreg ar12
addreg ar13
addreg ar14
addreg ar15
addreg cr0
addreg vbr
addreg cr2
addreg cr3
addreg cr4
addreg cr5
addreg cr6
addreg cr7
addreg cr8
addreg cr9
addreg cr10
addreg cr11
addreg cr12
addreg cr13
addreg cr14
addreg cr15
addreg wbbr
addreg psr
addreg pc
addreg ir
addreg ctl
addreg ocr
addreg osr
addreg otc
addreg mbca
addreg mbcb
addreg baba
addreg babb
addreg bama
addreg bamb
addreg fifo0
addreg fifo1
addreg fifo2
addreg fifo3
addreg fifo4
addreg fifo5
addreg fifo6
addreg fifo7
addreg nexus_csc
addreg nexus_dc
addreg nexus_ds
addreg nexus_uba
addreg nexus_rwcs
addreg nexus_rwa
addreg nexus_rwd
addreg nexus_dtc
addreg nexus_dtsa1
addreg nexus_dtsa2
addreg nexus_dtea1
addreg nexus_dtea2
addreg idcode

#Rainbow DEV/3G Only
addreg ftcr

set Stat(inst) 2
set Stat(icnt) 6
set Stat(vname) mc
set Stat(core) "MCore"
set Stat(reg)  {r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12 r13 r14 r15 psr osr ocr otc pc cr0 vbr cr2 cr3 cr4 cr5 cr6 cr7}
set Stat(size) {8  8  8  8  8  8  8  8  8  8  8   8   8   8   8   8   8   8   8   8   8  8   8   8   8    8  8   8   8}
set    Stat(fmt) { r0 $fmt(r0)\t  r1 $fmt(r1)\t  r2 $fmt(r2)\t  r3 $fmt(r3)\n}
append Stat(fmt) { r4 $fmt(r4)\t  r5 $fmt(r5)\t  r6 $fmt(r6)\t  r7 $fmt(r7)\n}
append Stat(fmt) { r8 $fmt(r8)\t  r9 $fmt(r9)\t r10 $fmt(r10)\t r11 $fmt(r11)\n}
append Stat(fmt) {r12 $fmt(r12)\t r13 $fmt(r13)\t r14 $fmt(r14)\t r15 $fmt(r15)\n}
append Stat(fmt) {psr $fmt(psr)\t osr $fmt(osr)\t otc $fmt(otc)\t ocr $fmt(ocr)\n}
append Stat(fmt) {cr0 $fmt(cr0)\t vbr $fmt(vbr)\t cr2 $fmt(cr2)\t cr3 $fmt(cr3)\n}
append Stat(fmt) {cr4 $fmt(cr4)\t cr5 $fmt(cr5)\t cr6 $fmt(cr6)\t cr7 $fmt(cr7)\n}
append Stat(fmt) {pc $fmt(pc)}

proc gofast {dev} {
ccs::write_mem $dev 0x0020c000 2 0 0x0002
ccs::set_max_tck $dev 1000
}

proc runstat {} {
    if {![ccs::core_run_mode 2]} {
        puts "ccs::run_core"
        ::ccs::run_core 2
        if {![ccs::core_run_mode 2]} {
            display ccs::read_reg 2 pc 1
        } else {
            display ccs::core_run_mode 2
        }
    } else {
        puts "core already running"
    }

}

proc stat {} {
  variable reg
  if {[info exists ::mc]} {
    set chain_pos $::mc
  } else {
    set chain_pos 0
  }
  if {[::ccs::core_run_mode $chain_pos] != 0} {
    puts "Not in debug mode"
    return
  }
  set pc [::ccs::read_reg $chain_pos $reg(pc) 1]
  puts -nonewline [format "R0  0x%08X\t" [::ccs::read_reg $chain_pos $reg(r0) 1]]
  puts -nonewline [format "R1  0x%08X\t" [::ccs::read_reg $chain_pos $reg(r1) 1]]
  puts -nonewline [format "R2  0x%08X\t" [::ccs::read_reg $chain_pos $reg(r2) 1]]
  puts -nonewline [format "R3  0x%08X\n" [::ccs::read_reg $chain_pos $reg(r3) 1]]
  puts -nonewline [format "R4  0x%08X\t" [::ccs::read_reg $chain_pos $reg(r4) 1]]
  puts -nonewline [format "R5  0x%08X\t" [::ccs::read_reg $chain_pos $reg(r5) 1]]
  puts -nonewline [format "R6  0x%08X\t" [::ccs::read_reg $chain_pos $reg(r6) 1]]
  puts -nonewline [format "R7  0x%08X\n" [::ccs::read_reg $chain_pos $reg(r7) 1]]
  puts -nonewline [format "R8  0x%08X\t" [::ccs::read_reg $chain_pos $reg(r8) 1]]
  puts -nonewline [format "R9  0x%08X\t" [::ccs::read_reg $chain_pos $reg(r9) 1]]
  puts -nonewline [format "R10 0x%08X\t" [::ccs::read_reg $chain_pos $reg(r10) 1]]
  puts -nonewline [format "R11 0x%08X\n" [::ccs::read_reg $chain_pos $reg(r11) 1]]
  puts -nonewline [format "R12 0x%08X\t" [::ccs::read_reg $chain_pos $reg(r12) 1]]
  puts -nonewline [format "R13 0x%08X\t" [::ccs::read_reg $chain_pos $reg(r13) 1]]
  puts -nonewline [format "R14 0x%08X\t" [::ccs::read_reg $chain_pos $reg(r14) 1]]
  puts -nonewline [format "R15 0x%08X\n" [::ccs::read_reg $chain_pos $reg(r15) 1]]
  puts -nonewline [format "PSR 0x%08X\t" [::ccs::read_reg $chain_pos $reg(psr) 1]]
  puts -nonewline [format "OSR 0x%08X\t" [::ccs::read_reg $chain_pos $reg(osr) 1]]
  puts -nonewline [format "OCR 0x%08X\t" [::ccs::read_reg $chain_pos $reg(ocr) 1]]
  puts -nonewline [format "OTC 0x%08X\n" [::ccs::read_reg $chain_pos $reg(otc) 1]]
  puts [format "PC  0x%08X \[0x%04X\]" [::ccs::read_reg $chain_pos $reg(pc) 1] [::ccs::read_mem $chain_pos $pc 2 0 1]]
}

proc step {} {
  if {[info exists ::mc]} {
    set chain_pos $::mc
  } else {
    set chain_pos 0
  }

  if {[::ccs::core_run_mode $chain_pos] != 0} {
    puts "Not in debug mode"
    return
  }
  ::ccs::step_core $chain_pos 1
  stat

}
