set top   0 ;#top level jtag controller
set zen   1 ;#zen
set ndedi 2 ;#eTPU
set etpu1 3 ;#engine 1
set etpu2 4 ;#engine 2
set npc   5 ;#npc
set nxdm  6 ;#nxdm

set tck 8000

;# initialize copperhead jtag chain, reset and confirm
proc init_cuhd {} {
    variable top
    variable zen
    variable ndedi
    variable etpu1
    variable etpu2
    variable npc
    variable nxdm
    variable tck

    puts "Configure JTAG chain"
    ccs::mpc5500_config_chain

    puts "Set max TCK rate"
    ccs::set_max_tck $top $tck
    ccs::set_max_tck $zen $tck
    ccs::set_max_tck $ndedi $tck
    ccs::set_max_tck $npc $tck
    ccs::set_max_tck $nxdm $tck

    puts "Reset cores to debug"
    ccs::reset_to_debug
    display ccs::core_run_mode $zen
;#    display ccs::core_run_mode $ndedi
    display ccs::core_run_mode $etpu1
    display ccs::core_run_mode $etpu2
;#    display ccs::core_run_mode $npc
;#    display ccs::core_run_mode $nxdm
    puts [format "Copperhead idcode is       0x%08X" [ccs::read_reg $top $mpc5500_sync::reg(idcode) 1]]
    puts [format "Copperhead Zen idcode is   0x%08X" [ccs::read_reg $zen $mpc5500_zen::reg(idcode) 1]]
    puts [format "Copperhead NPC idcode is   0x%08X" [ccs::read_reg $npc $mpc5500_npc::reg(idcode) 1]]
    puts [format "Copperhead NXDM idcode is  0x%08X" [ccs::read_reg $nxdm $mpc5500_nxdm::reg(idcode) 1]]
}

proc zen_mmu {} {
    variable zen

    puts "Initializing Zen MMU"
;# Periph B
    ccs::write_reg $zen $mpc5500_zen::reg(mas0) {0x10000000 0xc0000500 0xfff0000a 0xfff0003f}
    ccs::config_template $zen 2 1
;# l2 cache as sram at 0x40000000
    ccs::write_reg $zen $mpc5500_zen::reg(mas0) {0x10030000 0xc0000400 0x40000008 0x4000003f}
    ccs::config_template $zen 2 1
;# Periph A
    ccs::write_reg $zen $mpc5500_zen::reg(mas0) {0x10040000 0xc0000500 0xc3f00008 0xc3f0003f}
    ccs::config_template $zen 2 1
;# internal flash
    ccs::write_reg $zen $mpc5500_zen::reg(mas0) {0x10010000 0xc0000700 0x00000000 0x0000003f}
    ccs::config_template $zen 2 1
;# External Memory
    ccs::write_reg $zen $mpc5500_zen::reg(mas0) {0x10020000 0xc0000700 0x20000000 0x2000003f}
    ccs::config_template $zen 2 1

;# Tell the zen that the MMU is setup (for ETPU)
    ccs::config_template $zen 4 1
}

init_cuhd
zen_mmu

;# code is:
;# lis r20,0
;# addi r20,r20,1
;# nop
;# nop
;# b -12
puts "initializing ECC for RAM at 0x40000000 by writing 1024 bytes as 64 bit"
ccs::write_mem $zen 0x40000000 8 0 {0x3e800000 0x3a940001 0x60000000 0x60000000 0x4bfffff4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63}
ccs::write_mem $zen 0x40000100 8 0 {0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63}
ccs::write_mem $zen 0x40000200 8 0 {0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63}
ccs::write_mem $zen 0x40000300 8 0 {0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63}

puts "Setting PC to 0x40000000"
ccs::write_reg $zen pc 0x40000000

ccs::run_core $etpu1
ccs::run_core $etpu2

;# code (at location 0x200) is:
;# alu a = #0x0.
;# alu a=a + 1,ccs.
;# nop.
;# nop.
;# if branch_never == 0 jump 0x204, no_flush
;# code is written to SCM address 4 because 0 is used to set up services
puts "writing ETPU SCM"
ccs::write_mem $etpu1 0x200 4 2 {0x00000408 0x7df90ffb 0x4fffffff 0x4fffffff 0xf7c0103f}
ccs::fill_mem $etpu1 0x2800 4 2 32 0x40804080

puts "setting up ETPU service"
ccs::write_reg $etpu1 ecr 0x5
ccs::write_reg $etpu1 c1cr 0x20000000
ccs::write_reg $etpu1 c1hsrr 0x00000003
ccs::write_reg $etpu1 cier 0x2
ccs::write_reg $etpu2 ecr 0x5
ccs::write_reg $etpu2 c1cr 0x20000000
ccs::write_reg $etpu2 c1hsrr 0x00000003
ccs::write_reg $etpu2 cier 0x2
