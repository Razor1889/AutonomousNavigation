#include <Servo.h>

const int GRID_SIZE = 10;
const int SERVO_PIN = 9;
const int receiver_pin_CH1 = 2;  // Joystick on channel 1

int posX = 0, posY = 0;
int dirX = 0, dirY = 1;
Servo steeringServo;
char grid[GRID_SIZE][GRID_SIZE];

unsigned long pulse_duration_CH1;
bool remoteToggle = false;  // Toggle: true = remote mode, false = grid-based mode
int last_angle = 90;
int future_angle = 90;

void resetGrid() {
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            grid[y][x] = '.';
        }
    }
    grid[posY][posX] = 'X';  // Place boat at its current position
}

void displayGrid() {
    Serial.println("\n--------------------");
    Serial.println("      N");
    Serial.println("  W       E");
    Serial.println("      S");
    Serial.println("--------------------");
    Serial.println("\nGrid (10x10):");

    // Print X-axis labels
    Serial.print("   ");
    for (int x = 0; x < GRID_SIZE; x++) {
        Serial.print(x);
        Serial.print(" ");
    }
    Serial.println();

    for (int y = GRID_SIZE - 1; y >= 0; y--) {
        Serial.print(y);
        Serial.print(" | "); 
        for (int x = 0; x < GRID_SIZE; x++) {
            Serial.print(grid[y][x]);
            Serial.print(" ");
        }
        Serial.println();
    }

    Serial.print("Current Angle: ");
    Serial.print(last_angle);
    Serial.print("  Future Angle: ");
    Serial.println(future_angle);
}

void turnToAngle(int new_angle) {
    if (last_angle != new_angle) {
        future_angle = new_angle;
        steeringServo.write(future_angle);
        delay(1500);  // Delay for realistic turning
        last_angle = future_angle;
        displayGrid();  // Show updated angle after turning
    }
}

void moveToTarget(int targetX, int targetY) {
    while (posX != targetX || posY != targetY) {
        grid[posY][posX] = '.';  // Clear previous position

        int moveX = (targetX > posX) ? 1 : (targetX < posX) ? -1 : 0;
        int moveY = (targetY > posY) ? 1 : (targetY < posY) ? -1 : 0;

        // Turn before moving in X-direction
        if (moveX != 0) {
            turnToAngle((moveX == 1) ? 135 : 45);
            posX += moveX;
        } 
        // Move in the Y-direction after reaching correct X
        else if (posY != targetY) {
            turnToAngle((moveY == 1) ? 90 : 270);
            posY += moveY;
        }

        grid[posY][posX] = 'X';  // Update boat position
        displayGrid();  
        delay(1000);  // Delay to match movement
    }
}

void readRemote() {
    pulse_duration_CH1 = pulseIn(receiver_pin_CH1, HIGH);

    if (pulse_duration_CH1 >= 993 && pulse_duration_CH1 <= 1480) {
        turnToAngle(45);
    } else if (pulse_duration_CH1 >= 1500 && pulse_duration_CH1 <= 1985) {
        turnToAngle(135);
    }
}

void clearSerialBuffer() {
    while (Serial.available() > 0) {
        Serial.read();  // Clear any leftover input
    }
}

void setup() {
    Serial.begin(9600);
    steeringServo.attach(SERVO_PIN);
    pinMode(receiver_pin_CH1, INPUT);
    resetGrid();
    displayGrid();
    Serial.println("Enter target coordinates (x y) or 'R' to reset: ");
}

void loop() {
    if (remoteToggle) {
        readRemote();  // Only remote control works
    } else {
        while (Serial.available() == 0);

        if (Serial.peek() == 'R') {
            Serial.read();  // Consume 'R'
            resetGrid();
            displayGrid();
            Serial.println("Grid Reset! Enter new target: ");
            clearSerialBuffer();  // Clear any leftover input
            return;
        }

        int targetX = Serial.parseInt();
        int targetY = Serial.parseInt();
        clearSerialBuffer();  // Fully clear buffer after input

        moveToTarget(targetX, targetY);
        Serial.println("Enter next target or 'R' to reset: ");
    }
}
