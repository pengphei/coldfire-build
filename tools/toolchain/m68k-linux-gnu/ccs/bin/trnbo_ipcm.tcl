addreg gr0
addreg gr1
addreg gr2
addreg gr3
addreg gr4
addreg gr5
addreg gr6
addreg gr7
addreg ma
addreg md
addreg ms
addreg da
addreg dd
addreg ds
addreg ca
addreg cs
addreg rpc
addreg lm
addreg epc
addreg spc
addreg osr
addreg pc
addreg t
addreg df
addreg sf
addreg e0count
addreg e0ctl
addreg e0aa
addreg e0ab
addreg e0am
addreg e0d
addreg e0dm
addreg e1count
addreg e1ctl
addreg e1aa
addreg e1ab
addreg e1am
addreg e1d
addreg e1dm
addreg rtb
addreg tb0
addreg tb1
addreg tb2
addreg tb3
addreg tb4
addreg tb5
addreg tb6
addreg tb7
addreg tb8
addreg tb9
addreg tb10
addreg tb11
addreg tb12
addreg tb13
addreg tb14
addreg tb15

#Rainbow DEV/3G Only
addreg bcsr
addreg misr0
addreg misr1
addreg misr2
addreg misr3

set Stat(inst) 2
set Stat(icnt) 6
set Stat(vname) ip
set Stat(core) "IPCM"
set Stat(reg)  {gr0 gr1 gr2 gr3 gr4 gr5 gr6 gr7 osr t df sf rpc spc epc pc}
set Stat(size) {8   8   8   8   8   8   8   8   8   1 1  1  8   8   8   8}
set    Stat(fmt) {gr0 $fmt(gr0)\tgr1 $fmt(gr1)\tgr2 $fmt(gr2)\tgr3 $fmt(gr3)\t t $fmt(t)\n}
append Stat(fmt) {gr4 $fmt(gr4)\tgr5 $fmt(gr5)\tgr6 $fmt(gr6)\tgr7 $fmt(gr7)\tdf $fmt(df)\n}
append Stat(fmt) {osr $fmt(osr)\trpc $fmt(rpc)\tspc $fmt(spc)\tepc $fmt(epc)\tsf $fmt(sf)\n}
append Stat(fmt) { pc $fmt(pc)}

proc step {} {
  set chain_pos 3
  if {[::ccs::core_run_mode $chain_pos] != 0} {
    ::display ::ccs::core_run_mode $chain_pos
    return
  }
  ::ccs::step_core $chain_pos 1
  stat
}

proc stat {} {
  variable reg
  set chain_pos 3
  if {[::ccs::core_run_mode $chain_pos] != 0} {
    ::display ::ccs::core_run_mode $chain_pos
    return
  }
  puts -nonewline [format "GR0  0x%08X\t" [::ccs::read_reg $chain_pos $reg(gr0) 1]]
  puts -nonewline [format "GR1  0x%08X\t" [::ccs::read_reg $chain_pos $reg(gr1) 1]]
  puts -nonewline [format "GR2  0x%08X\t" [::ccs::read_reg $chain_pos $reg(gr2) 1]]
  puts -nonewline [format "GR3  0x%08X\n" [::ccs::read_reg $chain_pos $reg(gr3) 1]]
  puts -nonewline [format "GR4  0x%08X\t" [::ccs::read_reg $chain_pos $reg(gr4) 1]]
  puts -nonewline [format "GR5  0x%08X\t" [::ccs::read_reg $chain_pos $reg(gr5) 1]]
  puts -nonewline [format "GR6  0x%08X\t" [::ccs::read_reg $chain_pos $reg(gr6) 1]]
  puts -nonewline [format "GR7  0x%08X\n" [::ccs::read_reg $chain_pos $reg(gr7) 1]]
  puts -nonewline [format "OSR  0x%08X\t" [::ccs::read_reg $chain_pos $reg(osr) 1]]
  puts -nonewline [format "T    0x%08X\t" [::ccs::read_reg $chain_pos $reg(t) 1]]
  puts -nonewline [format "DF   0x%08X\t" [::ccs::read_reg $chain_pos $reg(df) 1]]
  puts -nonewline [format "SF   0x%08X\n" [::ccs::read_reg $chain_pos $reg(sf) 1]]
  puts -nonewline [format "RPC  0x%08X\t" [::ccs::read_reg $chain_pos $reg(rpc) 1]]
  puts -nonewline [format "SPC  0x%08X\t" [::ccs::read_reg $chain_pos $reg(spc) 1]]
  puts -nonewline [format "EPC  0x%08X\n" [::ccs::read_reg $chain_pos $reg(epc) 1]]
  set pc [::ccs::read_reg $chain_pos $reg(pc) 1]
  set mem [::ccs::read_mem $chain_pos $pc 2 0 1]
  puts [format "PC   0x%07X \[0x%04X\]" $pc $mem]
}
