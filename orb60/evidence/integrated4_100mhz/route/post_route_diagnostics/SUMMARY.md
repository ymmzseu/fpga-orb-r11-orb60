# Routed diagnostics

200 unique-endpoint paths; 47 distinct observed families.

| Hotspot / destination register | WNS/ns | Logic/ns | Routing/ns | Levels | Data fanout | Endpoint |
|---|---:|---:|---:|---:|---:|---|
| s_i / mem_reg | 0.188 | 0.398 | 8.581 | 0 | 326 | RSTRAMARSTRAM |
| s_i / ap_sync_reg_entry_proc28_U0_ap_ready_reg | 0.320 | 0.630 | 8.666 | 1 | 326 | D |
| FAST_score / select_ln36_1_reg_581_reg | 0.481 | 3.618 | 5.533 | 12 | 24 | D |
| s_i / ap_block_pp0_stage0_subdone_grp1_done_reg_reg | 0.554 | 0.630 | 8.468 | 1 | 326 | D |
| FAST_score / select_ln36_8_reg_616_reg | 0.571 | 3.828 | 5.362 | 12 | 24 | D |

Raw timing, fanout load types and detailed control sets: D:\pld\.work\vivado\orb60\i2c712921\vivado\post_route_diagnostics
