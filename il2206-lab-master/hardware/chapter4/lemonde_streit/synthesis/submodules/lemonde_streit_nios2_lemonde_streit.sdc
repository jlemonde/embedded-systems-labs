# Legal Notice: (C)2018 Altera Corporation. All rights reserved.  Your
# use of Altera Corporation's design tools, logic functions and other
# software and tools, and its AMPP partner logic functions, and any
# output files any of the foregoing (including device programming or
# simulation files), and any associated documentation or information are
# expressly subject to the terms and conditions of the Altera Program
# License Subscription Agreement or other applicable license agreement,
# including, without limitation, that your use is for the sole purpose
# of programming logic devices manufactured by Altera and sold by Altera
# or its authorized distributors.  Please refer to the applicable
# agreement for further details.

#**************************************************************
# Timequest JTAG clock definition
#   Uncommenting the following lines will define the JTAG
#   clock in TimeQuest Timing Analyzer
#**************************************************************

#create_clock -period 10MHz {altera_internal_jtag|tckutap}
#set_clock_groups -asynchronous -group {altera_internal_jtag|tckutap}

#**************************************************************
# Set TCL Path Variables 
#**************************************************************

set 	lemonde_streit_nios2_lemonde_streit 	lemonde_streit_nios2_lemonde_streit:*
set 	lemonde_streit_nios2_lemonde_streit_oci 	lemonde_streit_nios2_lemonde_streit_nios2_oci:the_lemonde_streit_nios2_lemonde_streit_nios2_oci
set 	lemonde_streit_nios2_lemonde_streit_oci_break 	lemonde_streit_nios2_lemonde_streit_nios2_oci_break:the_lemonde_streit_nios2_lemonde_streit_nios2_oci_break
set 	lemonde_streit_nios2_lemonde_streit_ocimem 	lemonde_streit_nios2_lemonde_streit_nios2_ocimem:the_lemonde_streit_nios2_lemonde_streit_nios2_ocimem
set 	lemonde_streit_nios2_lemonde_streit_oci_debug 	lemonde_streit_nios2_lemonde_streit_nios2_oci_debug:the_lemonde_streit_nios2_lemonde_streit_nios2_oci_debug
set 	lemonde_streit_nios2_lemonde_streit_wrapper 	lemonde_streit_nios2_lemonde_streit_jtag_debug_module_wrapper:the_lemonde_streit_nios2_lemonde_streit_jtag_debug_module_wrapper
set 	lemonde_streit_nios2_lemonde_streit_jtag_tck 	lemonde_streit_nios2_lemonde_streit_jtag_debug_module_tck:the_lemonde_streit_nios2_lemonde_streit_jtag_debug_module_tck
set 	lemonde_streit_nios2_lemonde_streit_jtag_sysclk 	lemonde_streit_nios2_lemonde_streit_jtag_debug_module_sysclk:the_lemonde_streit_nios2_lemonde_streit_jtag_debug_module_sysclk
set 	lemonde_streit_nios2_lemonde_streit_oci_path 	 [format "%s|%s" $lemonde_streit_nios2_lemonde_streit $lemonde_streit_nios2_lemonde_streit_oci]
set 	lemonde_streit_nios2_lemonde_streit_oci_break_path 	 [format "%s|%s" $lemonde_streit_nios2_lemonde_streit_oci_path $lemonde_streit_nios2_lemonde_streit_oci_break]
set 	lemonde_streit_nios2_lemonde_streit_ocimem_path 	 [format "%s|%s" $lemonde_streit_nios2_lemonde_streit_oci_path $lemonde_streit_nios2_lemonde_streit_ocimem]
set 	lemonde_streit_nios2_lemonde_streit_oci_debug_path 	 [format "%s|%s" $lemonde_streit_nios2_lemonde_streit_oci_path $lemonde_streit_nios2_lemonde_streit_oci_debug]
set 	lemonde_streit_nios2_lemonde_streit_jtag_tck_path 	 [format "%s|%s|%s" $lemonde_streit_nios2_lemonde_streit_oci_path $lemonde_streit_nios2_lemonde_streit_wrapper $lemonde_streit_nios2_lemonde_streit_jtag_tck]
set 	lemonde_streit_nios2_lemonde_streit_jtag_sysclk_path 	 [format "%s|%s|%s" $lemonde_streit_nios2_lemonde_streit_oci_path $lemonde_streit_nios2_lemonde_streit_wrapper $lemonde_streit_nios2_lemonde_streit_jtag_sysclk]
set 	lemonde_streit_nios2_lemonde_streit_jtag_sr 	 [format "%s|*sr" $lemonde_streit_nios2_lemonde_streit_jtag_tck_path]

#**************************************************************
# Set False Paths
#**************************************************************

set_false_path -from [get_keepers *$lemonde_streit_nios2_lemonde_streit_oci_break_path|break_readreg*] -to [get_keepers *$lemonde_streit_nios2_lemonde_streit_jtag_sr*]
set_false_path -from [get_keepers *$lemonde_streit_nios2_lemonde_streit_oci_debug_path|*resetlatch]     -to [get_keepers *$lemonde_streit_nios2_lemonde_streit_jtag_sr[33]]
set_false_path -from [get_keepers *$lemonde_streit_nios2_lemonde_streit_oci_debug_path|monitor_ready]  -to [get_keepers *$lemonde_streit_nios2_lemonde_streit_jtag_sr[0]]
set_false_path -from [get_keepers *$lemonde_streit_nios2_lemonde_streit_oci_debug_path|monitor_error]  -to [get_keepers *$lemonde_streit_nios2_lemonde_streit_jtag_sr[34]]
set_false_path -from [get_keepers *$lemonde_streit_nios2_lemonde_streit_ocimem_path|*MonDReg*] -to [get_keepers *$lemonde_streit_nios2_lemonde_streit_jtag_sr*]
set_false_path -from *$lemonde_streit_nios2_lemonde_streit_jtag_sr*    -to *$lemonde_streit_nios2_lemonde_streit_jtag_sysclk_path|*jdo*
set_false_path -from sld_hub:*|irf_reg* -to *$lemonde_streit_nios2_lemonde_streit_jtag_sysclk_path|ir*
set_false_path -from sld_hub:*|sld_shadow_jsm:shadow_jsm|state[1] -to *$lemonde_streit_nios2_lemonde_streit_oci_debug_path|monitor_go
