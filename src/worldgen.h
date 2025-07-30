#pragma once
#include <sys/util.h>
#include "world.h"
#include "player.h"

#define WATER_LEVEL 5

#define GRID_STEP 8
#define GRID_OFFSET (GRID_STEP / 2)
#define GRID_SIZE ((WORLD_SIZE / GRID_STEP) + 1)

// Makes a "natural" looking world with randomly generated terrain and trees
void generate_natural(world_t &world, player_t &player) {
    // Add bedrock floor
    world.fill_space(0, 0, 0, WORLD_SIZE - 1,           0, WORLD_SIZE - 1, BEDROCK);
    world.fill_space(0, 1, 0, WORLD_SIZE - 1, WATER_LEVEL, WORLD_SIZE - 1, WATER);

    uint16_t grid[GRID_SIZE][GRID_SIZE];

    for(uint8_t x = 0; x < GRID_SIZE; x++) {
        for(uint8_t z = 0; z < GRID_SIZE; z++) {
            grid[x][z] = randInt(3, 12);
        }
    }


    for(uint8_t x = 0; x < WORLD_SIZE; x++) {
        for(uint8_t z = 0; z < WORLD_SIZE; z++) {
            uint8_t grid_x = x / GRID_STEP;
            uint8_t grid_z = z / GRID_STEP;

            uint8_t lerp_x = x - (grid_x * GRID_STEP);
            uint8_t lerp_z = z - (grid_z * GRID_STEP);

            uint16_t height = 0;

            height += grid[grid_x    ][grid_z    ] * (GRID_STEP - lerp_x) * (GRID_STEP - lerp_z);
            height += grid[grid_x + 1][grid_z    ] * (            lerp_x) * (GRID_STEP - lerp_z);
            height += grid[grid_x    ][grid_z + 1] * (GRID_STEP - lerp_x) * (            lerp_z);
            height += grid[grid_x + 1][grid_z + 1] * (            lerp_x) * (            lerp_z);

            height /= GRID_STEP * GRID_STEP;

            if(height > 3) {
                world.fill_space(x, 1, z, x, height - 3, z, STONE);
                world.fill_space(x, height - 2, z, x, height, z, DIRT);
            }
            else 
            {   
                world.fill_space(x, 1, z, x, height, z, DIRT);
            }
            
            if(height >= WATER_LEVEL)
                world.fill_space(x, height, z, x, height, z, GRASS);
        }   
    }

    // Replace dirt with sand near water blocks
    for(uint8_t y = WATER_LEVEL - 1; y <= WATER_LEVEL; y++) {
        for(uint8_t x = 0; x < WORLD_SIZE; x++) {
            for(uint8_t z = 0; z < WORLD_SIZE; z++) {
                if(world.blocks[y][x][z] != GRASS && world.blocks[y][x][z] != DIRT) continue;
                if(y >= WORLD_HEIGHT - 1) continue;
                if(world.blocks[y + 1][x][z] != AIR && world.blocks[y + 1][x][z] != WATER) continue;

                bool near_water = false;

                for(int8_t by = -1; by <= 1; by++) {
                    if(y + by < 0 || y + by >= WORLD_HEIGHT) continue;

                    for(int8_t bx = -2; bx <= 2; bx++) {
                        if(x + bx < 0 || x + bx >= WORLD_SIZE) continue;
                        
                        for(int8_t bz = -2; bz <= 2; bz++) {
                            if(z + bz < 0 || z + bz >= WORLD_SIZE) continue;

                            if(world.blocks[by + y][bx + x][bz + z] == WATER)
                                near_water = true;
                        }
                    }
                }

                if(near_water)
                    world.blocks[y][x][z] = SAND;
            }
        }
    }

    //Add trees
    uint8_t tree_cnt = 12;
    for(uint8_t i = 0; i < tree_cnt; i++) {
        //uint8_t x = 2;//randInt(2, WORLD_SIZE - 2 - 1);
        //uint8_t z = 2;//randInt(2, WORLD_SIZE - 2 - 1);
        // Weird little hack I need to do since clang crashes with the above lines :(
        uint8_t x = 2 + (random() % (WORLD_SIZE - 1 - 4));
        uint8_t z = 2 + (random() % (WORLD_SIZE - 1 - 4));

        int8_t y = WORLD_HEIGHT - 8;

        if(world.blocks[y + 1][x][z] != AIR) continue;

        while(y > 0) {
            if(world.blocks[y][x][z] != AIR) break; 
            y--;
        }

        if(world.blocks[y][x][z] != GRASS) break;

        world.add_tree(x, y + 1, z);
        world.blocks[y][x][z] = DIRT;
    }

    // Replace a few stone blocks with coal
    for(uint8_t y = 1; y < WORLD_HEIGHT; y++) {
        for(uint8_t x = 0; x < WORLD_SIZE; x++) {
            for(uint8_t z = 0; z < WORLD_SIZE; z++) {
                if(world.blocks[y][x][z] != STONE) continue;

                int24_t r = randInt(0, 19);

                // Replace stone with coal or iron ore at a 10% chance
                if(r == 0) {
                    world.blocks[y][x][z] = COAL_ORE;
                }
                else if(r == 1) {
                    world.blocks[y][x][z] = IRON_ORE;
                }
            }
        }
    }

    // Center the player in the world and place them at the lowest open-air block
    player.x = WORLD_SIZE / 2;
    player.y = 0;
    player.z = WORLD_SIZE / 2;

    while(player.y <= (WORLD_HEIGHT - 1)) {
        if(world.blocks[player.y][player.x][player.z] == AIR) break;
        player.y++;
    }
}