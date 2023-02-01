/*
    Project: Tetris
    Author: Kanasapchet
    Tasks done:
    * "Gameplay"
    - Define gamespace, tetromino, gravity
    - Define horizontal, vertical, rotational movement.
    + Soft drop
    + Wall kick
    + Hard drop
    - Hold system
    - Leveling

    * "Interface"
    - Pause and countdown
    - Scoring system
    - Line and level

    * "Graphic"
    - Render game state
    - Render score
    - Render grid
    - Render hold section
    - Render pause and countdown 
    - Render game over (demo)

    * "Audio"
    - Background music
    - Sound effects

    To-do:
    - Complete "Graphics"
    - Factorize by function*
    - Factorize by file*

*/

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <time.h>
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

// check if the point is available
bool isValidPoint(const point &p, const board &b)
{
    if (p.x < 0 || p.x >= COLUMN || p.y < 0 || p.y >= ROWS)
        return 0;
    if (b[p.y][p.x])
        return 0;
    return 1;
}

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

// Just to random
int myrandom(int i)
{
    return std::rand() % i;
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

sf::Text TextSetup(const sf::Font &font, const int &size, const sf::Color &color, const std::string &string)
{
    sf::Text t;
    t.setFont(font);
    t.setCharacterSize(size);
    t.setFillColor(color);
    t.setString(string);
    return t;
}

bool isInside(const point &pos, const point &upperLeft, const point &lowerRight)
{
    return ((pos.x >= upperLeft.x) &&
            (pos.x <= lowerRight.x) &&
            (pos.y >= upperLeft.y) &&
            (pos.y <= lowerRight.y));
}

// main
int main()
{
    std::srand(std::time(NULL)); // Random seed

    // Graphics setup
    sf::RenderWindow window(sf::VideoMode(550, 600), "Kurisu"); // Create a window

    sf::Texture texture, backgroundTextures, pauseScreenTextures, helpScreenTextures;
    texture.loadFromFile("textures/Tetromino.png"); // Get the texture ready
    backgroundTextures.loadFromFile("textures/Background.jpg");
    pauseScreenTextures.loadFromFile("textures/Pause.jpg");
    helpScreenTextures.loadFromFile("textures/Help.jpg");

    const int BLOCK_SIZE = 25;
    sf::Sprite sprite, background, pauseScreen, helpScreen;
    sprite.setTexture(texture);

    background.setTexture(backgroundTextures);

    pauseScreen.setTexture(pauseScreenTextures);

    helpScreen.setTexture(helpScreenTextures);

    // Audio setup
    // SFX
    sf::SoundBuffer rotateSoundBuffer, movementSoundBuffer, hardDropSoundBuffer, holdSoundBuffer;
    rotateSoundBuffer.loadFromFile("audio/Rotate.wav");
    movementSoundBuffer.loadFromFile("audio/Movement.wav");
    hardDropSoundBuffer.loadFromFile("audio/HardDrop.wav");
    holdSoundBuffer.loadFromFile("audio/Hold.wav");

    sf::Sound rotateSound, movementSound, hardDropSound, holdSound;
    rotateSound.setBuffer(rotateSoundBuffer);
    movementSound.setBuffer(movementSoundBuffer);
    hardDropSound.setBuffer(hardDropSoundBuffer);
    holdSound.setBuffer(holdSoundBuffer);

    // BGM
    sf::Music music;
    if (!music.openFromFile("audio/KorobeinikiFast.ogg"))
    {
        exit(0);
    }
    music.setLoop(1);

    // tetra: the current tetromino potision
    // prev: the previous potision, served as a backup whenever the current potision is invalid
    Tetromino tetra, prev;

    // Game's time
    sf::Clock clock;
    double timer = 0, delay = DEFAULT_DELAY;

    // Represent the game's state
    board boardStates(ROWS, std::vector<int>(COLUMN, 0));

    /*
        Tetrominos will be delivered in "patch" of 7 types, each types will only have 1 tetromino
        The patch will be represented by a sequences contain numbers from 0 to 6
        The order of appearing will be represented by a random permutation of said sequences
        Getting a new Tetromino would be like taking them from a bag randomly, and when a bag is empty, we will get a new bag
        Because we don't care about the used bags, so 1 bag is enough to represent all the bags
    */
    int played = 1;
    std::vector<int> bag;
    for (int i = 0; i < 7; i++)
        bag.push_back(i);
    std::random_shuffle(bag.begin(), bag.end(), myrandom);
    tetra = getTetromino(bag[0]);
    sprite.setTextureRect(sf::IntRect(tetra.color * BLOCK_SIZE, 0, BLOCK_SIZE, BLOCK_SIZE));

    // Hold
    int hold = 0, isHeld = 0, heldTetromino = -1;

    // Score
    int score = 0, level = 1, line = 0;
    sf::Font font;
    font.loadFromFile("fonts/Retro Gaming.ttf");

    // Other necessary variables
    bool gameStarted = 0, isPlaying = 1, isReleased = 1, hardDrop = 0, softDrop = 0, isHelp = 0;
    int countdown = 3;

    while (window.isOpen())
    {
        int dx = 0, rotate_cw = 0, rotate_ccw = 0;

        // Handling time
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;

        // Event variable
        sf::Event event;

        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;

            case sf::Event::KeyPressed:
            {
                // move left
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                {
                    dx = -1;
                }

                // move right
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                {
                    dx = 1;
                }

                // rotate
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
                {
                    rotate_cw = 1;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
                {
                    rotate_ccw = 1;
                }

                // soft drop
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
                {
                    delay = DEFAULT_DELAY / 20;
                    softDrop = 1;
                }

                // hard drop
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                {
                    if (isReleased)
                        hardDrop = 1;
                    isReleased = 0;
                }

                // hold
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::C))
                {
                    if (!isHeld)
                        hold = 1;
                    isHeld = 1;
                }

                // pause
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                {
                    isPlaying ^= 1;
                }

                break;
            }

            case sf::Event::KeyReleased:
            {
                isReleased = 1;
                if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
                {
                    delay = DEFAULT_DELAY;
                    softDrop = 0;
                }
                break;
            }
            // window focus
            case sf::Event::LostFocus:
            {
                isPlaying = 0;
                break;
            }
            case sf::Event::GainedFocus:
            {
                isPlaying = 1;
                break;
            }
            // mouse action
            case sf::Event::MouseButtonPressed:
            {
                sf::Vector2i p = sf::Mouse::getPosition(window);
                point mousePotision(p.x, p.y);
                if (!isPlaying)
                {
                    if (!isHelp)
                    {
                        if (isInside(mousePotision, point(171, 217), point(373, 273)))
                        {
                            isPlaying = 1;
                        }

                        if (isInside(mousePotision, point(171, 295), point(373, 353)))
                        {
                            isHelp = 1;
                        }

                        if (isInside(mousePotision, point(171, 375), point(373, 433)))
                        {
                            window.close();
                        }
                    }
                    else 
                    {
                        if (isInside(mousePotision, point(475, 111), point(525, 161)))
                        {
                            isHelp = 0;
                        }
                    }
                }
                break;
            }

            default:
                break;
            }
        }
        prev = tetra;
        if (!isPlaying)
        {
            gameStarted = 0;
            countdown = 3;
            timer = 0;
        }

        // While the game is playing...
        if (isPlaying)
        {
            // Check whether the game have just been started or not
            if (!gameStarted)
            {
                // If the game have just been initiated, obviously we should have some spare seconds for preparation
                if (timer > 1)
                {
                    timer = 0;
                    countdown--;
                }
                if (countdown == 0)
                {
                    music.play();
                    gameStarted = 1;
                }
            }
            else
            {
                // Horizontal movement
                for (int i = 0; i < 4; i++)
                {
                    if (dx)
                        movementSound.play();

                    tetra.block[i].x += dx;
                }
                if (!isValidPotision(tetra, boardStates))
                    tetra = prev;

                // Rotation
                if (rotate_cw)
                {
                    rotateSound.play();

                    /*
                        Consider a clockwise rotation with angle T, original point (x,y), new point(x',y')
                        x' = xcos(T) - ysin(T)
                        y' = xsin(T) + ycos(T)
                        As T = 90deg => sin(T) = 1, cos(T) = 0
                        => x' = -y, y' = x
                        Now, consider a new coordinate that the origin is (a,b)
                        xn = x - a, yn = y - b
                        Thus: xn' = - y + b, yn' = x - a
                    */

                    point origin = tetra.block[1];
                    for (int i = 0; i < 4; i++)
                    {
                        // new coordinate in the new system
                        int xn = -tetra.block[i].y + origin.y;
                        int yn = tetra.block[i].x - origin.x;

                        // new coordinate in the original system
                        tetra.block[i].x = origin.x + xn;
                        tetra.block[i].y = origin.y + yn;
                    }

                    // wall kick
                    if (!isValidPotision(tetra, boardStates))
                    {
                        // check how the rotation change the tetromino's alignment
                        int newLeftMost = COLUMN, newUpMost = ROWS, newRightMost = 0;
                        for (int i = 0; i < 4; i++)
                        {
                            newLeftMost = std::min(newLeftMost, tetra.block[i].x);
                            newRightMost = std::max(newRightMost, tetra.block[i].x);
                            newUpMost = std::min(newUpMost, tetra.block[i].y);
                        }

                        if (newUpMost < 0)
                        {
                            for (int i = 0; i < 4; i++)
                            {
                                tetra.block[i].y -= newUpMost;
                            }
                        }

                        bool toLeft = 0;
                        for (int i = 0; i < 4; i++)
                        {
                            if (newLeftMost == tetra.block[i].x)
                            {
                                if (!isValidPoint(tetra.block[i], boardStates))
                                    toLeft = 1;
                            }
                        }

                        if (toLeft)
                        {
                            int move = 0;
                            for (int i = 0; i < 4; i++)
                            {
                                point p = point(tetra.block[i].y, tetra.block[i].x);
                                if (!isValidPoint(tetra.block[i], boardStates))
                                {
                                    move = std::max(move, p.y - newLeftMost + 1);
                                }
                            }

                            for (int i = 0; i < 4; i++)
                            {
                                tetra.block[i].x += move;
                            }
                        }
                        else
                        {
                            int move = 0;
                            for (int i = 0; i < 4; i++)
                            {
                                point p = point(tetra.block[i].y, tetra.block[i].x);
                                if (!isValidPoint(tetra.block[i], boardStates))
                                {
                                    move = std::max(move, newRightMost - p.y + 1);
                                }
                            }
                            for (int i = 0; i < 4; i++)
                            {
                                tetra.block[i].x -= move;
                            }
                        }
                    }
                }

                if (rotate_ccw)
                {
                    rotateSound.play();

                    /*
                        Rotating 90deg counter-clockwise is basically rotating -90deg clockwise
                        => sin(T) = -1, cos(T) = 0;
                        => x' = y, y' = -x;
                        Thus: xn' = y - b, yn' = - x + a
                    */

                    point origin = tetra.block[1];
                    for (int i = 0; i < 4; i++)
                    {
                        // new coordinate in the new system
                        int xn = tetra.block[i].y - origin.y;
                        int yn = -tetra.block[i].x + origin.x;

                        // new coordinate in the original system
                        tetra.block[i].x = origin.x + xn;
                        tetra.block[i].y = origin.y + yn;
                    }

                    // wall kick
                    if (!isValidPotision(tetra, boardStates))
                    {
                        // check how the rotation change the tetromino's alignment
                        int newLeftMost = COLUMN, newUpMost = COLUMN, newRightMost = 0;
                        for (int i = 0; i < 4; i++)
                        {
                            newLeftMost = std::min(newLeftMost, tetra.block[i].x);
                            newRightMost = std::max(newRightMost, tetra.block[i].x);
                            newUpMost = std::min(newUpMost, tetra.block[i].y);
                        }

                        if (newUpMost < 0)
                        {
                            for (int i = 0; i < 4; i++)
                            {
                                tetra.block[i].y -= newUpMost;
                            }
                        }

                        bool toLeft = 0;
                        for (int i = 0; i < 4; i++)
                        {
                            if (newLeftMost == tetra.block[i].x)
                            {
                                if (!isValidPoint(tetra.block[i], boardStates))
                                    toLeft = 1;
                            }
                        }

                        if (toLeft)
                        {
                            int move = 0;
                            for (int i = 0; i < 4; i++)
                            {
                                point p = point(tetra.block[i].y, tetra.block[i].x);
                                if (!isValidPoint(tetra.block[i], boardStates))
                                {
                                    move = std::max(move, p.y - newLeftMost + 1);
                                }
                            }

                            for (int i = 0; i < 4; i++)
                            {
                                tetra.block[i].x += move;
                            }
                        }
                        else
                        {
                            int move = 0;
                            for (int i = 0; i < 4; i++)
                            {
                                point p = point(tetra.block[i].y, tetra.block[i].x);
                                if (!isValidPoint(tetra.block[i], boardStates))
                                {
                                    move = std::max(move, newRightMost - p.y + 1);
                                }
                            }
                            for (int i = 0; i < 4; i++)
                            {
                                tetra.block[i].x -= move;
                            }
                        }
                    }
                }

                // hard drop
                if (hardDrop)
                {
                    hardDropSound.play();

                    // The tetromino is instantly slam to the ground
                    // We do that by moving it down vertically with no delay, until it hit the ground
                    int dist = 0;
                    while (isValidPotision(tetra, boardStates))
                    {
                        prev = tetra;
                        for (int i = 0; i < 4; i++)
                        {
                            tetra.block[i].y++;
                        }
                        dist++;
                    }
                    score += dist * 2;
                    hardDrop = 0;
                }

                // hold system
                if (hold)
                {
                    holdSound.play();

                    if (heldTetromino == -1)
                    {
                        // if there is currently no held tetromino, the current one will be held, and we will get the next one in the bag
                        heldTetromino = tetra.color;
                        if (played % 7 == 0)
                            std::random_shuffle(bag.begin(), bag.end(), myrandom);
                        if (isPlaying)
                            tetra = getTetromino(bag[played % 7]);
                        played++;
                    }
                    else
                    {
                        // Otherwise, we will just swap the current and the held tetromino
                        std::swap(tetra.color, heldTetromino);
                        tetra = getTetromino(tetra.color);
                    }
                    hold = 0;
                }

                // Gravity
                if (timer > delay)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        tetra.block[i].y++;
                    }
                    timer = 0;
                    if (softDrop)
                    {
                        score++;
                    }
                }

                // If the tetromino met the "ground", create another
                if (!isValidPotision(tetra, boardStates))
                {
                    tetra = prev;
                    isHeld = 0;

                    // Update the game's state
                    if (isPlaying)
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            boardStates[tetra.block[i].y][tetra.block[i].x] = tetra.color + 1;
                        }
                    }

                    // get a random Tetromino
                    if (played % 7 == 0)
                        std::random_shuffle(bag.begin(), bag.end(), myrandom);
                    if (isPlaying)
                        tetra = getTetromino(bag[played % 7]);
                    played++;

                    int lineCleared = clearLines(boardStates);

                    line += lineCleared;
                    score += getScore(lineCleared, level);
                    isPlaying = !isEnd(boardStates);

                    // it take 10 line to level up once, therefore
                    level = 1 + line / 10;

                    // level up
                    DEFAULT_DELAY = powd((0.8 - ((level - 1) * 0.007)), level - 1);
                }
            }
        }
        else
        {
            music.pause();
        }
        window.clear(sf::Color::White);

        // Draw the background
        window.draw(background);

        // Game over
        if (isEnd(boardStates))
        {
            sf::Text endText = TextSetup(font, 75, sf::Color::Red, "GAME OVER!");
            endText.move(15, 250);
            window.draw(endText);
        }
        else
        {
            // Draw the score
            std::string scoreString = intToStringFilled(score);
            sf::Text scoreText = TextSetup(font, 25, sf::Color::Black, scoreString);
            scoreText.move(372, 310);
            window.draw(scoreText);

            // Draw the level
            std::string levelString = intToStringFilled(level);
            sf::Text levelText = TextSetup(font, 25, sf::Color::Blue, levelString);
            levelText.move(372, 390);
            window.draw(levelText);

            // Draw the line
            std::string lineString = intToStringFilled(line);
            sf::Text lineText = TextSetup(font, 25, sf::Color::Green, lineString);
            lineText.move(372, 470);
            window.draw(lineText);

            // Draw the countdown
            if (countdown && isPlaying)
            {
                std::string cdString = intToString(countdown);
                sf::Text cdText = TextSetup(font, 150, sf::Color::Red, cdString);
                cdText.move(210, 200);
                window.draw(cdText);
            }

            // Draw the paused
            if (!isPlaying)
            {
                window.draw(pauseScreen);
                if (isHelp)
                {
                    window.draw(helpScreen);
                }
            }

            if (gameStarted)
            {
                // Draw the current blocks
                for (int i = 0; i < 20; i++)
                {
                    for (int j = 0; j < 10; j++)
                    {
                        if (boardStates[i][j])
                        {
                            sprite.setTextureRect(sf::IntRect((boardStates[i][j] - 1) * BLOCK_SIZE, 0, BLOCK_SIZE, BLOCK_SIZE));
                            sprite.setPosition(j * BLOCK_SIZE, i * BLOCK_SIZE);
                            sprite.move(50, 50);
                            window.draw(sprite);
                        }
                    }
                }

                // Draw the tetromino in "hold" section
                if (heldTetromino != -1)
                {
                    Tetromino holded = getTetromino(heldTetromino);
                    for (int i = 0; i < 4; i++)
                    {
                        sprite.setTextureRect(sf::IntRect(holded.color * BLOCK_SIZE, 0, BLOCK_SIZE, BLOCK_SIZE));
                        sprite.setPosition(holded.block[i].x * BLOCK_SIZE, holded.block[i].y * BLOCK_SIZE);
                        if (holded.color == 0)
                            sprite.move(330, 100);
                        else if (holded.color == 1)
                            sprite.move(305, 100);
                        else
                            sprite.move(317, 100);
                        window.draw(sprite);
                    }
                }

                // Draw the falling blocks
                for (int i = 0; i < 4; i++)
                {
                    sprite.setTextureRect(sf::IntRect(tetra.color * BLOCK_SIZE, 0, BLOCK_SIZE, BLOCK_SIZE));
                    sprite.setPosition(tetra.block[i].x * BLOCK_SIZE, tetra.block[i].y * BLOCK_SIZE);
                    sprite.move(50, 50);
                    window.draw(sprite);
                }
            }
        }
        window.display();
    }

    return 0;
}