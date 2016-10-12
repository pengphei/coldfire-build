# Load and initialze the CCSAPI Tcl interface

# Check If this is ccssim2
if {![info exists ccssim2]} { set ccssim2 0 }

set CCSTCL "${ccsExecPath}/ccstcl[info sharedlibextension]"
set CCSSIM "${ccsExecPath}/ccssim[info sharedlibextension]"
set SCEXT "${ccsExecPath}/scext[info sharedlibextension]"
set ELFIOLOADER "${ccsExecPath}/elfioloader[info sharedlibextension]"
set SYSCALL "${ccsExecPath}/syscall[info sharedlibextension]"
set PPCDASM  "${ccsExecPath}/ppcdasm[info sharedlibextension]"
set ac 0
set env(CCS_LIB_PATH) ${ccsExecPath}
set TKCON(hoterrors) 1


# couts ----------------------------------------------------------------------
set Cputs_nextcolortag stdout
set Cputs_colortag stdout
set Cputs_bgcolor white
set Cputs_fgcolor blue
proc cputs args {
    variable Cputs_colortag
    variable Cputs_nextcolortag
    variable Cputs_bgcolor
    variable Cputs_fgcolor
    if {[lindex $args 0] == "-nonewline"} {
        set str [lindex $args 1]
        set putnewline 0
    } else {
        set str [lindex $args 0]
        set putnewline 1
    }
    set regexpFBColor {^(.*?)<([fb]=[^ <>]+)>(.*)$}
    set regexpColorSwitch {([fb])=(.*)}
    while {[string length $str]} {
        if {[regexp -nocase $regexpFBColor $str match outtext colorswitch remain]} {
            set str $remain
            regexp -nocase $regexpColorSwitch $colorswitch match ground color
            if {[string equal -nocase $ground f]} {
                if {[string equal -nocase $color x]} {
                    set color blue
                }
                set Cputs_fgcolor $color
            } else {
                if {[string equal -nocase $color x]} {
                    set color white
                }
                set Cputs_bgcolor $color
            }
            set Cputs_nextcolortag ${Cputs_fgcolor}${Cputs_bgcolor}
            tkcon console tag configure $Cputs_nextcolortag -foreground ${Cputs_fgcolor} -background ${Cputs_bgcolor}
        } else {
            set outtext $str
            set str ""
        }
        if {[string length $outtext]} {
            tkcon console insert output $outtext $Cputs_colortag
        }
        set Cputs_colortag $Cputs_nextcolortag
    }
    if {$putnewline} {
        tkcon console insert output "\n" stdout
    }
    puts -nonewline ""
}

# ccssource ------------------------------------------------------------------
proc ccssource {filename} {
    global ccsExecPath

    if {[file readable $filename]} {
        set filetoload $filename
    } elseif {[file readable ${ccsExecPath}/$filename]} {
        set filetoload ${ccsExecPath}/$filename
    } else {
        cputs "CCS: <f=red> cannot find \"$filename\" file!<f=x>";
        return
    }

    if {[catch {uplevel  1 "source [list $filetoload]"}]} {
        cputs "CCS: <f=red> cannot load \"$filetoload\" file!<f=x>";
    } else {
        puts "CCS: $filename is sourced into \[[uplevel 1 {namespace current}]\] namespace"
    }
}


# Load simulator/board DLL
if {$ccssim2} {
	set sApiName "SIMAPI"
	set sApiDLL  $CCSSIM
} else {
	set sApiName "CCSAPI"
	set sApiDLL  $CCSTCL
}
puts -nonewline "Loading $sApiName Tcl interface ($sApiDLL)..."
if {[catch {::load $sApiDLL}]} {
	puts stderr "Failed"
    	return
} else {
	puts ""
}

if {[file readable $SCEXT]} {
    puts -nonewline "Loading StarCore Extensions ($SCEXT)..."
    if {[catch {::load $SCEXT}]} {
        puts stderr  "Failed"
        # return
    } else {
        puts ""
    }
}

if {[file readable $SYSCALL]} {
    puts -nonewline "Loading Syscall ($SYSCALL)..."
    if {[catch {::load $SYSCALL}]} {
        puts stderr "Failed"
        # return
    } else {
        puts ""
    }
}

if {[file readable $PPCDASM]} {
    puts -nonewline "Loading Ppcdasm ($PPCDASM)..."
    if {[catch {::load $PPCDASM}]} {
        puts stderr "Failed"
        # return
    } else {
        puts ""
    }
}

if {[file readable $ELFIOLOADER]} {
    puts -nonewline "Loading Elf Loader ($ELFIOLOADER)..."
    if {[catch {::load $ELFIOLOADER}]} {
        puts stderr "Failed"
        # return
    } else {
        puts ""
    }
}


proc showproc {proc} {
    set args [info args $proc]
    set arginfo [list]
    foreach arg $args {
        if {[info default $proc $arg def]} {
            lappend arginfo [list $arg $def]
        } else {
            lappend arginfo $arg
        }
    }
    puts -nonewline "proc $proc {$arginfo} {"
    puts -nonewline [info body $proc]
    puts "}"
}

proc showcfg {{nspace 0}} {

    if {[string is integer $nspace]} {
        set nspace [eval "set ::ccs::chain($nspace)"]
    }
    namespace eval $nspace {
        variable cindex
        variable cfg
        for {set i 0} {$i < $cindex} {incr i} {
            puts $cfg($i)
        }
    }
}
# Function name : help_ext
# Description: used when no extensions help file is available.
#     If an extensions help file is find in the release directory,
#     this function is be overwritten .
#     A demo help file for StarCore extensions resides at
#     ccs/ccs-common/bin/help.tcl . This file is sourced by the
#     autoexec.tcl from the trl8102 directory
#
proc help_ext {} {
    puts "No extensions help available."
}

