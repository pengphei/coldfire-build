set ctl_mask 0x1000
set dbg_mask 0x2000

#Data and address registers
addreg d0   [expr 0x0]
addreg d1   [expr 0x1]
addreg d2   [expr 0x2]
addreg d3   [expr 0x3]
addreg d4   [expr 0x4]
addreg d5   [expr 0x5]
addreg d6   [expr 0x6]
addreg d7   [expr 0x7]
addreg a0   [expr 0x8]
addreg a1   [expr 0x9]
addreg a2   [expr 0xa]
addreg a3   [expr 0xb]
addreg a4   [expr 0xc]
addreg a5   [expr 0xd]
addreg a6   [expr 0xe]
addreg a7   [expr 0xf]

#Control registers
addreg ctl_002  [expr 0x0002 | $ctl_mask]
addreg ctl_003  [expr 0x0003 | $ctl_mask]
addreg ctl_004  [expr 0x0004 | $ctl_mask]
addreg ctl_005  [expr 0x0005 | $ctl_mask]
addreg ctl_006  [expr 0x0006 | $ctl_mask]
addreg ctl_007  [expr 0x0007 | $ctl_mask]
addreg ctl_008  [expr 0x0008 | $ctl_mask]
addreg ctl_800  [expr 0x0800 | $ctl_mask]
addreg ctl_801  [expr 0x0801 | $ctl_mask]
addreg ctl_804  [expr 0x0804 | $ctl_mask]
addreg ctl_805  [expr 0x0805 | $ctl_mask]
addreg ctl_806  [expr 0x0806 | $ctl_mask]
addreg ctl_806  [expr 0x0806 | $ctl_mask]
addreg ctl_807  [expr 0x0807 | $ctl_mask]
addreg ctl_808  [expr 0x0808 | $ctl_mask]
addreg ctl_809  [expr 0x0809 | $ctl_mask]
addreg ctl_80a  [expr 0x080a | $ctl_mask]
addreg ctl_80b  [expr 0x080b | $ctl_mask]
addreg ctl_80e  [expr 0x080e | $ctl_mask]
addreg ctl_80f  [expr 0x080f | $ctl_mask]
addreg ctl_810  [expr 0x0810 | $ctl_mask]
addreg ctl_811  [expr 0x0811 | $ctl_mask]
addreg ctl_812  [expr 0x0812 | $ctl_mask]
addreg ctl_813  [expr 0x0813 | $ctl_mask]
addreg ctl_814  [expr 0x0814 | $ctl_mask]
addreg ctl_815  [expr 0x0815 | $ctl_mask]
addreg ctl_816  [expr 0x0816 | $ctl_mask]
addreg ctl_817  [expr 0x0817 | $ctl_mask]
addreg ctl_818  [expr 0x0818 | $ctl_mask]
addreg ctl_819  [expr 0x0819 | $ctl_mask]
addreg ctl_81a  [expr 0x081a | $ctl_mask]
addreg ctl_81b  [expr 0x081b | $ctl_mask]
addreg ctl_81c  [expr 0x081c | $ctl_mask]
addreg ctl_81d  [expr 0x081d | $ctl_mask]
addreg ctl_81e  [expr 0x081e | $ctl_mask]
addreg ctl_81f  [expr 0x081f | $ctl_mask]
addreg ctl_821  [expr 0x0821 | $ctl_mask]
addreg ctl_822  [expr 0x0822 | $ctl_mask]
addreg ctl_824  [expr 0x0824 | $ctl_mask]
addreg ctl_c00  [expr 0x0c00 | $ctl_mask]
addreg ctl_c01  [expr 0x0c01 | $ctl_mask]
addreg ctl_c04  [expr 0x0c04 | $ctl_mask]
addreg ctl_c04  [expr 0x0c04 | $ctl_mask]
addreg ctl_c05  [expr 0x0c05 | $ctl_mask]
addreg ctl_c0e  [expr 0x0c0e | $ctl_mask]
addreg ctl_c0f  [expr 0x0c0f | $ctl_mask]

#Debug registers
addreg dbg_00   [expr 0x0000 | $dbg_mask]
addreg dbg_06   [expr 0x0006 | $dbg_mask]
addreg dbg_07   [expr 0x0007 | $dbg_mask]
addreg dbg_08   [expr 0x0008 | $dbg_mask]
addreg dbg_09   [expr 0x0009 | $dbg_mask]
addreg dbg_0c   [expr 0x000c | $dbg_mask]
addreg dbg_0d   [expr 0x000d | $dbg_mask]
addreg dbg_0e   [expr 0x000e | $dbg_mask]
addreg dbg_0f   [expr 0x000f | $dbg_mask]
addreg dbg_05   [expr 0x0005 | $dbg_mask]
addreg dbg_04   [expr 0x0004 | $dbg_mask]
addreg dbg_14   [expr 0x0014 | $dbg_mask]
addreg dbg_16   [expr 0x0016 | $dbg_mask]
addreg dbg_17   [expr 0x0017 | $dbg_mask]
addreg dbg_18   [expr 0x0018 | $dbg_mask]
addreg dbg_1a   [expr 0x001a | $dbg_mask]
addreg dbg_1b   [expr 0x001b | $dbg_mask]
addreg dbg_1c   [expr 0x001c | $dbg_mask]
addreg dbg_1d   [expr 0x001d | $dbg_mask]
addreg dbg_1e   [expr 0x001e | $dbg_mask]
addreg dbg_1f   [expr 0x001f | $dbg_mask]

