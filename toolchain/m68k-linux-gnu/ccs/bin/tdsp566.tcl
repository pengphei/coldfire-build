addreg a0
addreg a1
addreg a2
addreg b0
addreg b1
addreg b2
addreg la
addreg lc
addreg m0
addreg m1
addreg m2
addreg m3
addreg m4
addreg m5
addreg m6
addreg m7
addreg n0
addreg n1
addreg n2
addreg n3
addreg n4
addreg n5
addreg n6
addreg n7
addreg omr
addreg r0
addreg r1
addreg r2
addreg r3
addreg r4
addreg r5
addreg r6
addreg r7
addreg sp
addreg sr
addreg ssh
addreg ssl
addreg x0
addreg x1
addreg y0
addreg y1
addreg ep
addreg sz
addreg sc
addreg vba
addreg pc
addreg unused0
addreg unused1
addreg unused2
addreg unused3
addreg unused4
addreg unused5
addreg unused6
addreg unused7
addreg unused8
addreg unused9
addreg unused10
addreg unused11
addreg unused12
addreg unused13
addreg unused14
addreg oscr
addreg opdbr
addreg opilr
addreg obc
addreg obcr
addreg omlr0
addreg omlr1
addreg ogdbr
addreg otc
addreg opabfr
addreg opabdr
addreg opabex
addreg ofifo0
addreg ofifo1
addreg ofifo2
addreg ofifo3
addreg ofifo4
addreg ofifo5
addreg ofifo6
addreg ofifo7
addreg idcode


set Stat(inst) 3
set Stat(icnt) 3
set Stat(vname) dsp566
set Stat(core) "OnyxUL"
set Stat(reg)  {vba ssh ssl ep sz sc a0 a1 a2 b0 b1 b2 x0 x1 y0 y1 r0 r1 r2 r3 r4 r5 r6 r7 m0 m1 m2 m3 m4 m5 m6 m7 n0 n1 n2 n3 n4 n5 n6 n7 omr la lc sp sr pc}
set Stat(size) {4   4   4   4  4  4  4  4  2  4  4  2  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  4  6   4  4  6  6  4}
set    Stat(fmt) {               x1=$fmt(x1)   x0=$fmt(x0) r7=$fmt(r7) n7=$fmt(n7) m7=$fmt(m7)\n}
append Stat(fmt) {               y1=$fmt(y1)   y0=$fmt(y0) r6=$fmt(r6) n6=$fmt(n6) m6=$fmt(m6)\n}
append Stat(fmt) {     a2 $fmt(a2)   a1=$fmt(a1)   a0=$fmt(a0) r5=$fmt(r5) n5=$fmt(n5) m5=$fmt(m5)\n}
append Stat(fmt) {     b2 $fmt(b2)   b1=$fmt(b1)   b0=$fmt(b0) r4=$fmt(r4) n4=$fmt(n4) m4=$fmt(m4)\n}
append Stat(fmt) {                                     r3=$fmt(r3) n3=$fmt(n3) m3=$fmt(m3)\n}
append Stat(fmt) { la $fmt(la)   lc=$fmt(lc)  omr=$fmt(omr) r2=$fmt(r2) n2=$fmt(n2) m2=$fmt(m2)\n}
append Stat(fmt) { ep $fmt(ep)   sz=$fmt(sz)   sc=$fmt(sc)   r1=$fmt(r1) n1=$fmt(n1) m1=$fmt(m1)\n}
append Stat(fmt) {vba $fmt(vba)  ssh=$fmt(ssh)  ssl=$fmt(ssl)   r0=$fmt(r0) n0=$fmt(n0) m0=$fmt(m0)\n}
append Stat(fmt) {pc $fmt(pc)}

