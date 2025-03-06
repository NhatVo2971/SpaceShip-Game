#include "uart.h"
#include "function.h"
#include "cli_function.h"
#include "mbox.h"
#include "framebf.h"
#include "game.h"
#include "display_image.h"
#include "object.h"
#include "printf.h"
#include "game_universe_background.h"

int wait_time_shoot = 100;

//initalize the game
//----------------------------------------------------------------------------
void init_game(Game *world)
{
    world->game_over = 0;
    world->game_start = 0;
    world->main_menu.on_game_menu = 1;
    world->main_menu.game_start_menu = 1;
    world->game_win = 0;
    restartGame = 0;
    pauseGame = 0;
    quitGame = 0;
    isStage2 = 0;
    check = 0;
    init_map(&world->world);
    framebf_init();
}
// Create the stage
void init_map(World *world)
{
    init_player(&world->player);
    init_enemies(world);
    init_playerScore(&world->playerScore);
    init_life(&world->life);
    world->game_menu.game_menu_option = 0;
    world->game_menu.on_gameMenu_menu = 0;
    world->game_over = 0;
}

// Setting the value for player
//----------------------------------------------------------------------------
void init_player(Entity *player)
{
    player->dimension.height = 90;
    player->dimension.width = 72;
    player->position.x = (MAP_WIDTH / 2) - (player->dimension.width / 2);
    player->position.y = MAP_HEIGHT - 50;
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        player->projectile[i].active = 0;
        player->projectile[i].position.y = 100;
    }
    player->health.current_health = 3;
    player->type = PLAYER;
    player->needs_update = 1;
    player->needs_render = 0;
    player->needs_clear = 0;
    player->combat_update = 0;
    player->enabled = 1;
}

// Setting the value for enemies
//----------------------------------------------------------------------------
void init_enemies(World *world)
{
    world->enemies.dimension.height = 0;
    world->enemies.dimension.width = 0;
    world->enemies.type = 0;
    
    world->enemies.needs_render = 0;
    world->enemies.needs_update = 1;
    world->enemies.enabled = 1;
    world->enemies.health.current_health = 1;
    world->enemies.combat_update = 0;
    for (int j = 0; j < MAX_BULLETS; j++)
    {
        world->enemies.projectile[j].active = 0;
        world->enemies.projectile[j].position.y = 1000;
    }
}

//init player life
//----------------------------------------------------------------------------
void init_life(Entity *life)
{
    life->health.player_health = 3;
    life->needs_update = 0;
    life->needs_render = 1;
}

//init player score
//----------------------------------------------------------------------------
void init_playerScore(Score *playerScore)
{
    playerScore->score = 0;
    playerScore->needsUpdate = 0;
    playerScore->needsRender = 1;
}


