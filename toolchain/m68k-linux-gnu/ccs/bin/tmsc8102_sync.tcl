;#Create all the registers
addreg idcode
addreg pireg

;#Create configuration options

;#Create Stat structure
set Stat(inst) 0
set Stat(icnt) 0
set Stat(vname) sc
set Stat(core) "MSC8102 Sync"
set Stat(reg)  {idcode pireg}
set Stat(size) {8  8}
set Stat(fmt) {idcode $fmt(idcode)\t  pireg $fmt(pireg)\n}

array set NS_chainid {
    core    3
    chip    2
    lssd    0
}
array set NS_bitlength {
    core    13303
    chip    19519
    lssd     5666
}
array set NS_scanid {
    a   0x0C
    b   0x1E
}

proc longChain {{core -1} {ecnt -1} {chain -1} {scan -1} {filename -1} {ms -1}} {
    variable NS_chainid
    variable NS_scanid

    if {$core == -1 || $ecnt == -1 || $chain == -1 || $scan == -1 || $filename == -1 || $ms == -1} {
        append msg "Usage:\n" \
                   "longChain core ecnt chain scan filename ms\n" \
                   " core: 0 1 2 3\n" \
                   " ecnt: ecnt value\n" \
                   " chain: core chip lssd\n" \
                   " scan: a b\n" \
                   " filename: file to write data\n" \
                   " ms: milliseconds to wait"
        error $msg
    }

    if {$core < 0 || $core > 3} {
        error "core must be one of: 0 1 2 3"
    }

    if {![info exist NS_chainid($chain)]} {
        error "chain must be one of: [array names NS_chainid]" 
    }

    if {![info exist NS_scanid($scan)]} {
        error "scan must be one of: [array names NS_scanid]" 
    }

    longChainSetup $core $ecnt
    puts "Waiting $ms ms for core $core to freeze clocks"
    after $ms
    longChainDump $core $chain $scan $filename
}

proc longChainSetup {core ecnt} {
    set chain_pos [expr $core + 1]
    ::ccs::write_reg $chain_pos ecnt_val $ecnt
    ::ccs::write_reg $chain_pos esel_dm 0x200
    ::ccs::write_reg $chain_pos esel_ctrl 0x2fc
    ::ccs::run_core $chain_pos
}

proc longChainScan {{core 0} {chain 0} {scan 0}} {
    variable NS_chainid
    variable NS_bitlength
    variable NS_scanid

    if {[string equal $scan 0]} {
        append msg \
            "Usage:\n"\
            "longChainScan core chain scan\n" \
            " core: 0 1 2 3\n" \
            " chain: [array names chainid]\n" \
            " scan: [array names scanid]\n" \
            "longChainScan will return a list of bytes scanned out from the\n" \
            "core's chain using the scan command."
        error $msg
    }

    if {$core < 0 || $core > 3} {
        error "core must be one of: 0 1 2 3"
    }

    if {![info exist NS_chainid($chain)]} {
        error "chain must be one of: [array names NS_chainid]" 
    }

    if {![info exist NS_scanid($scan)]} {
        error "scan must be one of: [array names NS_scanid]" 
    }

    set gpr [expr ($NS_chainid($chain) << 4) | ($core << 2)]

    set chain_pos 0
    set mem(addr) $gpr
    set mem(size) 1
    set mem(space) [expr (($NS_bitlength($chain)*2) << 16) | 0x8000 | $NS_scanid($scan) ]
    set bytes [expr (($NS_bitlength($chain)*2)+7)/8]

    ;# chain_pos must be 8102 synch core
    ;# mem(addr) will be used as gpr contents
    ;# mem(space)[31:16] will be used as bit count
    ;# mem(space)[15] must be 1 to indicate long scan
    ;# mem(space)[4:0] will be put in JTAG IR (SCAN_ENABLE_A or SCAN_ENABLE_B)
    ;# mem(size) must be 1 for byte access (ccstcl won't modify endianness)
    ;# bytes must be (bitcount+7)/8

    return [::ccs::read_mem $chain_pos $mem(addr) $mem(size) $mem(space) $bytes]
}

proc longChainDump {core chain scan filename} {
    puts -nonewline "Dumping core($core):chain($chain):scan($scan) to $filename..."
    longChainWrite [longChainScan $core $chain $scan] $filename
    puts "complete"
}

proc longChainWrite {data filename} {
    set fp [open $filename "w"]
    foreach byte $data {
        for {set bit 0x01} {$bit < 0xFF} {set bit [expr $bit << 2]} {
            puts $fp [expr ($byte & $bit) ? 1 : 0]
        }
    }
    close $fp
    return
}

proc msc8102sysInit {} {
    ccs::config_chain {msc8102_sync msc8102_sc140 msc8102_sc140 msc8102_sc140 msc8102_sc140 sc140}
    ccs::set_max_tck 0 1000
    ccs::set_max_tck 1 1000
    ccs::set_max_tck 2 1000
    ccs::set_max_tck 3 1000
    ccs::set_max_tck 4 1000
    ccs::set_max_tck 5 1000
}

proc msc8102dsiReset {{conf_word 0x24200604}} {
    msc8102sysInit
    ccs::config_template 0 0 1
    ccs::reset_to_debug
    ccs::write_mem 5 0xf0010108 4 0 0x14501801
    ccs::write_mem 5 0xf001010c 4 0 0xFFFF8820
    ccs::write_mem 5 0xf0010118 4 0 0x26001801
    ccs::write_mem 5 0xf001011c 4 0 0xffe00884
    ccs::write_mem 5 0x261be050 4 0 $conf_word
    ccs::stop_core 0
    ccs::core_run_mode 1
    ccs::core_run_mode 2
    ccs::core_run_mode 3
    ccs::core_run_mode 4
    return
}

proc msc8102init {} {
    ccs::msc8102_config_chain
    ccs::set_max_tck 0 1000
    ccs::set_max_tck 1 1000
    ccs::set_max_tck 2 1000
    ccs::set_max_tck 3 1000
    ccs::set_max_tck 4 1000

    set ::l2mem 0x01000000
    set ::l1mem 0x1000

    return
}

namespace export longChain
namespace eval :: [list namespace import -force [namespace current]::longChain*]
namespace export msc8102sysInit
namespace export msc8102init
namespace export msc8102dsiReset
namespace eval :: [list namespace import -force [namespace current]::msc8102*]
