addreg did
addreg csc
addreg e1_dc
addreg e1_ds
addreg e1_wt
addreg e1_dtc
addreg e1_bwc1
addreg e1_bwc2
addreg e1_bwc3
addreg e1_bwa1
addreg e1_bwa2
addreg e1_bwd1
addreg e1_bwd2
addreg e1_ptce
addreg e1_inst
addreg e1_mpc
addreg e1_csfr
addreg e2_dc
addreg e2_ds
addreg e2_wt
addreg e2_dtc
addreg e2_bwc1
addreg e2_bwc2
addreg e2_bwc3
addreg e2_bwa1
addreg e2_bwa2
addreg e2_bwd1
addreg e2_bwd2
addreg e2_ptce
addreg e2_inst
addreg e2_mpc
addreg e2_csfr
addreg cdc_dtc
addreg dtar0
addreg dtar1
addreg dtar2
addreg dtar3

set Stat(icnt) 0
set Stat(vname) ndedi
set Stat(core) "Nexus eTPU Development Interface"
set Stat(reg) {did csc e1_dc e1_ds e1_wt e1_dtc e1_bwc1 e1_bwc2 e1_bwc3 e1_bwa1 e1_bwa2 e1_bwd1 e1_bwd2 e1_ptce e1_inst e1_mpc e1_csfr e2_dc e2_ds e2_wt e2_dtc e2_bwc1 e2_bwc2 e2_bwc3 e2_bwa1 e2_bwa2 e2_bwd1 e2_bwd2 e2_ptce e2_inst e2_mpc e2_csfr cdc_dtc dtar0 dtar1 dtar2 dtar3}
set Stat(size) {8 2 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8}
set Stat(fmt) {   did $fmt(did)   csc $fmt(csc)\n}
append Stat(fmt) <f=darkgreen>Engine 1\n<f=x>
append Stat(fmt) {    dc $fmt(e1_dc)    ds $fmt(e1_ds)    wt $fmt(e1_wt)\n}
append Stat(fmt) {  bwc1 $fmt(e1_bwc1)  bwc2 $fmt(e1_bwc2)  bwc3 $fmt(e1_bwc3)\n}
append Stat(fmt) {  bwa1 $fmt(e1_bwa1)  bwa2 $fmt(e1_bwa2)  bwd1 $fmt(e1_bwd1)  bwd2 $fmt(e1_bwd2)\n}
append Stat(fmt) {  ptce $fmt(e1_ptce)  inst $fmt(e1_inst)   mpc $fmt(e1_mpc)  csfr $fmt(e1_csfr)\n}
append Stat(fmt) <f=darkgreen>Engine 2\n<f=x>
append Stat(fmt) {    dc $fmt(e2_dc)    ds $fmt(e2_ds)    wt $fmt(e2_wt)\n}
append Stat(fmt) {  bwc1 $fmt(e2_bwc1)  bwc2 $fmt(e2_bwc2)  bwc3 $fmt(e2_bwc3)\n}
append Stat(fmt) {  bwa1 $fmt(e2_bwa1)  bwa2 $fmt(e2_bwa2)  bwd1 $fmt(e2_bwd1)  bwd2 $fmt(e2_bwd2)\n}
append Stat(fmt) {  ptce $fmt(e2_ptce)  inst $fmt(e2_inst)   mpc $fmt(e2_mpc)  csfr $fmt(e2_csfr)\n}
append Stat(fmt) <f=darkgreen>CDC\n<f=x>
append Stat(fmt) {   dtc $fmt(e2_dtc)\n}
append Stat(fmt) <f=darkgreen>Shared\n<f=x>
append Stat(fmt) { dtar0 $fmt(dtar0) dtar1 $fmt(dtar1) dtar2 $fmt(dtar2) dtar3 $fmt(dtar3)}
