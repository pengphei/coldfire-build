set j2i2cv2_regs(core) {
    {idcode   0}}

set core_regs(core) {}

foreach mreg $j2i2cv2_regs(core) {
    addreg [lindex $mreg 0] [lindex $mreg 1]
    lappend core_regs(core) [lindex $mreg 0]
}

set Stat(inst)  2
set Stat(icnt)  6
set Stat(vname) j2i2cv2
set Stat(core)  "JTAG to I2C. Version 2"
set Stat(reg)   {}
set Stat(size)  {}
set Stat(fmt)   {}

