# Current Project Status Report
Date: 2025-11-22
Based on Codebase Analysis

## 1. Project Overview
Developing a Falling Sand/Powder Toy game using C++ and WebAssembly. The project focuses on high performance using WASM but currently faces limitations in physics realism due to simulation constraints.

## 2. Critical Issues & Sacrifices

### A. Temperature System (Incomplete)
- **Status:** Partially Implemented (Shell Only)
- **Details:** 
  - Heat conduction logic exists in `heat_conduction.cpp`.
  - However, temperature checks in chemical reactions (`reaction_registry.cpp`) are commented out.
  - User controls for increasing/decreasing temperature have been removed.
  - **Impact:** Temperature exists as a value but drives almost no gameplay mechanics.

### B. Fluid Dynamics (Buggy/Slow)
- **Status:** Suboptimal
- **Details:** 
  - Fluid leveling (water finding a flat surface) is extremely slow.
  - This is likely due to particles only moving 1 pixel/cell per frame or lacking multi-pass horizontal scanning.
  - **Risk:** At 60fps, the "settling" logic might be entering infinite loops or oscillation, preventing a true rest state.

### C. Gas Dynamics & Combustion (Flawed)
- **Status:** Unnatural Behavior
- **Details:** 
  - **Vertical Locking:** Gases move straight up without horizontal dispersion until they hit an obstacle.
  - **Choking Fire:** In `combustion.cpp`, Wood + Fire reaction instantly turns the catalyzing Fire particle into CO2.
  - Because Gas logic is "move up", if the CO2 cannot immediately escape (blocked by wood above), it stays in place, displacing the Fire and stopping the chain reaction.
  - **Result:** Fire burns poorly and looks suffocated.

## 3. Recommendations
1. **Fix Gas Logic First:** Implement "random horizontal movement" for gases when vertical movement is blocked. This solves the CO2 trapping issue.
2. **Adjust Combustion:** Change the reaction so CO2 spawns in an empty adjacent cell instead of replacing the active fire, or give CO2 a brief "hot gas" velocity boost.
3. **Optimize Fluids:** Implement a "slip" mechanic or increase horizontal velocity for liquids to settle faster.

## 4. Missing Features (Sacrificed)
- Advanced thermodynamics (State changes based on temp).
- Complex air pressure simulation.
- Manual temperature tools.
