/*
    Project: Tetris
    Author: Kanaluvu
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
    - Render game over 

    * "Audio"
    - Background music
    - Sound effects

    To-do:
    - Factorize by file*

*/

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <time.h>
#include <bits/stdc++.h>

#include "operation.h"

// main
int main()
{
    std::srand(std::time(NULL)); // Random seed

    // Graphics setup
    sf::RenderWindow window(sf::VideoMode(550, 600), "Kurisu"); // Create a window
    
    // Get the texture ready
    sf::Texture texture;
    texture.loadFromFile("textures/Tetromino.png");

    sf::Texture backgroundTextures;
    backgroundTextures.loadFromFile("textures/Background.jpg");

    sf::Texture pauseScreenTextures;
    pauseScreenTextures.loadFromFile("textures/Pause.png");

    sf::Texture helpScreenTextures;
    helpScreenTextures.loadFromFile("textures/Help.png");

    sf::Texture endScreenTextures;
    endScreenTextures.loadFromFile("textures/GameOver.png");

    sf::Texture musicButtonTextures;
    musicButtonTextures.loadFromFile("textures/speaker.png");

    sf::Texture soundEffectButtonTextures;
    soundEffectButtonTextures.loadFromFile("textures/musicnote.png");

    const int BLOCK_SIZE = 25;
    sf::Sprite sprite, background;
    sprite.setTexture(texture);

    background.setTexture(backgroundTextures);

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

    bool isBGM = 1, isSFX = 1;
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

            // mouse action
            case sf::Event::MouseButtonPressed:
            {
                // get mouse potision
                sf::Vector2i p = sf::Mouse::getPosition(window);
                point mousePotision(p.x, p.y);
                if (!isPlaying)
                {
                    if (!isHelp)
                    {
                        if (isInside(mousePotision, point(171, 217), point(373, 273)))
                        {
                            // if this is the game over screen, do some reset
                            if (isEnd(boardStates))
                            {
                                // board reset
                                boardStates = board(ROWS, std::vector<int>(COLUMN, 0));

                                // states reset
                                hold = 0, isHeld = 0, heldTetromino = -1;
                                gameStarted = 0;

                                // score, lines, level reset
                                score = 0, level = 1, line = 0;

                                // music reset
                                music.stop();
                            }
                            // otherwise, just keep playing
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
                        if (isInside(mousePotision, point(441, 134), point(491, 184)))
                        {
                            isHelp = 0;
                        }
                    }
                }
                else 
                {
                    if (isInside(mousePotision,point(375,200), point(425,250)))
                    {
                        isBGM ^= 1;
                        if (isBGM) music.play();
                        else music.pause();
                    }
                    if (isInside(mousePotision,point(440,200), point(490,250)))
                    {
                        isSFX ^= 1;
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
                    if (isBGM)
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
                    {
                        if (isSFX)
                            movementSound.play();
                    }
                    tetra.block[i].x += dx;
                }
                if (!isValidPotision(tetra, boardStates))
                    tetra = prev;

                // Rotation
                if (rotate_cw)
                {
                    if (isSFX)
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
                    if (isSFX)
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
                    if (isSFX)
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
                    if (isSFX)
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
            // Draw the end
            sf::Sprite endScreen;
            endScreen.setTexture(endScreenTextures);
            endScreen.move(146, 96);
            window.draw(endScreen);
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

            // Draw the buttons

            sf::Sprite musicButton;
            musicButton.setTexture(musicButtonTextures);
            musicButton.move(375,200);
            if (!isBGM) musicButton.setColor(sf::Color::Red);
            window.draw(musicButton);

            sf::Sprite soundEffectButton;
            soundEffectButton.setTexture(soundEffectButtonTextures);
            soundEffectButton.move(440,200);
            if (!isSFX) soundEffectButton.setColor(sf::Color::Red);
            window.draw(soundEffectButton);

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
                sf::Sprite pauseScreen;
                pauseScreen.setTexture(pauseScreenTextures);
                pauseScreen.move(146, 96);
                window.draw(pauseScreen);
                if (isHelp)
                {
                    sf::Sprite helpScreen;
                    helpScreen.setTexture(helpScreenTextures);
                    helpScreen.move(53, 134);
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