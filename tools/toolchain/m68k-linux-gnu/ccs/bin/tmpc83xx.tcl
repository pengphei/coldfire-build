set 83xx_core_regs(core) {
    {r0 2000}  {r1 2001} {r2 2002} {r3 2003} {r4 2004} {r5 2005} {r6 2006} {r7 2007} {r8 2008} {r9 2009} {r10 2010} {r11 2011} {r12 2012} {r13 2013} {r14 2014} {r15 2015} {r16 2016} {r17 2017} {r18 2018} {r19 2019} {r20 2020} {r21 2021} {r22 2022} {r23 2023} {r24 2024} {r25 2025} {r26 2026} {r27 2027} {r28 2028} {r29 2029} {r30 2030} {r31 2031}
    {mbar 311}
    {ibat0u 528} {ibat0l 529} {ibat1u 530} {ibat1l 531} {ibat2u 532} {ibat2l 533} {ibat3u 534} {ibat3l 535}
    {dbat0u 536} {dbat0l 537} {dbat1u 538} {dbat1l 539} {dbat2u 540} {dbat2l 541} {dbat3u 542} {dbat3l 543}
    {iar 2032}
    {msr 2034}
    {dsisr 18} {dar 19} {dec 22} {tbu_r 269} {tbl_r 268} {tbu_w 285} {tbl_w 284} {hid0 1008} {hid1 1009} {hid2 1011} {srr1 27} {sprg0 272} {sprg1 273} {sprg2 274} {sprg3 275} {ear 282} {lr 8} {ctr 9} {srr0 26} {cr 2033} {xer 1} {fpscr 3032} {sdr1 25} {dmiss 976} {imiss 980} {dcmp 977} {icmp 981} {rpa 982} {iabr 1010} {iabr2 1018} {dabr 1013} {dabr2 317} {ibcr 309} {dbcr 310} {pvr 287} {hash1 978} {hash2 979} {sr0 1100} {sr1 1101} {sr2 1102} {sr3 1103} {sr4 1104} {sr5 1105} {sr6 1106} {sr7 1107} {sr8 1108} {sr9 1109} {sr10 1110} {sr11 1111} {sr12 1112} {sr13 1113} {sr14 1114} {sr15 1115}

    {pmgc0  0x20001190}
    {pmc0   0x20001010}
    {pmc1   0x20001011}
    {pmc2   0x20001012}
    {pmc3   0x20001013}
    {pmlc0  0x20001090}
    {pmlc1  0x20001091}
    {pmlc2  0x20001092}
    {pmlc3  0x20001093}

    {upmgc0 0x20001180}
    {upmc0  0x20001000}
    {upmc1  0x20001001}
    {upmc2  0x20001002}
    {upmc3  0x20001003}
    {upmlc0 0x20001080}
    {upmlc1 0x20001081}
    {upmlc2 0x20001082}
    {upmlc3 0x20001083}

}

set core_regs(core) {}

foreach mreg $83xx_core_regs(core) {
    addreg [lindex $mreg 0] [lindex $mreg 1]
    lappend core_regs(core) [lindex $mreg 0]
}

set Stat(inst) 2
set Stat(icnt) 6
set Stat(vname) ppc
set Stat(core) "PowerPc"
set Stat(reg)  {iar msr srr0 srr1 iabr}
set Stat(size) {8  8  8  8  8}
set    Stat(fmt) { iar $fmt(iar)\t  msr $fmt(msr)\t  srr0 $fmt(srr0)\t  srr1 $fmt(srr1)\n}
append Stat(fmt) { iabr $fmt(iabr)}