//show main menu before game
//----------------------------------------------------------------------------
void show_main_menu(Game *game)
{

    uart_puts("Press s to move down: \n");
    uart_puts("Press w to move up: \n");
    uart_puts("Press space to confirm: \n\n");
    drawMainMenu(game);
    while (game->main_menu.on_game_menu)
    {
        char character = uart_getc_game();
        if (character == 'w')
        {
            game->main_menu.game_start_menu = 1; // select start
            drawMainMenu(game);
        }
        else if (character == 's')
        {
            game->main_menu.game_start_menu = 0; // select quit
            drawMainMenu(game);
        }

        else if (character == ' ')
        {
            game->main_menu.on_game_menu = 0;
            if (game->main_menu.game_start_menu)
                game->game_start = 1;
            else
                game->game_start = 0;
            displayGameUniverseBackground(0, 0);
        }
    }
}
// display pause menu
//----------------------------------------------------------------------------
void pause_menu(World *world)
{
    world->game_menu.game_menu_option = 1;
    world->game_menu.on_gameMenu_menu = 1;
    pauseGame = 1;

    uart_puts("\nPress s to move down: \n");
    uart_puts("Press w to move up: \n");
    uart_puts("Press space to choose: \n");

    while (world->game_menu.on_gameMenu_menu)
    {
        drawPauseMenu(world);
        char character = uart_getc_game();

        if (character == 'w') // up
        {
            if (world->game_menu.game_menu_option < 2)
            {
                world->game_menu.game_menu_option++;
            }
        }
        else if (character == 's') // down
        {
            if (world->game_menu.game_menu_option > 0)
            {
                world->game_menu.game_menu_option--;
            }
        }
        else if (character == ' ') // space to select
        {
            if (world->game_menu.game_menu_option == 2)
            {//resume game
                world->game_menu.on_gameMenu_menu = 0;
                displayGameUniverseBackground(0, 0);
                printf("\nSELECT: Resume\n");
                world->life.needs_render = 1;
                world->playerScore.needsRender = 1;
                world->player.needs_render = 1;
                pauseGame = 0;
            }
            else if (world->game_menu.game_menu_option == 1)
            {//restart game
                displayGameUniverseBackground(0, 0);
                printf("\nSELECT: Restart\n");
                restartGame = 1;
                return;
            }
            else if (world->game_menu.game_menu_option == 0)
            {//quit game
                displayGameUniverseBackground(0, 0);
                printf("\nSELECT: Quit\n");
                quitGame = 1;
                return;
            }
        }
    }
    return;
}
// Move the game forward
//----------------------------------------------------------------------------
void move_game(World *world)
{
    uart_puts("Press A to move left: \n");
    uart_puts("Press D to move right: \n");
    uart_puts("Press W to move up: \n");
    uart_puts("Press S to move down: \n");
    uart_puts("Press SPACE to shoot: \n");
    uart_puts("Press P to stop: \n");
    while (!quitGame && !restartGame)
    {
        while (!pauseGame)
        {//code to move player
            char character = uart_getc_game();
            if (character == 'a')
            {//move left
                move_entity(&world->player, LEFT);
                world->player.velocity.y = 0;
            }
            else if (character == 'd')
            {//move right
                move_entity(&world->player, RIGHT);
                world->player.velocity.y = 0;
            }
            else if (character == 'w')
            {//move up
                world->player.velocity.y = -VERTICAL_SPEED;
                world->player.velocity.x = 0;
                world->player.needs_update = 1;
            }
            else if (character == 's')
            {//move down
                world->player.velocity.y = VERTICAL_SPEED;
                world->player.velocity.x = 0;
                world->player.needs_update = 1;
            }
            else if (character == ' ')
            {//space is shoot
                entity_shoot(&world->player, UP);
            }
            else if (character == 'p')
            {//pause the game
                pause_menu(world);
            }
            //run functions to update all values before render the result
            update_AI(world);
            update_collision(world);
            update_combat(world);
            update_position(world);
            render(world);
            //shoot per 10 count of wait_time_shoot;
            if (wait_time_shoot % 10 == 0 && wait_time_shoot != 100)
            {
                enemy_shoot(world);
            }
            if (wait_time_shoot == 100)
            {
                enemy_shoot(world);
                wait_time_shoot = 0;
            }
            {
                wait_time_shoot++;
            }
        }
    }
}

// display end screen
//---------------------------------------------------------------------------- 
void endScreen(int won, World *world)
{

    pauseGame = 1;
    uart_puts("\n\n");
    uart_puts("Press e to exit: \n");
    uart_puts("Press r to restart: \n");
    char *type = "d";
    displayGameUniverseBackground(0, 0);

    clearscreen(0, 0);
    //win
    if (won)
    {
        displayGameUniverseBackground(0, 0);
        drawScore(world, type);
        displayGameWinImage(300, 100);
    }
    //lose
    else
    {
        displayGameUniverseBackground(0, 0);
        drawScore(world, type);
        displayGameOverImage(300, 100);
    }
    drawString(50, 180, "------------------", "bright red");
    drawString(285, 250, "PRESS KEY", "bright red");
    drawString(200, 320, "R-TO RESTART", "bright red");
    drawString(200, 390, "E-TO EXIT", "bright red");
    drawString(50, 460, "------------------", "red");
    // Display message to tell player to quit game or continue playing
    while (!restartGame)
    {
        char character = uart_getc();
        if (character == 'e')
        {
            quitGame = 1;
            uart_puts("\n\nSuccessfully out!\n");
            break;
        }
        if (character == 'r')
        {
            restartGame = 1;
        }
    }

    return;
}
// move player and enemy
//----------------------------------------------------------------------------
void move_entity(Entity *entity, Direction direction)
{//depend on direction case change their velocity and tell the game to update the entity
    switch (direction)
    {
    case LEFT:
    //if player then velocity is player speed else move enemy speed
        entity->velocity.x =
            (entity->type == PLAYER) ? -PLAYER_SPEED : -HORIZONTAL_SPEED;
        entity->needs_update = 1;
        break;
    case RIGHT:
    //if player then velocity is player speed else move enemy speed
        entity->velocity.x =
            (entity->type == PLAYER) ? PLAYER_SPEED : HORIZONTAL_SPEED;
        entity->needs_update = 1;
        break;
    case UP:
        entity->velocity.y = -VERTICAL_SPEED;
        entity->needs_update = 1;
        break;
    case DOWN:
        entity->velocity.y = VERTICAL_SPEED;
        entity->needs_update = 1;
        break;
    case RESET_VERTICAL:
        entity->velocity.y = 0;
        entity->needs_update = 1;
        break;
    case RESET_HORIZONTAL:
        entity->velocity.x = 0;
        entity->needs_update = 1;
        break;
    default:
        entity->velocity.x = 0;
        entity->velocity.y = 0;
        entity->needs_update = 1;
    }
}

