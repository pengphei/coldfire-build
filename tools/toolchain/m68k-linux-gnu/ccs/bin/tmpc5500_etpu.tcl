addreg pc  
addreg flags  
addreg a  
addreg p  
addreg diob  
addreg ert1  
addreg ert2  
addreg sr  
addreg macl  
addreg mach  
addreg c  
addreg link  
addreg rar  
addreg chan  
addreg tcr1  
addreg tcr2  
addreg b  
addreg tpr  
addreg trr  
addreg d  

addreg fc
addreg fn
addreg fv
addreg fz
addreg mn
addreg mc
addreg mz
addreg mv
addreg mb
addreg lsr
addreg pss

addreg pc_dummy

addreg mcr  
addreg cdcr  
addreg misccmpr  
addreg ecr  

addreg tbcr1
addreg tb1
addreg tb2
addreg redcr

addreg cisr
addreg cdtrsr
addreg ciosr
addreg cdtrosr
addreg cier
addreg cdtrer
addreg cpssr
addreg cssr

addreg srl
addreg sgl

addreg hsr

addreg c0cr
addreg c0scr
addreg c0hsrr
addreg c1cr
addreg c1scr
addreg c1hsrr
addreg c2cr
addreg c2scr
addreg c2hsrr
addreg c3cr
addreg c3scr
addreg c3hsrr
addreg c4cr
addreg c4scr
addreg c4hsrr
addreg c5cr
addreg c5scr
addreg c5hsrr
addreg c6cr
addreg c6scr
addreg c6hsrr
addreg c7cr
addreg c7scr
addreg c7hsrr
addreg c8cr
addreg c8scr
addreg c8hsrr
addreg c9cr
addreg c9scr
addreg c9hsrr
addreg c10cr
addreg c10scr
addreg c10hsrr
addreg c11cr
addreg c11scr
addreg c11hsrr
addreg c12cr
addreg c12scr
addreg c12hsrr
addreg c13cr
addreg c13scr
addreg c13hsrr
addreg c14cr
addreg c14scr
addreg c14hsrr
addreg c15cr
addreg c15scr
addreg c15hsrr
addreg c16cr
addreg c16scr
addreg c16hsrr
addreg c17cr
addreg c17scr
addreg c17hsrr
addreg c18cr
addreg c18scr
addreg c18hsrr
addreg c19cr
addreg c19scr
addreg c19hsrr
addreg c20cr
addreg c20scr
addreg c20hsrr
addreg c21cr
addreg c21scr
addreg c21hsrr
addreg c22cr
addreg c22scr
addreg c22hsrr
addreg c23cr
addreg c23scr
addreg c23hsrr
addreg c24cr
addreg c24scr
addreg c24hsrr
addreg c25cr
addreg c25scr
addreg c25hsrr
addreg c26cr
addreg c26scr
addreg c26hsrr
addreg c27cr
addreg c27scr
addreg c27hsrr
addreg c28cr
addreg c28scr
addreg c28hsrr
addreg c29cr
addreg c29scr
addreg c29hsrr
addreg c30cr
addreg c30scr
addreg c30hsrr
addreg c31cr
addreg c31scr
addreg c31hsrr

addreg c0match1
addreg c0capture1
addreg c0match2
addreg c0capture2
addreg c0mrl1
addreg c0mrl2
addreg c0mrle1
addreg c0mrle2
addreg c0tdl1
addreg c0tdl2
addreg c0sri
addreg c0ipac1
addreg c0ipac2
addreg c0opac1
addreg c0opac2
addreg c0tbs1
addreg c0tbs2
addreg c0pdcm
addreg c0ipin
addreg c0opin
addreg c0dummycr
addreg c0dummyscr
addreg c0dummyhsrr
addreg c0fm
addreg c0itdl1
addreg c0itld2
addreg c0imrl1
addreg c0imrl2
addreg c0obe
addreg c0pss
addreg c0smlck
addreg c0flag0
addreg c0flag1

set Stat(inst) 4
set Stat(icnt) 6
set Stat(memspace) 2
set Stat(vname) netpu
set Stat(core) "Nexus eTPU"
set Stat(reg) {pc flags p diob a b c d sr rar chan macl mach ert1 ert2 tcr1 tcr2 tpr trr mcr cdcr misccmpr ecr}
set Stat(size) {4 8 8 6 6 6 6 6 6 4 2 6 6 6 6 6 6 4 6 8 8 8 8}
set Stat(fmt) { flags $fmt(flags)      p $fmt(p)     diob $fmt(diob)\n}
append Stat(fmt) {     a $fmt(a)        b $fmt(b)          c $fmt(c)       d $fmt(d)\n}
append Stat(fmt) {    sr $fmt(sr)      rar $fmt(rar)         chan $fmt(chan)\n}
append Stat(fmt) {  macl $fmt(macl)     mach $fmt(mach)       ert1 $fmt(ert1)    ert2 $fmt(ert2)\n}
append Stat(fmt) {  tcr1 $fmt(tcr1)     tcr2 $fmt(tcr2)        tpr $fmt(tpr)       trr $fmt(trr)\n}
append Stat(fmt) {   mcr $fmt(mcr)   cdcr $fmt(cdcr)  misccmpr $fmt(misccmpr)  ecr $fmt(ecr)\n}
append Stat(fmt) { pc $fmt(pc)}
