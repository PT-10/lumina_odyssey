#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ostream>
#include <iostream>
#include <ctime>
#include <unistd.h>
using namespace std;


//define custom datatype GameState
enum GameState {
    PLAYING,
    GAME_OVER,
    GAME_WON
};

//Asteroid structure
struct Asteroid {
    float x;
    float y;
    float size;
    float speedX;
    float speedY;
};

//PowerUp structure
struct PowerUp {
    float x;
    float y;
    float size;
    time_t spawnTime;
    bool pickedUp;
};

//initial value
GameState gameState = PLAYING;

//initialize movement variables
float shipX = 0.0f;
float shipY = 0.0f;
float shipSize = 0.1f;
float shipVelX = 0.0f;
float shipVelY = 0.0f;
float maxSpeed = 0.02f;
float acceleration = 0.001f;
float friction = 0.99f;
static float stationX = 0.0f;
static float stationY = 0.0f;

//initialize game stat variables
int level = 1;
int score = 0;
int health = 100;
int numAsteroids = 25;
int asteroidsDodged = 0;
int levelDuration = 15;
float scoreMultiplier = 1.0f;
float healthBarWidth = 0.4f;
const int powerUpDuration = 10;
const int immunityDuration = 10;
bool powerUpSpawned = false;
static bool initialized = false;

//initialize time variables
time_t levelStartTime;
time_t gameStartTime;
time_t powerUpStartTime = 0;
time_t immunityStartTime = 0;

//Vector of datatype asteroids to store the generated asteroids
std::vector<Asteroid> asteroids;
void initAsteroids(int numAsteroids) {
    asteroids.clear();
    srand(time(NULL));
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    
    for (int i = 0; i < numAsteroids; ++i) {
        Asteroid asteroid;
        asteroid.size = 0.05f + static_cast<float>(rand()) / (RAND_MAX / 0.1f);
        asteroid.x = -1.0f + static_cast<float>(rand()) / (RAND_MAX / (2.0f - 2 * asteroid.size));
        asteroid.y = 3.5*(-1.0f + static_cast<float>(rand()) / (RAND_MAX / (2.0f - 2 * asteroid.size)));
        
        // Increase asteroid speed based on the current level
        float levelMultiplier = 1.0f + (level - 1) * 0.1f; // Adjust the multiplier as needed
        asteroid.speedX = levelMultiplier * (static_cast<float>(rand()) / RAND_MAX * 0.01f - 0.005f);
        asteroid.speedY = levelMultiplier * (static_cast<float>(rand()) / RAND_MAX * 0.01f - 0.005f);

        asteroids.push_back(asteroid);
    }
}

//Boolean function to check collision between two objects. Primarily used to check collision between objects of different types
bool checkCollision(float x1, float y1, float size1, float x2, float y2, float size2) {
    return (abs(x1 - x2) * 2 < (size1 + size2)) && (abs(y1 - y2) * 2 < (size1 + size2));
}

//Function to check collision between any two asteroids
void checkAsteroidCollision() {
    for (size_t i = 0; i < asteroids.size(); ++i) {
        for (size_t j = i + 1; j < asteroids.size(); ++j) {
            Asteroid& asteroid1 = asteroids[i];
            Asteroid& asteroid2 = asteroids[j];

            if (fabs(asteroid1.x - asteroid2.x) * 2 < (asteroid1.size + asteroid2.size) &&
                fabs(asteroid1.y - asteroid2.y) * 2 < (asteroid1.size + asteroid2.size)) {
                
                float totalMomentumX = asteroid1.speedX + asteroid2.speedX;
                float totalMomentumY = asteroid1.speedY + asteroid2.speedY;
                float comVelocityX = totalMomentumX / 2.0f;
                float comVelocityY = totalMomentumY / 2.0f;
                float relVelX1 = asteroid1.speedX - comVelocityX;
                float relVelY1 = asteroid1.speedY - comVelocityY;
                float relVelX2 = asteroid2.speedX - comVelocityX;
                float relVelY2 = asteroid2.speedY - comVelocityY;

                asteroid1.speedX = comVelocityX + relVelX2;
                asteroid1.speedY = comVelocityY + relVelY2;
                asteroid2.speedX = comVelocityX + relVelX1;
                asteroid2.speedY = comVelocityY + relVelY1;
            }
        }
    }
}

