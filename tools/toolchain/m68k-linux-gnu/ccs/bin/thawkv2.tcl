addcfg dakar_enable_lockout_recovery
addcfg dakar_hfm_divisor

addreg x0
addreg y0
addreg y1
addreg a0
addreg a1
addreg a2
addreg b0
addreg b1
addreg b2
addreg c0
addreg c1
addreg c2
addreg d0
addreg d1
addreg d2
addreg omr
addreg sr
addreg la
addreg la2
addreg lc
addreg lc2
addreg hws0
addreg hws1
addreg sp
addreg n3
addreg m01
addreg n
addreg r0
addreg r1
addreg r2
addreg r3
addreg r4
addreg r5
addreg shm01
addreg shn
addreg shr0
addreg shr1
addreg pc
addreg idcode
addreg ocr
addreg oscntr
addreg osr
addreg opdbr
addreg obase
addreg otxrxsr
addreg otx
addreg otx1
addreg orx
addreg orx1
addreg otbcr
addreg otbpr
addreg otb
addreg ob0cr
addreg ob0ar1
addreg ob0ar2
addreg ob0msk
addreg ob0cntr

set Stat(inst) 2
set Stat(icnt) 4
set Stat(vname) hv2
set Stat(core) "HawkV2"
set Stat(memspace) 0
set Stat(reg)  {r0 r1 r2 r3 r4 r5 n n3 m01 a0 a1 a2 b0 b1 b2 c0 c1 c2 d0 d1 d2 sr sp omr pc y0 y1 x0 osr} 
set Stat(size) {6  6  6  6  6  6  6  6 6   4  4  1  4  4  1  4  4  1  4  4  1  6  6  6   6  6  6  6  6}
set    Stat(fmt) {y1 $fmt(y1)\ty0 $fmt(y0)\t x0 $fmt(x0)\n}
append Stat(fmt) {a2 $fmt(a2)\t\ta1 $fmt(a1)\t a0 $fmt(a0)\tr0 $fmt(r0)\n}
append Stat(fmt) {b2 $fmt(b2)\t\tb1 $fmt(b1)\t b0 $fmt(b0)\tr1 $fmt(r1)\n}
append Stat(fmt) {c2 $fmt(c2)\t\tc1 $fmt(c1)\t c0 $fmt(c0)\tr2 $fmt(r2)\n}
append Stat(fmt) {d2 $fmt(d2)\t\td1 $fmt(d1)\t d0 $fmt(d0)\tr3 $fmt(r3)\n}
append Stat(fmt) { n $fmt(n)\tn3 $fmt(n3)\tm01 $fmt(m01)\tr4 $fmt(r4)\n}
append Stat(fmt) {sr $fmt(sr)\tsp $fmt(sp)\tomr $fmt(omr)\tr5 $fmt(r5)\n}
append Stat(fmt) {osr $fmt(osr)\n}
append Stat(fmt) {pc $fmt(pc)}

