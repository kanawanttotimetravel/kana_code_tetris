#ifndef LOGIC_H
#define LOGIC_H

#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>
#include "Tetromino.h"

// check whether the tetromino's potision is valid or not
bool isValidPotision(const Tetromino &t, const board &b)
{
    for (int i = 0; i < 4; i++)
    {
        if (!isValidPoint(t.block[i], b))
            return 0;
    }
    return 1;
}


// check if the game has ended
bool isEnd(const board &b)
{
    for (int i = 0; i < COLUMN; i++)
    {
        if (b[0][i])
            return 1;
    }
    return 0;
}

// check whether a line is full or not
bool checkLines(board &b, const int &row)
{
    for (int i = 0; i < COLUMN; i++)
    {
        if (!b[row][i])
            return 0;
    }
    return 1;
}

bool isInside(const point &pos, const point &upperLeft, const point &lowerRight)
{
    return ((pos.x >= upperLeft.x) &&
            (pos.x <= lowerRight.x) &&
            (pos.y >= upperLeft.y) &&
            (pos.y <= lowerRight.y));
}

#endif