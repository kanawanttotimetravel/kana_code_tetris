#ifndef TETROMINO_H
#define TETROMINO_H

#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>
#include "point.h"

struct Tetromino // a Tetromino consisted of 4 blocks, so we need 4 points
{
    point block[4];
    int color;
    Tetromino()
    {
        color = 0;
    }
};

Tetromino getTetromino(const int &x) // Create a Tetromino
{
    /*
        We need 8 points to be able to represent all type of tetrominos
        0 2 4 6
        1 3 5 7
    */
    int basis[7][4] = // 7 types of basic Tetrominos
        {
            1, 3, 5, 7, // I
            4, 5, 6, 7, // O
            2, 4, 5, 7, // S
            3, 5, 4, 6, // Z
            2, 6, 4, 7, // L
            3, 7, 5, 6, // J
            2, 4, 5, 6  // T
        };
    Tetromino t;
    for (int i = 0; i < 4; i++)
    {
        t.block[i].x = basis[x][i] / 2 + 2;
        t.block[i].y = basis[x][i] % 2;
    }
    t.color = x;
    return t;
}

#endif