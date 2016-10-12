set ppc_core_regs(core) {{r0 2000}  {r1 2001} {r2 2002} {r3 2003} {r4 2004} {r5 2005} {r6 2006} {r7 2007} {r8 2008} {r9 2009} {r10 2010} {r11 2011} {r12 2012} {r13 2013} {r14 2014} {r15 2015} {r16 2016} {r17 2017} {r18 2018} {r19 2019} {r20 2020} {r21 2021} {r22 2022} {r23 2023} {r24 2024} {r25 2025} {r26 2026} {r27 2027} {r28 2028} {r29 2029} {r30 2030} {r31 2031} {mbar 311} {ibat0u 528} {ibat0l 529} {ibat1u 530} {ibat1l 531} {ibat2u 532} {ibat2l 533} {ibat3u 534} {ibat3l 535} {dbat0u 536} {dbat0l 537} {dbat1u 538} {dbat1l 539} {dbat2u 540} {dbat2l 541} {dbat3u 542} {dbat3l 543} {iar 2032} {msr 2034} {dsisr 18} {dar 19} {dec 22} {tbu 269} {tbl 268} {hid0 1008} {hid1 1009} {hid2 1011} {srr1 27} {sprg0 272} {sprg1 273} {sprg2 274} {sprg3 275} {ear 282} {lr 8} {ctr 9} {srr0 26} {cr 2033} {xer 1} {fpscr 3032} {sdr1 25} {dmiss 976} {imiss 980} {dcmp 977} {icmp 981} {rpa 982} {iabr 1010} {pvr 287} {hash1 978} {hash2 979} {sr0 1100} {sr1 1101} {sr2 1102} {sr3 1103} {sr4 1104} {sr5 1105} {sr6 1106} {sr7 1107} {sr8 1108} {sr9 1109} {sr10 1110} {sr11 1111} {sr12 1112} {sr13 1113} {sr14 1114} {sr15 1115}}

set core_regs(core) {}

foreach mreg $ppc_core_regs(core) {
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



proc setup_ppc {} {
    set baseaddr 0x0f000000
    #sccr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10c80] 4 0 0x00000000
	#sypcr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10004] 4 0 0xc3ffffff
	#rmr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10c94] 4 0 0x01000000
	#mptpr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10184] 2 0 0x0032
	#br0
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10100] 4 0 0x011880ff
	#or0
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10104] 4 0 0x360880ff
	#br1
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10108] 4 0 0x01185004
	#or1
	ccs::write_mem $chain_pos [expr $baseaddr + 0x1010c] 4 0 0x1080ffff
 	#br2
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10110] 4 0 0x41000000
	#or2
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10114] 4 0 0x800c00ff
	#psrt
	ccs::write_mem $chain_pos [expr $baseaddr + 0x1019c] 1 0 0x10
	#msr
	ccs::write_reg $chain_pos 0x71 0x00001002
	#psdmr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10190] 4 0 0x52b48e01
	#psdmr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10190] 4 0 0x52b48e29
	ccs::write_mem $chain_pos 0x00000000 1 0 0xff
	#psdmr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10190] 4 0 0x52b48e09
	ccs::write_mem $chain_pos 0x00000000 1 0 0xff
	ccs::write_mem $chain_pos 0x00000000 1 0 0xff
	ccs::write_mem $chain_pos 0x00000000 1 0 0xff
	ccs::write_mem $chain_pos 0x00000000 1 0 0xff
	ccs::write_mem $chain_pos 0x00000000 1 0 0xff
	ccs::write_mem $chain_pos 0x00000000 1 0 0xff
	ccs::write_mem $chain_pos 0x00000000 1 0 0xff
	ccs::write_mem $chain_pos 0x00000000 1 0 0xff
	#psdmr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10190] 4 0 0x52b48e19
	ccs::write_mem $chain_pos 0x00000000 1 0 0xff
	#psdmr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10190] 4 0 0x52b48e41
	#lsrt
	ccs::write_mem $chain_pos [expr $baseaddr + 0x101a4] 1 0 0x38
	#br4
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10120] 4 0 0x8014c0ff
	#or4
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10124] 4 0 0x61180004
	#lsdmr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10194] 4 0 0x52b58618
	#siumcr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10000] 4 0 0x0000200e
	#tescr1
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10040] 4 0 0x00400000
	#tescr2
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10044] 4 0 0x00000000
	#l_tescr1
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10048] 4 0 0x00400000
	#l_tescr2
	ccs::write_mem $chain_pos [expr $baseaddr + 0x1004c] 4 0 0x00000000
	#lsdmr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10194] 4 0 0x52b58628
	ccs::write_mem $chain_pos 0x04000000 1 0 0xff
	#lsdmr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10194] 4 0 0x52b58688
	ccs::write_mem $chain_pos 0x04000000 4 0 0xffffffff
	ccs::write_mem $chain_pos 0x04000000 4 0 0xffffffff
	ccs::write_mem $chain_pos 0x04000000 4 0 0xffffffff
	ccs::write_mem $chain_pos 0x04000000 4 0 0xffffffff
	ccs::write_mem $chain_pos 0x04000000 4 0 0xffffffff
	ccs::write_mem $chain_pos 0x04000000 4 0 0xffffffff
	ccs::write_mem $chain_pos 0x04000000 4 0 0xffffffff
	ccs::write_mem $chain_pos 0x04000000 4 0 0xffffffff
	#lsdmr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10194] 4 0 0x52b58618
	ccs::write_mem $chain_pos 0x04000000 1 0 0xff
	#lsdmr
	ccs::write_mem $chain_pos [expr $baseaddr + 0x10194] 4 0 0x12a58640
}
