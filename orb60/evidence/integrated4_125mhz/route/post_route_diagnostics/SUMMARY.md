# Routed diagnostics

200 unique-endpoint paths; 44 distinct observed families.

| Hotspot / destination register | WNS/ns | Logic/ns | Routing/ns | Levels | Data fanout | Endpoint |
|---|---:|---:|---:|---:|---:|---|
| grid_selection / ram_reg_0 | 0.167 | 3.267 | 4.200 | 7 | 98 | DIBDI[4] |
| s_i / ap_loop_init_int_reg | 0.276 | 0.588 | 7.001 | 1 | 325 | D |
| grid_selection / ram_reg_0 | 0.285 | 3.267 | 4.109 | 7 | 98 | DIADI[4] |
| grid_selection / ram_reg_1 | 0.304 | 3.267 | 4.032 | 7 | 98 | DIADI[11] |
| s_i / ap_block_pp0_stage0_subdone_grp1_done_reg_reg | 0.348 | 0.588 | 6.950 | 1 | 325 | D |

Raw timing, fanout load types and detailed control sets: D:\pld\.work\vivado\orb60\i6685ae8e\vivado\post_route_diagnostics