foreach n {
    rnbo_sync rnbo_mcore rnbo_sc140 rnbo_ipcm sc140 hawkv2 mcore
    mcore340 dsp563 dsp566 dsp568 rcp_sync rcp_dsp566 rcp_mcore msc8102_sync
    msc8102_sc140 ms116 msfpga mgt5200 odyssey_sync odyssey_ms1162 zen
    mpc5500_sync mpc5500_zen mpc5500_npc mpc5500_ndedi mpc5500_nxdm
    mpc5500_etpu filter sjc msc8144sc msc8144tp skyeplus zas armetb mxc2
    skyedpa skyeslim msc815xsc
} {
    set fn "${ccsExecPath}/t${n}.tcl"
    if {[file readable $fn]} {

#        puts -nonewline "Initializing $n namespace..."

        namespace eval $n {
            variable gindex
            variable cindex
            variable cp

            set gindex 0
            set cindex 0
            set cp 0
            proc mkreg {name index} {
                variable reg
                set reg(${name}) ${index}
                set reg(${index}) ${name}
            }
            proc VarAccess {name1 name2 op} {
                set regname [namespace tail $name1]
                set regnamespace [namespace qualifiers $name1]
                if {[string match $op r]} {
                    set $name1 [ccs::read_reg [set ${regnamespace}::cp] $regname 1]
                } else {
                    ccs::write_reg [set ${regnamespace}::cp] $regname [set $name1]
                    set $name1 [ccs::read_reg [set ${regnamespace}::cp] $regname 1]
                }
            }
            proc addreg {name} {
                variable reg
                variable gindex
                upvar $name rname

                if {[info exists reg(${name})] == 0} {
                    set reg(${name})   ${gindex}
                    set reg(${gindex}) ${name}
                    if {0} {
                        #set rcmd "eval \{set $name \\\[::ccs::read_reg \$::ac $gindex 1\\\]\}"
                        set wcmd "::ccs::write_reg \$::ac $gindex \[set $name\]"
                        set rname "XXXX"
                        trace variable [namespace current]::$name rw "[namespace current]::VarAccess"
                        #trace variable [set ::[namespace current]::name] w {eval $wcmd}
                        #trace variable [set ::[namespace current]::name] r $rcmd
                    }
                    incr gindex
                }

            }
            proc addcfg {name} {
                variable cfg
                variable cindex

                if {![info exists cfg(${name})]} {
                    set cfg(${name}) ${cindex}
                    set cfg(${cindex}) ${name}
                    incr cindex
                }
            }
            ccssource t${n}.tcl
            if {[array exists Stat]} {
                foreach R $Stat(reg) {
                    for {set m 0} {$m < 32} {incr m} {
                        set StatLast${m}($R) X
                    }
                }
            }
        }

#        puts ""
    }
}

foreach n {
    mpc603e   mpc603ei  e500      mpc7410
    mpc74x7   mpc7447a  j2i2c     mpc83xx
    mpc755    arm11     draco8    mpc7448
    ce        sap       msc8144   coldfire
    mpc8641   sdma      mxc_sdma_filter
    arm       simdev    simcor    mxcsc
    j2i2cv2   mxc_zassc_filter    mpc8572
    mxc2_nosdma e500mc  msc815x   cortex_a8
} {
    set fn "${ccsExecPath}/t${n}.tcl"
    if {[file readable $fn]} {

#        puts -nonewline "Initializing $n namespace..."

        namespace eval $n {
            variable gindex
            variable cindex
            variable cp

            set gindex 0
            set cindex 0
            set cp 0
            proc mkreg {name index} {
                variable reg
                set reg(${name}) ${index}
                set reg(${index}) ${name}
            }
            proc VarAccess {name1 name2 op} {
                set regname [namespace tail $name1]
                set regnamespace [namespace qualifiers $name1]
                if {[string match $op r]} {
                    set $name1 [ccs::read_reg [set ${regnamespace}::cp] $regname 1]
                } else {
                    ccs::write_reg [set ${regnamespace}::cp] $regname [set $name1]
                    set $name1 [ccs::read_reg [set ${regnamespace}::cp] $regname 1]
                }
            }
            proc addreg {name gindex} {
                variable reg
                upvar $name rname

                set reg(${name}) ${gindex}
                set reg(${gindex}) ${name}
            }
            proc addcfg {name} {
                variable cfg
                variable cindex

                if {[info exists cfg(${name})] == 0} {
                    set cfg(${name}) ${cindex}
                    set cfg(${cindex}) ${name}
                    incr cindex
                }
            }
            proc addres {name gindex} {
                variable resource
                upvar $name rname

                set resource(${name}) ${gindex}
                set resource(${gindex}) ${name}
            }
            ccssource t${n}.tcl
            if {[array exists Stat]} {
                foreach R $Stat(reg) {
                    for {set m 0} {$m < 32} {incr m} {
                        set StatLast${m}($R) X
                    }
                }
            }
        }

#        puts ""
    }
}

