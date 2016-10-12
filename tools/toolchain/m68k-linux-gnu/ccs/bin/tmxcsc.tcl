#Create all the registers
addreg d0           [expr   0]
addreg d0_e         [expr   1]
addreg d1           [expr   2]
addreg d1_e         [expr   3]
addreg d2           [expr   4]
addreg d2_e         [expr   5]
addreg d3           [expr   6]
addreg d3_e         [expr   7]
addreg d4           [expr   8]
addreg d4_e         [expr   9]
addreg d5           [expr  10]
addreg d5_e         [expr  11]
addreg d6           [expr  12]
addreg d6_e         [expr  13]
addreg d7           [expr  14]
addreg d7_e         [expr  15]
addreg d8           [expr  16]
addreg d8_e         [expr  17]
addreg d9           [expr  18]
addreg d9_e         [expr  19]
addreg d10          [expr  20]
addreg d10_e        [expr  21]
addreg d11          [expr  22]
addreg d11_e        [expr  23]
addreg d12          [expr  24]
addreg d12_e        [expr  25]
addreg d13          [expr  26]
addreg d13_e        [expr  27]
addreg d14          [expr  28]
addreg d14_e        [expr  29]
addreg d15          [expr  30]
addreg d15_e        [expr  31]
addreg r0           [expr  32]
addreg r1           [expr  33]
addreg r2           [expr  34]
addreg r3           [expr  35]
addreg r4           [expr  36]
addreg r5           [expr  37]
addreg r6           [expr  38]
addreg r7           [expr  39]
addreg r8           [expr  40]
addreg r9           [expr  41]
addreg r10          [expr  42]
addreg r11          [expr  43]
addreg r12          [expr  44]
addreg r13          [expr  45]
addreg r14          [expr  46]
addreg r15          [expr  47]
addreg n0           [expr  48]
addreg n1           [expr  49]
addreg n2           [expr  50]
addreg n3           [expr  51]
addreg m0           [expr  52]
addreg m1           [expr  53]
addreg m2           [expr  54]
addreg m3           [expr  55]
addreg sa0          [expr  56]
addreg sa1          [expr  57]
addreg sa2          [expr  58]
addreg sa3          [expr  59]
addreg lc0          [expr  60]
addreg lc1          [expr  61]
addreg lc2          [expr  62]
addreg lc3          [expr  63]
addreg sr           [expr  64]
addreg sp           [expr  65]
addreg osp          [expr  66]
addreg mctl         [expr  67]
addreg emr          [expr  68]
addreg vba          [expr  69]
addreg pctl0        [expr  70]
addreg pctl1        [expr  71]
addreg pc           [expr  72]
addreg idcode       [expr  73]
addreg esr          [expr  74]
addreg emcr         [expr  75]
addreg ercv_lo      [expr  76]
addreg ercv_hi      [expr  77]
addreg etrsmt_lo    [expr  78]
addreg etrsmt_hi    [expr  79]
addreg ee_ctrl      [expr  80]
addreg pc_excp      [expr  81]
addreg pc_next      [expr  82]
addreg pc_last      [expr  83]
addreg pc_detect    [expr  84]
addreg edca0_ctrl   [expr  85]
addreg edca1_ctrl   [expr  86]
addreg edca2_ctrl   [expr  87]
addreg edca3_ctrl   [expr  88]
addreg edca4_ctrl   [expr  89]
addreg edca5_ctrl   [expr  90]
addreg edca0_refa   [expr  91]
addreg edca1_refa   [expr  92]
addreg edca2_refa   [expr  93]
addreg edca3_refa   [expr  94]
addreg edca4_refa   [expr  95]
addreg edca5_refa   [expr  96]
addreg edca0_refb   [expr  97]
addreg edca1_refb   [expr  98]
addreg edca2_refb   [expr  99]
addreg edca3_refb   [expr 100]
addreg edca4_refb   [expr 101]
addreg edca5_refb   [expr 102]
addreg edca0_mask   [expr 103]
addreg edca1_mask   [expr 104]
addreg edca2_mask   [expr 105]
addreg edca3_mask   [expr 106]
addreg edca4_mask   [expr 107]
addreg edca5_mask   [expr 108]
addreg edcd_ctrl    [expr 109]
addreg edcd_ref     [expr 110]
addreg edcd_mask    [expr 111]
addreg ecnt_ctrl    [expr 112]
addreg ecnt_val     [expr 113]
addreg ecnt_ext     [expr 114]
addreg esel_ctrl    [expr 115]
addreg esel_dm      [expr 116]
addreg esel_di      [expr 117]
addreg esel_etb     [expr 118]
addreg esel_dtb     [expr 119]
addreg tb_ctrl      [expr 120]
addreg tb_rd        [expr 121]
addreg tb_wr        [expr 122]
addreg tb_buff      [expr 123]
addreg btr0         [expr 124]
addreg btr1         [expr 125]
addreg gcr          [expr 126]
addreg idcr         [expr 127]
addreg pc_mmu       [expr 0x1000    + 0x0000]
addreg corerev      [expr 0x1000    + 0x0001]
addreg corecfg      [expr 0x1000    + 0x0002]
addreg ecfg_lsb     [expr 0x1000    + 0x0003]
addreg ecfg_msb     [expr 0x1000    + 0x0004]
addreg edca0_refid  [expr 0x1000    + 0x0005]
addreg edca1_refid  [expr 0x1000    + 0x0006]
addreg edca2_refid  [expr 0x1000    + 0x0007]
addreg edca3_refid  [expr 0x1000    + 0x0008]
addreg edca4_refid  [expr 0x1000    + 0x0009]
addreg edca5_refid  [expr 0x1000    + 0x000a]

