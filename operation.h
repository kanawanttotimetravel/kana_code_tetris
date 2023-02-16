#ifndef OPERATION_H
#define OPERATION_H

#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>

#include "logic.h"


// If a line is full, that line will be erased, and others line will fall down
int clearLines(board &b)
{
    std::vector<int> empty(COLUMN, 0);
    board newBoard(ROWS, std::vector<int>(COLUMN, 0));
    int newRow = ROWS - 1;
    int rowCleared = 0;
    for (int i = ROWS - 1; i >= 0; i--)
    {
        if (!checkLines(b, i))
            newBoard[newRow--] = b[i];
        else
            rowCleared++;
    }
    b = newBoard;
    return rowCleared;
}

// convert integer into string
std::string intToString(int x)
{
    std::string s = "";
    while (x > 0)
    {
        s = char(x % 10 + '0') + s;
        x /= 10;
    }
    return s;
}

std::string intToStringFilled(int x)
{
    std::string s = intToString(x);
    while (s.size() < 6)
    {
        s = "0" + s;
    }
    return s;
}

// get score from clearing
int getScore(const int &x, int level)
{
    int line[5] = {0, 100, 300, 500, 600};
    return line[x];
}

sf::Text TextSetup(const sf::Font &font, const int &size, const sf::Color &color, const std::string &string)
{
    sf::Text t;
    t.setFont(font);
    t.setCharacterSize(size);
    t.setFillColor(color);
    t.setString(string);
    return t;
}

// Just to random
int myrandom(int i)
{
    return std::rand() % i;
}

// exponent
double powd(double x, int y)
{
    if (y == 0)
        return 1;
    if (y == 1)
        return x;
    double t = powd(x, y / 2);
    return t * t * powd(x, y % 2);
}

#endif