proc showbits {val {bits 32}} {
    set str ""

    set boarderbg "darkblue"
    set boarderfg "blue"
    set textbg "x"
    set textfg "x"
    set bitnofg "yellow"

    set boarderbg "white"
    set boarderfg "white"
    set textbg "skyblue"
    set textfg "black"
    set bitnofg "red"

    if {$bits == 32} {
        set tlist {31 15}
        set blist {16 0}
    } elseif {$bits == 16} {
        set tlist {15}
        set blist {0}
    } elseif {$bits == 8} {
        set tlist {7}
        set blist {0}
    }
    foreach t $tlist b $blist {
        append str "<b=$boarderbg>"
        for {set i $t} {$i>=$b} {incr i -1} {
            append str [format "<f=$boarderfg>|<f=$bitnofg>%02d" $i]
        }
        append str "<f=$boarderfg>|<b=x>\n"
        for {set i $t} {$i>=$b} {incr i -1} {
            append str [format "<b=$boarderbg><f=$boarderfg>|<f=$textfg><b=$textbg> %d" [expr ($val >> $i) & 1]]
        }
        if {$b == 0} {
            append str "<b=$boarderbg><f=$boarderfg>|<b=x>\n"
        } else {
            append str "<b=$boarderbg><f=$boarderfg>|<b=x>\n<b=$boarderbg>"
            for {set i $t} {$i>=$b} {incr i -1} {
                append str "+--"
            }
            append str "+<b=x>\n"
        }
    }
    append str "<b=$boarderbg><f=$boarderfg>"
            for {set i $t} {$i>=$b} {incr i -1} {
                append str "+--"
            }
            append str "+<b=x>\n"
    append str "<f=x><b=x>"
    cputs -nonewline $str
}
proc display {args} {

    set arg_count [llength $args]

    ;# get chain configuration
    ::ccs::get_config_chain

    switch -glob [lindex $args 0] {
        *read_reg {
            if {$arg_count == 4} {
                ccs::display_reg [lindex $args 1] [lindex $args 2] \
                                 [lindex $args 3]
            } elseif {$arg_count == 5} {
                ccs::display_register [lindex $args 1] [lindex $args 2]\
                                      [lindex $args 3] [lindex $args 4]
            } else {
                ccs::read_reg
            }
        }
        *read_mem {
            if {$arg_count == 6} {
                ccs::display_mem [lindex $args 1] [lindex $args 2] \
                                 [lindex $args 3] [lindex $args 4] \
                                 [lindex $args 5]
            } elseif {$arg_count == 7} {
                ccs::display_memory [lindex $args 1] [lindex $args 2] \
                                    [lindex $args 3] [lindex $args 4] \
                                    [lindex $args 5] [lindex $args 6]
            } else {
                ccs::read_mem
            }
        }
        *read_piped_mem {
            if {[llength $args] == 6} {
                set val [ccs::read_piped_mem [lindex $args 1] [lindex $args 2] \
                                             [lindex $args 3] [lindex $args 4] \
                                             [lindex $args 5]]
            } elseif {$arg_count == 7} {
                set val [ccs::read_piped_mem [lindex $args 1] [lindex $args 2] \
                                             [lindex $args 3] [lindex $args 4] \
                                             [lindex $args 5] [lindex $args 6]]
            } else {
                ccs::read_piped_mem
                return
            }
            foreach value $val {
                puts [format "0x%08X" $value]
            }
        }
        *rtd_stat {
            if {[llength $args] != 2} {
                ccs::rtd_stat
            }
            ccs::display_rtd_stat [lindex $args 1]
        }
        *core_run_mode {
            if {[llength $args] != 2} {
                ccs::core_run_mode
            }
            ccs::display_core_run_mode [lindex $args 1]
        }
        *get_config_chain {
            if {[llength $args] != 1} {
                ccs::get_config_chain
            }
            ccs::display_get_config_chain
        }
        *template_version {
            if {[llength $args] != 2} {
                ccs::template_version
            }
            puts "Version: [ccs::template_version [lindex $args 1]]"
        }
        *get_template_config {
            if {[llength $args] != 3} {
                ccs::get_template_config
            }
            set value [eval $args]
            puts [format "0x%08X" $value]
        }
        *read_resource {
            if {[llength $args] != 6} {
                ccs::read_resource
            }
            ccs::display_resource [lindex $args 1] [lindex $args 2] \
                             [lindex $args 3] [lindex $args 4] \
                             [lindex $args 5]
        }
        *dra_read {
            set values [eval $args]
            foreach value $values {
                puts [format "0x%08X" $value]
            }
        }
        default {
            set values [eval $args]
            puts $values
            #foreach value $values {
            #    puts [format "0x%08X" $value]
            #}
        }
    }
}

namespace eval ccs {
    #proc gofast {} {
    #wig::write_large_memory 2 0x0020c000 2 0 0x0002
    #wig::set_speed 1
    #}

    proc mkcore {name index} {
        variable core
        set core(${name}) ${index}
        set core(${index}) ${name}
    }

    mkcore generic         0
    mkcore dsp563          1
    mkcore dsp566          2
    mkcore dsp568          3
    mkcore mcore           4
    mkcore arm             5
    mkcore sc140           6
    mkcore mcore340        7
    mkcore ipcm            8
    mkcore hawkv2          9
    mkcore rcp_sync       10
    mkcore rcp_dsp566     11
    mkcore rcp_mcore      12
    mkcore wcp_sync       13
    mkcore wcp_dsp566     14
    mkcore wcp_arm7tdmi   15
    mkcore rnbo_sync      16
    mkcore rnbo_sc140     17
    mkcore rnbo_mcore     18
    mkcore rnbo_ipcm      19
    mkcore testcore       20
    mkcore msc8102_sync   21
    mkcore msc8102_sc140  22
    mkcore elf            23
    mkcore etpu           24
    mkcore coldfire       25
    mkcore esys_sync      26
    mkcore esys_elf       27
    mkcore esys_etpu0     28
    mkcore esys_etpu1     29
    mkcore esys_ace       30
    mkcore mpc603e        31
    mkcore mpc603ei       32
    mkcore ms116          33
    mkcore msfpga         34
    mkcore e500           35
    mkcore mgt5200        36
    mkcore mpc7410        37
    mkcore mpc74x7        38
    mkcore odyssey_sync   39
    mkcore odyssey_ms1162 40
    mkcore mpc5500_sync   41
    mkcore mpc5500_zen    42
    mkcore mpc5500_ndedi  43
    mkcore mpc5500_npc    44
    mkcore mpc5500_nxdm   45
    mkcore mpc7447a       46
    mkcore mpc5500_etpu   47
    mkcore j2i2c          48
    mkcore filter         49
    mkcore cfv3_deprecated 50
    mkcore cfv4_deprecated 51
    mkcore ce             52
    mkcore draco8         53
    mkcore mpc866         54
    mkcore mpc83xx        55
    mkcore mpc755         56
    mkcore arm11          57
    mkcore sjc            58
    mkcore sdma           59
    mkcore j2i2cs         60
    mkcore zassc          61
    mkcore svf            62
    mkcore mrc6800_zen    63
    mkcore mrc6800_rcf    64
    mkcore mpc7448        65
    mkcore sap            74
    mkcore msc8144        75
    mkcore e700           76
    mkcore msc8144sc      77
    mkcore msc8144tp      78
    mkcore mpc8641        79
    mkcore mpc8641d       80
    mkcore mpc8641core    81
    mkcore mpc8641sap     82
    mkcore skyeplus       83
    mkcore armetb         84
    mkcore simdev         85
    mkcore simcor         86
    mkcore mpc832x_sim    87
    mkcore mpc83xx_sim_tm 88
    mkcore mpc8568_sim    89
    mkcore mxcsc          90
    mkcore arm9sim        91
    mkcore arm11sim       92
    mkcore sne            93
    mkcore j2i2cv2        94
    mkcore zas            95
    mkcore ti_paccp       96
    mkcore mpc5121        97
    mkcore axe            98
    mkcore mpc8610        99
    mkcore mpc8610core    100
    mkcore mpc8610sap     101
    mkcore mpc8572        102
    mkcore marley         103
    mkcore marley_sc      104
    mkcore mxc2           105
	mkcore sc3450plat_iss 106
	mkcore sc3450plat_rsg 107
	mkcore sc3450plat_pacc 108
	mkcore sc3850plat_iss 109
	mkcore sc3850plat_rsg 110
	mkcore sc3850plat_pacc 111
    mkcore mxc2_nosdma    112
    mkcore p2002hp        113
    mkcore skyedpa        114
    mkcore skyeslim       115
    mkcore titanium2_qe   116
    mkcore e700core       117
    mkcore mpc8578        118
    mkcore e500mc         119
    mkcore mpc8578_iss    120
    mkcore mpc8578_pacc   121
    mkcore msc815x        122
    mkcore msc815xsc      123
    mkcore cortex_a8      124

