#Create configuration options
addcfg fast_download
addcfg l2cc_type
addcfg l2cc_base
addcfg mem_delay

set arm11_core_regs(core) {
    {r0           10000}
    {r1           10001}
    {r2           10002}
    {r3           10003}
    {r4           10004}
    {r5           10005}
    {r6           10006}
    {r7           10007}
    {r8           10008}
    {r9           10009}
    {r10          10010}
    {r11          10011}
    {r12          10012}
    {r13          10013}
    {r14          10014}
    {r15          10015}
    {pc           10015}

    {r8_sys       10008}
    {r9_sys       10009}
    {r10_sys      10010}
    {r11_sys      10011}
    {r12_sys      10012}
    {r13_sys      10013}
    {r14_sys      10014}
    {r15_sys      10015}

    {r8_fiq       10016}
    {r9_fiq       10017}
    {r10_fiq      10018}
    {r11_fiq      10019}
    {r12_fiq      10020}
    {r13_fiq      10021}
    {r14_fiq      10022}

    {r13_svc      10023}
    {r14_svc      10024}

    {r13_abt      10025}
    {r14_abt      10026}

    {r13_irq      10027}
    {r14_irq      10028}

    {r13_und      10029}
    {r14_und      10030}

    {cpsr         2000}
    {spsr_fiq     2001}
    {spsr_svc     2002}
    {spsr_abt     2003}
    {spsr_irq     2004}
    {spsr_und     2005}

    {idcode       100}

    {didr         0x001e0000}
    {dscr         0x001e0010}
    {dtr          0x001e0050}
    {vcr          0x001e0070}

    {bvr0         0x001e0004}
    {bvr1         0x001e0014}
    {bvr2         0x001e0024}
    {bvr3         0x001e0034}
    {bvr4         0x001e0044}
    {bvr5         0x001e0054}

    {bcr0         0x001e0005}
    {bcr1         0x001e0015}
    {bcr2         0x001e0025}
    {bcr3         0x001e0035}
    {bcr4         0x001e0045}
    {bcr5         0x001e0055}

    {wvr0         0x001e0006}
    {wvr1         0x001e0016}

    {wcr0         0x001e0007}
    {wcr1         0x001e0017}

    {midr         0x001f0000}
    {ctr          0x001f0001}
    {tcmsr        0x001f0002}
    {tlbtr        0x001f0003}
    {cr           0x001f0100}
    {acr          0x001f0101}
    {cacr         0x001f0102}
    {ttb0r        0x001f0200}
    {ttb1r        0x001f0201}
    {ttbcr        0x001f0202}
    {dacr         0x001f0300}
    {dfsr         0x001f0500}
    {ifsr         0x001f0501}
    {dfar         0x001f0600}
    {ifar         0x001f0601}
    {cor          0x001f0700}
    {tlbor        0x001f0800}
    {dclr         0x001f0900}
    {iclr         0x001f0901}
    {dtcmrr       0x001f0910}
    {itcmrr       0x001f0911}
    {tlblr        0x001f0a00}
    {dmaisr0      0x001f0b00}
    {dmaisr1      0x001f0b01}
    {dmaisr2      0x001f0b02}
    {dmaisr3      0x001f0b03}
    {dmauar       0x001f0b10}
    {dmacnr       0x001f0b20}
    {dmaer        0x001f0b30}
    {dmacr        0x001f0b40}
    {dmaisar      0x001f0b50}
    {dmaesar      0x001f0b60}
    {dmaiear      0x001f0b70}
    {dmacsr       0x001f0b80}
    {dmacidr      0x001f0bf0}
    {fcsepidr     0x001f0d00}
    {cidr         0x001f0d01}
    {dmrr         0x001f0f20}
    {imrr         0x001f0f21}
    {dmamrr       0x001f0f22}
    {ppmrr        0x001f0f24}
    {pmcr         0x001f0fc0}
    {ccr          0x001f0fc1}
    {cr0          0x001f0fc2}
    {cr1          0x001f0fc3}
    {ddcr         0x001f3f00}
    {idcr         0x001f3f01}
    {dtagramro    0x001f3f20}
    {itagramro    0x001f3f21}
    {icdramro     0x001f3f41}
    {icmvr        0x001f3f80}
    {iscmvr       0x001f3fa0}
    {dcmvr        0x001f3fc0}
    {dscmvr       0x001f3fe0}
    {dmtlbeo      0x001f5f40}
    {imtlbeo      0x001f5f41}
    {rmtlber      0x001f5f42}
    {wmtlber      0x001f5f44}
    {dmtlbvar     0x001f5f50}
    {imtlbvar     0x001f5f51}
    {mtlbvar      0x001f5f52}
    {dmtlbpar     0x001f5f60}
    {imtlbpar     0x001f5f61}
    {mtlbpar      0x001f5f62}
    {dmtlbar      0x001f5f70}
    {imtlbar      0x001f5f71}
    {mtlbar       0x001f5f72}
    {mtlbmvr      0x001f5fe0}
    {cdcr         0x001f7f00}
    {tlbdcr       0x001f7f10}
    {invicache    0x001f0750}
    {invilineidx  0x001f0752}
    {invdcache 	  0x001F0760}
    {invdlineidx  0x001F0762}
    {flushdcache  0x001F07A0}
    {fldlineidx   0x001F07A2}

    {fpsid        0x001a7000}
    {fpscr        0x001a7100}
    {fpexc        0x001a7800}
    {fpinst       0x001a7900}
    {fpinst2      0x001a7a00}
    {mvfr0        0x001a7700}
    {mvfr1        0x001a7600}}

