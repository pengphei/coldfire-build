set j2i2c_regs(core) {
    {idcode   0}
    {adr      1}
    {fdr      2}
    {cr       3}
    {sr       4}
    {dr       5}
    {dfsr     6}
    {gpr      7}
    {bcsr0  100}
    {bcsr1  101}
    {bcsr2  102}
    {bcsr3  103}
    {bcsr4  104}
    {bcsr5  105}
    {bcsr6  106}
    {bcsr7  107}
    {bcsr8  108}
    {bcsr9  109}
    {bcsr10 110}
    {bcsr11 111}
    {bcsr12 112}
    {bcsr13 113}
    {bcsr14 114}
    {bcsr15 115}}

set core_regs(core) {}

foreach mreg $j2i2c_regs(core) {
    addreg [lindex $mreg 0] [lindex $mreg 1]
    lappend core_regs(core) [lindex $mreg 0]
}

set Stat(inst)  2
set Stat(icnt)  6
set Stat(vname) j2i2c
set Stat(core)  "JTAG to IIC"
set Stat(reg)   {}
set Stat(size)  {}
set Stat(fmt)   {}