# AHB Nexus Registers
addreg ahb_nexus_csc   0x3001
addreg ahb_nexus_dc1   0x3002
addreg ahb_nexus_dc2   0x3003
addreg ahb_nexus_wt    0x300b
addreg ahb_nexus_dtc   0x300d
addreg ahb_nexus_dtsa1 0x300e
addreg ahb_nexus_dtsa2 0x300f
addreg ahb_nexus_dtea1 0x3012
addreg ahb_nexus_dtea2 0x3013
addreg ahb_nexus_bwc1  0x3016
addreg ahb_nexus_bwc2  0x3017
addreg ahb_nexus_bwa1  0x301e
addreg ahb_nexus_bwa2  0x301f
addreg ahb_nexus_bwd1  0x3026
addreg ahb_nexus_bwd2  0x3027
addreg ahb_nexus_bwam1 0x3042
addreg ahb_nexus_bwam2 0x3043
addreg ahb_nexus_bwdm1 0x3044
addreg ahb_nexus_bwdm2 0x3045
addreg ahb_nexus_pcr   0x307f

# SC3400 Nexus Registers
addreg sc_nexus_csc    0x3101
addreg sc_nexus_dc1    0x3102
addreg sc_nexus_dc2    0x3103
addreg sc_nexus_ds     0x3104
addreg sc_nexus_uba    0x3106
addreg sc_nexus_rwcs   0x3107
addreg sc_nexus_rwa    0x3109
addreg sc_nexus_rwd    0x310a
addreg sc_nexus_wt1    0x310b
addreg sc_nexus_wt2    0x310c
addreg sc_nexus_dtc    0x310d
addreg sc_nexus_dtsa1  0x310e
addreg sc_nexus_dtsa2  0x310f
addreg sc_nexus_dtsa3  0x3110
addreg sc_nexus_dtsa4  0x3111
addreg sc_nexus_dtea1  0x3112
addreg sc_nexus_dtea2  0x3113
addreg sc_nexus_dtea3  0x3114
addreg sc_nexus_dtea4  0x3115
addreg sc_nexus_pic    0x3140
addreg sc_nexus_pmsk   0x3141
addreg sc_nexus_pcr    0x317f

# NPC (Arbiter) Nexus Registers
addreg npc_nexus_did   0x3200
addreg npc_nexus_dc    0x3202
addreg npc_nexus_tsc   0x3247
addreg npc_nexus_pcr   0x327f

# AXI Nexus Registers (only on MXC)
addreg axi_nexus_csc   0x3301
addreg axi_nexus_dc1   0x3302
addreg axi_nexus_dc2   0x3303
addreg axi_nexus_ds    0x3304
addreg axi_nexus_wt1   0x330b
addreg axi_nexus_dtc   0x330d
addreg axi_nexus_dtsa1 0x330e
addreg axi_nexus_dtsa2 0x330f
addreg axi_nexus_dtea1 0x3312
addreg axi_nexus_dtea2 0x3313
addreg axi_nexus_bwc1  0x3316
addreg axi_nexus_bwc2  0x3317
addreg axi_nexus_bwa1  0x331e
addreg axi_nexus_bwa2  0x331f
addreg axi_nexus_bwd1  0x3326
addreg axi_nexus_bwd2  0x3327
addreg axi_nexus_bwam1 0x3342
addreg axi_nexus_bwam2 0x3343
addreg axi_nexus_bwdm1 0x3344
addreg axi_nexus_bwdm2 0x3345
addreg axi_nexus_pcr   0x337f

# NPC_TS Nexus Registers (only on MXC)
addreg npcts_nexus_did 0x3400
addreg npcts_nexus_dc  0x3402
addreg npcts_nexus_tsc 0x3447
addreg npcts_nexus_pcr 0x347f

#Create configuration options
addcfg deprecated_0
addcfg deprecated_1
addcfg fast_mem
addcfg endian
addcfg slow_memory
addcfg msc8101_sypcr
addcfg disable_polling
addcfg eonce_base
addcfg use_esr_status
addcfg reset_delay_ms
addcfg no_pctl
addcfg debug_rom
addcfg accelerate_hsst