    proc mkmode {name index} {
        variable mode
        set mode(${name}) ${index}
        set mode(${index}) ${name}
    }

    mkmode debug     0
    mkmode execute   1
    mkmode sleep     2
    mkmode waitstate 3

    proc rcp_config_chain {} {
        variable core
        set ::top 0
        set ::dsp 1
        set ::mcu 2
        set ::mc 2
        config_chain [list $core(rcp_sync) $core(rcp_dsp566) $core(rcp_mcore)]
    }

    proc wcp_config_chain {} {
        variable core
        config_chain [list $core(wcp_sync) $core(wcp_dsp56600) $core(wcp_arm7tdmi)]
    }

    proc rnbo_config_chain {} {
        variable core
        set ::ed 0
        set ::sc 1
        set ::mc 2
        set ::ip 3
        set ::top 0
        set ::dsp 1
        set ::mcu 2
        set ::dma 3
        config_chain [list $core(rnbo_sync) $core(rnbo_sc140) $core(rnbo_mcore) $core(rnbo_ipcm)]
    }

    proc msc8102_config_chain {} {
        variable core
        set ::top 0
        set ::sc0 1
        set ::sc1 2
        set ::sc2 3
        set ::sc3 4
        config_chain [list $core(msc8102_sync) $core(msc8102_sc140) $core(msc8102_sc140) $core(msc8102_sc140) $core(msc8102_sc140)]
    }

    proc mpc5500_config_chain {} {
        variable core
        set ::top   0
        set ::zen   1
        set ::ndedi 2
        set ::etpu  3
        set ::etpu1 3
        set ::etpu2 4
        set ::npc   5
        set ::nxdm  6
        config_chain [list $core(mpc5500_sync) $core(mpc5500_zen) $core(mpc5500_ndedi) $core(mpc5500_etpu) $core(mpc5500_etpu) $core(mpc5500_npc) $core(mpc5500_nxdm)]
    }

    proc display_core_run_mode {chain_pos} {
        variable chain
        variable mode
        set modenum [core_run_mode $chain_pos]
        puts "$chain($chain_pos): $mode($modenum)"
    }

    proc display_get_config_chain {} {
        variable chain
        variable core
        set i 0

        set lchain [get_config_chain]
        if {![llength $lchain]} {
            puts "Chain not configured"
            return
        }
        foreach dev $lchain {
	    set core_type [ccs::parse_core_type $i]
            puts "Chain Position $i: $core_type";#$core($dev)"
            ;#set chain($i) $core($dev)
            incr i
        }
    }

    proc display_rtd_stat {chain_pos} {
        variable chain
        set stat [rtd_stat $chain_pos]
        puts "$chain($chain_pos): available to read: [lindex $stat 0], left to write: [lindex $stat 1]"
    }

    #display ccs::read_reg in a nice way
    proc display_reg {chain_pos reg_index count} { 
        variable chain       
        set columns 4
        set maxpad 7
        set vals [read_reg $chain_pos $reg_index $count]
        if {![string is integer $reg_index]} {
            set nspace [eval "set chain($chain_pos)"]
            set reg_index [eval "set ::${nspace}::reg($reg_index)"]
        }
        set regnum $reg_index
        set j 0
        foreach val $vals {
            set regname ""
            catch {
                set nspace [eval "set chain($chain_pos)"]
                set regname [eval "set ::${nspace}::reg($regnum)"]
            }
            if {![string length $regname]} {
                set regname "reg.[expr ${regnum}]"
            }
            set padlen [expr $maxpad - [string length $regname]]
            set padstr ""
            incr j
            if {$padlen < 0} {

            } else {
                for {set i 0} {$i < $padlen} {incr i} {
                    append padstr " "
                }          
            }
            puts -nonewline [format "%s%s=0x%08X" $padstr $regname $val]
            if {!($j % $columns)} {
               puts ""     
            } else {
               puts -nonewline " "
            }
            incr regnum
        }
        if {$j % $columns} {
            puts ""
        }
    }