//Vector of datatype powerups to store the randomly spawned powerups, however only one is displayed at a time
std::vector<PowerUp> powerUps;
void spawnPowerUp() {
    if (!powerUpSpawned) {
        PowerUp powerUp;
        powerUp.size = 0.05f;
        powerUp.x = -1.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f);
        powerUp.y = -1.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f);
        powerUp.spawnTime = time(NULL);
        powerUp.pickedUp = false;

        powerUps.clear(); // Remove previous power-up
        powerUps.push_back(powerUp);
        powerUpSpawned = true;
    }
}

//Function to render powerups
void renderPowerUps() {
    glColor3f(1.0f, 1.0f, 0.0f);
    for (const auto& powerUp : powerUps) {
        if (!powerUp.pickedUp) {
            glPushMatrix();
            glTranslatef(powerUp.x, powerUp.y, 0.0f);
            glBegin(GL_TRIANGLES);
            glVertex2f(0.0f, 0.05f); // Top vertex
            glVertex2f(-0.03f, -0.05f); // Bottom left vertex
            glVertex2f(0.03f, -0.05f); // Bottom right vertex
            glVertex2f(0.0f, 0.05f); // Top vertex
            glVertex2f(0.05f, 0.0f); // Right vertex
            glVertex2f(-0.05f, 0.0f); // Left vertex
            glEnd();
            glPopMatrix();
        }
    }
}

void renderPowerUpRing(){
    glColor3f(1.0f, 1.0f, 1.0f); // White color
    glPushMatrix();
    glTranslatef(shipX, shipY, 0.0f);
    glBegin(GL_LINE_LOOP);
    const int numSegments = 100;
    const float radius = 0.12f;
    for (int i = 0; i < numSegments; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(numSegments);
        float x = radius * cosf(theta);
        float y = radius * sinf(theta);
        glVertex2f(x, y);
    }
    glEnd();
    glPopMatrix();
}

//Boolean function to check if spaceship is immune to collision
bool isImmune() {
    return (immunityStartTime != 0 && difftime(time(NULL), immunityStartTime) < immunityDuration);
}

//Function to handle powerup
void handlePowerUp() {
    if (powerUpStartTime != 0 && difftime(time(NULL), powerUpStartTime) >= powerUpDuration) {
        powerUpStartTime = 0;
        // Check if health is less than 100 before increasing it
        if (health < 100) {
            health += 5; // Increase health by +5
            if (health > 100) {
                health = 100; // Ensure health doesn't exceed 100
            }
        }
    }

    // Check if immunity power-up is active and handle its duration
    if (immunityStartTime != 0 && difftime(time(NULL), immunityStartTime) >= immunityDuration) {
        immunityStartTime = 0;
    }
}

void spawnPowerUpPeriodically(int value) {
    if (gameState == PLAYING) {
        spawnPowerUp();
        glutTimerFunc(5000, spawnPowerUpPeriodically, 0);
    }
}

void updateShipPosition() {
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    shipX += shipVelX;
    shipY += shipVelY;

    float boundaryX =  aspectRatio;
    float boundaryY = 1.0f - shipSize / aspectRatio;

    if (shipX > boundaryX) {
        shipX = boundaryX;
        shipVelX *= friction;
    } else if (shipX < -boundaryX) {
        shipX = -boundaryX;
        shipVelX *= friction;
    }

    if (shipY > 3*boundaryY) {
        shipY = 3*boundaryY;
        shipVelY *= friction;
    } else if (shipY < -boundaryY) {
        shipY = -boundaryY;
        shipVelY *= friction;
    }
}

void renderSpaceShip(){
    glPushMatrix();
    glTranslatef(shipX, shipY, 0.0f);
    float elongationFactor = 1.0f + fabs(shipVelY) * 10.0f;

    glBegin(GL_TRIANGLES);
    // Main body of the 
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(0.0f, 0.1f);
    glVertex2f(-0.05f * elongationFactor, -0.1f);
    glVertex2f(0.05f * elongationFactor, -0.1f);

    // Left wing
    glColor3f(1.0f, 0.0f, 0.0f); // Red color
    glVertex2f(-0.05f * elongationFactor, -0.05f);
    glVertex2f(-0.1f * elongationFactor, -0.1f);
    glVertex2f(-0.05f * elongationFactor, -0.1f);

    // Right wing
    glColor3f(1.0f, 0.0f, 0.0f); // Red color
    glVertex2f(0.05f * elongationFactor, -0.05f);
    glVertex2f(0.1f * elongationFactor, -0.1f);
    glVertex2f(0.05f * elongationFactor, -0.1f);
    glEnd();
    glPopMatrix();

}

