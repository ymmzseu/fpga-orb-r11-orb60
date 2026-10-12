# @brief Validate clock coverage from the Vivado verbose report body.
# @param report Complete check_timing report text, not its table of contents.
# @return Two validated zero counts; errors on missing sections or nonzero counts.
proc next_check_timing_coverage {report} {
    set checks {
        no_clock {register/latch pins with no clock\.}
        unconstrained_internal_endpoints {pins that are not constrained for maximum delay\.}
    }
    set counts {}
    foreach {category sentence} $checks {
        # The section heading must be followed by its divider and count sentence.
        # This excludes the identically named table-of-contents entry.
        set pattern [format {(?m)^[0-9]+\.[ \t]+checking %s(?:[ \t]+\(([0-9]+)\))?[ \t]*\r?\n-+\r?\n[ \t]*There are ([0-9]+) %s} $category $sentence]
        if {![regexp $pattern $report unused heading_count count]} {
            error "Cannot validate routed timing coverage for $category; review the check_timing report"
        }
        if {$heading_count ne "" && $heading_count != $count} {error "Inconsistent timing coverage counts for $category"}
        if {$count != 0} { error "Unresolved routed timing coverage: $category=$count" }
        lappend counts $category $count
    }
    return $counts
}