#Register aliases
addreg cacr     [expr 0x0002 | $ctl_mask]
addreg asid     [expr 0x0003 | $ctl_mask]
addreg acr0     [expr 0x0004 | $ctl_mask]
addreg acr1     [expr 0x0005 | $ctl_mask]
addreg acr2     [expr 0x0006 | $ctl_mask]
addreg acr3     [expr 0x0007 | $ctl_mask]
addreg mmubar   [expr 0x0008 | $ctl_mask]
addreg other_a7 [expr 0x0800 | $ctl_mask]
addreg vbr      [expr 0x0801 | $ctl_mask]
addreg macsr    [expr 0x0804 | $ctl_mask]
addreg mask     [expr 0x0805 | $ctl_mask]
addreg acc      [expr 0x0806 | $ctl_mask]
addreg acc0     [expr 0x0806 | $ctl_mask]
addreg accext01 [expr 0x0807 | $ctl_mask]
addreg accext23 [expr 0x0808 | $ctl_mask]
addreg acc1     [expr 0x0809 | $ctl_mask]
addreg acc2     [expr 0x080a | $ctl_mask]
addreg acc3     [expr 0x080b | $ctl_mask]
addreg sr       [expr 0x080e | $ctl_mask]
addreg pc       [expr 0x080f | $ctl_mask]
addreg fpu0     [expr 0x0810 | $ctl_mask]
addreg fpl0     [expr 0x0811 | $ctl_mask]
addreg fpu1     [expr 0x0812 | $ctl_mask]
addreg fpl1     [expr 0x0813 | $ctl_mask]
addreg fpu2     [expr 0x0814 | $ctl_mask]
addreg fpl2     [expr 0x0815 | $ctl_mask]
addreg fpu3     [expr 0x0816 | $ctl_mask]
addreg fpl3     [expr 0x0817 | $ctl_mask]
addreg fpu4     [expr 0x0818 | $ctl_mask]
addreg fpl4     [expr 0x0819 | $ctl_mask]
addreg fpu5     [expr 0x081a | $ctl_mask]
addreg fpl5     [expr 0x081b | $ctl_mask]
addreg fpu6     [expr 0x081c | $ctl_mask]
addreg fpl6     [expr 0x081d | $ctl_mask]
addreg fpu7     [expr 0x081e | $ctl_mask]
addreg fpl7     [expr 0x081f | $ctl_mask]
addreg fpiar    [expr 0x0821 | $ctl_mask]
addreg fpsr     [expr 0x0822 | $ctl_mask]
addreg fpcr     [expr 0x0824 | $ctl_mask]
addreg rombar0  [expr 0x0c00 | $ctl_mask]
addreg rombar1  [expr 0x0c01 | $ctl_mask]
addreg flashbar [expr 0x0c04 | $ctl_mask]
addreg rambar0  [expr 0x0c04 | $ctl_mask]
addreg rambar1  [expr 0x0c05 | $ctl_mask]
addreg mbar2    [expr 0x0c0e | $ctl_mask]
addreg mbar     [expr 0x0c0f | $ctl_mask]

addreg csr      [expr 0x0000 | $dbg_mask]
addreg aatr     [expr 0x0006 | $dbg_mask]
addreg tdr      [expr 0x0007 | $dbg_mask]
addreg pbr      [expr 0x0008 | $dbg_mask]
addreg pbmr     [expr 0x0009 | $dbg_mask]
addreg abhr     [expr 0x000c | $dbg_mask]
addreg ablr     [expr 0x000d | $dbg_mask]
addreg dbr      [expr 0x000e | $dbg_mask]
addreg dbmr     [expr 0x000f | $dbg_mask]
addreg baar     [expr 0x0005 | $dbg_mask]
addreg pbac     [expr 0x0004 | $dbg_mask]
addreg pbaisd   [expr 0x0014 | $dbg_mask]
addreg aatr1    [expr 0x0016 | $dbg_mask]
addreg xtdr     [expr 0x0017 | $dbg_mask]
addreg pbr1     [expr 0x0018 | $dbg_mask]
addreg pbr2     [expr 0x001a | $dbg_mask]
addreg pbr3     [expr 0x001b | $dbg_mask]
addreg abhr1    [expr 0x001c | $dbg_mask]
addreg ablr1    [expr 0x001d | $dbg_mask]
addreg dbr1     [expr 0x001e | $dbg_mask]
addreg dbmr1    [expr 0x001f | $dbg_mask]

addreg sp       [expr 0xf]; #a7