    proc display_resource {chain_pos res_group addr count size_in_bits} {
        variable chain
        set vals [read_resource $chain_pos $res_group $addr $count $size_in_bits]
        set nspace [eval "set chain($chain_pos)"]
        if {![string is integer $addr]} {
            set addr [eval "set ::${nspace}::resource($addr)"]
        }

        set ressize [expr ($size_in_bits + 7) / 8]
        set num_pre_fmt [expr $ressize % 4]
        switch -glob $num_pre_fmt {
            0 {
                set prefmtstr " "
                set use_pre_fmt 0
            }
            1 {
                set prefmtstr " %02X"
                set use_pre_fmt 1
            }
            2 {
                set prefmtstr " %04X"
                set use_pre_fmt 1
            }
            3 {
                set prefmtstr " %06X"
                set use_pre_fmt 1
            }
        }
        set fmtstr "%08X"
        set num_in_loop [expr $ressize / 4]
        set chars_per_unit [expr ($num_pre_fmt * 2) + ($num_in_loop * 8) + 1]
        set units_per_line [expr (80 - 13) / $chars_per_unit]
        set vals_per_unit [expr $use_pre_fmt + $num_in_loop]

        for {set cur_unit 0} {$cur_unit < $count} {} {
            if {!($cur_unit % $units_per_line)} {
                puts -nonewline [format "\n\[0x%08X\]" [expr $addr + $cur_unit]]
            }
            if {$use_pre_fmt == 0} {
                puts -nonewline $prefmtstr
            } else {
                puts -nonewline [format $prefmtstr [lindex $vals [expr $cur_unit * $vals_per_unit]]]
            }
            for {set i 0} {$i < $num_in_loop} {incr i} {
                puts -nonewline [format $fmtstr [lindex $vals [expr ($cur_unit * $vals_per_unit) + $i + $use_pre_fmt]]]
            }
            incr cur_unit
            if {!($cur_unit % $units_per_line)} {
                puts ""
                puts -nonewline {            }
            }
        }
        puts ""
    }

    proc display_register {chain_pos reg_index count size} {
        variable chain
        set columns 4
        set maxpad 7
        set vals [read_reg $chain_pos $reg_index $count $size]
        if {![string is integer $reg_index]} {
            set nspace [eval "set chain($chain_pos)"]
            set reg_index [eval "set ::${nspace}::reg($reg_index)"]
        }
        set regnum $reg_index
        set j 0
        foreach val $vals {
            set regname ""
            catch {
                set nspace [eval "set chain($chain_pos)"]
                set regname [eval "set ::${nspace}::reg($regnum)"]
            }
            if {![string length $regname]} {
                set regname "reg.[expr ${regnum}]"
            }
            set padlen [expr $maxpad - [string length $regname]]
            set padstr ""
            incr j
            if {$padlen < 0} {

            } else {
                for {set i 0} {$i < $padlen} {incr i} {
                    append padstr " "
                }
            }
            puts -nonewline [format "%s%s=0x%08X" $padstr $regname $val]
            if {!($j % $columns)} {
               puts ""
            } else {
               puts -nonewline " "
            }
            incr regnum
        }
        if {$j % $columns} {
            puts ""
        }
    }

    # display_mem ------------------------------------------------------------
    # display ccs::read_mem in a nice way
    proc display_mem {chain_pos addr size space count} {
        variable chain
        set vals [read_mem $chain_pos $addr $size $space $count]
        set nspace [eval "set chain($chain_pos)"]
        set msize $size
        puts -nonewline {            }
        if {$nspace == "dsp568" || $nspace == "hawkv2"} {
            for {set i 0} {$i<8} {incr i} {
               puts -nonewline [format "   +%X" $i]
            }
            set fmtstr " %04X"
            set msize 1
        } elseif {$size == 1} {
            for {set i 0} {$i<16} {incr i} {
               puts -nonewline [format " +%X" $i]
            }
            set fmtstr " %02X"
        } elseif {$size == 2} {
            for {set i 0} {$i<16} {incr i 2} {
               puts -nonewline [format "   +%X" $i]
            }
            set fmtstr " %04X"
        } elseif {$size == 8} {
            for {set i 0} {$i<16} {incr i 8} {
               puts -nonewline [format "               +%X" $i]
            }
            set fmtstr " %08X%08X"
        } else {
            for {set i 0} {$i<16} {incr i 4} {
               puts -nonewline [format "       +%X" $i]
            }
            set fmtstr " %08X"
        }
        for {set i 0} {$i < $count} {incr i} {
            if {!($i%(16/$size))} {
                puts -nonewline [format "\n\[0x%08X\]" [expr $addr+($i*$msize)]]
            }
            if {$size == 8} {
                puts -nonewline [format $fmtstr [lindex $vals [expr $i * 2]] [lindex $vals [expr [expr $i * 2] + 1]]]
            } else {
                puts -nonewline [format $fmtstr [lindex $vals $i]]
            }
        }
        puts ""
    }

    proc display_memory {chain_pos addr_hi addr size space count} {
        variable chain
        set vals [read_mem $chain_pos $addr_hi $addr $size $space $count]
        set nspace [eval "set chain($chain_pos)"]
        set msize $size
        puts -nonewline {                    }
        if {$nspace == "dsp568" || $nspace == "hawkv2"} {
            for {set i 0} {$i<8} {incr i} {
               puts -nonewline [format "   +%X" $i]
            }
            set fmtstr " %04X"
            set msize 1
        } elseif {$size == 1} {
            for {set i 0} {$i<16} {incr i} {
               puts -nonewline [format " +%X" $i]
            }
            set fmtstr " %02X"
        } elseif {$size == 2} {
            for {set i 0} {$i<16} {incr i 2} {
               puts -nonewline [format "   +%X" $i]
            }
            set fmtstr " %04X"
        } elseif {$size == 8} {
            for {set i 0} {$i<16} {incr i 8} {
               puts -nonewline [format "               +%X" $i]
            }
            set fmtstr " %08X%08X"
        } else {
            for {set i 0} {$i<16} {incr i 4} {
               puts -nonewline [format "       +%X" $i]
            }
            set fmtstr " %08X"
        }
        set current_addr $addr
        set current_addr_hi $addr_hi
        for {set i 0} {$i < $count} {incr i} {
            if {!($i%(16/$size))} {
                puts -nonewline [format "\n\[0x%08X%08X\]" $current_addr_hi $current_addr]
                set new_addr [expr $addr+$msize]
                if {$new_addr < $current_addr} {
                    incr current_addr_hi
                }
                set current_addr $new_addr
            }
            if {$size == 8} {
                puts -nonewline [format $fmtstr [lindex $vals [expr $i * 2]] [lindex $vals [expr [expr $i * 2] + 1]]]
            } else {
                puts -nonewline [format $fmtstr [lindex $vals $i]]
            }
        }
        puts ""
    }