//track all position and if boss and player collide
//----------------------------------------------------------------------------
void update_position(World *world)
{//move player
    if (world->player.needs_update)
    {
        world->player.previous_pos = world->player.position;
        world->player.position.x += world->player.velocity.x;
        world->player.position.y += world->player.velocity.y;
        // Stop moving at edge
        if (world->player.position.x < 0)
        {
            world->player.position.x = 0;
        }
        else if (world->player.position.x >
                 MAP_WIDTH - world->player.dimension.width)
        {
            world->player.position.x =
                MAP_WIDTH - world->player.dimension.width;
        }
        if (world->player.position.y < 100)
        {
            world->player.position.y = 100;
        }
        else if (world->player.position.y >
                 MAP_HEIGHT - world->player.dimension.height)
        {
            world->player.position.y =
                MAP_HEIGHT - world->player.dimension.height;
        }
        //player dont need update but need render
        world->player.needs_render = 1;
        world->player.needs_update = 0;
    }

    //move enemy
    if (world->enemies.needs_update)
    {
        world->enemies.previous_pos = world->enemies.position;
        world->enemies.position.x += world->enemies.velocity.x;
        world->enemies.needs_render = 1;
        world->enemies.needs_update = 0;
    }
    if (isStage2){
        // Check if boss hit player then end game over
        if (world->enemies.position.x < world->player.position.x +
                                            world->player.dimension.width &&
            world->enemies.position.x + world->enemies.dimension.width >
                world->player.position.x &&
            world->enemies.position.y < world->player.position.y +
                                            world->player.dimension.height &&
            world->enemies.position.y + world->enemies.dimension.height >
                world->player.position.y)
        {
            endScreen(0, world);
        }
    }
    //move player laser
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (world->player.projectile[i].needs_update)
        {//if not reach top then move
            if (world->player.projectile[i].position.y > TOP_MAX)
            {
                world->player.projectile[i].previous_pos =
                    world->player.projectile[i].position;
                world->player.projectile[i].position.x +=
                    world->player.projectile[i].velocity.x;
                world->player.projectile[i].position.y +=
                    world->player.projectile[i].velocity.y;
                world->player.projectile[i].needs_render = 1;
            }
            else
            {//clear
                world->player.projectile[i].needs_render = 0;
                world->player.projectile[i].active = 0;
                world->player.projectile[i].needs_clear = 1;
            }
        }
    }
    //move enemy laser
    for (int j = 0; j < MAX_BULLETS; j++)
        {//if not reach bottom then move
        if (world->enemies.projectile[j].needs_update)
        {
            if (world->enemies.projectile[j].position.y < BOTTOM_MAX)
            {
                world->enemies.projectile[j].previous_pos =
                    world->enemies.projectile[j].position;
                world->enemies.projectile[j].position.x +=
                    world->enemies.projectile[j].velocity.x;
                world->enemies.projectile[j].position.y +=
                    world->enemies.projectile[j].velocity.y;
                world->enemies.projectile[j].needs_render = 1;
            }
            else
            {
                world->enemies.projectile[j].needs_render = 0;
                world->enemies.projectile[j].active = 0;
                world->enemies.projectile[j].needs_clear = 1;
            }
        }
    }
}
// shoot projectile for enemy
//----------------------------------------------------------------------------
void enemy_shoot(World *world)
{

    int random = (rand() % 100) % 10;
    if (isStage2)
    {//boss shoot
        entity_shoot(&world->enemies, DOWN);
    }
    else
    {//random shot meteor
        entity_shoot(&world->enemies, DOWN);
    }
}