set Stat(inst) 2
set Stat(icnt) 6
set Stat(vname) mxcsc
set Stat(core) "MXC StarCore"
set Stat(reg)  {d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 d10 d11 d12 d13 d14 d15 osp r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12 r13 r14 r15 m0 m1 m2 m3 n0 n1 n2 n3 mctl sr esr sp sa0 sa1 sa2 sa3 lc0 lc1 lc2 lc3 vba pctl0 pctl1 emcr d0_e d1_e d2_e d3_e d4_e d5_e d6_e d7_e d8_e d9_e d10_e d11_e d12_e d13_e d14_e d15_e idcode pc}
set Stat(size) {8  8  8  8  8  8  8  8  8  8  8   8   8   8   8   8   8   8  8  8  8  8  8  8  8  8  8  8   8   8   8   8   8   8  8  8  8  8  8  8  8  8    8  8   8  8   8   8   8   8   8   8   8   8   8     8     8    3    3    3    3    3    3    3    3    3    3    3     3     3     3     3     3     8      8} 
set    Stat(fmt) { d0 $fmt(d0)\t  d1 $fmt(d1)\t  d2 $fmt(d2)\t  d3 $fmt(d3)\n}
append Stat(fmt) { d4 $fmt(d4)\t  d5 $fmt(d5)\t  d6 $fmt(d6)\t  d7 $fmt(d7)\n}
append Stat(fmt) { d8 $fmt(d8)\t  d9 $fmt(d9)\t d10 $fmt(d10)\t d11 $fmt(d11)\n}
append Stat(fmt) {d12 $fmt(d12)\t d13 $fmt(d13)\t d14 $fmt(d14)\t d15 $fmt(d15)\n}
append Stat(fmt) { r0 $fmt(r0)\t  m0 $fmt(m0)\t  n0 $fmt(n0)\tmctl $fmt(mctl)\n}
append Stat(fmt) { r1 $fmt(r1)\t  m1 $fmt(m1)\t  n1 $fmt(n1)\t  sr $fmt(sr)\n}
append Stat(fmt) { r2 $fmt(r2)\t  m2 $fmt(m2)\t  n2 $fmt(n2)\t esr $fmt(esr)\n}
append Stat(fmt) { r3 $fmt(r3)\t  m3 $fmt(m3)\t  n3 $fmt(n3)\t  sp $fmt(sp)\n}
append Stat(fmt) { r4 $fmt(r4)\t sa0 $fmt(sa0)\t lc0 $fmt(lc0)\t osp $fmt(osp)\n}
append Stat(fmt) { r5 $fmt(r5)\t sa1 $fmt(sa1)\t lc1 $fmt(lc1)\t vba $fmt(vba)\n}
append Stat(fmt) { r6 $fmt(r6)\t sa2 $fmt(sa2)\t lc2 $fmt(lc2)\tpctl0$fmt(pctl0)\n}
append Stat(fmt) { r7 $fmt(r7)\t sa3 $fmt(sa3)\t lc3 $fmt(lc3)\tpctl1$fmt(pctl1)\n}
append Stat(fmt) { r8 $fmt(r8)\td0_e $fmt(d0_e)\td1_e $fmt(d1_e)\temcr $fmt(emcr)\n}
append Stat(fmt) { r9 $fmt(r9)\td2_e $fmt(d2_e)\td3_e $fmt(d3_e)\n}
append Stat(fmt) {r10 $fmt(r10)\td4_e $fmt(d4_e)\td5_e $fmt(d5_e)\n}
append Stat(fmt) {r11 $fmt(r11)\td6_e $fmt(d6_e)\td7_e $fmt(d7_e)\n}
append Stat(fmt) {r12 $fmt(r12)\td8_e $fmt(d8_e)\td9_e $fmt(d9_e)\n}
append Stat(fmt) {r13 $fmt(r13)\td10e $fmt(d10_e)\td11e $fmt(d11_e)\n}
append Stat(fmt) {r14 $fmt(r14)\td12e $fmt(d12_e)\td13e $fmt(d13_e)\n}
append Stat(fmt) {r15 $fmt(r15)\td14e $fmt(d14_e)\td15e $fmt(d15_e)\tidcode $fmt(idcode)\n}
append Stat(fmt) { pc $fmt(pc)}

proc load {chain_pos filename {memspace 0}} {
    if {$memspace == 0} {
        cload $chain_pos $filename
    } else {
        cload $chain_pos $filename $memspace
    }
    
}
proc cload {chain_pos filename {memspace 0}} {
    set prog_addr [list]
    set prog_data [list]

    set fp [open $filename r]
    set tmpdat [list]
    set origin_found 0
    set n 0
    while {![eof $fp]} {
        set line [gets $fp]
        if       {[regexp {^                     ([0-9A-F][0-9A-F])$} $line match dat]} {
            set byte($n) $dat
            if {$n > 0} {
                lappend tmpdat "0x$byte(1)$byte(0)"
                set n 0
            } else {
                incr n
            }
        } elseif {[regexp {^..........P:........ ([0-9A-F][0-9A-F]) .*$} $line match dat]} {
            set byte($n) $dat
            incr n
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
