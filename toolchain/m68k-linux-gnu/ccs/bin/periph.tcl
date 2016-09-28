;#
;# periph.tcl
;# Copyright (C) 2001 Metrowerks
;# Metrowerks is a Motorola Company
;#
namespace eval periph {
    set initialized 0
    set ed 0
    set sc 1

    proc init {} {
        set failure [catch {set chain [ccs::get_config_chain]}]
        if {$failure || [llength $chain] != 4} {
            error "CodeWarrior has not connected."
        }
        verify_debug
        readpdat
        init_bank
        set initialized 1
    }

    proc check_init {} {
        variable initialized
        if {!$initialized} {
            init
        }
    }

    proc verify_debug {} {
        variable sc
        set failure [catch {set runmode [::ccs::core_run_mode $sc]} runmodeerror]
        if {$failure} {
            error "Unable to determine StarCore execution mode" $runmodeerror
        }
        if {$runmode != $::ccs::mode(debug)} {
            error "StarCore is in $::ccs::mode($runmode) mode. Must be in debug mode"
        }
    }

    proc readpdat {} {
        variable peripherals
        variable base

        set fp [open "${::ccsExecPath}/periph.dat" "r"]
        set peripherals [concat [read $fp]]
        close $fp
        set fp [open "${::ccsExecPath}/pbase.dat" "r"]
        set pbase [concat [read $fp]]
        close $fp
        foreach {pname baseaddr} $pbase {
            set base([string tolower $pname]) [format "0x%X" $baseaddr]
        }
    }

    proc init_bank {} {
        variable sc
        variable ed

        set QBUS_MASK_1_ADDR 0x0100ff04
        set QBUS_MASK_1_SIZE 2
        set QBUS_MASK_1_VAL 0xFFF8
        set QBUS_BASE_1_ADDR 0x0100ff06
        set QBUS_BASE_1_SIZE 2
        set QBUS_BASE_1_VAL 0x0200

        if {[catch {::ccs::write_reg $ed plltest 0}]} {
            error "Failed initializing PLLs"
            return 1
        } elseif {[catch  {::ccs::write_mem $sc $QBUS_MASK_1_ADDR $QBUS_MASK_1_SIZE 0 $QBUS_MASK_1_VAL}]} {
            error "Failed initializing QBUS MASK 1"
            return 1
        } elseif {[catch  {::ccs::write_mem $sc $QBUS_BASE_1_ADDR $QBUS_BASE_1_SIZE 0 $QBUS_BASE_1_VAL}]} {
            error "Failed initializing QBUS BASE 1"
            return 1
        }
        return 0
    }

    proc pinfo {peripheral_name args} {
        variable peripherals
        variable base
        set pname $peripheral_name
        check_init
        if {[llength $args] == 1} {
            set rname [lindex $args 0]
        } elseif {[llength $args] > 1} {
            error "called pinfo with too many arguments"
            return
        } else {
            set rname all
        }

        set found 0
        foreach {module regname offset size access} $peripherals {
            if {([string equal -nocase $module $pname] || [string equal -nocase all $pname]) &&
                ([string equal -nocase $regname $rname] || [string equal -nocase all $rname])} {
                set found 1
                set addr [expr $base([string tolower ${module}]) + $offset]
                set pad " "
                for {set j [string length "${module}:${regname}"]} {$j < 21} {incr j} {
                    append pad " "
                }
                puts [format "${module}:${regname}${pad}address(0x%08X) size(${size}) access(${access})" $addr]
            }
        }
        if {!$found} {
            error "Peripheral ${pname}:${rname} not found."
        }
    }

    proc pread {peripheral_name args} {
        variable peripherals
        variable base
        set pname $peripheral_name
        check_init
        if {[llength $args] == 1} {
            set rname [lindex $args 0]
        } elseif {[llength $args] > 1} {
            error "called pread with too many arguments"
            return
        } else {
            set rname all
        }

        set found 0
        foreach {module regname offset size access} $peripherals {
            if {([string equal -nocase $module $pname] || [string equal -nocase all $pname]) &&
                ([string equal -nocase $regname $rname] || [string equal -nocase all $rname])} {
                set found 1
                if {[string first r $access] != -1} {
                    set addr [expr $base([string tolower ${module}]) + $offset]
                    set data [readmem $addr $size]
                    set pad " "
                    for {set j [string length "${module}:${regname}"]} {$j < 21} {incr j} {
                        append pad " "
                    }
                    puts "${module}:${regname}${pad}${data}"
                }
            }
        }
        if {!$found} {
            error "Peripheral ${pname}:${rname} not found."
        }
    }

    proc plist {} {
        variable peripherals
        check_init
        set foo() 0
        set i 0
        set need_newline 0
        foreach {module regname offset size access} $peripherals {
            if {[llength [array get foo $module]] == 0} {
                set foo($module) 1
                puts -nonewline "$module"
                incr i
                if {$i%5} {
                    set pad " "
                    for {set j [string length $module]} {$j < 9} {incr j} {
                        append pad " "
                    }
                    puts -nonewline $pad
                    set need_newline 1
                } else {
                    puts ""
                    set need_newline 0
                }
            }
        }
        if {$need_newline} {
            puts ""
        }
    }

    proc plistreg {peripheral_name} {
        variable peripherals
        check_init
        set periph $peripheral_name
        foreach {module regname offset size access} $peripherals {
            if {[string equal -nocase $module $periph] || [string equal -nocase all $periph]} {
                puts "${module}:${regname}"
            }
        }
    }

    proc pwrite {peripheral_name register_name register_data} {
        variable peripherals
        variable base
        set pname $peripheral_name
        set rname $register_name
        set val $register_data
        check_init
        set found 0
        foreach {module regname offset size access} $peripherals {
            if {[string equal -nocase $module $pname] && [string equal -nocase $regname $rname]} {
                if {[string first w $access] != -1} {
                    set addr [expr $base([string tolower ${module}]) + $offset]
                    if {[writemem $addr $size $val]} {
                        puts "Write failed to ${module}:${regname}"
                    } else {
                        pread ${module} ${regname}
                    }
                    return
                } else {
                    puts "${module}:${regname} is read only"
                    return
                }
            }
        }
        error "${pname}:${rname} not found."
        return
    }

    proc readmem {addr size} {
        variable sc
        verify_debug
        set failure [catch {set data [::ccs::read_mem $sc $addr $size 0 1]}]
        if {$failure} {
            set retval "Error reading ${size} bytes at address ${addr}."
        } else {
            set retval [format "0x%0[expr 2*${size}]X" $data]
        }
        return $retval
    }

    proc writemem {addr size val} {
        variable sc
        verify_debug
        return [catch {::ccs::write_mem $sc $addr $size 0 $val}]
    }

    ;#Export entrypoints
    set entrypoints [list pread pwrite pinfo plist]
    foreach entrypoint $entrypoints {
        puts "Added command $entrypoint"
        namespace export $entrypoint
    }
}

namespace import -force ::periph::*
