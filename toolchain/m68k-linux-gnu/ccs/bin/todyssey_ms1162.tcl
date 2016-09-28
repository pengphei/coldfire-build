addreg idcode
addreg dbgstat
addreg pc
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
addreg ba0
addreg ba1
addreg ba2
addreg ba3
addreg ms1id
addreg rc_banksel
addreg rc_r0
addreg rc_r1
addreg rc_r2
addreg rc_r3
addreg rc_r4
addreg rc_r5
addreg rc_r6
addreg rc_r7
addreg rc_r8
addreg rc_r9
addreg rc_r10
addreg rc_r11
addreg rc_r12
addreg rc_r13
addreg rc_r14
addreg rc_r15
addreg rc_ctxtl
addreg rc_ctxth1
addreg rc_ctxth2
addreg rc_fbcolin
addreg rc_fbrowin
addreg rc_outmux
addreg rc_z1
addreg rc_z2
addreg rc_macl
addreg rc_mach
addreg rc_chanih
addreg rc_chanil
addreg rc_chanqh
addreg rc_chanql
addreg rc_codeih
addreg rc_codeil
addreg rc_codeqh
addreg rc_codeql
addreg rc_flag
addreg rc_ctxt32

# HACKS
addreg xpc
addreg xsfctrl
addreg xrcastat
addreg xdbgstat
addreg xhack1
addreg xsleep
addreg xbreak0
addreg xbreak1
addreg xbreak2
addreg xbreak3
addreg xresume
addreg xtrst


set Stat(inst) 4
set Stat(icnt) 3
set Stat(vname) ms1
set Stat(core) "MS1-16-002"
set Stat(reg)  {r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12 r13 r14 r15 dbgstat idcode pc ba0 ba1 ba2 ba3}
set Stat(size) {8  8  8  8  8  8  8  8  8  8  8   8   8   8   8   8   8       8      8  8   8   8   8  }
set    Stat(fmt) { r0 $fmt(r0)\t  r1 $fmt(r1)\t  r2 $fmt(r2)\t  r3 $fmt(r3)\n}
append Stat(fmt) { r4 $fmt(r4)\t  r5 $fmt(r5)\t  r6 $fmt(r6)\t  r7 $fmt(r7)\n}
append Stat(fmt) { r8 $fmt(r8)\t  r9 $fmt(r9)\t r10 $fmt(r10)\t r11 $fmt(r11)\n}
append Stat(fmt) {r12 $fmt(r12)\t r13 $fmt(r13)\t r14 $fmt(r14)\t r15 $fmt(r15)\n}
append Stat(fmt) {ba0 $fmt(ba0)\t ba1 $fmt(ba1)\t ba2 $fmt(ba2)\t ba3 $fmt(ba3)\n}
append Stat(fmt) {dbg $fmt(dbgstat)\t idcode $fmt(idcode)\n}
append Stat(fmt) { pc $fmt(pc)}



proc convertHex {ch} {
    if {$ch >= 0 && $ch <= 9} {
        return $ch
    }
    
    if {"$ch" == "A"} { return 10 }
    if {"$ch" == "B"} { return 11 }
    if {"$ch" == "C"} { return 12 }
    if {"$ch" == "D"} { return 13 }
    if {"$ch" == "E"} { return 14 }
    if {"$ch" == "F"} { return 15 }
    
}
proc processStype1 {pos fp} {
}
proc processStype2 {pos fp} {
    variable err
    variable address
    variable buffer [list]
    variable length
    set err 0

#   puts "Processing type 2 record"
    
    # get length
    set ch [read $fp 1]
    set ch [convertHex $ch]
    set length [expr $ch * 16]
    set ch [read $fp 1]
    set ch [convertHex $ch]
    set length [expr $length + $ch]
#   puts "length addr + data + csum = $length"
    
    # sub checksum off
    set length [expr $length - 1]

    # get address 3 bytes = 6 chars
    set address 0
    for {set x 0} {$x < 6} {incr x} {
        set ch [read $fp 1]
        set ch [convertHex $ch]
        set address [expr $address * 16]
        set address [expr $address + $ch]
    }
    set length [expr $length - 3]
#   puts [format "address = 0x%8.8X" $address]

#   puts "length data = $length"

    set buffer 0    
    for {set x 0} {$x < $length} {incr x} {
        set data 0
        set ch [read $fp 1]
        set ch [convertHex $ch]
        set data [expr $ch * 16]
        set ch [read $fp 1]
        set ch [convertHex $ch]
        set data [expr $data + $ch]
#       puts [format "data = %x " $data]
        
        if {$x == 0} {
            set buffer [format "0x%2.2X " $data]
            continue
        }
        if {$x != 0} {
            append buffer [format "0x%2.2X " $data]
        }
    }
    
#   puts " --- data: {$buffer}"

    #puts "1 ccs::write_mem $pos [format "0x%8.8X" $address] 1 0 {$buffer}"
    #ccs::write_mem $pos $address 1 0 {$buffer}
    writeMem $pos [format "0x%8.8X" $address] $buffer
    
    return $err
}
proc processStype3 {pos fp} {
    variable err
    variable address
    variable buffer [list]
    variable length
    set err 0

#   puts "Processing type 3 record"
    
    # get length
    set ch [read $fp 1]
    set ch [convertHex $ch]
    set length [expr $ch * 16]
    set ch [read $fp 1]
    set ch [convertHex $ch]
    set length [expr $length + $ch]
#   puts "length addr + data + csum = $length"
    
    # sub checksum off
    set length [expr $length - 1]

    # get address 4 bytes = 8 chars
    set address 0
    for {set x 0} {$x < 8} {incr x} {
        set ch [read $fp 1]
        set ch [convertHex $ch]
        set address [expr $address * 16]
        set address [expr $address + $ch]
    }
    set length [expr $length - 4]
#   puts [format "address = 0x%8.8X" $address]

#   puts "length data = $length"

    set buffer 0    
    for {set x 0} {$x < $length} {incr x} {
        set data 0
        set ch [read $fp 1]
        set ch [convertHex $ch]
        set data [expr $ch * 16]
        set ch [read $fp 1]
        set ch [convertHex $ch]
        set data [expr $data + $ch]
#       puts [format "data = %x " $data]
        
        if {$x == 0} {
            set buffer [format "0x%2.2X " $data]
            continue
        }
        if {$x != 0} {
            append buffer [format "0x%2.2X " $data]
        }
    }
    
#   puts " --- data: {$buffer}"

    writeMem $pos [format "0x%8.8X" $address] $buffer

    #puts "2 ccs::write_mem $pos [format "0x%8.8X" $address] 1 0 {$buffer}"
    
#   ccs::write_mem $pos [format "0x%8.8X" $address] 1 0 $buffer
    return $err
}

