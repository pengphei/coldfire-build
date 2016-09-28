set ed 0 ;#extra debug / tap controller
set sc 1 ;#starcore
set mc 2 ;#mcore
set ip 3 ;#ipcm
set word 4
set half 2
set byte 1

proc pci {} {
    ccs::rnbo_config_chain
    ccs::set_max_tck 0 8
    display ccs::read_reg 0 idcode 1
}

#setup simple data for memory testing
set fdat [list]
for {set i 0} {$i < 64} {incr i} {
    lappend fdat 0xffffffff
}
set ndat [list]
for {set i 0} {$i < 64} {incr i} {
    lappend ndat [format "0x%08X" [expr (0x11111111 * ($i%16))]]
}
set ipcmbkpts [list]
for {set i 0} {$i < 64} {incr i} {
    lappend ipcmbkpts 0x00050005
}
set ipcmgotosleep [list 0x08100a00 0x090e0111 0x78031aff 0x5a011901 0x00880400]
set ipcmgotosleep2 [list 0x08100a00 0x090e0111 0x00880005 0x78031aff 0x5a011901 0x00880400]

set tck 2000

;# initialize rainbow jtag chain, reset and confirm
proc init_rnbo {} {
    variable ed
    variable sc
    variable mc
    variable ip
    variable tck

    puts "Configure JTAG chain"
    ccs::rnbo_config_chain

    puts "Set max TCK rate"
    ccs::set_max_tck $ed $tck
    ccs::set_max_tck $sc $tck
    ccs::set_max_tck $mc $tck
    ccs::set_max_tck $ip $tck

    puts "Reset cores to debug"
    ccs::reset_to_debug
    display ccs::core_run_mode $sc
    display ccs::core_run_mode $mc
    display ccs::core_run_mode $ip
    puts [format "Rainbow idcode is 0x%08X" [ccs::read_reg $ed idcode 1]]
}

proc quiet_init_rnbo {} {
    variable ed
    variable sc
    variable mc
    variable ip

    ccs::rnbo_config_chain

    variable tck
    ccs::set_max_tck $ed $tck
    ccs::set_max_tck $sc $tck
    ccs::set_max_tck $mc $tck
    ccs::set_max_tck $ip $tck

    ccs::reset_to_debug
    ccs::core_run_mode $sc
    ccs::core_run_mode $mc
    ccs::core_run_mode $ip
    ccs::read_reg $ed idcode 1
}
#setup MCore EIM registers for memory @ 0x14000000-0x143fffff
proc setup_eim {} {
    variable mc
    variable word

    ccs::write_mem $mc 0x220018 $word 0 0x00010402
    ccs::write_mem $mc 0x22001c $word 0 0x40000701
}

proc superchunk {} {
    variable mc
    variable ip
    variable sc
    init_rnbo
    puts "Write MCore memory 0xCAFE000X"
    ccs::write_mem $mc 0x100000 4 0 { 0xcafe0001 0xcafe0002 0xcafe0003 0xcafe0004 0xcafe0005 0xcafe0006 0xcafe0007 0xcafe0008}
    puts "Write IPCM memory 0xFACE000X"
    ccs::write_mem $ip 0x800 4 0 { 0xface0001 0xface0002 0xface0003 0xface0004 0xface0005 0xface0006 0xface0007 0xface0008}
    puts "Write StarCore memory 0xBEEF000X"
    ccs::write_mem $sc 0x00 4 0 { 0xbeef0001 0xbeef0002 0xbeef0003 0xbeef0004 0xbeef0005 0xbeef0006 0xbeef0007 0xbeef0008}
    puts "Read MCore memory 0xCAFE000X"
    display ccs::read_mem $mc 0x100000 4 0 8
    puts "Read IPCM memory 0xFACE000X"
    display ccs::read_mem $ip 0x800 4 0 8
    puts "Read StarCore memory 0xBEEF000X"
    display ccs::read_mem $sc 0x00 4 0 8
}