    # get_config_chain_2 -----------------------------------------------------
    proc get_config_chain_2 {} {
        variable chain
        set cores [original_get_config_chain]
        set i 0
        foreach core $cores {
            set chain($i) $::ccs::core($core)

	    # if this is a filtered core, query the device for the filter name
	    if {$chain($i) == "filter"} {
		set chain($i) [lindex [split [ccs::parse_core_type $i] :] 0]
	    }
            incr i
        }
        return $cores
    }

    # config_chain_2 ---------------------------------------------------------
    # new config_chain which keeps track of core namespaces
    proc config_chain_2 {core_names} {

        # config chain -------------------------------------------------------
        variable chain
        set cores [list]
        foreach core $core_names {
            if {[string is integer $core]} {
                # core index
                lappend cores $core
            } elseif {[llength $core] == 1} {
                # core string
                lappend cores $::ccs::core($core)
            } else {
                # generic device
                lappend cores $core
            }
        }
        set result [original_config_chain $cores]

        # set ::ccs::chain array ---------------------------------------------
        get_config_chain

	return $result
    }

    # read_reg_2 -------------------------------------------------------------
    proc read_reg_2 {chain_pos reg_index count {reg_size -1}} {
        variable chain

        if {![string is integer $reg_index]} {
            set nspace [eval "set chain($chain_pos)"]
            set reg_index [eval "set ::${nspace}::reg($reg_index)"]
        }
        if {$reg_size == -1} {
            return [original_read_reg $chain_pos $reg_index $count]
        } else {
            return [original_read_reg $chain_pos $reg_index $count $reg_size]
        }
    }

    # in_2 -------------------------------------------------------------------
    proc in_2 {chain_pos byte_size reg_or_mem reg_index {count 1}} {
        variable chain

        if {![string is integer $reg_index]} {
            set nspace [eval "set chain($chain_pos)"]
            set reg_index [eval "set ::${nspace}::reg($reg_index)"]
        }
        return [original_in $chain_pos $byte_size $reg_or_mem $reg_index $count]
    }

# if reg_list is -1, then there is no size, and reg_size contains the
# data to write.
    proc write_reg_2 {chain_pos reg_index {reg_size "invalid"} {reg_list "invalid"}} {
        variable chain

        if {![string is integer $reg_index]} {
            set nspace [eval "set chain($chain_pos)"]
            set reg_index [eval "set ::${nspace}::reg($reg_index)"]
        }
        if {$reg_size == "invalid"} {
            return [write_reg]
        }
        if {$reg_list == "invalid"} {
            return [original_write_reg $chain_pos $reg_index $reg_size]
        } else {
            return [original_write_reg $chain_pos $reg_index $reg_size $reg_list]
        }
    }

    proc out_2 {chain_pos value reg_or_mem reg_index {count 1}} {
        variable chain

        if {![string is integer $reg_index]} {
            set nspace [eval "set chain($chain_pos)"]
            set reg_index [eval "set ::${nspace}::reg($reg_index)"]
        }
        return [original_out $chain_pos $value $reg_or_mem $reg_index $count]
    }

    # read_resource_2---------------------------------------------------------
    proc read_resource_2 {chain_pos res_group addr count size} {
        variable chain

        if {![string is integer $addr]} {
            set nspace [eval "set chain($chain_pos)"]
            set addr [eval "set ::${nspace}::resource($addr)"]
        }
        return [original_read_resource $chain_pos $res_group $addr $count $size]
    }

    # write_resource_2--------------------------------------------------------
    proc write_resource_2 {chain_pos res_group addr size res_list} {
        variable chain

        if {![string is integer $addr]} {
            set nspace [eval "set chain($chain_pos)"]
            set addr [eval "set ::${nspace}::resource($addr)"]
        }
        return [original_write_resource $chain_pos $res_group $addr $size $res_list]
    }

    proc config_template_2 {chain_pos index value} {
        variable chain
        if  {![string is integer $index]} {
            set nspace [eval "set chain($chain_pos)"]
            set index [eval "set ::${nspace}::cfg($index)"]
        }
        return [original_config_template $chain_pos $index $value]
    }

    proc get_template_config_2 {chain_pos index} {
        variable chain
        if  {![string is integer $index]} {
            set nspace [eval "set chain($chain_pos)"]
            set index [eval "set ::${nspace}::cfg($index)"]
        }
        return [original_get_template_config $chain_pos $index]
    }

    proc view_messages {chain_pos} {
        return [get_messages $chain_pos 5318008]
    }

    # Rename procedures if necessary
    if {!$ccssim2 && ![info exists procsRenamed]} {
        set procsRenamed 0
        rename config_chain original_config_chain
        rename config_chain_2 config_chain

        rename get_config_chain original_get_config_chain
        rename get_config_chain_2 get_config_chain

        rename read_reg original_read_reg
        rename read_reg_2 read_reg

        rename in original_in
        rename in_2 in

        rename write_reg original_write_reg
        rename write_reg_2 write_reg

        rename out original_out
        rename out_2 out

        rename read_resource original_read_resource
        rename read_resource_2 read_resource

        rename write_resource original_write_resource
        rename write_resource_2 write_resource

        rename config_template original_config_template
        rename config_template_2 config_template

        rename get_template_config original_get_template_config
        rename get_template_config_2 get_template_config
    }