proc writeMem {pos addr data} {
    if {$addr < 0x1000000} {
        if {[llength $data] % 4} {
            error "MEM($addr) needs multiple of 4 bytes: [llength $data]"
        }
    } elseif {[llength $data] % (4*4)} {
        error "MEM($addr) needs multiple of 16 bytes: [llength $data]"
    }

    foreach {a b c d} $data {
        lappend data4 [format 0x%08X [expr ($a<<24)|($b<<16)|($c<<8)|$d]]
    }

    #puts "ccs::write_mem $pos $addr 4 0 {$data4}"
    ccs::write_mem $pos $addr 4 0 $data4
}

proc findnextS {fp} {
    variable err
    variable ch

    set err 0
#   puts "searching for next record"
#   puts ""
    while {$err == 0} {
        set ch [read $fp 1]
#       puts -nonewline "$ch "
        if [eof $fp] {
            set err 1
            break
        }
        if {"$ch" == "S"} {
            break
        }
    }
#   puts ""
#   puts "err is $err"
    return $err
}
proc loadsrec {{pos help} {filename -1}} {
    variable ch
    variable err
    variable num_processed
    
    if {"$pos" == "help" || "$filename" == "-1"} {
        puts "Usage: loadsrec core filename"
        puts "core is one of 1,2,3,4,5,6"
        puts "filename is the SRec file to load"
        return
    }

    set num_processed 0
    puts -nonewline " file $filename"
    puts ""
    set fp [open $filename "r"]

    set err [findnextS $fp]
    if {$err != 0} {
        close $fp
        return
    }
    set err 0
    while {$err == 0} {
        set ch [read $fp 1]
        if [eof $fp] {
            puts "error finding record"
            set err 1
            break
        }
        if {"$ch" == "0"} {
            puts "bypassing type 0"
            set err [findnextS $fp]
            if {$err != 0} {
                break
            }
            continue
        }
        if {"$ch" == "1"} {
            set err [processStype1 $pos $fp]
            if {$err != 0} {
                break
            }
            incr num_processed
            set err [findnextS $fp]
            if {$err != 0} {
                break
            }
            continue
        }
        if {"$ch" == "2"} {
            set err [processStype2 $pos $fp]
            if {$err != 0} {
                break
            }
            incr num_processed
            set err [findnextS $fp]
            if {$err != 0} {
                break
            }
            continue
        }
        if {"$ch" == "3"} {
            set err [processStype3 $pos $fp]
            if {$err != 0} {
                break
            }
            incr num_processed
            set err [findnextS $fp]
            if {$err != 0} {
                break
            }
            continue
        }
        if {$ch >= 7 && $ch <= 9} {
            puts "found end record"
            break
        }
        puts "invalid stype"
        set err 1
        break
    }
    if {$err != 0} {
        puts "error processing file"
    }

    puts "number processed = $num_processed"

    close $fp
}

namespace export loadsrec
namespace eval ::ccs [list namespace import -force [namespace current]::loadsrec]