// Function to generate random number
//----------------------------------------------------------------------------
unsigned long int next = 1;
int rand(void)
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}

// Function to generate a random number between 100 and 900
//----------------------------------------------------------------------------
int randEnemiesPosition()
{
    // Generate a random number between 0 and 800 (inclusive)
    int randomNumber = rand() % 801;

    // Add 100 to the generated number to get a range from 100 to 900
    return randomNumber + 100;
}

// init bullet for entity
//----------------------------------------------------------------------------
void entity_shoot(Entity *entity, Direction direction)
{

    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!entity->projectile[i].active)
        {
            // Initial a bullet red laser from player in the middle
            if (entity->type == PLAYER)
            {
                entity->projectile[i].position.x =
                    entity->position.x + (entity->dimension.width / 2);
                entity->projectile[i].position.y =
                    entity->position.y - entity->dimension.height;
                entity->projectile[i].dimension.height = red_laser.height;
                entity->projectile[i].dimension.width = red_laser.width;
            }
            else
            {
                if (isStage2)
                {// Initial a bullet green laser from Boss
                    entity->projectile[i].position.x =
                        entity->position.x + (entity->dimension.width / 2);
                    entity->projectile[i].position.y =
                        entity->position.y + entity->dimension.height;
                    entity->projectile[i].dimension.height = boss_bomb.height;
                    entity->projectile[i].dimension.width = boss_bomb.width;
                }
                else
                {// Initial asteroid (bullet from invisible enemy)
                    entity->projectile[i].position.x =
                        randEnemiesPosition();
                    entity->projectile[i].position.y =
                        50;
                    entity->projectile[i].dimension.height = asteroid_image.height;
                    entity->projectile[i].dimension.width = asteroid_image.width;
                }
            }
            entity->projectile[i].needs_update = 1;
            entity->projectile[i].needs_render = 1;
            entity->projectile[i].active = 1;
            move_bullet(&entity->projectile[i], direction);
            return;
        }
    }
}
// move enemy
//----------------------------------------------------------------------------
void update_AI(World *world)
{
    /* check wall collisions */
    
    if ((world->enemies.position.x +
            world->enemies.dimension.width) >= (RIGHT_MAX))
    {
        travel_right = 0;
    }
    else if ((world->enemies.position.x) <= (LEFT_MAX))
    {
        travel_right = 1;

    }
    
    if (travel_right)
    {
        move_entity(&world->enemies, RIGHT);
    }
    else
    {
        move_entity(&world->enemies, LEFT);
    }
}
// move projectile up or down
//----------------------------------------------------------------------------
void move_bullet(Projectile *projectile, Direction direction)
{
    switch (direction)
    {
    case UP:
        projectile->velocity.y = -BULLET_VELOCITY;
        break;
    case DOWN:
        projectile->velocity.y = BULLET_VELOCITY;
        break;
    default:
        projectile->velocity.y = 0;
    }
}

// check intersect projectile and entity
//----------------------------------------------------------------------------
int intersectPtoE(Projectile *projectile, Entity *entity)
{
    return projectile->position.x <
               (entity->position.x + entity->dimension.width) &&
           (projectile->position.x + projectile->dimension.width) >
               entity->position.x &&
           projectile->position.y <
               (entity->position.y + entity->dimension.height) &&
           (projectile->position.y + projectile->dimension.height) >
               entity->position.y;
}
//check intersect of 2 projectile 
//----------------------------------------------------------------------------
int intersectPtoP(Projectile *projectile, Projectile *projectile2)
{
    return projectile->position.x <
               (projectile2->position.x + projectile2->dimension.width) &&
           (projectile->position.x + projectile->dimension.width) >
               projectile2->position.x &&
           projectile->position.y <
               (projectile2->position.y + projectile2->dimension.height) &&
           (projectile->position.y + projectile->dimension.height) >
               projectile2->position.y;
}