    proc show_connections {} {
        variable clients
        variable bytesSent
        variable bytesRecv
        variable connectTime
        set i 0
        set currentTime [clock seconds]
        foreach client $clients sent $bytesSent recv $bytesRecv time $connectTime {
            incr i
            set deltaT [expr $currentTime-$time]
            set duration [format "%02d:%02d:%02d" [expr $deltaT/(60*60)] [expr $deltaT/60] [expr $deltaT%60]]
            puts "#$i: $client connected for $duration, [expr $sent+$recv] bytes transferred"
        }
    }

    proc step {{chain_pos 0}} {
        variable chain
        ::ccs::get_config_chain
        set nspace [eval "set chain($chain_pos)"]
        array set Stat [array get "::${nspace}::Stat"]
        set runmode [::ccs::core_run_mode $chain_pos]
        if {$runmode != 0} {
            puts "$Stat(core) in $::ccs::mode($runmode) mode"
            return
        }
        ::ccs::step_core $chain_pos 1
        stat $chain_pos
    }

    proc stat {{chain_pos 0}} {
        variable chain
        variable statList
        ::ccs::get_config_chain
        set nspace [eval "set chain($chain_pos)"]
        set runmode [::ccs::core_run_mode $chain_pos]
        array set Stat [array get "::${nspace}::Stat"]
        array set StatLast [array get "::${nspace}::StatLast${chain_pos}"]
        puts -nonewline ""
        cputs "<f=darkgreen>{ $chain_pos : $Stat(core) }<f=x>"
        if {$runmode != 0} {
            cputs "$::ccs::mode($runmode) mode"
            return
        }
        foreach r $Stat(reg) s $Stat(size) {
            set $r [::ccs::read_reg $chain_pos $r 1]
            if {[string is integer $StatLast($r)] && $StatLast($r) != [set $r]} {
                set formatstr "<f=red>0x%0${s}X<f=x>"
            } else {
                set formatstr "0x%0${s}X"
            }
            set fmt(${r}) [format $formatstr [set $r]]
            set "::${nspace}::StatLast${chain_pos}($r)" [set $r]
        }
        if {![info exists Stat(memspace)]} {
            set Stat(memspace) 0
        }
        eval "set foutput \"$Stat(fmt)\""
        cputs -nonewline "$foutput"
        if {[info exists pc]} {
            set meml [ccs::read_mem $chain_pos $pc $Stat(inst) $Stat(memspace) $Stat(icnt)]
            set i 0
            cputs -nonewline " \["
            foreach dat $meml {
                if {$i} {
                    cputs -nonewline " "
                }
                incr i
                cputs -nonewline [format "%0[expr $Stat(inst)*2]X" $dat]
            }
            cputs -nonewline "\]"
            set intpc [expr $pc]
            if {[info exist statList(${chain_pos},${intpc})]} {
                cputs "$statList(${chain_pos},${intpc})"
            }
        }
        puts ""
    }

    array set statList [list]
    proc uselst {chain_pos lstfile} {
        variable statList
        array set statList [list]
        set pcint -1
        set fp [open $lstfile "r"]
        while {![eof $fp]} {
            set line [gets $fp]
            set asmline [regexp {^([0-9]+) +P:(........) ([0-9A-F]+) +(.*)} $line mVar lnum pc opcode asm]
            set hexline [regexp {^ +([0-9]+) *$} $line mVar operand]
            if {$asmline} {
                set pcint [expr "0x${pc}"]
                regsub -all { +} $asm { } asm ;# collapse all spaces
                set statList(${chain_pos},${pcint}) "<f=yellow><b=darkblue> [file tail $lstfile]#${lnum} <b=x><f=darkgreen> ${asm} => ${opcode}<f=x>"
            } elseif {$hexline && ($pcint != -1)} {
                append statList(${chain_pos},${pcint}) " <f=darkgreen>${operand}<f=x>"
            }
        }
        close $fp
    }

    proc load {chain_pos lstfile} {
        variable chain
        variable statList
        ::ccs::get_config_chain
        set nspace [eval "set chain($chain_pos)"]
        array set Stat [array get "::${nspace}::Stat"]
        if {![llength [namespace eval ::${nspace} {info procs load}]]} {
            cputs "$Stat(core) does not support load"
            return
        }
        set runmode [::ccs::core_run_mode $chain_pos]
        if {$runmode != 0} {
            cputs "$Stat(core) in $::ccs::mode($runmode) mode"
            return
        }
        ::${nspace}::load $chain_pos $lstfile
    }

    proc all_run_mode {} {
        set lchain [get_config_chain]
        set i 0
        foreach core $lchain {
            display core_run_mode $i
            incr i
        }
        if {!$i} {
            puts "Chain not configured"
        }
    }

    proc datlist {length} {
        for {set i 0} {$i<$length} {incr i} {
            lappend dat [expr 0xDA7A0000 + $i]
        }
        return $dat
    }
    proc download {file chain_pos addr size} {
        set fp [open $file r]
        set data [read $fp]
        close $fp
        ccs::write_mem $chain_pos $addr $size 0 $data
        set bytes [expr [llength $data]*$size]
        puts "$bytes bytes written"
    }

    proc dumpParam {} {
        variable param
        if {![array exists param]} {
            puts "param is not an array"
            return 0
        }
        foreach {name value} [array get param] {
            puts "$value param->$name"
        }
        return 0
    }

    proc dumpReply {} {
        variable reply
        if {![array exists reply]} {
            puts "reply is not an array"
            return 0
        }
        foreach {name value} [array get ::reply] {
            puts "$value reply->$name"
        }
        return 0
    }
    proc initEnum {varBase procBase enumList} {
        variable enumName_${varBase}
        variable enumIndex_${varBase}
        catch {unset enumName_${varBase}}
        catch {unset enumIndex_${varBase}}
        set i 0
        foreach name $enumList {
            set enumName_${varBase}($i) $name
            set enumIndex_${varBase}($name) $i
            incr i
        }
        set newProcs [subst {
            proc get${procBase}Name {index} {
                return \[getEnumName $varBase \$index\]
            }
            proc get${procBase}Index {name} {
                return \[getEnumIndex $varBase \$name\]
            }
        }]
        eval $newProcs
        namespace export get${procBase}Name
        namespace export get${procBase}Index
        if {0} {
        namespace eval :: [list namespace import -force [namespace current]::get${procBase}Name]
        namespace eval :: [list namespace import -force [namespace current]::get${procBase}Index]
        }
    }

