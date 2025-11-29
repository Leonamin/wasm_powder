// ============================================================================
// Pure JavaScript Powder Toy Implementation
// Mirrors the WASM C++ structure for performance comparison
// ============================================================================

(function(global) {
'use strict';

// ============================================================================
// Constants (from types.h)
// ============================================================================
const JS_WIDTH = 400;
const JS_HEIGHT = 300;
const GRID_SIZE = JS_WIDTH * JS_HEIGHT;

const CHUNK_SIZE = 16;
const CHUNK_WIDTH = Math.ceil(JS_WIDTH / CHUNK_SIZE);
const CHUNK_HEIGHT = Math.ceil(JS_HEIGHT / CHUNK_SIZE);
const CHUNK_COUNT = CHUNK_WIDTH * CHUNK_HEIGHT;

const GRAVITY = 0.3;
const VELOCITY_DAMPING = 0.8;
const MAX_VELOCITY_Y = 5.0;
const MAX_VELOCITY_X = 3.0;

const HEAT_CONDUCTION_BASE = 0.05;
const HEAT_CHANGE_THRESHOLD = 0.1;

// ============================================================================
// Particle Types (from particle.h)
// ============================================================================
const ParticleType = {
  EMPTY: 0,
  WALL: 1,
  SAND: 2,
  WATER: 3,
  ICE: 4,
  STEAM: 5,
  FIRE: 6,
  OXYGEN: 7,
  HYDROGEN: 8,
  STEAM_OIL: 9,
  WOOD: 10,
  IRON: 11,
  LITHIUM: 12,
  SODIUM: 13,
  OIL: 14,
  CO2: 15
};

const PhysicalState = {
  STATE_SOLID: 0,
  STATE_POWDER: 1,
  STATE_LIQUID: 2,
  STATE_GAS: 3
};

// ============================================================================
// Particle Structure
// ============================================================================
class Particle {
  constructor() {
    this.type = ParticleType.EMPTY;
    this.temperature = 20.0;
    this.state = PhysicalState.STATE_SOLID;
    this.vx = 0.0;
    this.vy = 0.0;
    this.latent_heat_storage = 0.0;
    this.life = -1;
    this.updated_this_frame = false;
  }
  
  copy() {
    const p = new Particle();
    p.type = this.type;
    p.temperature = this.temperature;
    p.state = this.state;
    p.vx = this.vx;
    p.vy = this.vy;
    p.latent_heat_storage = this.latent_heat_storage;
    p.life = this.life;
    p.updated_this_frame = this.updated_this_frame;
    return p;
  }
}

// ============================================================================
// Material Database (from material_db.h)
// ============================================================================
const MaterialDB = [
  // EMPTY
  { name: "Air", default_state: PhysicalState.STATE_GAS, density: 1.2, specific_heat: 1005.0, 
    melting_point: -999.0, boiling_point: -999.0, latent_heat_fusion: 0.0, 
    latent_heat_vaporization: 0.0, viscosity: 0.00001, color: [0, 0, 0] },
  // WALL
  { name: "Wall", default_state: PhysicalState.STATE_SOLID, density: 2500.0, specific_heat: 840.0,
    melting_point: 1500.0, boiling_point: 2800.0, latent_heat_fusion: 0.0,
    latent_heat_vaporization: 0.0, viscosity: 0.0, color: [136, 136, 136] },
  // SAND
  { name: "Sand", default_state: PhysicalState.STATE_POWDER, density: 1600.0, specific_heat: 830.0,
    melting_point: 1700.0, boiling_point: 2230.0, latent_heat_fusion: 0.0,
    latent_heat_vaporization: 0.0, viscosity: 0.0, color: [240, 230, 140] },
  // WATER
  { name: "Water", default_state: PhysicalState.STATE_LIQUID, density: 1000.0, specific_heat: 4186.0,
    melting_point: 0.0, boiling_point: 100.0, latent_heat_fusion: 334000.0,
    latent_heat_vaporization: 2260000.0, viscosity: 0.001, color: [30, 144, 255] },
  // ICE
  { name: "Ice", default_state: PhysicalState.STATE_SOLID, density: 917.0, specific_heat: 2050.0,
    melting_point: 0.0, boiling_point: 100.0, latent_heat_fusion: 334000.0,
    latent_heat_vaporization: 2260000.0, viscosity: 0.0, color: [175, 238, 238] },
  // STEAM
  { name: "Steam", default_state: PhysicalState.STATE_GAS, density: 0.6, specific_heat: 2080.0,
    melting_point: 0.0, boiling_point: 100.0, latent_heat_fusion: 334000.0,
    latent_heat_vaporization: 2260000.0, viscosity: 0.00001, color: [245, 245, 245] },
  // FIRE
  { name: "Fire", default_state: PhysicalState.STATE_GAS, density: 0.3, specific_heat: 1000.0,
    melting_point: -999.0, boiling_point: -999.0, latent_heat_fusion: 0.0,
    latent_heat_vaporization: 0.0, viscosity: 0.0, color: [255, 69, 0] },
  // OXYGEN
  { name: "Oxygen", default_state: PhysicalState.STATE_GAS, density: 1.4, specific_heat: 920.0,
    melting_point: -218.0, boiling_point: -183.0, latent_heat_fusion: 0.0,
    latent_heat_vaporization: 0.0, viscosity: 0.00002, color: [200, 220, 255] },
  // HYDROGEN
  { name: "Hydrogen", default_state: PhysicalState.STATE_GAS, density: 0.09, specific_heat: 14300.0,
    melting_point: -259.0, boiling_point: -253.0, latent_heat_fusion: 0.0,
    latent_heat_vaporization: 0.0, viscosity: 0.00001, color: [255, 200, 200] },
  // STEAM_OIL
  { name: "Oil Steam", default_state: PhysicalState.STATE_GAS, density: 2.5, specific_heat: 2000.0,
    melting_point: -999.0, boiling_point: 300.0, latent_heat_fusion: 0.0,
    latent_heat_vaporization: 500000.0, viscosity: 0.00003, color: [80, 70, 50] },
  // WOOD
  { name: "Wood", default_state: PhysicalState.STATE_SOLID, density: 600.0, specific_heat: 1700.0,
    melting_point: -999.0, boiling_point: -999.0, latent_heat_fusion: 0.0,
    latent_heat_vaporization: 0.0, viscosity: 0.0, color: [139, 69, 19] },
  // IRON
  { name: "Iron", default_state: PhysicalState.STATE_SOLID, density: 7874.0, specific_heat: 449.0,
    melting_point: 1538.0, boiling_point: 2862.0, latent_heat_fusion: 247000.0,
    latent_heat_vaporization: 6090000.0, viscosity: 0.0, color: [192, 192, 192] },
  // LITHIUM
  { name: "Lithium", default_state: PhysicalState.STATE_POWDER, density: 534.0, specific_heat: 3582.0,
    melting_point: 180.5, boiling_point: 1342.0, latent_heat_fusion: 432000.0,
    latent_heat_vaporization: 20900000.0, viscosity: 0.0, color: [220, 220, 220] },
  // SODIUM
  { name: "Sodium", default_state: PhysicalState.STATE_POWDER, density: 971.0, specific_heat: 1230.0,
    melting_point: 97.7, boiling_point: 883.0, latent_heat_fusion: 113000.0,
    latent_heat_vaporization: 4210000.0, viscosity: 0.0, color: [200, 200, 210] },
  // OIL
  { name: "Oil", default_state: PhysicalState.STATE_LIQUID, density: 900.0, specific_heat: 2000.0,
    melting_point: -40.0, boiling_point: 300.0, latent_heat_fusion: 0.0,
    latent_heat_vaporization: 500000.0, viscosity: 0.05, color: [100, 80, 40] },
  // CO2
  { name: "CO2", default_state: PhysicalState.STATE_GAS, density: 1.98, specific_heat: 840.0,
    melting_point: -78.5, boiling_point: -78.5, latent_heat_fusion: 0.0,
    latent_heat_vaporization: 0.0, viscosity: 0.00001, color: [180, 180, 180] }
];

function getMaterial(type) {
  if (type < 0 || type >= MaterialDB.length) return MaterialDB[0];
  return MaterialDB[type];
}

// ============================================================================
// Grid System
// ============================================================================
class SimulationGrid {
  constructor() {
    this.grid = new Array(GRID_SIZE);
    this.nextGrid = new Array(GRID_SIZE);
    this.renderBuffer = new Int32Array(GRID_SIZE);
    this.activeChunks = new Array(CHUNK_COUNT).fill(true);
    
    this.initGrid();
  }
  
  initGrid() {
    for (let i = 0; i < GRID_SIZE; i++) {
      this.grid[i] = new Particle();
      this.nextGrid[i] = new Particle();
      this.renderBuffer[i] = ParticleType.EMPTY;
    }
    this.activeChunks.fill(true);
  }
  
  getIndex(x, y) {
    return y * JS_WIDTH + x;
  }
  
  inBounds(x, y) {
    return x >= 0 && x < JS_WIDTH && y >= 0 && y < JS_HEIGHT;
  }
  
  getChunkIndex(x, y) {
    const cx = Math.floor(x / CHUNK_SIZE);
    const cy = Math.floor(y / CHUNK_SIZE);
    if (cx < 0 || cx >= CHUNK_WIDTH || cy < 0 || cy >= CHUNK_HEIGHT) return -1;
    return cy * CHUNK_WIDTH + cx;
  }
  
  markChunkActive(x, y) {
    const chunkIdx = this.getChunkIndex(x, y);
    if (chunkIdx >= 0 && chunkIdx < CHUNK_COUNT) {
      this.activeChunks[chunkIdx] = true;
    }
  }
  
  updateRenderBuffer() {
    for (let i = 0; i < GRID_SIZE; i++) {
      this.renderBuffer[i] = this.grid[i].type;
    }
  }
  
  addParticle(x, y, type) {
    if (!this.inBounds(x, y)) return;
    
    const idx = this.getIndex(x, y);
    if (this.grid[idx].type !== ParticleType.EMPTY) return;
    
    this.grid[idx].type = type;
    const mat = getMaterial(type);
    this.grid[idx].state = mat.default_state;
    
    // Initialize based on type
    switch (type) {
      case ParticleType.FIRE:
        this.grid[idx].temperature = 150.0;
        this.grid[idx].life = 30 + Math.floor(Math.random() * 30);
        break;
      case ParticleType.ICE:
        this.grid[idx].temperature = -10.0;
        this.grid[idx].life = -1;
        break;
      case ParticleType.STEAM:
        this.grid[idx].temperature = 110.0;
        this.grid[idx].life = -1;
        break;
      default:
        this.grid[idx].temperature = 20.0;
        this.grid[idx].life = -1;
        break;
    }
    
    this.grid[idx].vx = 0.0;
    this.grid[idx].vy = 0.0;
    this.grid[idx].latent_heat_storage = 0.0;
    
    this.markChunkActive(x, y);
  }
}

// ============================================================================
// Physics: Movement (from movement.cpp)
// ============================================================================
class MovementSystem {
  constructor(simGrid) {
    this.simGrid = simGrid;
  }
  
  canMoveTo(x, y, myDensity) {
    if (!this.simGrid.inBounds(x, y)) return false;
    
    const target = this.simGrid.grid[this.simGrid.getIndex(x, y)];
    if (target.type === ParticleType.EMPTY) return true;
    if (target.state === PhysicalState.STATE_SOLID) return false;
    
    const targetMat = getMaterial(target.type);
    return myDensity > targetMat.density;
  }
  
  update() {
    const grid = this.simGrid.nextGrid;
    
    // Bottom to top, random left-right order
    for (let y = JS_HEIGHT - 1; y >= 0; y--) {
      const leftToRight = Math.random() < 0.5;
      const startX = leftToRight ? 0 : JS_WIDTH - 1;
      const endX = leftToRight ? JS_WIDTH : -1;
      const stepX = leftToRight ? 1 : -1;
      
      for (let x = startX; x !== endX; x += stepX) {
        const idx = this.simGrid.getIndex(x, y);
        const p = grid[idx];
        
        if (p.type === ParticleType.EMPTY || p.type === ParticleType.WALL) continue;
        if (p.updated_this_frame) continue;
        
        const mat = getMaterial(p.type);
        
        if (p.state === PhysicalState.STATE_SOLID) continue;
        
        // FIRE: moves up with random motion
        if (p.type === ParticleType.FIRE) {
          this.updateFire(x, y, idx, p, mat);
          continue;
        }
        
        let moved = false;
        
        // POWDER: falls down
        if (p.state === PhysicalState.STATE_POWDER) {
          moved = this.updatePowder(x, y, idx, p, mat);
        }
        // LIQUID: falls and spreads
        else if (p.state === PhysicalState.STATE_LIQUID) {
          moved = this.updateLiquid(x, y, idx, p, mat);
        }
        // GAS: rises up
        else if (p.state === PhysicalState.STATE_GAS) {
          moved = this.updateGas(x, y, idx, p, mat);
        }
        
        if (!moved) {
          grid[idx].vx *= VELOCITY_DAMPING;
          grid[idx].vy *= VELOCITY_DAMPING;
        }
      }
    }
  }
  
  updateFire(x, y, idx, p, mat) {
    const grid = this.simGrid.nextGrid;
    let fireMoved = false;
    const randomDir = Math.floor(Math.random() * 3) - 1; // -1, 0, 1
    
    if (this.canMoveTo(x, y - 1, mat.density)) {
      this.swap(idx, this.simGrid.getIndex(x, y - 1));
      this.simGrid.markChunkActive(x, y);
      this.simGrid.markChunkActive(x, y - 1);
      fireMoved = true;
    } else if (randomDir !== 0 && this.canMoveTo(x + randomDir, y - 1, mat.density)) {
      this.swap(idx, this.simGrid.getIndex(x + randomDir, y - 1));
      this.simGrid.markChunkActive(x, y);
      this.simGrid.markChunkActive(x + randomDir, y - 1);
      fireMoved = true;
    } else if (this.canMoveTo(x + randomDir, y, mat.density)) {
      this.swap(idx, this.simGrid.getIndex(x + randomDir, y));
      this.simGrid.markChunkActive(x, y);
      this.simGrid.markChunkActive(x + randomDir, y);
      fireMoved = true;
    }
    
    if (!fireMoved) {
      const horizDir = Math.random() < 0.5 ? -1 : 1;
      const fireDispersion = 3;
      
      for (let dist = 1; dist <= fireDispersion; dist++) {
        if (this.canMoveTo(x + horizDir * dist, y, mat.density)) {
          this.swap(idx, this.simGrid.getIndex(x + horizDir * dist, y));
          this.simGrid.markChunkActive(x, y);
          this.simGrid.markChunkActive(x + horizDir * dist, y);
          break;
        }
      }
    }
  }
  
  updatePowder(x, y, idx, p, mat) {
    if (this.canMoveTo(x, y + 1, mat.density)) {
      this.swap(idx, this.simGrid.getIndex(x, y + 1));
      this.simGrid.markChunkActive(x, y);
      this.simGrid.markChunkActive(x, y + 1);
      return true;
    } else {
      const dir = Math.random() < 0.5 ? -1 : 1;
      if (this.canMoveTo(x + dir, y + 1, mat.density)) {
        this.swap(idx, this.simGrid.getIndex(x + dir, y + 1));
        this.simGrid.markChunkActive(x, y);
        this.simGrid.markChunkActive(x + dir, y + 1);
        return true;
      } else if (this.canMoveTo(x - dir, y + 1, mat.density)) {
        this.swap(idx, this.simGrid.getIndex(x - dir, y + 1));
        this.simGrid.markChunkActive(x, y);
        this.simGrid.markChunkActive(x - dir, y + 1);
        return true;
      }
    }
    return false;
  }
  
  updateLiquid(x, y, idx, p, mat) {
    if (this.canMoveTo(x, y + 1, mat.density)) {
      this.swap(idx, this.simGrid.getIndex(x, y + 1));
      this.simGrid.markChunkActive(x, y);
      this.simGrid.markChunkActive(x, y + 1);
      return true;
    } else {
      let preferredDir = 0;
      if (Math.abs(p.vx) > 0.1) {
        preferredDir = p.vx > 0 ? 1 : -1;
      } else {
        preferredDir = Math.random() < 0.5 ? -1 : 1;
      }
      
      if (this.canMoveTo(x + preferredDir, y + 1, mat.density)) {
        this.swap(idx, this.simGrid.getIndex(x + preferredDir, y + 1));
        this.simGrid.markChunkActive(x, y);
        this.simGrid.markChunkActive(x + preferredDir, y + 1);
        return true;
      } else if (this.canMoveTo(x - preferredDir, y + 1, mat.density)) {
        this.swap(idx, this.simGrid.getIndex(x - preferredDir, y + 1));
        this.simGrid.markChunkActive(x, y);
        this.simGrid.markChunkActive(x - preferredDir, y + 1);
        return true;
      } else {
        const horizDir = preferredDir;
        const dispersionRate = 10;
        
        for (let dist = 1; dist <= dispersionRate; dist++) {
          if (this.canMoveTo(x + horizDir * dist, y, mat.density)) {
            this.swap(idx, this.simGrid.getIndex(x + horizDir * dist, y));
            this.simGrid.markChunkActive(x, y);
            this.simGrid.markChunkActive(x + horizDir * dist, y);
            return true;
          }
        }
        
        for (let dist = 1; dist <= dispersionRate; dist++) {
          if (this.canMoveTo(x - horizDir * dist, y, mat.density)) {
            this.swap(idx, this.simGrid.getIndex(x - horizDir * dist, y));
            this.simGrid.markChunkActive(x, y);
            this.simGrid.markChunkActive(x - horizDir * dist, y);
            return true;
          }
        }
      }
    }
    return false;
  }
  
  updateGas(x, y, idx, p, mat) {
    const randomChoice = Math.floor(Math.random() * 10);
    let moved = false;
    
    if (randomChoice < 7) {
      const diagDir = Math.random() < 0.5 ? -1 : 1;
      
      if (this.canMoveTo(x, y - 1, mat.density)) {
        this.swap(idx, this.simGrid.getIndex(x, y - 1));
        this.simGrid.markChunkActive(x, y);
        this.simGrid.markChunkActive(x, y - 1);
        moved = true;
      } else if (this.canMoveTo(x + diagDir, y - 1, mat.density)) {
        this.swap(idx, this.simGrid.getIndex(x + diagDir, y - 1));
        this.simGrid.markChunkActive(x, y);
        this.simGrid.markChunkActive(x + diagDir, y - 1);
        moved = true;
      } else if (this.canMoveTo(x - diagDir, y - 1, mat.density)) {
        this.swap(idx, this.simGrid.getIndex(x - diagDir, y - 1));
        this.simGrid.markChunkActive(x, y);
        this.simGrid.markChunkActive(x - diagDir, y - 1);
        moved = true;
      }
    }
    
    if (!moved) {
      const horizDir = Math.random() < 0.5 ? -1 : 1;
      const dispersionRate = 5;
      
      for (let dist = 1; dist <= dispersionRate; dist++) {
        if (this.canMoveTo(x + horizDir * dist, y, mat.density)) {
          this.swap(idx, this.simGrid.getIndex(x + horizDir * dist, y));
          this.simGrid.markChunkActive(x, y);
          this.simGrid.markChunkActive(x + horizDir * dist, y);
          moved = true;
          break;
        }
      }
      
      if (!moved) {
        for (let dist = 1; dist <= dispersionRate; dist++) {
          if (this.canMoveTo(x - horizDir * dist, y, mat.density)) {
            this.swap(idx, this.simGrid.getIndex(x - horizDir * dist, y));
            this.simGrid.markChunkActive(x, y);
            this.simGrid.markChunkActive(x - horizDir * dist, y);
            moved = true;
            break;
          }
        }
      }
    }
    
    return moved;
  }
  
  swap(idx1, idx2) {
    const grid = this.simGrid.nextGrid;
    const temp = grid[idx1].copy();
    grid[idx1] = grid[idx2].copy();
    grid[idx2] = temp;
    grid[idx2].updated_this_frame = true;
  }
}

// ============================================================================
// Physics: Life and Special Materials
// ============================================================================
class LifeSystem {
  constructor(simGrid) {
    this.simGrid = simGrid;
  }
  
  update() {
    for (let i = 0; i < GRID_SIZE; i++) {
      const p = this.simGrid.nextGrid[i];
      
      if (p.type === ParticleType.EMPTY) continue;
      
      // Life countdown
      if (p.life > 0) {
        p.life--;
        if (p.life === 0) {
          p.type = ParticleType.EMPTY;
          p.state = PhysicalState.STATE_GAS;
        }
      }
    }
  }
}

// ============================================================================
// Chemistry System: Reaction Base Structures
// ============================================================================

// Reaction Result
class ReactionResult {
  constructor() {
    this.occurred = false;
    this.new_type_center = -1;
    this.new_type_neighbor = -1;
    this.heat_released = 0.0;
    this.explosion_radius = 0;
    this.explosion_force = 0.0;
    this.life_center = -2;
    this.life_neighbor = -2;
  }
}

// Reaction Rule
class ReactionRule {
  constructor(reactant_a, reactant_b, handler, probability, min_temperature, name) {
    this.reactant_a = reactant_a;
    this.reactant_b = reactant_b;
    this.handler = handler;
    this.probability = probability;
    this.min_temperature = min_temperature;
    this.name = name;
  }
}

// ============================================================================
// Combustion Reaction Handlers
// ============================================================================

// Wood + Fire → Fire + CO2
function react_wood_fire(wood, fire, wx, wy, fx, fy) {
  const result = new ReactionResult();
  if (Math.random() > 0.5) return result;
  
  result.occurred = true;
  result.new_type_center = ParticleType.FIRE;
  result.new_type_neighbor = ParticleType.CO2;
  result.heat_released = 15000.0;
  result.life_center = 30 + Math.floor(Math.random() * 30);
  result.life_neighbor = -1;
  
  return result;
}

// Oil + Fire → Fire + CO2
function react_oil_fire(oil, fire, ox, oy, fx, fy) {
  const result = new ReactionResult();
  if (Math.random() > 0.7) return result;
  
  result.occurred = true;
  result.new_type_center = ParticleType.FIRE;
  result.new_type_neighbor = ParticleType.CO2;
  result.heat_released = 30000.0;
  result.life_center = 40 + Math.floor(Math.random() * 40);
  result.life_neighbor = -1;
  
  return result;
}

// Hydrogen + Fire → Fire + Steam (Explosion)
function react_hydrogen_fire(hydrogen, fire, hx, hy, fx, fy) {
  const result = new ReactionResult();
  if (Math.random() > 0.8) return result;
  
  result.occurred = true;
  result.new_type_center = ParticleType.FIRE;
  result.new_type_neighbor = ParticleType.STEAM;
  result.heat_released = 50000.0;
  result.explosion_radius = 5;
  result.explosion_force = 3.0;
  result.life_center = 20 + Math.floor(Math.random() * 20);
  result.life_neighbor = -1;
  
  return result;
}

// Ice + Fire → Water + Steam
function react_ice_fire(ice, fire, ix, iy, fx, fy) {
  const result = new ReactionResult();
  
  result.occurred = true;
  result.new_type_center = ParticleType.WATER;
  result.new_type_neighbor = ParticleType.STEAM;
  result.heat_released = -33400.0;
  result.life_center = -1;
  result.life_neighbor = -1;
  
  return result;
}

// ============================================================================
// Water-Metal Reaction Handlers
// ============================================================================

// Water + Lithium → Hydrogen + Fire (Explosion)
function react_water_lithium(water, lithium, wx, wy, lx, ly) {
  const result = new ReactionResult();
  if (Math.random() > 0.8) return result;
  
  result.occurred = true;
  result.new_type_center = ParticleType.HYDROGEN;
  result.new_type_neighbor = ParticleType.FIRE;
  result.heat_released = 40000.0;
  result.explosion_radius = 4;
  result.explosion_force = 2.5;
  result.life_center = -1;
  result.life_neighbor = 25 + Math.floor(Math.random() * 25);
  
  return result;
}

// Water + Sodium → Hydrogen + Fire (Explosion)
function react_water_sodium(water, sodium, wx, wy, sx, sy) {
  const result = new ReactionResult();
  if (Math.random() > 0.75) return result;
  
  result.occurred = true;
  result.new_type_center = ParticleType.HYDROGEN;
  result.new_type_neighbor = ParticleType.FIRE;
  result.heat_released = 35000.0;
  result.explosion_radius = 3;
  result.explosion_force = 2.0;
  result.life_center = -1;
  result.life_neighbor = 20 + Math.floor(Math.random() * 20);
  
  return result;
}

// ============================================================================
// Evaporation Reaction Handlers
// ============================================================================

// Oil Steam + Fire → Fire + CO2
function react_oil_steam_fire(oil_steam, fire, sx, sy, fx, fy) {
  const result = new ReactionResult();
  if (Math.random() > 0.8) return result;
  
  result.occurred = true;
  result.new_type_center = ParticleType.FIRE;
  result.new_type_neighbor = ParticleType.CO2;
  result.heat_released = 35000.0;
  result.life_center = 35 + Math.floor(Math.random() * 35);
  result.life_neighbor = -1;
  
  return result;
}

// Reaction Registry (Singleton pattern)
class ReactionRegistry {
  constructor() {
    this.reactions = [];
  }
  
  registerReaction(rule) {
    this.reactions.push(rule);
  }
  
  checkReaction(p1, p2, x1, y1, x2, y2) {
    for (const rule of this.reactions) {
      // Check if reactants match
      if (p1.type === rule.reactant_a && p2.type === rule.reactant_b) {
        // Probability check
        if (Math.random() > rule.probability) continue;
        
        // Call reaction handler
        const result = rule.handler(p1, p2, x1, y1, x2, y2);
        if (result.occurred) return result;
      }
    }
    return new ReactionResult();
  }
  
  initializeAllReactions() {
    this.reactions = [];
    this.registerCombustionReactions();
    this.registerWaterMetalReactions();
    this.registerEvaporationReactions();
  }
  
  // Phase 2: Combustion Reactions
  registerCombustionReactions() {
    // Wood + Fire → Fire + CO2
    this.registerReaction(new ReactionRule(
      ParticleType.WOOD, ParticleType.FIRE,
      react_wood_fire, 0.5, -999.0, "Wood Combustion"
    ));
    this.registerReaction(new ReactionRule(
      ParticleType.FIRE, ParticleType.WOOD,
      (f, w, fx, fy, wx, wy) => react_wood_fire(w, f, wx, wy, fx, fy),
      0.5, -999.0, "Wood Combustion (rev)"
    ));
    
    // Oil + Fire → Fire + CO2
    this.registerReaction(new ReactionRule(
      ParticleType.OIL, ParticleType.FIRE,
      react_oil_fire, 0.7, -999.0, "Oil Combustion"
    ));
    this.registerReaction(new ReactionRule(
      ParticleType.FIRE, ParticleType.OIL,
      (f, o, fx, fy, ox, oy) => react_oil_fire(o, f, ox, oy, fx, fy),
      0.7, -999.0, "Oil Combustion (rev)"
    ));
    
    // Hydrogen + Fire → Fire + Steam (Explosion)
    this.registerReaction(new ReactionRule(
      ParticleType.HYDROGEN, ParticleType.FIRE,
      react_hydrogen_fire, 0.8, -999.0, "Hydrogen Explosion"
    ));
    this.registerReaction(new ReactionRule(
      ParticleType.FIRE, ParticleType.HYDROGEN,
      (f, h, fx, fy, hx, hy) => react_hydrogen_fire(h, f, hx, hy, fx, fy),
      0.8, -999.0, "Hydrogen Explosion (rev)"
    ));
    
    // Ice + Fire → Water + Steam
    this.registerReaction(new ReactionRule(
      ParticleType.ICE, ParticleType.FIRE,
      react_ice_fire, 1.0, -999.0, "Ice Melting by Fire"
    ));
    this.registerReaction(new ReactionRule(
      ParticleType.FIRE, ParticleType.ICE,
      (f, i, fx, fy, ix, iy) => react_ice_fire(i, f, ix, iy, fx, fy),
      1.0, -999.0, "Ice Melting by Fire (rev)"
    ));
  }
  
  registerWaterMetalReactions() {
    // Water + Lithium
    this.registerReaction(new ReactionRule(
      ParticleType.WATER, ParticleType.LITHIUM,
      react_water_lithium, 0.8, -999.0, "Water-Lithium Reaction"
    ));
    this.registerReaction(new ReactionRule(
      ParticleType.LITHIUM, ParticleType.WATER,
      (l, w, lx, ly, wx, wy) => react_water_lithium(w, l, wx, wy, lx, ly),
      0.8, -999.0, "Water-Lithium Reaction (rev)"
    ));
    
    // Water + Sodium
    this.registerReaction(new ReactionRule(
      ParticleType.WATER, ParticleType.SODIUM,
      react_water_sodium, 0.75, -999.0, "Water-Sodium Reaction"
    ));
    this.registerReaction(new ReactionRule(
      ParticleType.SODIUM, ParticleType.WATER,
      (s, w, sx, sy, wx, wy) => react_water_sodium(w, s, wx, wy, sx, sy),
      0.75, -999.0, "Water-Sodium Reaction (rev)"
    ));
  }
  
  registerEvaporationReactions() {
    // Oil Steam + Fire
    this.registerReaction(new ReactionRule(
      ParticleType.STEAM_OIL, ParticleType.FIRE,
      react_oil_steam_fire, 0.8, -999.0, "Oil Steam Combustion"
    ));
    this.registerReaction(new ReactionRule(
      ParticleType.FIRE, ParticleType.STEAM_OIL,
      (f, s, fx, fy, sx, sy) => react_oil_steam_fire(s, f, sx, sy, fx, fy),
      0.8, -999.0, "Oil Steam Combustion (rev)"
    ));
  }
}

// ============================================================================
// Explosion System
// ============================================================================
class ExplosionSystem {
  constructor(simGrid) {
    this.simGrid = simGrid;
  }
  
  applyExplosion(cx, cy, radius, force) {
    for (let dy = -radius; dy <= radius; dy++) {
      for (let dx = -radius; dx <= radius; dx++) {
        const dist = Math.sqrt(dx * dx + dy * dy);
        if (dist > radius || dist < 0.1) continue;
        
        const x = cx + dx;
        const y = cy + dy;
        if (!this.simGrid.inBounds(x, y)) continue;
        
        const idx = this.simGrid.getIndex(x, y);
        const p = this.simGrid.nextGrid[idx];
        
        // Distance-based strength
        const strength = force * (1.0 - dist / radius);
        
        // Add velocity (radial)
        p.vx += (dx / dist) * strength;
        p.vy += (dy / dist) * strength;
        
        // Add heat
        p.temperature += strength * 50.0;
        
        // Destroy solids (except walls)
        if (p.type === ParticleType.WALL) continue;
        
        if (strength > 0.5 && 
            (p.state === PhysicalState.STATE_SOLID || 
             p.state === PhysicalState.STATE_POWDER)) {
          if (Math.random() < strength * 0.3) {
            p.type = ParticleType.EMPTY;
            p.state = PhysicalState.STATE_GAS;
          }
        }
      }
    }
  }
}

// ============================================================================
// Chemistry System
// ============================================================================
class ChemistrySystem {
  constructor(simGrid, registry, explosionSystem) {
    this.simGrid = simGrid;
    this.registry = registry;
    this.explosionSystem = explosionSystem;
  }
  
  update() {
    // 8-direction neighbors
    const dx = [0, 1, 1, 1, 0, -1, -1, -1];
    const dy = [-1, -1, 0, 1, 1, 1, 0, -1];
    
    for (let y = 0; y < JS_HEIGHT; y++) {
      for (let x = 0; x < JS_WIDTH; x++) {
        const idx = this.simGrid.getIndex(x, y);
        const center = this.simGrid.grid[idx];
        
        if (center.type === ParticleType.EMPTY) continue;
        
        for (let dir = 0; dir < 8; dir++) {
          const nx = x + dx[dir];
          const ny = y + dy[dir];
          
          if (!this.simGrid.inBounds(nx, ny)) continue;
          
          const nidx = this.simGrid.getIndex(nx, ny);
          const neighbor = this.simGrid.grid[nidx];
          
          if (neighbor.type === ParticleType.EMPTY) continue;
          
          // Check reaction
          const result = this.registry.checkReaction(
            center, neighbor, x, y, nx, ny
          );
          
          if (result.occurred) {
            this.applyReaction(idx, nidx, result, x, y);
            break; // One reaction per frame
          }
        }
      }
    }
  }
  
  applyReaction(idx, nidx, result, x, y) {
    // Change center particle
    if (result.new_type_center >= 0) {
      this.simGrid.nextGrid[idx].type = result.new_type_center;
      const mat = getMaterial(result.new_type_center);
      this.simGrid.nextGrid[idx].state = mat.default_state;
      
      if (result.life_center >= -1) {
        this.simGrid.nextGrid[idx].life = result.life_center;
      }
    }
    
    // Change neighbor particle
    if (result.new_type_neighbor >= 0) {
      this.simGrid.nextGrid[nidx].type = result.new_type_neighbor;
      const mat = getMaterial(result.new_type_neighbor);
      this.simGrid.nextGrid[nidx].state = mat.default_state;
      
      if (result.life_neighbor >= -1) {
        this.simGrid.nextGrid[nidx].life = result.life_neighbor;
      }
    }
    
    // Heat release
    if (result.heat_released !== 0.0) {
      this.simGrid.nextGrid[idx].temperature += result.heat_released * 0.001;
      this.simGrid.nextGrid[nidx].temperature += result.heat_released * 0.001;
    }
    
    // Explosion effect
    if (result.explosion_radius > 0) {
      this.explosionSystem.applyExplosion(
        x, y, 
        result.explosion_radius, 
        result.explosion_force
      );
    }
  }
}

// ============================================================================
// Main Simulation
// ============================================================================
class JSSimulation {
  constructor() {
    this.grid = new SimulationGrid();
    this.movementSystem = new MovementSystem(this.grid);
    this.lifeSystem = new LifeSystem(this.grid);
    
    // Chemistry system
    this.reactionRegistry = new ReactionRegistry();
    this.explosionSystem = new ExplosionSystem(this.grid);
    this.chemistrySystem = new ChemistrySystem(
      this.grid,
      this.reactionRegistry,
      this.explosionSystem
    );
  }
  
  init() {
    this.grid.initGrid();
    this.reactionRegistry.initializeAllReactions();
  }
  
  update() {
    // Copy grid to nextGrid
    for (let i = 0; i < GRID_SIZE; i++) {
      this.grid.nextGrid[i] = this.grid.grid[i].copy();
      this.grid.nextGrid[i].updated_this_frame = false;
    }
    
    // PASS 1: Chemistry reactions
    this.chemistrySystem.update();
    
    // PASS 2: Update life and special materials
    this.lifeSystem.update();
    
    // PASS 3: Update movement
    this.movementSystem.update();
    
    // Copy nextGrid back to grid
    for (let i = 0; i < GRID_SIZE; i++) {
      this.grid.grid[i] = this.grid.nextGrid[i].copy();
    }
    
    // Update render buffer
    this.grid.updateRenderBuffer();
  }
  
  getRenderBuffer() {
    return this.grid.renderBuffer;
  }
  
  getParticleArray() {
    return this.grid.grid;
  }
  
  addParticle(x, y, type) {
    this.grid.addParticle(x, y, type);
  }
}

// Export to global scope
global.JSSimulation = JSSimulation;
global.JSParticleType = ParticleType;
global.JS_SIM_WIDTH = JS_WIDTH;
global.JS_SIM_HEIGHT = JS_HEIGHT;

})(typeof window !== 'undefined' ? window : global);