//handle intersect projectile and entity
//----------------------------------------------------------------------------
void collisionsPE(Projectile *projectile, Entity *entity)
{//if both projectile and entity is active and they intersect then update value
    int isEnabled = entity->enabled;
    int intersects = intersectPtoE(projectile, entity);
    if (isEnabled && intersects)
    {//clear projectile and update entity
        projectile->active = 0;
        projectile->needs_update = 0;
        projectile->needs_render = 0;
        projectile->needs_clear = 1;
        entity->combat_update = 1;
    }
}

//handle intersect of 2 projectile 
//----------------------------------------------------------------------------
void collisionsPP(Projectile *projectile, Projectile *projectile2, World *world)
{//if both projectile intersect then update value
    int intersects = intersectPtoP(projectile, projectile2);
    if (intersects)
    {//explode, clear both projectile and update score
        drawExplosionBig(projectile2);
        wait_msec(1000);
        projectile->active = 0;
        projectile->needs_update = 0;
        projectile->needs_render = 0;
        projectile->needs_clear = 1;
        projectile2->active = 0;
        projectile2->needs_update = 0;
        projectile2->needs_render = 0;
        projectile2->needs_clear = 1;
        update_score(world);
        world->playerScore.needsRender = 1;
        uart_puts("\n +++ HIT +++ \n");
    }
}

// Function to check collison constantly
//----------------------------------------------------------------------------
void update_collision(World *world)
{
    Entity *player = &world->player;
    Entity *enemy = &world->enemies;
//check collision of player projectile and enemy projectile
    for (int j = 0; j < MAX_BULLETS; j++)
        {
            if (enemy->projectile[j].active)
            {//check if enemy projectile collide with player
                collisionsPE(&enemy->projectile[j], player);
                for (int a = 0; a < MAX_BULLETS; a++)
                {
                    if (player->projectile[a].active)
                    {// check player and enemy projectile collide
                        collisionsPP(&player->projectile[a], &enemy->projectile[j], world);
                    }
                }
            }
        }
        //check collision between player projectile and Boss
        if (isStage2)
        {
            for (int a = 0; a < MAX_BULLETS; a++)
            {
                if (player->projectile[a].active)
                {
                    collisionsPE(&player->projectile[a], enemy);
                }
            }
        }
}

// Function to update the score and health from gameplay
//----------------------------------------------------------------------------
void update_combat(World *world)
{//player lose health
    if (world->player.combat_update)
    {
        drawExplosion(world->player);
        wait_msec(500);

        drawSpaceShip(world->player, world);
        world->life.needs_render = 1;
        world->player.health.current_health -= 1;
        
        uart_puts("\n !!! CRASHED !!! \n");
        uart_puts("\n [-1 HEALTH POINT] \n");

        if (world->player.health.current_health <= 0)
        {
            world->player.enabled = 0;
            world->player.needs_clear = 1;
        }
        world->player.combat_update = 0;
        if (world->player.health.current_health == 0)
        {
            if(world->player.enabled == 0)
                printf("\n======YOU DIED======\n");
            clearPlayerLife(170, 20);
            drawString(170, 10, "0", "white");
            endScreen(0, world);
        }
    }
    if (world->playerScore.score >= 300)
    {//spawn boss
        isStage2 = 1;
        if (isStage2 == 1 && check == 0)
        {render(world);
            for (int j = 0; j < MAX_BULLETS; j++)
            { clear_projectile(
                world->enemies.projectile[j].previous_pos,
                world->enemies.projectile[j].dimension);
                clear_projectile(
                world->enemies.projectile[j].position,
                world->enemies.projectile[j].dimension);
                world->enemies.projectile[j].active = 0;
                world->enemies.projectile[j].position.y = 1000;
            }

            check = 1;
            world->enemies.position.x =
                enemy_initial_x + (HORIZONTAL_OFFSET * 0);
            world->enemies.position.y =
                enemy_initial_y + 50;

            world->enemies.dimension.height = boss_image.height;
            world->enemies.dimension.width = boss_image.width;
            world->enemies.health.current_health = BOSS_HEALTH;
            world->enemies.type = BOSS;
            if (check == 1)
                {
                    printf("\n <=====BOSS WAVE=====> \n");
                    printf("\n BOSS HEALTH: %d\n", world->enemies.health.current_health);
                }
        }
    }

    if (isStage2)
    {
        
        //boss lose health
        if (world->enemies.combat_update)
        {
            drawExplosion(world->enemies);
            world->enemies.health.current_health -= 1;
            uart_puts("\n +++ HIT +++ \n");
            printf("\n BOSS HEALTH: %d\n", world->enemies.health.current_health);
            if (world->enemies.health.current_health <= 0)
            {//clear boss and end game when boss die
                world->enemies.enabled = 0;
                world->enemies.needs_clear = 1;
                printf("\n======YOU WON======\n");
                wait_msec(500);
                endScreen(1, world);
            }
            world->enemies.combat_update = 0;
        }
    }
}