foreach mreg $arm11_core_regs(core) {
    addreg [lindex $mreg 0] [lindex $mreg 1]
    lappend core_regs(core) [lindex $mreg 0]
}

# name ETM registers "etm##" where ## is the hex ETM register index
# (e.g. etm79 = ETM ID Register)
proc etm {etm_reg} {
    return [expr 0x1e1000 | (($etm_reg & 0xf) << 4) | (($etm_reg >> 4) & 7)]
}
for {set etm_reg 0} {$etm_reg < 0x80} {incr etm_reg} {
    addreg [join [list "etm" [format "%02x" $etm_reg]] ""] [etm $etm_reg]
}

# name single-precision FPR registers "s#" where # is the decimal FPR register
# index
proc s {s_reg} {
    return [expr 0x1a0000 | (($s_reg & 0x1e) << 7) | (($s_reg & 0x1) << 2)]
}
for {set s_reg 0} {$s_reg < 32} {incr s_reg} {
    addreg [join [list "s" $s_reg] ""] [s $s_reg]
}

# name double-precision FPR registers "d#" where # is the decimal FPR
# register index
proc d {d_reg} {
    return [expr 0x3b10 | ($d_reg & 0xf)]
}
for {set d_reg 0} {$d_reg < 16} {incr d_reg} {
    addreg [join [list "d" $d_reg] ""] [d $d_reg]
}

# name double-precision low FPR registers "dl#" where # is the decimal FPR
# register index
proc dl {dl_reg} {
    return [expr 0x1b0000 | (($dl_reg & 0xf) << 8)]
}
for {set dl_reg 0} {$dl_reg < 16} {incr dl_reg} {
    addreg [join [list "dl" $dl_reg] ""] [dl $dl_reg]
}

# name double-precision high FPR registers "dh#" where # is the decimal FPR
# register index
proc dh {dh_reg} {
    return [expr 0x1b1000 | (($dh_reg & 0xf) << 8)]
}
for {set dh_reg 0} {$dh_reg < 16} {incr dh_reg} {
    addreg [join [list "dh" $dh_reg] ""] [dh $dh_reg]
}