#setup MCore EDRAM refresh
set mc_edram_base 0x7800000
proc setup_edram {} {
    variable mc
    variable half

    ccs::write_mem $mc 0x207000 $half 0 0xc
}

proc runmodes {} {
    for {set i 0} {$i<[llength [ccs::get_config_chain]]} {incr i} {
        display ccs::core_run_mode $i
    }
}

proc rominit {address size} {
    ccs::config_template $::mc $::rnbo_mcore::cfg(corelis_rom_address) $address
    ccs::config_template $::mc $::rnbo_mcore::cfg(corelis_rom_size) $size
}

proc wakeup {{ch0addr 0x1500}} {
    variable mc
    ccs::write_mem $mc 0x21605C 4 1 $ch0addr
    ccs::write_mem $mc 0x216100 4 1 0x00000007
    ccs::write_mem $mc 0x216010 4 1 0x00000001
    ccs::write_mem $mc 0x216014 4 1 0xFFFFFFFF
    ccs::write_mem $mc 0x216018 4 1 0x00000080
    ccs::write_mem $mc 0x21600C 4 1 0x00000001
}

set sc_sram_base 0x00000000
set mc_sram_base 0x00100000
set sc_swbkpt 0x9E70
set mc_swbkpt 0x0000

#init_rnbo

#source scregchk.tcl
#source rtdx.tcl
#source ipcmmem.tcl

proc ninit {{pins 8}} {
    variable mc
    if {$pins != 8} {
        set pins 2
    }
    puts "Setup Nexus for $pins pins"

    ccs::write_reg $mc nexus_dtc   0xF0000000 ;# NEXUS DTC
    ccs::write_reg $mc nexus_dtsa1 0x0797FFFC ;# NEXUS DTSA1
    ccs::write_reg $mc nexus_dtea1 0x07980804 ;# NEXUS DTEA1
    ccs::write_reg $mc nexus_dtsa2 0x000FFFFC ;# NEXUS DTSA2
    ccs::write_reg $mc nexus_dtea2 0x00100804 ;# NEXUS DTEA2

    if {$pins == 8} {
        ccs::write_reg $mc nexus_dc 0x80000002 ;# NEXUS DC for 8 PIN
    } else {
        ccs::write_reg $mc nexus_dc 0x00000002 ;# NEXUS DC for 2 PIN
    }
    return
}

proc nget {} {
    variable mc
    variable nexusDataCount
    variable nTcod
    variable nAddr
    variable nData
    ccs::config_template $mc 10 2
    set rcnt [lindex [ccs::rtd_stat $mc] 0]
    if {$rcnt == 0} {
        puts "No Nexus data collected"
        return
    }
    if {$rcnt > 16383} {
        set rcnt 16383
    }
    puts "Reading $rcnt words of Nexus data"
    set rawNexusData [ccs::rtd_read $mc $rcnt]
    set i 0
    foreach {t a d} $rawNexusData {
        set nTcod($i) [format "0x%08X" $t]
        set nAddr($i) [format "0x%08X" $a]
        set nData($i) [format "0x%08X" $d]
        incr i
    }
    set nexusDataCount $i
    puts "$nexusDataCount packets read"
    nclear
    return
}

proc nshow {{start 0} {count -1} {file 0}} {
    variable nexusDataCount
    variable nTcod
    variable nAddr
    variable nData

    if {$count < 1} {
        set count $nexusDataCount
    }
    if {($count + $start) > $nexusDataCount} {
        set count [expr ($nexusDataCount - $start)]
    }

    set fp 0
    if {$file != 0} {
        set fp [open $file "w"]
    }

    set end [expr $start + $count]
    for {set i $start} {$i < $end} {incr i} {
        set output [format "p:%04d * t:%08X a:%08X d:%08X" $i $nTcod($i) $nAddr($i) $nData($i)]
        if {$fp != 0} {
            puts $fp $output
        } else {
            puts $output
        }
    }
    if {$fp != 0} {
        close $fp
    }
    return
}

proc nclear {} {
    ccs::config_template $::mc 10 1
}


list
