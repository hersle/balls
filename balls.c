#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

/* TODO: clean up */
/* TODO: pause and resume */
/* TODO: draw half-transparent when spawning ball */
/* TODO: use scrollwheel for velocity multiplier */
/* TODO: 2 gravity modes */
/* TODO: explosions */

#define MAX_BALLS 1000

enum mouse_mode {SELECT_CENTER, SELECT_RADIUS, SELECT_VELOCITY};
enum gravity_mode {GRAVITY_DISABLED, GRAVITY_VERTICAL, GRAVITY_UNIVERSAL};

struct ball {
    double x, y;
    double vel_x, vel_y;
    double acc_x, acc_y;
    double radius;
    double mass;
    GLubyte color_r, color_g, color_b;
};

double width, height;
int n_balls = 0;
struct ball balls[MAX_BALLS];
struct ball *spawning_ball = NULL;

double gravitational_constant = 1000.0;

enum mouse_mode mode = SELECT_CENTER;
enum gravity_mode mode_gravity = GRAVITY_DISABLED;

int explosions_enabled = 0;

void collide_balls(struct ball *b1, struct ball *b2, double dx, double dy)
{
    double dist = b1->radius + b2->radius;
    if (dx * dx + dy * dy < dist * dist) {
        double sp = dx * (b2->vel_x - b1->vel_x) + dy * (b2->vel_y - b1->vel_y);
        if (sp <= 0) {
            double k = sp / ((b1->mass + b2->mass) * dist * dist);
            b1->vel_x += 2 * b2->mass * k * dx;
            b1->vel_y += 2 * b2->mass * k * dy;
            b2->vel_x -= 2 * b1->mass * k * dx;
            b2->vel_y -= 2 * b1->mass * k * dy;

            /* TODO: move balls outside each other */
            double angle = atan2(dy, dx);
            b2->x = b1->x + (b1->radius + b2->radius) * cos(angle);
            b2->y = b1->y + (b1->radius + b2->radius) * sin(angle);
        }
    }
}

void move_balls(double secs)
{
    struct ball *b1, *b2;

    /*
    int i, j;
    for (i = 0; i < n_balls; i++) {
        b1 = &balls[i];
        b1->x += b1->vel_x * secs;
        b1->y += b1->vel_y * secs;
        
        // TODO: gravity
        b1->vel_x += b1->acc_x * secs;
        b1->vel_y += b1->acc_y * secs;
        // b1->vel_y += 1000.0 * secs;
    }
    */

    int i, j;

    for (i = 0; i < n_balls; i++) {
        b1 = &balls[i];
        b1->acc_x = 0.0;
        b1->acc_y = 0.0;
    }

    double k;
    double dx, dy, dist;

    for (i = 0; i < n_balls; i++) {
        b1 = &balls[i];
        if (mode_gravity == GRAVITY_VERTICAL) {
            b1->acc_y = 500.0;
        } else if (mode_gravity == GRAVITY_UNIVERSAL) {
            for (j = i + 1;  j < n_balls; j++) {
                b2 = &balls[j];
                dx = b2->x - b1->x;
                dy = b2->y - b1->y;
                dist = sqrt(dx * dx + dy * dy);
                k = gravitational_constant * b1->mass * b2->mass / (dist * dist * dist);
                b1->acc_x += k / b1->mass * dx;
                b1->acc_y += k / b1->mass * dy;
                b2->acc_x -= k / b2->mass * dx;
                b2->acc_y -= k / b2->mass * dy;
            }
        }
    }

    for (i = 0; i < n_balls; i++) {
        b1 = &balls[i];
        b1->vel_x += b1->acc_x * secs;
        b1->vel_y += b1->acc_y * secs;
        b1->x += b1->vel_x * secs;
        b1->y += b1->vel_y * secs;
    }

    for (i = 0; i < n_balls; i++) {
        b1 = &balls[i];
        for (j = i + 1; j < n_balls; j++) {
            b2 = &balls[j];
            dx = b2->x - b1->x;
            dy = b2->y - b1->y;
            collide_balls(b1, b2, dx, dy);
        }

        if (balls[i].x < balls[i].radius && balls[i].vel_x < 0)
            balls[i].vel_x *= -1;
        if (balls[i].y < balls[i].radius && balls[i].vel_y < 0)
            balls[i].vel_y *= -1;
        if (balls[i].x > width - balls[i].radius && balls[i].vel_x > 0)
            balls[i].vel_x *= -1;
        if (balls[i].y > height - balls[i].radius && balls[i].vel_y > 0)
            balls[i].vel_y *= -1;
    }
}

double random_double(double min, double max)
{
    return min + ((double) rand() / (double) RAND_MAX) * (max - min);
}

void display_ball(struct ball *b)
{
    /* TODO: flag to draw velocity vector */
    glPushMatrix();
    glTranslated(b->x, b->y, 0.0);
    glColor3ub(b->color_r, b->color_g, b->color_b);
    gluDisk(gluNewQuadric(), 0.0, b->radius, 64, 1);
    glPopMatrix();
}