void renderSpaceStation() {
    if (level > 2) {
        if (!initialized) {
            int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
            int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
            float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
            float boundaryY = 1.0f - shipSize / aspectRatio;

            // Random position along the X-axis within the specified Y range
            stationX = -1.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f);
            // Y position within the specified range
            stationY = 2.0f * boundaryY + static_cast<float>(rand()) / (RAND_MAX / (boundaryY));

            initialized = true;
        }

        // Station color: Yellow and blue
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow
    glPushMatrix();

    glTranslatef(stationX, stationY, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-0.1f, 0.1f); // Top left vertex
    glVertex2f(0.1f, 0.1f);  // Top right vertex
    glVertex2f(0.1f, -0.1f); // Bottom right vertex
    glVertex2f(-0.1f, -0.1f); // Bottom left vertex
    glEnd();

    // Antennas
    glColor3f(0.0f, 0.0f, 1.0f); // Blue
    glBegin(GL_LINES);
    glVertex2f(0.0f, 0.1f); // Top center
    glVertex2f(0.0f, 0.2f); // Antenna top

    glVertex2f(-0.05f, 0.05f); // Left top
    glVertex2f(-0.05f, 0.15f); // Antenna left top

    glVertex2f(0.05f, 0.05f); // Right top
    glVertex2f(0.05f, 0.15f); // Antenna right top
    glEnd();

    // Satellite dish
    glColor3f(0.5f, 0.5f, 0.5f); // Gray
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; ++i) {
        float angle = static_cast<float>(i) * 3.14159f / 180.0f;
        float x = 0.05f * cos(angle);
        float y = 0.05f * sin(angle) + 0.05f;
        glVertex2f(x, y);
    }
    glEnd();

    //Window
    glColor3f(0.8f, 0.8f, 0.8f); // Light gray
    glBegin(GL_QUADS);
    // Left window
    glVertex2f(-0.08f, 0.02f);
    glVertex2f(-0.05f, 0.02f);
    glVertex2f(-0.05f, -0.02f);
    glVertex2f(-0.08f, -0.02f);

    // Right window
    glVertex2f(0.08f, 0.02f);
    glVertex2f(0.05f, 0.02f);
    glVertex2f(0.05f, -0.02f);
    glVertex2f(0.08f, -0.02f);
    glEnd();
    
    glPopMatrix();
    }
}

void resetGameForNextLevel() {
    shipX = 0.0f;
    shipY = 0.0f;
    shipVelX = 0.0f;
    shipVelY = 0.0f;
    initAsteroids(numAsteroids);
    levelStartTime = time(NULL);
}

void checkLevelTransition() {
    time_t currentTime = time(NULL);
    int elapsedTime = difftime(currentTime, levelStartTime);

    if (level == 1 || level == 2) {
        // The first two levels are time-bound
        if (elapsedTime >= levelDuration) {
            level++;
            numAsteroids += 5;
            resetGameForNextLevel();
        }
    } else {
        // Third level: Space station appears, no time limit
        if (gameState == PLAYING) {
            // Check if the player reached the space station
            float distance = sqrt(pow(shipX - stationX, 2) + pow(shipY - stationY, 2));
            if (distance < 0.2) {
                gameState = GAME_WON; // Change game state to GAME_WON
            }
        }
    }

    if (level > 3) {
        gameState = GAME_WON;
    }
}