// Draw the entity using the data has set
//----------------------------------------------------------------------------
void render(World *world)
{
    if (quitGame)
    {
        return;
    }
    wait_msec(30000);
    //render player bullet
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        Type type = world->player.type;

        if (world->player.projectile[i].needs_render)
        {
            clear_projectile(world->player.projectile[i].previous_pos,
                             world->player.projectile[i].dimension);
            draw_projectile(type, world->player.projectile[i].position,
                            world->player.projectile[i].dimension);
        }
        else if (world->player.projectile[i].needs_clear)
        {
            clear_projectile(world->player.projectile[i].position,
                             meteors);
            world->player.projectile[i].needs_clear = 0;
        }
    }
    //render enemy
    if (world->enemies.needs_render && world->enemies.enabled)
    {
        clear(world->enemies);
        drawEntity(world->enemies);

        world->enemies.needs_render = 1; // 0 default
    }
    else if (world->enemies.needs_clear)
    {
        clear(world->enemies);
        // fix bug bullet not clear
        for (int j = 0; j < MAX_BULLETS; j++)
        {
            clear_projectile(world->enemies.projectile[j].position,
                                world->enemies.projectile[j].dimension);
        }

        drawSpaceShip(world->player, world);
        world->enemies.needs_clear = 0;
        }
    //render enemy bullet
    for (int j = 0; j < MAX_BULLETS; j++)
    {
        Type type = world->enemies.type;
        if (world->enemies.projectile[j].needs_render)
        {

            clear_projectile(
                world->enemies.projectile[j].previous_pos,
                world->enemies.projectile[j].dimension);
            draw_projectile(type,
                            world->enemies.projectile[j].position,
                            world->enemies.projectile[j].dimension);
        }
        else if (world->enemies.projectile[j].needs_clear)
        {
            clear_projectile(world->enemies.projectile[j].position,
                                world->enemies.projectile[j].dimension);
            world->enemies.projectile[j].needs_clear = 0;
        }
    }
//render player
    if (world->player.needs_render && world->player.enabled)
    {
        clear(world->player);
        drawSpaceShip(world->player, world);
        world->player.needs_render = 0;
    }
    else if (world->player.needs_clear)
    {
        drawExplosion(world->player);
        wait_msec(500);
        world->player.needs_clear = 0;
    }
//render life
    if (world->life.needs_render)
    {
        render_health(world);
    }
    //render score
    if (world->playerScore.needsRender)
    {
        char *type = "";
        drawScore(world, type);
        world->playerScore.needsRender = 0;
    }
}

