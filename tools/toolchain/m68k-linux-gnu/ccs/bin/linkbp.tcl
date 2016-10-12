;#
;# linkbp.tcl
;# Copyright (C) 2001 Metrowerks
;# Metrowerks is a Motorola Company
;#
namespace eval linkbp {
    proc linkbp {args} {

        ;#
        ;# Setup bitfields
        ;#
        ;# bit positions are the order in the list
        set bit_names [list dsp_in mcu_in ipcm_in dsp_out mcu_out ipcm_out]
        set dcrbit(all) 0
        set i 0
        foreach bit $bit_names {
            set dcrbit($bit) [expr 1<<$i]
            set dcrbit(all) [expr $dcrbit(all) | $dcrbit($bit)]
            incr i
        }
        set dcrbit(reset) [expr $dcrbit(dsp_in) | $dcrbit(mcu_in) | $dcrbit(ipcm_in)]
        set dcr $dcrbit(all)

        set arg1 [lindex $args 0]
        if {![string compare -nocase -length 4 $arg1 "help"]} {
            ;#
            ;# Show help text
            ;#
            puts "Usage: linkbp \[options\]"
            puts "Options:"
            puts "  help             - Display this help text"
            puts "  helpmore         - Display more help text with examples"
            puts "  all {on|off}     - Turn on/off all DE inputs and outputs"
            puts "  de_ctrl {on|off} - Turn on/off individual DE inputs and outputs"
            puts "  reset on         - Reset configuration (all DE inputs on, all DE outputs off)"
            puts ""
            puts "de_ctrl is one or more of: $bit_names"
            puts "If no options are specified the current configuration is displayed."
            puts ""
            if {![string compare -nocase $arg1 "helpmore"]} {
                ;#
                ;# Show more help text
                ;#
                puts "Multiple de_ctrl settings may be specified at a time."
                puts "All options are case insensitive."
                puts "On, true and 1 are equivilent. Off, false and 0 are equivilent"
                puts "CodeWarrior must have connected to CCS for this feature to be enabled."
                puts "If Rainbow is reset (during connect or download), the linked breakpoint"
                puts "  settings will be lost and must be reenabled manually."
                puts "Later de_ctrl options override earlier ones"
                puts "  (e.g. \"linkbp dsp_out off dsp_out on\" will turn on DSP DE output)"
                puts "Unspecified de_ctrl settings are turned off"
                puts ""
                puts "Examples:"
                puts "Stop MCU whenever DSP stops:"
                puts "  linkbp mcu_in on dsp_out on"
                puts "Stop DSP and IPCM whenever MCU stops:"
                puts "  linkbp mcu_out on dsp_in on ipcm_in on"
                puts "Stop all other cores whenever any core stops:"
                puts "  linkbp all on"
                puts "Disable linked breakpoints:"
                puts "  linkbp all off"
            }
            return
        }
        ;#
        ;# Check for configured JTAG chain
        ;#
        if {[catch {set chain [::ccs::get_config_chain]}]} {
            puts "*** ERROR ***"
            puts "Failed to determine chain configuration."
            puts "Do ccsapi.tcl, autoexec.tcl and ccstcl.dll exist in $::ccsExecPath?"
            return
        }
        if {![llength $chain]} {
            puts "CodeWarrior has not connected yet."
            puts "Type \"linkbp help\" for help."
            return
        }

        if {[llength $args]} {
            ;#
            ;# Change current settings
            ;#
            foreach {bit val} $args {
                set bit [string tolower $bit]
                set val [string tolower $val]
                if {![llength [array get dcrbit $bit]]} {
                    if {![string compare "de_ctrl" $bit]} {
                        puts "de_ctrl is one or more of $bit_names"
                    } else {
                        puts "Invalid option: $bit"
                    }
                    puts "Type \"linkbp help\" for help."
                    return
                }
                switch $val {
                    on      -
                    true    -
                    1       {
                        set dcr [expr $dcr & ~$dcrbit($bit)]
                    }
                    off     -
                    false   -
                    0       {
                        set dcr [expr $dcr | $dcrbit($bit)]
                    }
                    default {
                        if {[string length $val]} {
                            puts "Invalid setting for $bit: $val."
                        } else {
                            puts "No setting specified for $bit."
                        }
                        puts "Must be one of on, off, true, false, 1 or 0."
                        puts "Type \"linkbp help\" for help."
                        return
                    }
                }
            }
            ;#
            ;# Save current settings - write debug_control (2)
            ;#
            if {[catch {::ccs::write_reg 0 2 $dcr}]} {
                puts "*** ERROR ***"
                puts "Failed writing to Debug Control register."
                puts "Is CodeWarrior running and able to connect to Rainbow?"
                puts "Do ccsapi.tcl, autoexec.tcl and ccstcl.dll exist in $::ccsExecPath?"
                return
            }
        } else {
            ;#
            ;# Display current settings - read debug_control (2)
            ;#
            if {[catch {set dcr [::ccs::read_reg 0 2 1]}]} {
                puts "*** ERROR ***"
                puts "Failed reading from Debug Control register."
                puts "Is CodeWarrior running and able to connect to Rainbow?"
                puts "Do ccsapi.tcl, autoexec.tcl and ccstcl.dll exist in $::ccsExecPath?"
                return
            }
            puts [format "Debug Control register = 0x%02X" $dcr]
            set shift 0
            foreach bit $bit_names {
                ;#These are 'off' bits so 1 means off
                if {![expr (($dcr >> $shift) & 1)]} {
                    set stat on
                } else {
                    set stat off
                }
                puts "${bit}:${stat}"
                incr shift
            }
        }
        return
    }
    set entrypoints [list linkbp]
    foreach entrypoint $entrypoints {
        puts "Added command $entrypoint"
        namespace export $entrypoint
    }
}

namespace import -force ::linkbp::*
