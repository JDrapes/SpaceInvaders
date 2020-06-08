/**
* File: main.cpp
* Author: Jordan Draper
* Date 06/02/2018
* Desc: My game is called blaster, you tilt the micro:uBit left
* or right to move the player model, the aim is to destroy the
* enemies moving down the screen before they reach the bottom.
* Sources: List of sources i used to help me - YouTube, os.mbed.com and Lancaster university github.
*
* Controls :
* Left button  --> Shoot
* Right button --> Shoot
* Back button  --> Restart game
*
*/

#include "MicroBit.h"

#define GAME_START 0
#define GAME_OVER 1


struct Ledpos
{
    int     x;
    int     y;
};

MicroBit        uBit;
MicroBitImage   enemies(5,5);
int             score;
int             game_over;
int             level;
int             ENEMY_SPEED = 750;
//int             ENEMY_SPEED = 10; //Used for testing attack speed to slow down enemies
int             PLAYER_SPEED = 150;
int             ATTACK_SPEED = 50;
Ledpos           player;
Ledpos           laser;

//Function to add a row of enemies to the top of the LED interface
int addRow() {
    for (int x=0; x<5; x++)
        if (enemies.getPixelValue(x,4))
            return GAME_OVER;
    enemies.shiftDown(1); //Move the enemies a row down the LED interface, if there is no space the player loses
    for (int x=1; x<4; x++)
        enemies.setPixelValue(x,0,255);
    return GAME_START;
}

//Function ending the game and displaying the players score by scrolling it across the LEDs
void gameOver(){
    uBit.display.clear();
    uBit.display.scroll("SCORE: ");
    uBit.display.scroll(score);
}

// Calculate the speed of an enemies movement, based on time alive
int enemiespeed(){
    return max(ENEMY_SPEED - level*50, 50);
}

//Boolean to check if the y axis is clear for a new enemy to join the row
bool enemiesInColumn(int x){
    for (int y = 0; y < 5; y++)
        if (enemies.getPixelValue(x,y))
            return true;
    return false;
}

//Checks how many enemies are currently on screen so it knows when it can add more
bool enemyCount(){
    int count = 0;
    for (int x=0; x<5; x++)
        for (int y=0; y<5; y++)
            if (enemies.getPixelValue(x,y))
                count++;
    return count;
}


 //Function that moves the enemies on the screen
void enemyUpdate(){
    bool movingRight = true;
    while(!game_over)
    {
        // Wait for next update;
        uBit.sleep(enemiespeed());
        if (movingRight)
        {
            if(enemiesInColumn(4))
            {
                movingRight = false;
                if (addRow() == GAME_OVER)
                {
                    game_over = true;
                    return;
                }
            }
            else
            {
                enemies.shiftRight(1);
            }
        }
        else
        {
            if(enemiesInColumn(0))
            {
                movingRight = true;
                if (addRow() == GAME_OVER)
                {
                    game_over = true;
                    return;
                }
            }
            else
            {
                enemies.shiftLeft(1);
            }
        }

        if (enemyCount() == 0)
        {
            level++;
            addRow();
        }
    }
}

//Function that creates the mechanic to move the players laser shot up the LED interface
void laserUpdate(){
    while (!game_over)
    {
        uBit.sleep(ATTACK_SPEED);
        if (laser.y != -1)
            laser.y--;
        if (enemies.getPixelValue(laser.x, laser.y) > 0)
        {
            score++; //if an enemy dies to the laser increment the players score
            enemies.setPixelValue(laser.x, laser.y, 0);
            laser.x = -1;
            laser.y = -1;
        }
    }
}

//Function that enables the built in accelerometer, this allows the user to control the game by tilting.
void playerUpdate(){
    while (!game_over)
    {
        uBit.sleep(PLAYER_SPEED);
        if(uBit.accelerometer.getX() < -300 && player.x > 0)
            player.x--;
        if(uBit.accelerometer.getX() > 300 && player.x < 4)
            player.x++;
    }
}

//Fire a laser from the player
void fire(MicroBitEvent){
    if (laser.y == -1)
    {
        laser.y = 4;
        laser.x = player.x;
    }
}

//This is the main block of the code, it sets all the variables such as score and level
//It also spawns the fibres to allow the game to keep updating as it plays through
void theenemies(){
    game_over = 0;
    level = 0;
    score = 0;
    player.x = 2;
    player.y = 4;
    laser.x = -1;
    laser.y = -1;
//Add first wave of enemies and then repeat.
    enemies.clear();
    addRow();
//Creating fibres to prevent deadlocks.
    create_fiber(enemyUpdate);
    create_fiber(laserUpdate);
    create_fiber(playerUpdate);

//Listeners to allow button A and B to shoot when pressed
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, fire);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, fire);

//Keeps the game running here.
    while (!game_over)
    {
        uBit.sleep(10);
        uBit.display.image.paste(enemies);
        uBit.display.image.setPixelValue(player.x, player.y, 255);
        uBit.display.image.setPixelValue(laser.x, laser.y, 255);
    }

//Once they lose run the game end function to display score and restart the game
    gameOver();
}

int main()
{
    // Initialise
    uBit.init();
    uBit.display.scroll("Prepare to battle!");
    while(1)
        theenemies();
        //Constantly run the game.

}

