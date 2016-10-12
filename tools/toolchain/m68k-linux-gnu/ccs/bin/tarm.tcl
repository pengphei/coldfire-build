set arm_core_regs(core) {
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

    {dcr          200}
    {dsr          201}
    {asr          202}
    {vcr          202}
    {dccr         203}
    {dcdr         204}
    {w0av         205}
    {w0am         206}
    {w0dv         207}
    {w0dm         208}
    {w0cv         209}
    {w0cm         210}
    {w1av         211}
    {w1am         212}
    {w1dv         213}
    {w1dm         214}
    {w1cv         215}
    {w1cm         216}

    {midr           0x001f0000}
    {ctr            0x001f0001}
    {tcmsr          0x001f0002}
    {tlbtr          0x001f0003}
    {cr             0x001f0100}
    {ttbr           0x001f0200}
    {dacr           0x001f0300}
    {dfsr           0x001f0500}
    {ifsr           0x001f0501}
    {far            0x001f0600}
    {cor            0x001f0700}
    {invicache      0x001f0750}
    {invimva        0x001f0751}
    {invilineidx    0x001f0752}
    {invipfline     0x001f07c1}
    {invdcache      0x001f0760}
    {invdmva        0x001f0761}
    {invdlineidx    0x001f0762}
    {invidcache     0x001f0770}
    {flushdcache    0x001f07a0}
    {fldmva         0x001f07a1}
    {fldlineidx     0x001f07a2}
    {tstfldcache    0x001f07a3}
    {flinvdmva      0x001f07d1}
    {flinvdlineidx  0x001f07d2}
    {tstflinvdcache 0x001f07d3}
    {drainwbuf      0x001f07a4}
    {waitforint     0x001f0704}
    {tlbor          0x001f0800}
    {invitlb        0x001f0850}
    {invitlbmva     0x001f0851}
    {invdtlb        0x001f0860}
    {invdtlbmva     0x001f0861}
    {invtlb         0x001f0870}
    {invtlbmva      0x001f0871}
    {dclr           0x001f0900}
    {iclr           0x001f0901}
    {dtcmrr         0x001f0910}
    {itcmrr         0x001f0911}
    {tlblr          0x001f0a00}
    {fcsepidr       0x001f0d00}
    {cidr           0x001f0d01}
    {dor            0x001f0f00}
    {dtar           0x001f0f10}
    {tcr            0x001f1f10}
    {tlbrtag        0x001f4f20}
    {tlbwtag        0x001f4f30}
    {tlbrpa         0x001f4f40}
    {tlbwpa         0x001f4f50}
    {tlbtfr         0x001f4f70}
    {tlbrtagld      0x001f4f21}
    {tlbwtagld      0x001f4f31}
    {tlbrpald       0x001f4f41}
    {tlbwpald       0x001f4f51}
    {tlbtfrld       0x001f4f71}
    {mmudcr         0x001f7f10}
    {mrrr           0x001f0f20}}
foreach mreg $arm_core_regs(core) {
    addreg [lindex $mreg 0] [lindex $mreg 1]
    lappend core_regs(core) [lindex $mreg 0]
}