proc load {chain_pos filename {memspace 0}} {
    set prog_addr [list]
    set prog_data [list]

    set fp [open $filename r]
    set tmpdat [list]
    set origin_found 0
    while {![eof $fp]} {
        set line [gets $fp]
        if       {[regexp {^                     ([0-9A-F][0-9A-F][0-9A-F][0-9A-F])$} $line match dat]} {
            lappend tmpdat "0x${dat}"
        } elseif {[regexp {^..........P:........ ([0-9A-F][0-9A-F][0-9A-F][0-9A-F]) .*$} $line match dat]} {
            lappend tmpdat "0x${dat}"
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

proc ! {args} {
    puts [concat $args]
    return [eval [concat $args]]
}

proc oldrecover {{hfm 20}} {
    ! ccs::config_chain hawkv2
    ! ccs::config_template 0 dakar_enable_lockout_recovery 1
    ! ccs::config_template 0 dakar_hfm_divisor $hfm
    ! ccs::reset_to_user
    ! ccs::config_template 0 dakar_enable_lockout_recovery 0
    ! ccs::reset_to_debug
    ! ccs::stat
}

proc recover {{hfm 20}} {
    ! ccs::config_chain hawkv2
    ! ccs::config_template 0 dakar_enable_lockout_recovery 1
    ! ccs::config_template 0 dakar_hfm_divisor $hfm
    ! ccs::reset_to_debug
    ! ccs::config_template 0 dakar_enable_lockout_recovery 0
    ! ccs::reset_to_debug
    ! ccs::stat
}


proc lockout {} {
    ! ccs::config_chain hawkv2
    ! ccs::reset_to_debug
    # showbits [! ccs::read_mem 0 0xFFF400 2 1 1]
    ! ccs::write_mem 0 0xFFF400 2 1 0x54
    # showbits [! ccs::read_mem 0 0xFFF413 2 1 1]
    ! ccs::write_mem 0 0xFFF401 2 1 0x0000
    ! ccs::write_mem 0 0xFFF410 2 1 0x0000
    ! ccs::write_mem 0 0x01FFF7 2 0 0xE70A
    ! ccs::write_mem 0 0xFFF414 2 1 0x0020
    ! ccs::write_mem 0 0xFFF413 2 1 0x0080
    # puts "look for bits 5 & 4 clear and bit 7 set"
    # showbits [! ccs::read_mem 0 0xFFF413 2 1 1]
    after 100
    # puts "look for bit 6 set"
    # showbits [! ccs::read_mem 0 0xFFF413 2 1 1]
    # puts "read 0xE70A"
    set verify [ccs::read_mem 0 0x01FFF7 2 0 1]
    if {$verify == 0xE70A} {
        puts "Lockout successful; use hawkv2::recover to unlock"
    } else {
        puts [format "Lockout failed, read 0x%04X from X:0x01FFF7" $verify]
    }
}

proc diag {} {
    set test(chiptap) "not run"
    set test(coretap) "not run"
    set test(once) "not run"
    set test(core) "not run"

    ######################################################################
    puts "\n*** Chip tap test ***"
    ######################################################################
    ! ccs::config_chain {}
    ! ccs::config_chain hawkv2
    ! ccs::reset_to_user

    set idcode [ccs::read_reg 0 0x1000 1]
    puts [format "Read IDCODE: 0x%08X" $idcode]
    if {($idcode & 0xFFF00FFF) == 0x01F0001D} {
        puts "IDCODE looks ok"
        puts "This means the chip tap is functioning"
        set test(chiptap) "Passed all tests"
    } else {
        puts "Expected IDCODE to look like 0x01Fxx01D"
        if {($idcode & 0x000000FF) == 0x0000001D} {
            puts "However, this still looks like a Motorola part"
            puts "This means the chip may be functioning"
            set test(chiptap) "Failed some tests"
        } else {
            if {$idcode == 0xFFFFFFFF} {
                puts "Looks like TDO, TMS or TRST is stuck high"
            } elseif {$idcode == 0x00000000} {
                puts "Looks like TDO is stuck low, or TMS or TRST is stuck high"
            } else {
                puts "This value is completely unexpected"
            }
            set test(chiptap) "Failed all tests"
        }
    }

    ######################################################################
    puts "\n*** Core tap test ***"
    ######################################################################

    set jir [ccs::read_reg 0 0x1001 1]
    puts [format "Core tap JTAG Instruction register contains: 0x%X" $jir]
    if {($jir & 0x3) == 0x01} {
        puts -nonewline "This is a reasonable value which corresponds to "
        set test(coretap) "Failed some tests"
        switch $jir {
            1 {puts "user mode (executing)\nThis is an expected mode"}
            5 {puts "stop mode\nThis is interesting, but not totally unexpected"}
            9 {puts "exacc mode (waiting for bus)\nThis is an expected mode"}
            13 {puts "debug mode\nThis is not expected"}
        }
    } else {
        set test(coretap) "Failed all tests"
        if {$jir == 15 || $jir == 0} {
            puts "This value normally indicates cable or power issues."
        } else {
            puts "This value is completely unexpected"
        }
    }
    puts -nonewline "Attempting to reset to debug..."
    set error [catch {ccs::reset_to_debug}]
    if {$error} {
        puts "Failed"
    } else {
        puts "Succeeded"
    }
    set jir [ccs::read_reg 0 0x1001 1]
    puts [format "Core tap JTAG Instruction register contains: 0x%X" $jir]
    if {($jir & 0x3) == 0x01} {
        puts -nonewline "This is a reasonable value which corresponds to "
        switch $jir {
            1 {puts "user mode (executing)\nThis is unexpected"}
            5 {puts "stop mode\nThis is unexpected"}
            9 {puts "exacc mode (waiting for bus)\nThis is unexpected"}
            13 {puts "debug mode\nThis means the core tap is functioning"
                set test(coretap) "Passed all tests"}
        }
    } else {
        if {$jir == 15 || $jir == 0} {
            puts "This value normally indicates cable or power issues."
        } else {
            puts "This value is completely unexpected"
        }
    }

    ######################################################################
    puts "\n*** OnCE Test ***"
    ######################################################################

    ! ccs::config_chain {}
    ! ccs::config_chain hawkv2
    ! ccs::reset_to_user
    catch {! display ccs::core_run_mode 0}

    puts "Reading the OTXRXSR"
    set error [catch {set otxrxsr [ccs::read_reg 0 otxrxsr 1]}]
    if {!$error} {
        puts [format "OTXRXSR contains %08X" $otxrxsr]
        if {!$otxrxsr} {
            puts "This is expected"
            puts "Writing to ORX"
            catch {ccs::rtd_write 0 {1 2 3 4}}
            update
            set error [catch {set otxrxsr [ccs::read_reg 0 otxrxsr 1]}]
            if {!$error} {
                puts [format "OTXRXSR contains %08X" $otxrxsr]
                if {($otxrxsr & 1) == 1} {
                    puts "This means the OnCE is functioning"
                    set test(once) "Passed all tests"
                } else {
                    puts "This indicates the OnCE is not functioning"
                    set test(once) "Failed some tests"
                }
            } else {
                puts "Reading the OTXRXSR failed"
                set test(once) "Failed most tests"
            }
        } else {
            puts "This should have been 0x00000000"
            set test(once) "Failed most tests"
        }
    } else {
        puts "Failed to read the otxrxsr"
        set test(once) "Failed all tests"
    }

    ######################################################################
    puts "\n*** Core Test ***"
    ######################################################################

    puts -nonewline "Attempting to reset to debug..."
    set error [catch {ccs::reset_to_debug}]
    if {!$error} {
        set d0 0
        puts "Succeeded"
        puts "Write/Read test on core register d0"
        puts "Write 0xCAFE"
        catch {ccs::write_reg 0 d0 0xCAFE}
        catch {set d0 [ccs::read_reg 0 d0 1]}
        puts [format "Read 0x%04X" $d0]
        if {$d0 == 0xCAFE} {
            puts "Passed test 1"
            puts "Write 0xFACE"
            catch {ccs::write_reg 0 d0 0xFACE}
            catch {set d0 [ccs::read_reg 0 d0 1]}
            puts [format "Read 0x%04X" $d0]
            if {$d0 == 0xFACE} {
                puts "Passed test 2"
                puts "This means the Core is functioning"
                set test(core) "Passed all tests"
            } else {
                puts "Failed test 2"
                set test(core) "Failed some tests"
            }
        } else {
            puts "Failed test 1"
            if {$d0 == 0} {
                puts "Core appears to be in lockout mode"
            }
            set test(core) "Failed most tests"
        }

    } else {
        puts "Failed"
        set test(core) "Failed all tests"
    }

    ######################################################################
    puts "\n*** Test Summary ***"
    ######################################################################
    foreach module {chiptap coretap once core} {
        puts "$module : $test($module)"
    }
}
