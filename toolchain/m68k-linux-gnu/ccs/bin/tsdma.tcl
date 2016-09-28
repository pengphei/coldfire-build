addreg rpc      [expr 0]
addreg lm       [expr 1]
addreg epc      [expr 2]
addreg spc      [expr 3]
addreg pc       [expr 4]
addreg t        [expr 5]
addreg df       [expr 6]
addreg sf       [expr 7]

addreg gr0      [expr 12]
addreg gr1      [expr 13]
addreg gr2      [expr 14]
addreg gr3      [expr 15]
addreg gr4      [expr 16]
addreg gr5      [expr 17]
addreg gr6      [expr 18]
addreg gr7      [expr 19]
addreg mda      [expr 20]
addreg msa      [expr 21]
addreg ms       [expr 22]
addreg md       [expr 23]
addreg pda      [expr 24]
addreg psa      [expr 25]
addreg ps       [expr 26]
addreg pd       [expr 27]
addreg ca       [expr 28]
addreg cs       [expr 29]
addreg dda      [expr 30]
addreg dsa      [expr 31]
addreg ds       [expr 32]
addreg dd       [expr 33]
addreg sr0      [expr 34]
addreg sr1      [expr 35]
addreg sr2      [expr 36]
addreg sr3      [expr 37]
addreg sr4      [expr 38]
addreg sr5      [expr 39]
addreg sr6      [expr 40]
addreg sr7      [expr 41]

addreg mc0ptr   [expr 50]
addreg dc0ptr   [expr 51]
addreg ccptr    [expr 52]
addreg ccr      [expr 53]
addreg ncr      [expr 54]
addreg events   [expr 55]
addreg ccpri    [expr 56]
addreg ncpri    [expr 57]
addreg ecount   [expr 58]
addreg ectl     [expr 59]
addreg eaa      [expr 60]
addreg eab      [expr 61]
addreg eam      [expr 62]
addreg ed       [expr 63]
addreg edm      [expr 64]
addreg rtb      [expr 73]
addreg osr      [expr 75]
addreg mchn0addr [expr 77]
addreg endianess [expr 78]
addreg lock      [expr 79]
addreg events2  [expr 80]

addreg tb0      [expr 90]
addreg tb1      [expr 91]
addreg tb2      [expr 92]
addreg tb3      [expr 93]
addreg tb4      [expr 94]
addreg tb5      [expr 95]
addreg tb6      [expr 96]
addreg tb7      [expr 97]
addreg tb8      [expr 98]
addreg tb9      [expr 99]
addreg tb10     [expr 100]
addreg tb11     [expr 101]
addreg tb12     [expr 102]
addreg tb13     [expr 103]
addreg tb14     [expr 104]
addreg tb15     [expr 105]
addreg tb16     [expr 106]
addreg tb17     [expr 107]
addreg tb18     [expr 108]
addreg tb19     [expr 109]
addreg tb20     [expr 110]
addreg tb21     [expr 111]
addreg tb22     [expr 112]
addreg tb23     [expr 113]
addreg tb24     [expr 114]
addreg tb25     [expr 115]
addreg tb26     [expr 116]
addreg tb27     [expr 117]
addreg tb28     [expr 118]
addreg tb29     [expr 119]
addreg tb30     [expr 120]
addreg tb31     [expr 121]

set arm11       2

proc wakeup {} {

    upvar sdma::arm11 arm11

    #Point to user defined bood routine at address 0x00001500
    #CHN0ADDR
    ccs::write_mem $arm11 0x53fd405c 4 0 0x00001500

    # Set channel 0 priority to highest
    #CHNPRIn
    ccs::write_mem $arm11 0x53fd4100 4 0 0x00000007

    # Ignore scheduling events on channel 0
    #EVTOVR
    ccs::write_mem $arm11 0x53fd4010 4 0 0x00000001

    #DSPOVR
    ccs::write_mem $arm11 0x53fd4014 4 0 0xFFFFFFFF

    #HOSTOVR
    ccs::write_mem $arm11 0x53fd4018 4 0 0x00000080

    # Start channel 0
    #HSTART
    ccs::write_mem $arm11 0x53fd400c 4 0 0x00000001
}


proc wakeupBE {} {

    upvar sdma::arm11 arm11

    #Point to user defined bood routine at address 0x00001500
    #CHN0ADDR
    ccs::write_mem $arm11 0x53fd405c 4 0 0x00150000

    # Set channel 0 priority to highest
    #CHNPRIn
    ccs::write_mem $arm11 0x53fd4100 4 0 0x07000000

    # Ignore scheduling events on channel 0
    #EVTOVR
    ccs::write_mem $arm11 0x53fd4010 4 0 0x01000000

    #DSPOVR
    ccs::write_mem $arm11 0x53fd4014 4 0 0xFFFFFFFF

    #HOSTOVR
    ccs::write_mem $arm11 0x53fd4018 4 0 0x80000000

    # Start channel 0
    #HSTART
    ccs::write_mem $arm11 0x53fd400c 4 0 0x01000000
}
