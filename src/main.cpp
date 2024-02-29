#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ostream>
#include <iostream>
#include <ctime>
#include <stddef.h>

using namespace std;

// Global variables for spaceship position and velocity
float shipX = 0.0f;
float shipY = 0.0f;
float shipSize = 0.1f;
float shipVelX = 0.0f;
float shipVelY = 0.0f;
float maxSpeed = 0.02f;
float acceleration = 0.001f;
float friction = 0.99f;

// Structure for asteroid
struct Asteroid {
    float x;
    float y;
    float size;
    float speedX;
    float speedY;
};

std::vector<Asteroid> asteroids;

// Function to initialize asteroids
void initAsteroids(int numAsteroids) {
    asteroids.clear();
    srand(time(NULL)); // Seed the random number generator

    for (int i = 0; i < numAsteroids; ++i) {
        Asteroid asteroid;
        asteroid.x = -1.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f); // Random x position between -1 and 1
        asteroid.y = 0.5f + static_cast<float>(rand()) / (RAND_MAX / 2.0f); // Random y position between 0.5 and 1
        asteroid.size = 0.05f + static_cast<float>(rand()) / (RAND_MAX / 0.1f); // Random size between 0.05 and 0.15
        asteroid.speedX = static_cast<float>(rand()) / RAND_MAX * 0.01f - 0.005f; // Random speed between -0.005 and 0.005 in x direction
        asteroid.speedY = static_cast<float>(rand()) / RAND_MAX * 0.01f - 0.005f; // Random speed between -0.005 and 0.005 in y direction
        asteroids.push_back(asteroid);
    }
}

// Function to check collision between two objects (bounding box collision)
bool checkCollision(float x1, float y1, float size1, float x2, float y2, float size2) {
    return (fabs(x1 - x2) * 2 < (size1 + size2)) && (fabs(y1 - y2) * 2 < (size1 + size2));
}

int lives = 3; // Number of lives

// Keyboard input function
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
    }
}

void updateShipPosition() {
    // Update spaceship position based on velocity
    shipX += shipVelX;
    shipY += shipVelY;

    // Check and correct if spaceship goes beyond the boundaries
    if (shipX + shipSize > 1.0f) {
        shipX = 1.0f - shipSize;
        shipVelX *= friction;
    } else if (shipX - shipSize < -1.0f) {
        shipX = -1.0f + shipSize;
        shipVelX *= friction;
    }

    if (shipY + shipSize > 1.0f) {
        shipY = 1.0f - shipSize;
        shipVelY *= friction;
    } else if (shipY - shipSize < -1.0f) {
        shipY = -1.0f + shipSize;
        shipVelY *= friction;
    }
}


void checkAsteroidCollision() {
    for (size_t i = 0; i < asteroids.size(); ++i) {
        for (size_t j = i + 1; j < asteroids.size(); ++j) {
            Asteroid& asteroid1 = asteroids[i];
            Asteroid& asteroid2 = asteroids[j];

            // Check if the boundaries of the two asteroids touch
            if (fabs(asteroid1.x - asteroid2.x) * 2 < (asteroid1.size + asteroid2.size) &&
                fabs(asteroid1.y - asteroid2.y) * 2 < (asteroid1.size + asteroid2.size)) {
                
                // Calculate new velocities based on conservation of momentum and kinetic energy

                // Calculate total momentum before collision in x and y directions
                float totalMomentumX = asteroid1.speedX + asteroid2.speedX;
                float totalMomentumY = asteroid1.speedY + asteroid2.speedY;

                // Calculate center of mass velocity in x and y directions
                float comVelocityX = totalMomentumX / 2.0f;
                float comVelocityY = totalMomentumY / 2.0f;

                // Calculate relative velocity in x and y directions
                float relVelX1 = asteroid1.speedX - comVelocityX;
                float relVelY1 = asteroid1.speedY - comVelocityY;
                float relVelX2 = asteroid2.speedX - comVelocityX;
                float relVelY2 = asteroid2.speedY - comVelocityY;

                // Calculate new velocities after collision using conservation of momentum and kinetic energy
                asteroid1.speedX = comVelocityX + relVelX2;
                asteroid1.speedY = comVelocityY + relVelY2;
                asteroid2.speedX = comVelocityX + relVelX1;
                asteroid2.speedY = comVelocityY + relVelY1;
            }
        }
    }
}



// Render function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0); // Set fixed view window

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Render asteroids
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for asteroids
    for (auto& asteroid : asteroids) {
        asteroid.x += asteroid.speedX;
        asteroid.y += asteroid.speedY;

        // Boundary checking for asteroids
        if (asteroid.x < -1.0f || asteroid.x > 1.0f)
            asteroid.speedX = -asteroid.speedX;
        if (asteroid.y < -1.0f || asteroid.y > 1.0f)
            asteroid.speedY = -asteroid.speedY;

        glPushMatrix();
        glTranslatef(asteroid.x, asteroid.y, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(-asteroid.size, asteroid.size);
        glVertex2f(-asteroid.size, -asteroid.size);
        glVertex2f(asteroid.size, -asteroid.size);
        glVertex2f(asteroid.size, asteroid.size);
        glEnd();
        glPopMatrix();

        // Check collision with spaceship
        if (checkCollision(shipX, shipY, shipSize, asteroid.x, asteroid.y, asteroid.size)) {
            // Collision detected with spaceship
            // Reduce life
            lives--;
            cout << "Lives Remaining: " << lives << endl;

            // If no lives left, game over
            if (lives <= 0) {
                cout << "Game Over!" << endl;
                exit(0);
            } else {
                // Reset spaceship position
                shipX = 0.0f;
                shipY = 0.0f;
            }
        }

    }
    checkAsteroidCollision();

    updateShipPosition(); // Update ship position

    // Render spaceship
    glPushMatrix();
    glTranslatef(shipX, shipY, 0.0f);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(0.0f, 0.1f);      // Top
    glVertex2f(-0.1f, -0.1f);    // Bottom left
    glVertex2f(0.1f, -0.1f);     // Bottom right
    glEnd();
    glPopMatrix();

    // Render lives remaining
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(-0.9f, 0.9f, 0.0f); // Position lives text
    glColor3f(1.0f, 1.0f, 1.0f); // White color for text
    string livesText = "Lives: " + to_string(lives);
    for (char& c : livesText) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }
    glPopMatrix();

    glutSwapBuffers();
    glutPostRedisplay(); // Request redisplay to continue rendering
}


// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Space Dodge Game");

    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    initAsteroids(10); // Initialize 10 asteroids

    glutMainLoop();
    return 0;
}
