#ifndef POINT_H
#define POINT_H

#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>

typedef std::vector<std::vector<int>> board;
const int ROWS = 20;
const int COLUMN = 10;
double DEFAULT_DELAY = 1;

struct point // represent spacial position
{
    int x, y;
    point()
    {
        x = 0, y = 0;
    }
    point(int _x, int _y)
    {
        x = _x, y = _y;
    }
};

// check if the point is available
bool isValidPoint(const point &p, const board &b)
{
    if (p.x < 0 || p.x >= COLUMN || p.y < 0 || p.y >= ROWS)
        return 0;
    if (b[p.y][p.x])
        return 0;
    return 1;
}


#endif