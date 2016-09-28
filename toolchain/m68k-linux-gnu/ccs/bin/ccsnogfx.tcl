## ccs needs to be cleaned up before exiting, so catch exit commands
catch {rename exit ccsRudeExit}
;proc exit {{exitValue 0}} {
        quit
}

;proc quit {} {
    ccsNiceExit
}

if {[info exists ccsExecPath]} {
    if {[file readable ${ccsExecPath}/ccsapi.tcl]} {
        after idle {
            if {[catch {source ${ccsExecPath}/ccsapi.tcl} ccsSourceError]} {
                puts stderr $ccsSourceError
            }
        }
    }
    if {[file readable ${ccsExecPath}/fsldbg_sock_api.tcl]} {
        after idle {
            if {[catch {source ${ccsExecPath}/fsldbg_sock_api.tcl} ccsSourceError]} {
                puts stderr $ccsSourceError
            }
        }
    }
}

