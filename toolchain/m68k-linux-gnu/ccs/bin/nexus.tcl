;#
;# nexus.tcl
;# Copyright (C) 2001 Metrowerks
;# Metrowerks is a Motorola Company
;#
namespace eval nexus {

    set ed 0
    set mc 2

    set reg(dtea2) 81
    set reg(dtea1) 80
    set reg(dtsa2) 79
    set reg(dtsa1) 78
    set reg(dtc) 77
    set reg(rwd) 76
    set reg(rwa) 75
    set reg(rwcs) 74
    set reg(uba) 73
    set reg(dc) 71

    proc verify_debug {} {
        variable mc

        set failure [catch {set chain [ccs::get_config_chain]}]
        if {$failure || [llength $chain] != 4} {
            error "CodeWarrior has not connected."
        }

        set failure [catch {set runmode [::ccs::core_run_mode $mc]} runmodeerror]
        if {$failure} {
            error "Unable to determine MCore execution mode" $runmodeerror
        }
        if {$runmode != $::ccs::mode(debug)} {
            error "MCore is in $::ccs::mode($runmode) mode. Must be in debug mode"
        }
    }

    proc nlist {} {
        variable reg
        set i 0
        foreach name [lsort [array names reg]] {
            puts -nonewline "${name}"
            incr i
            if {$i%4} {
                set pad " "
                for {set j [string length $name]} {$j < 6} {incr j} {
                    append pad " "
                }
                puts -nonewline $pad
                set need_newline 1
            } else {
                puts ""
                set nee_newline 0
            }
        }
        if {$need_newline} {
            puts ""
        }
    }

    proc nwrite {nexus_register value} {
        variable mc
        variable reg

        verify_debug

        set nreg [string tolower $nexus_register]
        if {[llength [array names reg $nreg]] != 1} {
            error "$nexus_register is not a valid nexus register."
        }
        if {![string is integer -strict $value]} {
            error "$value is not a valid integer"
        }
        ;# if writing to register dc, force bit 31 high
        if {[string equal -nocase $nreg dc]} {
            set value [expr $value | 1<<31]
        }
        if {[catch {::ccs::write_reg $mc $reg($nreg) $value}]} {
            error "Error writing ${nexus_register}."
        }
    }

    proc nread {nexus_register} {
        variable reg
        variable mc

        verify_debug

        set pattern [string tolower $nexus_register]
        if {[string equal -nocase $pattern all]} {
            set pattern "*"
        }
        set regs [lsort [array names reg $pattern]]
        if {![llength $regs]} {
            error "$nexus_register is not a valid nexus register."
        }
        foreach nreg $regs {
            if {[catch {set value [::ccs::read_reg $mc $reg($nreg) 1]}]} {
                error "Error reading $reg(${nreg})."
            } else {
                set pad " "
                for {set j [string length $nreg]} {$j < 6} {incr j} {
                    append pad " "
                }
                puts [format "${nreg}${pad}0x%08X" ${value}]
            }

        }
    }

    set entrypoints [list nlist nread nwrite]
    foreach entrypoint $entrypoints {
        puts "Added command $entrypoint"
        namespace export $entrypoint
    }
}
namespace import -force ::nexus::*
