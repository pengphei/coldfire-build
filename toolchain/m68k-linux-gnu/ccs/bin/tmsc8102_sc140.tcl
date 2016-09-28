#Create all the registers
addreg d0
addreg d0_e
addreg d1
addreg d1_e
addreg d2
addreg d2_e
addreg d3
addreg d3_e
addreg d4
addreg d4_e
addreg d5
addreg d5_e
addreg d6
addreg d6_e
addreg d7
addreg d7_e
addreg d8
addreg d8_e
addreg d9
addreg d9_e
addreg d10
addreg d10_e
addreg d11
addreg d11_e
addreg d12
addreg d12_e
addreg d13
addreg d13_e
addreg d14
addreg d14_e
addreg d15
addreg d15_e
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
addreg n0
addreg n1
addreg n2
addreg n3
addreg m0
addreg m1
addreg m2
addreg m3
addreg sa0
addreg sa1
addreg sa2
addreg sa3
addreg lc0
addreg lc1
addreg lc2
addreg lc3
addreg sr
addreg sp
addreg osp
addreg mctl
addreg emr
addreg vba
addreg pctl0
addreg pctl1
addreg pc
addreg idcode
addreg esr
addreg emcr
addreg ercv_lo
addreg ercv_hi
addreg etrsmt_lo
addreg etrsmt_hi
addreg ee_ctrl
addreg pc_excp
addreg pc_next
addreg pc_last
addreg pc_detect
addreg edca0_ctrl
addreg edca1_ctrl
addreg edca2_ctrl
addreg edca3_ctrl
addreg edca4_ctrl
addreg edca5_ctrl
addreg edca0_refa
addreg edca1_refa
addreg edca2_refa
addreg edca3_refa
addreg edca4_refa
addreg edca5_refa
addreg edca0_refb
addreg edca1_refb
addreg edca2_refb
addreg edca3_refb
addreg edca4_refb
addreg edca5_refb
addreg edca0_mask
addreg edca1_mask
addreg edca2_mask
addreg edca3_mask
addreg edca4_mask
addreg edca5_mask
addreg edcd_ctrl
addreg edcd_ref
addreg edcd_mask
addreg ecnt_ctrl
addreg ecnt_val
addreg ecnt_ext
addreg esel_ctrl
addreg esel_dm
addreg esel_di
addreg esel_etb
addreg esel_dtb
addreg tb_ctrl
addreg tb_rd
addreg tb_wr
addreg tb_buff

#Create configuration options
addcfg deprecated_0
addcfg deprecated_1
addcfg fast_mem
addcfg endian
addcfg slow_memory
addcfg msc8101_sypcr
addcfg disable_polling
addcfg eonce_base
addcfg use_esr_status
addcfg reset_delay_ms
addcfg no_pctl
addcfg debug_rom
addcfg accelerate_hsst

