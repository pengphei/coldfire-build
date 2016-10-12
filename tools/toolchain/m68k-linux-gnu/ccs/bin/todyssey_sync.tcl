;#Create all the registers
addreg sfctrl

;#Create configuration options

;#Create Stat structure
set Stat(inst) 0
set Stat(icnt) 0
set Stat(vname) ody
set Stat(core) "Odyssey Sync"
set Stat(reg)  {sfctrl}
set Stat(size) {8}
set Stat(fmt) {sfctrl $fmt(sfctrl)\n}

proc odyssey_config_chain {} {
    set chain {
        odyssey_sync
        odyssey_ms1162
        odyssey_ms1162
        odyssey_ms1162
        odyssey_ms1162
        odyssey_ms1162
        odyssey_ms1162
    }

    set ::top 0
    set ::mr0 1
    set ::mr1 2
    set ::mr2 3
    set ::mr3 4
    set ::mr4 5
    set ::mr5 6

    ccs::config_chain $chain
    set i 0
    for {set i 0} {$i < [llength $chain]} {incr i} {
        ccs::set_max_tck $i 1000
        if {$i} {
            ccs::config_template $i 0 1
        }
    }
}
proc od8102_config_chain {} {
    set chain {
        msc8102_sync
        msc8102_sc140
        msc8102_sc140
        msc8102_sc140
        msc8102_sc140
        odyssey_sync
        odyssey_ms1162
        odyssey_ms1162
        odyssey_ms1162
        odyssey_ms1162
        odyssey_ms1162
        odyssey_ms1162
    }
    set ::sctop 0
    set ::sc0 1
    set ::sc1 2
    set ::sc2 3
    set ::sc3 4
    set ::mrtop 5
    set ::mr0 6
    set ::mr1 7
    set ::mr2 8
    set ::mr3 9
    set ::mr4 10
    set ::mr5 11

    ccs::config_chain $chain
    set i 0
    for {set i 0} {$i < [llength $chain]} {incr i} {
        ccs::set_max_tck $i 1000
        if {$i > $::mrtop} {
            ccs::config_template $i 0 1
        }
    }
}
namespace export odyssey_config_chain
namespace export od8102_config_chain
namespace eval ::ccs [list namespace import -force [namespace current]::odyssey_config_chain]
namespace eval ::ccs [list namespace import -force [namespace current]::od8102_config_chain]