void renderHealthBar() {
    float barHeight = 0.05f;
    float barPadding = 0.01f;
    float barPositionX = 1.0f - healthBarWidth - barPadding;
    float barPositionY = 1.0f - barHeight - barPadding;

    glColor4f(0.2f, 0.2f, 0.2f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(barPositionX, barPositionY);
    glVertex2f(barPositionX + healthBarWidth, barPositionY);
    glVertex2f(barPositionX + healthBarWidth, barPositionY + barHeight);
    glVertex2f(barPositionX, barPositionY + barHeight);
    glEnd();

    glColor4f(0.0f, 1.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(barPositionX, barPositionY);
    glVertex2f(barPositionX + healthBarWidth * (health / 100.0f), barPositionY);
    glVertex2f(barPositionX + healthBarWidth * (health / 100.0f), barPositionY + barHeight);
    glVertex2f(barPositionX, barPositionY + barHeight);
    glEnd();
}

void renderMiniMap(){
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    // Draw minimap
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(-aspectRatio + 0.25f, -0.75f, 0.0f); // Adjust translation to bottom left corner
    glColor4f(0.2f, 0.2f, 0.2f, 0.5f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; ++i) {
        float angle = static_cast<float>(i) * 3.14159f / 180.0f;
        float x = 0.23f * cos(angle);
        float y = 0.23f * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();

    // Draw spaceship position on minimap
    glPushMatrix();
    glTranslatef((shipX) * 0.1f, (shipY-0.75) * 0.1f, 0.0f); // Scale and adjust position
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for spaceship
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; ++i) {
        float angle = static_cast<float>(i) * 3.14159f / 180.0f;
        float x = 0.015f * cos(angle); // Scale the dot size
        float y = 0.015f * sin(angle); // Scale the dot size
        glVertex2f(x, y);
    }
    glEnd();
    glPopMatrix();

    // Draw space station position on minimap for level 3
    if (level == 3) {
        glPushMatrix();
        glTranslatef((stationX) * 0.1f, (stationY-0.75) * 0.1f, 0.0f); // Scale and adjust position
        glColor3f(0.0f, 0.0f, 1.0f); // Blue color for space station
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; ++i) {
            float angle = static_cast<float>(i) * 3.14159f / 180.0f;
            float x = 0.015f * cos(angle); // Scale the dot size
            float y = 0.015f * sin(angle); // Scale the dot size
            glVertex2f(x, y);
        }
        glEnd();
        glPopMatrix();
    }
    glPopMatrix();
}

void keyboard(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            shipVelX -= acceleration;
            if (shipVelX < -maxSpeed)
                shipVelX = -maxSpeed;
            break;
        case GLUT_KEY_RIGHT:
            shipVelX += acceleration;
            if (shipVelX > maxSpeed)
                shipVelX = maxSpeed;
            break;
        case GLUT_KEY_UP:
            shipVelY += acceleration;
            if (shipVelY > maxSpeed)
                shipVelY = maxSpeed;
            break;
        case GLUT_KEY_DOWN:
            shipVelY -= acceleration;
            if (shipVelY < -maxSpeed)
                shipVelY = -maxSpeed;
            break;

        case GLUT_KEY_F1:
            // Restart the game
            if (gameState == GAME_OVER || gameState == GAME_WON) {
                // Reset all game parameters
                gameState = PLAYING;
                score = 0;
                health = 100;
                level = 1;
                numAsteroids = 25;
                asteroidsDodged = 0;
                resetGameForNextLevel();
                gameStartTime = time(NULL);
            }
            break;
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (aspectRatio > 1.0f) {
        glViewport(0, 0, windowWidth, windowHeight);
        gluOrtho2D(-1.0 * aspectRatio, 1.0 * aspectRatio, -1.0, 1.0);
    } else {
        glViewport(0, 0, windowWidth, windowHeight);
        gluOrtho2D(-1.0, 1.0, -1.0 / aspectRatio, 1.0 / aspectRatio);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Translate the world based on ship position within the bounds
    float boundaryY = 1.0f - shipSize / aspectRatio; // Define the upper boundary
    float cameraY = -shipY; // Initial camera Y position

    // Ensure camera position respects the upper boundary
    if (cameraY < -2*boundaryY)
        cameraY = -2*boundaryY;
    if (cameraY > 0.0)
        cameraY = 0.0;

    time_t currentTime = time(NULL);
    int elapsedTime = difftime(currentTime, gameStartTime);

    if (gameState == PLAYING) {
        // Update score based on asteroids dodged and asteroid proximity
        for (auto& asteroid : asteroids) {
            float distance = sqrt(pow(shipX - asteroid.x, 2) + pow(shipY - asteroid.y, 2));
            if (distance < 0.3) { // Adjust proximity threshold as needed
                // Increase score based on asteroid size
                score += static_cast<int>(20 * asteroid.size); // Adjust multiplier as needed
                // Increment asteroids dodged count
                asteroidsDodged++;
            }
        }

        // Apply translation with limited camera position
        glTranslatef(0.0f, cameraY, 0.0f);
        glColor3f(0.5f, 0.5f, 0.5f);
        for (auto& asteroid : asteroids) {
            asteroid.x += asteroid.speedX;
            asteroid.y += asteroid.speedY;

            if (asteroid.x + asteroid.size > aspectRatio || asteroid.x - asteroid.size < -aspectRatio)
                asteroid.speedX = -asteroid.speedX;
            if (asteroid.y + asteroid.size > 3*1.0f || asteroid.y - asteroid.size < -1.0f)
                asteroid.speedY = -asteroid.speedY;

            glPushMatrix();
            glTranslatef(asteroid.x, asteroid.y, 0.0f);
            glBegin(GL_POLYGON);
            // Generate random number of vertices for each asteroid
            int numVertices = rand() % 5 + 5; // Random number between 5 and 10
            for (int i = 0; i < numVertices; ++i) {
                // Generate random angles for each vertex
                float angle = static_cast<float>(i) * 2 * M_PI / numVertices;
                // Generate random radius for each vertex
                float radius = asteroid.size * (0.5f + static_cast<float>(rand()) / RAND_MAX * 0.5f);
                // Calculate vertex position
                float x = radius * cos(angle);
                float y = radius * sin(angle);
                // Draw the vertex
                glVertex2f(x, y);
            }

            glEnd();
            glPopMatrix();

            if (!isImmune() && checkCollision(shipX, shipY, shipSize, asteroid.x, asteroid.y, asteroid.size)) {
            health -= static_cast<int>(10 * asteroid.size);
            cout << "Health: " << health << endl;

            if (health <= 0) {
                cout << "Game Over! Press F1 to restart" << endl;
                gameState = GAME_OVER;
            }
            float jerkMagnitude = 0.02f;
            shipX += jerkMagnitude * (shipX < asteroid.x ? -1 : 1);
            shipY += jerkMagnitude * (shipY < asteroid.y ? -1 : 1);
            }
        }

        renderPowerUps();
        handlePowerUp();

        for (auto& powerUp : powerUps) {
            if (!powerUp.pickedUp && checkCollision(shipX, shipY, shipSize, powerUp.x, powerUp.y, powerUp.size)) {
                powerUp.pickedUp = true;
                powerUpStartTime = time(NULL);
                immunityStartTime = time(NULL);
                powerUpSpawned = false; // Reset power-up spawn flag
            }
        }

        // Draw immunity ring if the player is immune
        if (difftime(time(NULL), immunityStartTime) < immunityDuration) {
            renderPowerUpRing();
        }

        renderSpaceShip();
        checkAsteroidCollision();
        checkLevelTransition();
        updateShipPosition();
        renderSpaceStation();
        renderMiniMap();

        // Draw health bar without translation
        glLoadIdentity(); // Reset transformation matrix
        renderHealthBar();
        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(-0.9f, 0.9f);
        string scoreText = "Score: " + to_string(score);
        for (char c : scoreText) {
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
        }
    }   

        else if (gameState == GAME_OVER) {
            glColor3f(1.0f, 0.0f, 0.0f);
            glRasterPos2f(-0.5f, 0.0f);
            string gameOverText = "Game Over!";
            for (char c : gameOverText) {
                glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
            }
            // Display final score including remaining health as bonus
            int finalScore = score + health;
            glColor3f(1.0f, 1.0f, 1.0f);
            glRasterPos2f(-0.5f, -0.1f);
            string finalScoreText = "Final Score: " + to_string(finalScore);
            for (char c : finalScoreText) {
                glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
            }
            glColor3f(0.0f, 1.0f, 0.0f);
            glRasterPos2f(-0.5f, -0.2f);
            string gameWonText2 = "Press F1 to restart";
            for (char c : gameWonText2) {
                glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
            }
    } 
        else if (gameState == GAME_WON) {
            glColor3f(0.0f, 1.0f, 0.0f);
            glRasterPos2f(-0.5f, 0.0f);
            string gameWonText = "You Won!";
            for (char c : gameWonText) {
                glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
            }
            int finalScore = score + health;
            glColor3f(1.0f, 1.0f, 1.0f);
            glRasterPos2f(-0.5f, -0.1f);
            string finalScoreText = "Final Score: " + to_string(finalScore);
            for (char c : finalScoreText) {
                glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
            }
            glColor3f(0.0f, 1.0f, 0.0f);
            glRasterPos2f(-0.5f, -0.2f);
            string gameWonText2 = "Press F1 to restart";
            for (char c : gameWonText2) {
                glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
            }
    }

    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Lumina Odyssey");
    glutTimerFunc(5000, spawnPowerUpPeriodically, 0);
    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gameStartTime = time(NULL);
    resetGameForNextLevel();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glutMainLoop();
    return 0;
}