void display(GLFWwindow *window)
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    int i;
    for (i = 0; i < n_balls; i++)
        display_ball(&balls[i]);

    if (spawning_ball) {
        display_ball(spawning_ball);
        glLineWidth(10.0);
        glColor3ub(255, 0, 0);
        if (mode == SELECT_VELOCITY) {
            glBegin(GL_LINES);
            glVertex2d(spawning_ball->x, spawning_ball->y);
            glVertex2d(spawning_ball->x + spawning_ball->vel_x, spawning_ball->y + spawning_ball->vel_y);
            glEnd();
        }
    }

    glfwSwapBuffers(window);
}

void spawn_random_ball()
{
    struct ball *b = &balls[n_balls];

    /* TODO: ... */
    b->x = random_double(0.0, width);
    b->y = random_double(0.0, height);
    b->vel_x = random_double(-500.0, 500.0);
    b->vel_y = random_double(-500.0, 500.0);
    b->radius = random_double(10.0, 100.0);
    b->mass = b->radius * b->radius;
    b->color_r = rand() % 256;
    b->color_g = rand() % 256;
    b->color_b = rand() % 256;

    n_balls++;
}

void resize_cb(GLFWwindow *window, int window_width, int window_height)
{
    width = (double) window_width;
    height = (double) window_height;

    glLoadIdentity();
    glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
    glViewport(0, 0, window_width, window_height);
}

void cursor_pos_cb_radius(GLFWwindow *window, double x, double y)
{
    double dx = x - spawning_ball->x;
    double dy = y - spawning_ball->y;
    spawning_ball->radius = sqrt(dx * dx + dy * dy);
}

void cursor_pos_cb_velocity(GLFWwindow *window, double x, double y)
{
    double dx = x - spawning_ball->x;
    double dy = y - spawning_ball->y;
    spawning_ball->vel_x = -dx;
    spawning_ball->vel_y = -dy;
}

void spawn_ball_finalize()
{
    n_balls++;
    spawning_ball = NULL;
}

void mouse_button_cb(GLFWwindow *window, int button, int action, int mods)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    if (mode == SELECT_CENTER && action == GLFW_PRESS) {
        if (explosions_enabled) {
            /* TODO: explosion */
            int i;
            struct ball *b;
            double dx, dy;
            double k;
            for (i = 0; i < n_balls; i++) {
                b = &balls[i];
                dx = b->x - x;
                dy = b->y - y;
                k = 1.0 / (dx * dx + dy * dy);
                b->vel_x += k * 100000.0 * dx;
                b->vel_y += k * 100000.0 * dy;
            }
        } else if (button == GLFW_MOUSE_BUTTON_LEFT) {
            spawning_ball = &balls[n_balls];
            spawning_ball->x = x;
            spawning_ball->y = y;
            spawning_ball->color_r = rand() % 256;
            spawning_ball->color_g = rand() % 256;
            spawning_ball->color_b = rand() % 256;
            spawning_ball->radius = 0.0;
            mode = SELECT_RADIUS;
            glfwSetCursorPosCallback(window, cursor_pos_cb_radius);
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            int i;
            struct ball *b;
            double dx, dy;
            for (i = 0; i < n_balls; i++) {
                b = &balls[i];
                dx = b->x - x;
                dy = b->y - y;
                if (dx * dx + dy * dy < b->radius * b->radius) {
                    balls[i] = balls[--n_balls];
                    i--;
                }
            }
        }
    } else if (mode == SELECT_RADIUS && action == GLFW_RELEASE) {
        spawning_ball->mass = spawning_ball->radius * spawning_ball->radius;
        mode = SELECT_VELOCITY;
        glfwSetCursorPosCallback(window, cursor_pos_cb_velocity);
        cursor_pos_cb_velocity(window, x, y);  /* make sure called */
    } else if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            spawning_ball = NULL;
            mode = SELECT_CENTER;
            glfwSetCursorPosCallback(window, NULL);
            n_balls++;
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            spawning_ball->vel_x = 0.0;
            spawning_ball->vel_y = 0.0;
            spawning_ball = NULL;
            mode = SELECT_CENTER;
            glfwSetCursorPosCallback(window, NULL);
            n_balls++;
        } else {
            return;
        }
    }
}

void key_cb(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_RELEASE) {
        if (key == GLFW_KEY_R) {
            n_balls = 0;
        } else if (key == GLFW_KEY_SPACE) {
            spawn_random_ball();
        } else if (key == GLFW_KEY_G) {
            mode_gravity = (mode_gravity + 1) % 3;
        } else if (key == GLFW_KEY_E) {
            explosions_enabled = !explosions_enabled;
        }
    }
}

int main()
{
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(640, 640, "balls", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resize_cb);
    glfwSetMouseButtonCallback(window, mouse_button_cb);
    glfwSetKeyCallback(window, key_cb);
	resize_cb(window, 640, 640);

    srand(time(NULL));

    double secs_last = glfwGetTime();
    double secs_now;
    double secs;

    while (!glfwWindowShouldClose(window)) {
        display(window);
        secs_now = glfwGetTime();
        secs = secs_now - secs_last;
        secs_last = secs_now;
        move_balls(secs);
        glfwPollEvents();
    }

    free(window);
    return 0;
}
