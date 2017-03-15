EMTF firmware modules
=====================

The 'top' module in firmware is the module `sp`. The following is the dependency graph:

- module `sp`
  - module `prim_conv_sector`
    - module `prim_conv`
    - module `prim_conv11`
  - module `zones`
  - module `extend_sector`
    - module `extender`
  - module `ph_pattern_sector`
    - module `ph_pattern`
  - module `sort_sector`
    - module `zone_best3`
      - module `zone_best` (in sort_zone.sv)
  - module `coord_delay`
  - module `match_ph_segments`
    - module `find_segment`
  - module `deltas_sector`
    - module `deltas`
      - module `best_delta`
  - module `best_tracks`
  - module `single`
  - module `ptlut_address`

In the emulator, the following processor classes perform the jobs of the firmware modules:

- class `SectorProcessor`
  - class `PrimitiveSelection`
    - N/A (the job is done by input links/cables)
  - class `PrimitiveConversion`
    - module `prim_conv_sector`
  - class `PatternRecognition`
    - modules `zones`, `extend_sector`, `ph_pattern_sector`, `sort_sector`
  - class `PrimitiveMatching`
    - module `match_ph_segments`
  - class `AngleCalculation`
    - module `deltas_sector`
  - class `BestTrackSelection`
    - module `best_tracks`
  - classes `PtAssignment`, `PtAssignmentEngine`
    - module `ptlut_address`

