#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../../src/game_rng/game_rng.h"

#define MAX_PARTICLES 1000
#define OUTPUT_FRAMES 100

typedef struct {
    float x, y;        // Position
    float vx, vy;      // Velocity
    float lifetime;    // Remaining lifetime
    int active;        // Whether particle is active
} Particle;

typedef struct {
    GameRNG rng;
    Particle particles[MAX_PARTICLES];
    int active_count;
} ParticleSystem;

void init_particle_system(ParticleSystem *ps) {
    init_game_rng(&ps->rng);
    ps->active_count = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) {
        ps->particles[i].active = 0;
    }
}

void emit_particle(ParticleSystem *ps, float x, float y) {
    if (ps->active_count >= MAX_PARTICLES) return;
    
    // Find inactive particle
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!ps->particles[i].active) {
            Particle *p = &ps->particles[i];
            
            // Random initial velocity
            float angle = random_float(&ps->rng) * 2 * M_PI;
            float speed = random_float(&ps->rng) * 2.0f + 1.0f;
            
            p->x = x;
            p->y = y;
            p->vx = cosf(angle) * speed;
            p->vy = sinf(angle) * speed;
            p->lifetime = random_float(&ps->rng) * 2.0f + 1.0f;
            p->active = 1;
            
            ps->active_count++;
            break;
        }
    }
}

void update_particles(ParticleSystem *ps, float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (ps->particles[i].active) {
            Particle *p = &ps->particles[i];
            
            // Update position
            p->x += p->vx * dt;
            p->y += p->vy * dt;
            
            // Apply gravity
            p->vy -= 9.81f * dt;
            
            // Update lifetime
            p->lifetime -= dt;
            if (p->lifetime <= 0) {
                p->active = 0;
                ps->active_count--;
            }
        }
    }
}

void output_frame(ParticleSystem *ps, int frame) {
    printf("Frame %d\n", frame);
    printf("Active particles: %d\n", ps->active_count);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (ps->particles[i].active) {
            printf("Particle %d: pos=(%.2f, %.2f) vel=(%.2f, %.2f) life=%.2f\n",
                   i, ps->particles[i].x, ps->particles[i].y,
                   ps->particles[i].vx, ps->particles[i].vy,
                   ps->particles[i].lifetime);
        }
    }
    printf("\n");
}

int main() {
    ParticleSystem ps;
    init_particle_system(&ps);
    
    float dt = 1.0f / 60.0f;  // 60 FPS simulation
    
    for (int frame = 0; frame < OUTPUT_FRAMES; frame++) {
        // Emit new particles
        if (frame % 10 == 0) {  // Emit every 10 frames
            for (int i = 0; i < 10; i++) {
                emit_particle(&ps, 0.0f, 0.0f);
            }
        }
        
        // Update simulation
        update_particles(&ps, dt);
        
        // Output state
        output_frame(&ps, frame);
    }
    
    return 0;
}
