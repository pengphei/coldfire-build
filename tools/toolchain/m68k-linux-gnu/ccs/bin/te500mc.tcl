set e500mc_regs(core) {
    {spr_first 0}
    {xer 1}
    {lr 8}
    {ctr 9}
    {dsisr 18}
    {dar 19}
    {dec 22}
    {sdr1 25}
    {srr0 26}
    {srr1 27}
    {pid0 48}
    {decar 54}
    {csrr0 58}
    {csrr1 59}
    {dear 61}
    {esr 62}
    {ivpr 63}
    {eie 80}
    {eid 81}
    {nri 82}
    {cmpa 144}
    {cmpb 145}
    {cmpc 146}
    {cmpd 147}
    {ecr 148}
    {icr 148}
    {der 149}
    {counta 150}
    {countb 151}
    {cmpe 152}
    {cmpf 153}
    {cmpg 154}
    {cmph 155}
    {lctrl1 156}
    {lctrl2 157}
    {ictrl 158}
    {bar 159}
    {vrsave 256}
    {usprg0 256}
    {usprg3 259}
    {usprg4 260}
    {usprg5 261}
    {usprg6 262}
    {usprg7 263}
    {tbl 268}
    {tbu 269}
    {sprg0 272}
    {sprg1 273}
    {sprg2 274}
    {sprg3 275}
    {sprg4 276}
    {sprg5 277}
    {sprg6 278}
    {sprg7 279}
    {ear 282}
    {tblw 284}
    {tbuw 285}
    {epir 286}
    {svrzeppo 286}
    {pvr 287}
    {dbsr 304}
    {dbcr0 308}
    {dbcr1 309}
    {dbcr2 310}
    {mbar 311}
    {iac1 312}
    {iac2 313}
    {dac1 316}
    {dac2 317}
    {tsr 336}
    {tcr 340}
    {ivor0 400}
    {ivor1 401}
    {ivor2 402}
    {ivor3 403}
    {ivor4 404}
    {ivor5 405}
    {ivor6 406}
    {ivor7 407}
    {ivor8 408}
    {ivor9 409}
    {ivor10 410}
    {ivor11 411}
    {ivor12 412}
    {ivor13 413}
    {ivor14 414}
    {ivor15 415}
    {l1cfg0 515}
    {l1cfg1 516}
    {npidr 517}
    {iarr 518}
    {l2cfg0 519}
    {mi_gra 528}
    {ivor35 531}
    {ivor36 532}
    {ivor37 533}
    {ivor38 534}
    {ivor39 535}
    {ivor40 536}
    {ivor41 537}
    {l2u_gra 536}
    {cxtr 560}
    {bbcmcr 560}
    {ic_cst 560}
    {ic_adr 561}
    {ic_dat 562}
    {dbsrwr 564}
    {l2u_mcr 568}
    {dc_cst 568}
    {dc_adr 569}
    {dc_dat 570}
    {mcsrr0 570}
    {mcsrr1 571}
    {mcsr 572}
    {mcar 573}
    {dssr0 574}
    {dssr0 575}
    {gsprg0 592}
    {gsprg1 593}
    {gsprg2 594}
    {gsprg3 595}
    {gser 596}
    {sprg9 605}
    {mas0 624}
    {mas1 625}
    {mas2 626}
    {mas3 627}
    {mas4 628}
    {mas5 629}
    {mas6 630}
    {mas8 631}
    {lpid 638}
    {msrp 639}
    {tlbcfg0 688}
    {tlbcfg1 689}
    {cdcsr0 696}
    {gepr 698}
    {gssr0 699}
    {gssr1 700}
    {gpir 701}
    {epr 702}
    {uvcsr 703}
    {mi_ctr 784}
    {mi_rba0 784}
    {mi_rba1 785}
    {mi_rba2 786}
    {mi_rba3 787}
    {mi_ap 786}
    {mi_epn 787}
    {mi_twc 789}
    {mi_rpn 790}
    {l2u_rba0 792}
    {l2u_rba1 793}
    {l2u_rba2 794}
    {l2u_rba3 795}
    {mi_cam 816}
    {mi_ram0 817}
    {mi_ram1 818}
    {md_ctr 792}
    {m_casid 793}
    {md_ap 794}
    {md_epn 795}
    {m_twb 796}
    {md_twc 797}
    {md_rpn 798}
    {m_tw 799}
    {mi_ra0 816}
    {mi_ra1 817}
    {mi_ra2 818}
    {mi_ra3 819}
    {l2u_ra0 824}
    {l2u_ra1 825}
    {l2u_ra2 826}
    {l2u_ra3 827}
    {md_cam 824}
    {md_ram0 825}
    {md_ram1 826}
    {givpr 912}
    {givor2 913}
    {givor3 914}
    {givor4 915}
    {givor8 918}
    {givor13 919}
    {givor14 920}
    {wpar 921}
    {dmau 922}
    {dmal 923}
    {ummcr2 928}
    {upmc5 929}
    {upmc6 930}
    {ubamr 935}
    {ummcr0 936}
    {upmc1 937}
    {upmc2 938}
    {usia 939}
    {usiar 939}
    {ummcr1 940}
    {upmc3 941}
    {upmc4 942}
    {usda 943}
    {zpr 944}
    {mmcr2 944}
    {pmc5 945}
    {pid 945}
    {pmc6 946}
    {eplc 947}
    {epsc 948}
    {bamr 951}
    {mmcr0 952}
    {sgr 953}
    {pmc1 953}
    {dcwr 954}
    {pmc2 954}
    {sia 955}
    {siar 955}
    {mmcr1 956}
    {pmc3 957}
    {pmc4 958}
    {sda 959}
    {tbhu 972}
    {tblu 973}
    {dmiss 976}
    {hdbcr0 976}
    {dcmp 977}
    {hdbcr1 977}
    {hash1 978}
    {hdbcr2 978}
    {hash2 979}
    {hdbcr3 979}
    {icdbdr 979}
    {imiss 980}
    {hdbcr4 980}
    {tlbmiss 980}
    {icmp 981}
    {hdbcr5 981}
    {ptehi 981}
    {ptelo 982}
    {rpa 982}
    {evpr 982}
    {cdbcr 983}
    {l2errinjhi 985}
    {l2errinjlo 986}
    {l2errinjctl 987}
    {l2captdatahi 988}
    {l2captdatalo 989}
    {l2captecc 990}
    {l2errdet 991}
    {l2errdis 992}
    {l2errinten 993}
    {l2errattr 994}
    {l2erraddr 995}
    {l2erreaddr 996}
    {l2errctl 997}
    {dabr2 1000}
    {ibcr 1002}
    {hid0 1008}
    {iabr 1010}
    {dbcr 1001}
    {l1csr0 1010}
    {l1csr1 1011}
    {hid2 1011}
    {ictrl_7450 1011}
    {mmucsr0 1012}
    {bucsr0 1013}
    {dabr 1013}
    {msscr0 1014}
    {mmucfg 1015}
    {msssr0 1015}
    {ldstcr 1016}
    {l2pm 1016}
    {l2pmcr 1016}
    {l2csr0 1017}
    {dccr 1018}
    {iabr2 1018}
    {l3cr 1018}
    {iccr 1019}
    {ictc 1019}
    {pbl1 1020}
    {thrm1 1020}
    {thrm2 1021}
    {pbu1 1021}
    {pbl2 1022}
    {thrm3 1022}
    {fpecr 1022}
    {pbu2 1023}
    {pir 1023}
    {svr 1023}
    {spr_last 1023}
    {sr0 1100}
    {sr1 1101}
    {sr2 1102}
    {sr3 1103}
    {sr4 1104}
    {sr5 1105}
    {sr6 1106}
    {sr7 1107}
    {sr8 1108}
    {sr9 1109}
    {sr10 1110}
    {sr11 1111}
    {sr12 1112}
    {sr13 1113}
    {sr14 1114}
    {sr15 1115}
    {r0 2000}
    {r1 2001}
    {sp 2001}
    {r2 2002}
    {r3 2003}
    {r4 2004}
    {r5 2005}
    {r6 2006}
    {r7 2007}
    {r8 2008}
    {r9 2009}
    {r10 2010}
    {r11 2011}
    {r12 2012}
    {r13 2013}
    {r14 2014}
    {r15 2015}
    {r16 2016}
    {r17 2017}
    {r18 2018}
    {r19 2019}
    {r20 2020}
    {r21 2021}
    {r22 2022}
    {r23 2023}
    {r24 2024}
    {r25 2025}
    {r26 2026}
    {r27 2027}
    {r28 2028}
    {r29 2029}
    {r30 2030}
    {r31 2031}
    {iar 2032}
    {cr 2033}
    {msr 2034}
    {fpr0 3000}
    {fpr1 3001}
    {fpr2 3002}
    {fpr3 3003}
    {fpr4 3004}
    {fpr5 3005}
    {fpr6 3006}
    {fpr7 3007}
    {fpr8 3008}
    {fpr9 3009}
    {fpr10 3010}
    {fpr11 3011}
    {fpr12 3012}
    {fpr13 3013}
    {fpr14 3014}
    {fpr15 3015}
    {fpr16 3016}
    {fpr17 3017}
    {fpr18 3018}
    {fpr19 3019}
    {fpr20 3020}
    {fpr21 3021}
    {fpr22 3022}
    {fpr23 3023}
    {fpr24 3024}
    {fpr25 3025}
    {fpr26 3026}
    {fpr27 3027}
    {fpr28 3028}
    {fpr29 3029}
    {fpr30 3030}
    {fpr31 3031}
    {fpscr 3032}
}

