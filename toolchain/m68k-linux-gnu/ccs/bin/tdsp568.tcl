addreg x0
addreg a0
addreg a1
addreg a2
addreg b0
addreg b1
addreg b2
addreg la
addreg lc
addreg m01
addreg n
addreg omr
addreg r0
addreg r1
addreg r2
addreg r3
addreg sp
addreg sr
addreg hws
addreg y0
addreg y1
addreg opdbr
addreg opilr
addreg ocntr
addreg ocr
addreg obar
addreg opgdbr
addreg obar2
addreg obmsk2
addreg obctl2
addreg opabfr
addreg opaber
addreg opabdr
addreg opabf0
addreg opabf1
addreg opabf2
addreg opabf3
addreg opabf4
addreg opabf5
addreg opabf6
addreg opabf7
addreg osr
addreg idcode
addreg pc

set Stat(inst) 2
set Stat(icnt) 4
set Stat(vname) hv1
set Stat(core) "HawkV1"
set Stat(reg)  {r0 r1 r2 r3 la lc m01 a0 a1 a2 b0 b1 b2 sr sp omr hws pc y0 y1 x0 n} 
set Stat(size) {4  4  4  4  4  4  4   4  4  1  4  4  1  4  4  4   4   4  4  4  4  4}
set    Stat(fmt) {x0 $fmt(x0)\ty1 $fmt(y1)\t y0 $fmt(y0)\tr0 $fmt(r0)\n}
append Stat(fmt) {a2 $fmt(a2)\t\ta1 $fmt(a1)\t a0 $fmt(a0)\tr1 $fmt(r1)\n}
append Stat(fmt) {b2 $fmt(b2)\t\tb1 $fmt(b1)\t b0 $fmt(b0)\tr2 $fmt(r2)\n}
append Stat(fmt) {lc $fmt(lc)\tla $fmt(la)\tm01 $fmt(m01)\tr3 $fmt(r3)\n}
append Stat(fmt) {sr $fmt(sr)\tsp $fmt(sp)\tomr $fmt(omr)\t n $fmt(n)\n}
append Stat(fmt) {pc $fmt(pc)}