// Function to get the score
//----------------------------------------------------------------------------
void drawScore(World *world, char *type)
{
    int x = 0;
    int y = 0;
    if (type == "d")
    {
        x = 350;
        y = 590;

        // Print the score at end game
        displayScore(700 - x, 3 + y);
    }
    if (type == "")
    {
        // Print the word score
        displayScore(700, 3);
    }

    int ones = (world->playerScore.score % 10);
    int tens = (world->playerScore.score % 100) / 10;
    int hundreds = (world->playerScore.score % 1000) / 100;
    int thousands = (world->playerScore.score % 10000) / 1000;
//render score up to 4 decimal
    int score = world->playerScore.score;
    if (score >= 0 && score < 10)
    {
        clearScore(ones, SCORE_ORIGINX - x, SCORE_ORIGINY + y);
        render_score(ones, SCORE_ORIGINX - x, SCORE_ORIGINY + y);
    }
    else if (score >= 10 && score < 100)
    {
        clearScore(tens, SCORE_ORIGINX - x, SCORE_ORIGINY + y);
        clearScore(ones, SCORE_ORIGINX - x + SHIFT, SCORE_ORIGINY + y);
        render_score(tens, SCORE_ORIGINX - x, SCORE_ORIGINY + y);
        render_score(ones, SCORE_ORIGINX - x + SHIFT, SCORE_ORIGINY + y);
    }
    else if (score >= 100 && score < 1000)
    {
        clearScore(hundreds, SCORE_ORIGINX - x, SCORE_ORIGINY + y);
        clearScore(tens, SCORE_ORIGINX - x + SHIFT, SCORE_ORIGINY + y);
        clearScore(ones, SCORE_ORIGINX - x + SHIFT + SHIFT, SCORE_ORIGINY + y);
        render_score(hundreds, SCORE_ORIGINX - x, SCORE_ORIGINY + y);
        render_score(tens, SCORE_ORIGINX - x + SHIFT, SCORE_ORIGINY + y);
        render_score(ones, SCORE_ORIGINX - x + SHIFT + SHIFT, SCORE_ORIGINY + y);
    }
    else if (score >= 1000)
    {
        clearScore(thousands, SCORE_ORIGINX, SCORE_ORIGINY);
        clearScore(hundreds, SCORE_ORIGINX + SHIFT, SCORE_ORIGINY);
        clearScore(tens, SCORE_ORIGINX + SHIFT + SHIFT, SCORE_ORIGINY);
        clearScore(ones, SCORE_ORIGINX + SHIFT + SHIFT + SHIFT,
                   SCORE_ORIGINY);

        render_score(thousands, SCORE_ORIGINX, SCORE_ORIGINY);
        render_score(hundreds, SCORE_ORIGINX + SHIFT, SCORE_ORIGINY);
        render_score(tens, SCORE_ORIGINX + SHIFT + SHIFT, SCORE_ORIGINY);
        render_score(ones, SCORE_ORIGINX + SHIFT + SHIFT + SHIFT,
                     SCORE_ORIGINY);
    }
}

// Function to display health (have 3 hearts in total)
//----------------------------------------------------------------------------
void render_health(World *world)
{
    int clife = (world->player.health.current_health);
    displayWordPlayerLife(13, 10);
    int trigger;

    if (clife == 0)
    {
        clearPlayerLife(170, 30);
        drawString(170, 10, "0", "white");
        if(trigger == 0)
            printf("\nHEALTH: %d\n", clife);
        trigger = 1;
    }

    if (clife == 1)
    {
        clearPlayerLife(210, 20);
        displayPlayerLife(170, 20);
        printf("\nHEALTH: %d\n", clife);
    }
    if (clife == 2)
    {
        clearPlayerLife(250, 20);
        displayPlayerLife(170, 20);
        displayPlayerLife(210, 20);
        printf("\nHEALTH: %d\n", clife);
    }
    if (clife == 3)
    {
        displayPlayerLife(170, 20);
        displayPlayerLife(210, 20);
        displayPlayerLife(250, 20);
        printf("\nHEALTH: %d\n", clife);
        trigger = 0;
    }

    world->life.needs_render = 0;
}

// Function to clear 1 heart health whenever spaceship gets hit
//----------------------------------------------------------------------------
void clear_health(int x, int y)
{

    drawString(x, y, " ", "black");
}

// Function to display score with font
//----------------------------------------------------------------------------
void render_score(int num, int x, int y)
{
    if (num == 1)
        drawString(x, y, "1", "yellow");
    else if (num == 2)
        drawString(x, y, "2", "yellow");
    else if (num == 3)
        drawString(x, y, "3", "yellow");
    else if (num == 4)
        drawString(x, y, "4", "yellow");
    else if (num == 5)
        drawString(x, y, "5", "yellow");
    else if (num == 6)
        drawString(x, y, "6", "yellow");
    else if (num == 7)
        drawString(x, y, "7", "yellow");
    else if (num == 8)
        drawString(x, y, "8", "yellow");
    else if (num == 9)
        drawString(x, y, "9", "yellow");
    else if (num == 0)
        drawString(x, y, "0", "yellow");
}
// Function to update score per hit
//----------------------------------------------------------------------------
void update_score(World *world)
{
    // Score per hit
    // world->playerScore.score += 100;
    world->playerScore.score += 30;
    //set cap
    if (world->playerScore.score>9999) world->playerScore.score =9999;
     printf("\nScore: %d [+30 POINTS]\n", world->playerScore.score);
}