set core_regs(core) {}

set e500mc_resource(core) {
    {dc 0x408}
    {dc2 0x40c}
    {wt1 0x42c}
    {wmsk 0x458}
    {ovcr 0x45c}
}

set core_res(core) {}

foreach mreg $e500mc_regs(core) {
    addreg [lindex $mreg 0] [lindex $mreg 1]
    lappend core_regs(core) [lindex $mreg 0]
}

foreach mresource $e500mc_resource(core) {
    addres [lindex $mresource 0] [lindex $mresource 1]
    lappend core_res(core) [lindex $mresource 0]
}

set Stat(inst) 2
set Stat(icnt) 6
set Stat(vname) ppc
set Stat(core) "PowerPc"
set Stat(reg)  {iar msr srr0 srr1 iabr}
set Stat(size) {8  8  8  8  8}
set    Stat(fmt) { iar $fmt(iar)\t  msr $fmt(msr)\t  srr0 $fmt(srr0)\t  srr1 $fmt(srr1)\n}
append Stat(fmt) { iabr $fmt(iabr)}

proc setup_e500mc {} {
    global base_reg_value
    global stop_go_loop
    global chain_pos
    mm 0xfffff000 0x3c60f800 0x480000d1 0x38600000 0x7c799ba6 0x7c7a9ba6 0x4c00012c 0x3860001e 0x7c74fba6 0x4c00012c 0x3c601003 0x3c808000 0x60840800 0x3ca0f800 0x60a50008 0x3cc0f800 0x60c60015 0x48000141 0x3c601004 0x3c808000 0x60840800 0x3ca0fc00 0x60a50008 0x3cc0fc00 0x60c60015 0x48000121 0x3c601000 0x3c808000 0x60840500 0x3ca01000 0x60a50008 0x3cc01000 0x60c60015 0x48000101 0x3c601001 0x3c808000 0x60840800 0x3ca0f000 0x60a50008 0x3cc0f000 0x60c60015 0x480000e1 0x3c601002 0x3c808000 0x60840800 0x3ca0f400 0x60a50008 0x3cc0f400 0x60c60015 0x480000c1 0x7c008146 0x60000000 0x60000000 0x48000000 0x7c7f0ba6 0x3c600000 0x60640100 0x7c9063a6 0x60640200 0x7c9163a6 0x60640300 0x7c9263a6 0x60640400 0x7c9363a6 0x60640500 0x7c9463a6 0x60640600 0x7c9563a6 0x60640700 0x7c9663a6 0x60640800 0x7c9763a6 0x60640900 0x7c9a63a6 0x60640a00 0x7c9963a6 0x60640b00 0x7c9b63a6 0x60640c00 0x7c9863a6 0x60640d00 0x7c9c63a6 0x60640f00 0x7c9383a6 0x60641000 0x7c9e63a6 0x60641100 0x7c9d63a6 0x60641200 0x7c9183a6 0x60641300 0x7c9283a6 0x60641500 0x7c9f63a6 0x60641600 0x7c9083a6 0x4e800020 0x7c709ba6 0x4c00012c 0x7c919ba6 0x4c00012c 0x7cb29ba6 0x4c00012c 0x7cd39ba6 0x4c00012c 0x7c0007a4 0x4e800020
    mm 0xfffffffc 0x4bfff002
    rm iar 0xfffffffc
    go
    set stop_go_loop 0
    while {$stop_go_loop == 0} {
        stop
	set addr [ccs::read_reg $chain_pos iar 1]
	set inst [ccs::read_mem $chain_pos $addr 4 0 1]
	q
	if {$inst == 1207959552} {
		set stop_go_loop 1
	}
    }
    stop
    set base [ccs:read_reg $chain_pos 30000 1]
    set base_reg_value [expr $base * 0x100000]

    rm ccsrbar        0x00010000
    rm lawbar0        0x000f8000
    rm lawsr0         0x80f0001a
    rm timing_cfg_1   0x3754c321
    rm timing_cfg_2   0x00000c00
    rm sdram_cfg_1    0x42200000
    rm interval       0x048a0000
    rm sdram_cfg_2    0x00000061
    rm cs0_bounds     0x00f800ff
    rm cs0_config     0x80000101
    rm sdram_cfg_1    0xc2200000
    rm eebacr         0x10000001
    rm eebpcr         0x01000002
}