    proc getEnumName {base index} {
        variable enumName_${base}
        if {[info exists enumName_${base}($index)]} {
            return [set enumName_${base}($index)]
        }
        return ""
    }
    proc getEnumIndex {base name} {
        variable enumIndex_${base}
        foreach casename [list $name [string toupper $name] [string tolower $name]] {
            if {[info exists enumIndex_${base}($casename)]} {
                return [set enumIndex_${base}($casename)]
            }
        }
        return ""
    }

    # ccsapi/ccsio enumerations
    initEnum ccs_command Cmd {
        SETUP_CC
        DELETE_CC
        SERVER_VERSION
        CC_VERSION
        AVAILABLE_CONNECTIONS
        CONFIG_CHAIN
        SET_MAX_TCK
        TEMPLATE_VERSION
        CONFIG_TEMPLATE
        CORE_RUN_MODE
        RESET_TO_DEBUG
        RESET_TO_USER
        RESET_TO_MIXED
        READ_MEM
        WRITE_MEM
        FILL_MEM
        READ_REG
        IN
        WRITE_REG
        OUT
        RUN_CORE
        RUN_MULTI_CORE
        STOP_CORE
        STOP_MULTI_CORE
        STEP_CORE
        STEP_MULTI_CORE
        RTD_READ
        RTD_WRITE
        RTD_STAT
        RESET_INTERFACE
        SERVER_ALIVE
        KILL_SERVER
        UNSUPPORTED_0
        UNSUPPORTED_1
        UNSUPPORTED_2
        UNSUPPORTED_3
        GET_CONFIG_CHAIN
        SEND_MESSAGE
        GET_MESSAGES
        CHECK_MIN_VERSION
        UPDATE_NHTI
        GET_SPEED
        READ_PIPED_REG
        UNSUPPORTED_4
        UNSUPPORTED_5
        STRCMD
        SUPPORTED_CONNECTIONS
        HTI_GET_CRC32
        HTI_SET_CRC32
        COUNT
    }

    initEnum ccs_error Error {
        ok
        unimplemented
        network_timeout
        network_fault
        cable_disconnected
        core_not_responding
        invalid_parameter
        invalid_handle
        library_not_found
        core_not_in_debug
        connection_refused
        max_cc_reached
        api_below_min_ver
        ccs_below_min_ver
        invalid_sequence_number
        download_invalid_length
        download_invalid_checksum
        download_invalid_srec
        cc_communications_timeout
        cc_communications_fault
        rtd_buffer_full
        internal_failure
        data_overflow
        cc_not_present
        cc_driver_failure
        usb_open_failure
        usb_not_configured
        usb_transfer_failure
        usb_receive_wait
        usb_receive_reply
        usb_transmit_setup
        usb_transmit_command
        usb_download_setup
        usb_download_checksum
        usb_download_data
        usb_download_wait
        usb_restart_failure
        count
    }

    initEnum ccs_mode Mode {
        debug
        execute
        sleep
        waitstate
        undefined
    }

    namespace export config_chain
    namespace export config_template
    namespace export get_template_config
    namespace export core_run_mode
    namespace export read_mem
    namespace export read_reg
    namespace export in
    namespace export reset_to_debug
    namespace export reset_to_user
    namespace export run_core
    namespace export stop_core
    namespace export write_mem
    namespace export write_reg
    namespace export out
    namespace export download
}

proc call {filename} {
    set fullpath "${::ccsExecPath}/${filename}"
    puts "Calling ${fullpath}..."
    set error [catch {uplevel #0 "source \{${fullpath}\}"}]
    if {$error} {
        puts "Error calling (${fullpath})"
    }
}

proc makelist {varname length} {
    upvar $varname foo
    set foo [list]
    for {set i 0} {$i < $length} {incr i} {
        lappend foo $i
    }
    return
}

proc repeat {count script} {
    if {$count < 1} {
        return
    }
    for {set i 0} {$i < $count} {incr i} {
        uplevel 1 $script
    }
    return
}

# The next three functions enable displaying ETAP console messages in the CCS
# window.
proc get_etap_console_data {} {
    variable etap_console_socket
    if {![eof $etap_console_socket]} {
	set line [gets $etap_console_socket]
	tkcon console insert output "$line\n" "etap_console"
	tkcon console see output
	update idletasks
    }
}

proc disable_etap_console {} {
    variable etap_console_socket
    if {[info exists etap_console_socket]} {
	catch {fileevent $etap_console_socket readable ""}
	catch {close $etap_console_socket}
	unset etap_console_socket
    }
}

proc enable_etap_console {hostname} {
    variable etap_console_socket

    disable_etap_console

    tkcon console tag configure "etap_console" \
	-foreground LightGray -background black

    set etap_console_socket [socket $hostname 23]
    fconfigure $etap_console_socket -blocking false
    fileevent $etap_console_socket readable get_etap_console_data
}

#
# if autoexec.tcl is found in current directory
# source the it, otherwise source the autoexec from the execpath
#

set ui_common ${ccsExecPath}/ui_common.tcl
if {[file readable ${ui_common}]} {
    ccssource ${ui_common}
}

ccssource autoexec.tcl
#set autoexec autoexec.tcl
#if {[file readable ${autoexec}]} {
#    puts "Executing ${autoexec}..."
#    source ${autoexec}
#} else {
#    set autoexec ${ccsExecPath}/autoexec.tcl
#    if {[file readable ${autoexec}]} {
#        puts "Executing ${autoexec}..."
#        source ${autoexec}
#    }
#}

if {[info exists command_line_file]} {
    ccssource ${command_line_file}
#    if {[file readable ${command_line_file}]} {
#        puts "Executing ${command_line_file}..."
#        source ${command_line_file}
#    }
}