set Stat(inst) 2
set Stat(icnt) 6
set Stat(vname) sc
set Stat(core) "StarCore"
set Stat(reg)  {d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 d10 d11 d12 d13 d14 d15 osp r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12 r13 r14 r15 m0 m1 m2 m3 n0 n1 n2 n3 mctl sr esr sp sa0 sa1 sa2 sa3 lc0 lc1 lc2 lc3 vba pctl0 pctl1 emcr d0_e d1_e d2_e d3_e d4_e d5_e d6_e d7_e d8_e d9_e d10_e d11_e d12_e d13_e d14_e d15_e idcode pc}
set Stat(size) {8  8  8  8  8  8  8  8  8  8  8   8   8   8   8   8   8   8  8  8  8  8  8  8  8  8  8  8   8   8   8   8   8   8  8  8  8  8  8  8  8  8    8  8   8  8   8   8   8   8   8   8   8   8   8     8     8    3    3    3    3    3    3    3    3    3    3    3     3     3     3     3     3     8      8} 
set    Stat(fmt) { d0 $fmt(d0)\t  d1 $fmt(d1)\t  d2 $fmt(d2)\t  d3 $fmt(d3)\n}
append Stat(fmt) { d4 $fmt(d4)\t  d5 $fmt(d5)\t  d6 $fmt(d6)\t  d7 $fmt(d7)\n}
append Stat(fmt) { d8 $fmt(d8)\t  d9 $fmt(d9)\t d10 $fmt(d10)\t d11 $fmt(d11)\n}
append Stat(fmt) {d12 $fmt(d12)\t d13 $fmt(d13)\t d14 $fmt(d14)\t d15 $fmt(d15)\n}
append Stat(fmt) { r0 $fmt(r0)\t  m0 $fmt(m0)\t  n0 $fmt(n0)\tmctl $fmt(mctl)\n}
append Stat(fmt) { r1 $fmt(r1)\t  m1 $fmt(m1)\t  n1 $fmt(n1)\t  sr $fmt(sr)\n}
append Stat(fmt) { r2 $fmt(r2)\t  m2 $fmt(m2)\t  n2 $fmt(n2)\t esr $fmt(esr)\n}
append Stat(fmt) { r3 $fmt(r3)\t  m3 $fmt(m3)\t  n3 $fmt(n3)\t  sp $fmt(sp)\n}
append Stat(fmt) { r4 $fmt(r4)\t sa0 $fmt(sa0)\t lc0 $fmt(lc0)\t osp $fmt(osp)\n}
append Stat(fmt) { r5 $fmt(r5)\t sa1 $fmt(sa1)\t lc1 $fmt(lc1)\t vba $fmt(vba)\n}
append Stat(fmt) { r6 $fmt(r6)\t sa2 $fmt(sa2)\t lc2 $fmt(lc2)\tpctl0$fmt(pctl0)\n}
append Stat(fmt) { r7 $fmt(r7)\t sa3 $fmt(sa3)\t lc3 $fmt(lc3)\tpctl1$fmt(pctl1)\n}
append Stat(fmt) { r8 $fmt(r8)\td0_e $fmt(d0_e)\td1_e $fmt(d1_e)\temcr $fmt(emcr)\n}
append Stat(fmt) { r9 $fmt(r9)\td2_e $fmt(d2_e)\td3_e $fmt(d3_e)\n}
append Stat(fmt) {r10 $fmt(r10)\td4_e $fmt(d4_e)\td5_e $fmt(d5_e)\n}
append Stat(fmt) {r11 $fmt(r11)\td6_e $fmt(d6_e)\td7_e $fmt(d7_e)\n}
append Stat(fmt) {r12 $fmt(r12)\td8_e $fmt(d8_e)\td9_e $fmt(d9_e)\n}
append Stat(fmt) {r13 $fmt(r13)\td10e $fmt(d10_e)\td11e $fmt(d11_e)\n}
append Stat(fmt) {r14 $fmt(r14)\td12e $fmt(d12_e)\td13e $fmt(d13_e)\n}
append Stat(fmt) {r15 $fmt(r15)\td14e $fmt(d14_e)\td15e $fmt(d15_e)\tidcode $fmt(idcode)\n}
append Stat(fmt) { pc $fmt(pc)}

proc reset_8101 {} {
  if {[info exists ::sc]} {
    set chain_pos $::sc
  } else {
    set chain_pos 0
  }
  ccs::reset_to_debug
  ccs::write_mem $chain_pos 0xf0010006 2 0 0xfbc3
}

proc setup_8101 {} {
  if {[info exists ::sc]} {
    set chain_pos $::sc
  } else {
    set chain_pos 0
  }
  puts "reset"
  ccs::reset_to_debug
  puts "sypcr"
  ccs::write_mem $chain_pos 0xf0010006 2 0 0xfbc3
  puts "immr"
  ccs::write_mem $chain_pos 0xf00101a8 2 0 0x1470
  puts "bcr"
  ccs::write_mem $chain_pos 0x14710108 4 0 0x00900000 ;#BCR
  puts "or0"
  ccs::write_mem $chain_pos 0x14710104 4 0 0xff800866 ;#OR0
  puts "br0"
  ccs::write_mem $chain_pos 0x14710100 4 0 0xff801801 ;#BR0
  puts "or1"
  ccs::write_mem $chain_pos 0x1471010C 4 0 0xffff8010 ;#OR1
  puts "br1"
  ccs::write_mem $chain_pos 0x14710024 4 0 0x14501801 ;#BR1
}

proc load {chain_pos filename {memspace 0}} {
    if {$memspace == 0} {
        cload $chain_pos $filename
    } else {
        cload $chain_pos $filename $memspace
    }
    
}
proc cload {chain_pos filename {memspace 0}} {
    set prog_addr [list]
    set prog_data [list]

    set fp [open $filename r]
    set tmpdat [list]
    set origin_found 0
    set n 0
    while {![eof $fp]} {
        set line [gets $fp]
        if       {[regexp {^                     ([0-9A-F][0-9A-F])$} $line match dat]} {
            set byte($n) $dat
            if {$n > 0} {
                lappend tmpdat "0x$byte(1)$byte(0)"
                set n 0
            } else {
                incr n
            }
        } elseif {[regexp {^..........P:........ ([0-9A-F][0-9A-F]) .*$} $line match dat]} {
            set byte($n) $dat
            incr n
        } elseif {[regexp {^..........P:(........) * org .*$} $line match addr]} {
            if {$origin_found} {
                lappend prog_data $tmpdat
            }
            set origin_found 1
            lappend prog_addr "0x${addr}"

            set tmpdat [list]
        } else {
        }
    }
    if {$origin_found} {
        lappend prog_data $tmpdat
    }
    close $fp

    foreach addr $prog_addr data $prog_data {
        puts -nonewline "write => $addr..."
        ccs::write_mem $chain_pos $addr 2 $memspace $data
        puts ""
    }
    ccs::uselst $chain_pos $filename
}