//draw the pause menu
//----------------------------------------------------------------------------
void drawPauseMenu(World *game)
{
    int *colorptrMenu;
    int widthMenu = game_menu_pause.width;
    int heightMenu = game_menu_pause.height;
    if (game->game_menu.game_menu_option == 2)
        colorptrMenu = (int *)game_menu_pause.image_pixels;
    else if (game->game_menu.game_menu_option == 1)
        colorptrMenu = (int *)game_menu_restart.image_pixels;
    else if (game->game_menu.game_menu_option == 0)
        colorptrMenu = (int *)game_menu_quit.image_pixels;

    int xMenu = (int)((MAP_WIDTH / 2) - (widthMenu / 2));
    int yMenu = TOP_MAX;

    for (int i = 0; i < (widthMenu * heightMenu); i++)
    {
        xMenu++;
        if (i % widthMenu == 0)
        {
            yMenu++;
            xMenu = (int)((MAP_WIDTH / 2) - (widthMenu / 2));
        }
        if (colorptrMenu[i] != 0x00000000)
        {
            
            drawPixelARGB32(xMenu, yMenu, colorptrMenu[i]);
        }
        else
        {
            drawPixelARGB32(xMenu, yMenu, background_universe_image[yMenu * universe_background_width + xMenu]);
        }
        
    }
}
//draw the main menu
//----------------------------------------------------------------------------
void drawMainMenu(Game *game)
{
    int *colorptrMenu;
    int widthMenu = start_image.width;
    int heightMenu = start_image.height;

    if ((game->main_menu.game_start_menu))
        colorptrMenu = (int *)start_image.image_pixels;
    else
        colorptrMenu = (int *)quit_image.image_pixels;

    int xMenu = (int)((MAP_WIDTH / 2) - (widthMenu / 2));
    int yMenu = TOP_MAX + 455;

    for (int i = 0; i < (widthMenu * heightMenu); i++)
    {
        xMenu++;
        if (i % widthMenu == 0)
        {
            yMenu++;
            xMenu = (int)((MAP_WIDTH / 2) - (widthMenu / 2));
        }
        if (colorptrMenu[i] != 0xFF000000)
        {
            drawPixelARGB32(xMenu, yMenu, colorptrMenu[i]);
        }
        else
        {
            drawPixelARGB32(xMenu, yMenu, background_universe_image[yMenu * universe_background_width + xMenu]);
        }
    }
}

//draw explosion for entity
//----------------------------------------------------------------------------
void drawExplosion(Entity entity)
{

    int x = entity.position.x;
    int oldX = x;
    int y = entity.position.y;

    if (entity.type == PLAYER)
    {
        displayExplosion(x, y);
    }
}
//draw exposion for projectile
//----------------------------------------------------------------------------
void drawExplosionBig(Projectile* projectile)
{
    int x = projectile->position.x;
    int oldX = x;
    int y = projectile->position.y;

    displayExplosionBig(x, y);
}
//draw spaceship
//----------------------------------------------------------------------------
void drawSpaceShip(Entity entity, World *world)
{
    int score = world->playerScore.score;
    int width = entity.dimension.width;
    int height = entity.dimension.height;

    int x = entity.position.x;
    int oldX = x;
    int y = entity.position.y;
    
    if (entity.type == PLAYER)
    {

        displaySpaceShipImage(x, y);
    }
}
//restart the game
//----------------------------------------------------------------------------
void restart_game(Game *world)
{
    clearscreen(0, 0);
    init_map(&world->world);
    world->game_over = 0;
    world->game_start = 0;
    world->main_menu.on_game_menu = 1;
    world->main_menu.game_start_menu = 1;
    world->game_win = 0;
    restartGame = 0;
    isStage2 = 0;
    check = 0;
    displayGameUniverseBackground(0, 0);
    pauseGame = 0;
    quitGame = 0;
}
//clear entity
//----------------------------------------------------------------------------
void clear(Entity entity)
{
    int width = entity.dimension.width;
    int height = entity.dimension.height;

    int x = entity.previous_pos.x;
    int oldX = x;
    int y = entity.previous_pos.y;

    for (int i = 0; i < (width * height); i++)
    {
        x++;
        if (i % width == 0)
        {
            y++;
            x = oldX;
        }
        drawPixelARGB32(x, y, background_universe_image[y * universe_background_width + x]);
    }
}