set Stat(inst)   1
set Stat(icnt)   1
set Stat(vname)  cf
set Stat(core)   "coldfire"
set Stat(reg)    {d0 d1 d2 d3 d4 d5 d6 d7 a0 a1 a2 a3 a4 a5 a6 a7 cacr vbr sr pc mbar asid acr0 acr1 acr2 acr3 mmubar other_a7 rombar0 rombar1 rambar0 rambar1 macsr mask acc0 accext01 accext23 acc1 acc2 acc3 fpu0 fpl0 fpu1 fpl1 fpu2 fpl2 fpu3 fpl3 fpu4 fpl4 fpu5 fpl5 fpu6 fpl6 fpu7 fpl7 fpsr fpiar fpcr csr pbac baar aatr tdr pbr pbmr abhr ablr dbr dbmr pbaisd aatr1 xtdr pbr1 pbr2 pbr3 abhr1 ablr1 dbr1 dbmr1}
set Stat(size)   { 4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4    4   4  2  4    4    4    4    4    4    4      4        4       4       4       4       4     4    4    4        4        4    4    4    4    4    4    4    4    4    4    4    4    4    4    4    4    4    4    4    4    4     4    4   4    4    4    4   4   4    4    4    4   4    4      4     4    4    4    4    4     4     4    4     4} 
set Stat(fmt)    { d0   $fmt(d0)\t   d1  $fmt(d1)\t  d2  $fmt(d2)\t  d3  $fmt(d3)\n}
append Stat(fmt) { d4   $fmt(d4)\t   d5  $fmt(d5)\t  d6  $fmt(d6)\t  d7  $fmt(d7)\n}
append Stat(fmt) { a0   $fmt(a0)\t   a1  $fmt(a1)\t  a2  $fmt(a2)\t  a3  $fmt(a3)\n}
append Stat(fmt) { a4   $fmt(a4)\t   a5  $fmt(a5)\t  a6  $fmt(a6)\t  a7  $fmt(a7)\n}
append Stat(fmt) { cacr $fmt(cacr)\t vbr   $fmt(vbr) \t sr   $fmt(sr)  \t  mbar $fmt(mbar)\n }
append Stat(fmt) { asid $fmt(asid)\t acr0  $fmt(acr0)\t acr1 $fmt(acr1)\t  acr2 $fmt(acr2)\t acr3 $fmt(acr3)\n }
append Stat(fmt) { mmubar  $fmt(mmubar) \t other_a7 $fmt(other_a7)\n }
append Stat(fmt) { rombar0 $fmt(rombar0)\t rombar1  $fmt(rombar1) \n }
append Stat(fmt) { rambar0 $fmt(rambar0)\t rambar1  $fmt(rambar1) \n }
append Stat(fmt) { macsr   $fmt(macsr)  \t mask     $fmt(mask)    \n }
append Stat(fmt) { acc0 $fmt(acc0)\t accext01 $fmt(accext01)\t accext23 $fmt(accext23)\n }
append Stat(fmt) { acc1 $fmt(acc1)\t acc2     $fmt(acc2)    \t acc3   $fmt(acc3)\n }
append Stat(fmt) { fpu0 $fmt(fpu0)\t fpl0     $fmt(fpl0)    \t fpu1   $fmt(fpu1)\t fpl1 $fmt(fpl1)\n}
append Stat(fmt) { fpu2 $fmt(fpu2)\t fpl2     $fmt(fpl2)    \t fpu3   $fmt(fpu3)\t fpl3 $fmt(fpl3)\n}
append Stat(fmt) { fpu4 $fmt(fpu4)\t fpl4     $fmt(fpl4)    \t fpu5   $fmt(fpu5)\t fpl5 $fmt(fpl5)\n}
append Stat(fmt) { fpu6 $fmt(fpu6)\t fpl6     $fmt(fpl6)    \t fpu7   $fmt(fpu7)\t fpl7 $fmt(fpl7)\n}
append Stat(fmt) { fpcr $fmt(fpcr)\t fpsr     $fmt(fpsr)    \t fpiar  $fmt(fpiar)\n }
append Stat(fmt) { csr  $fmt(csr) \t pbac     $fmt(baar)    \t baar   $fmt(baar)\t aatr $fmt(aatr)\n }
append Stat(fmt) { tdr  $fmt(tdr)\t  pbr  $fmt(pbr)\t     pbmr  $fmt(pbmr)\n }
append Stat(fmt) { abhr $fmt(abhr)\t ablr $fmt(ablr)\t    dbr   $fmt(dbr)\t  dbmr $fmt(dbmr)\n }
append Stat(fmt) { pbaisd $fmt(pbaisd)\t  aatr1  $fmt(aatr1)\n }
append Stat(fmt) { xtdr $fmt(xtdr)\t pbr1 $fmt(pbr1)\t    pbr2 $fmt(pbr2)\t  pbr3  $fmt(pbmr3)\n }
append Stat(fmt) { abhr1  $fmt(abhr1)\t   ablr1  $fmt(ablr1)\t dbr1 $fmt(dbr1)\t   dbmr1 $fmt(dbmr1)\n }
