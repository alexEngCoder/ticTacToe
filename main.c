#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void singlePlayer();  // Function prototype

int main()
{
    int choice;
    while (1)
    {
        printf("0: AI vs AI\n1: Player vs Player\n2: Player vs AI\n");
        scanf("%d", &choice);

        if (choice == 0)
        {
            printf("Running AI vs AI...\n");
            system("mosquitto_pub -h 35.233.231.192 -t esp32/liquid -m 0");
        }
        else if (choice == 1)
        {
            printf("Running Player vs Player...\n");
            system("mosquitto_pub -h 35.233.231.192 -t esp32/liquid -m 1");
            singlePlayer();
        }
        else if (choice == 2)
        {
            printf("Running player vs AI...\n");
            system("mosquitto_pub -h 35.233.231.192 -t esp32/liquid -m 2");
            singlePlayer();
        }
        else
        {
            printf("Not an option\n");
        }
    }

    return 0;
}

void singlePlayer()
{
    while (1)
    {
        int x, y;

        printf("Enter the x coordinate: ");
        scanf("%d", &x);

        printf("Enter the y coordinate: ");
        scanf("%d", &y);

        if (x >= 0 && x <= 2 && y >= 0 && y <= 2)
        {
            char msg[100];
            snprintf(msg, sizeof(msg), "%d,%d", x, y);

            char command[256];
            snprintf(command, sizeof(command),
                     "mosquitto_pub -h 35.233.231.192 -t esp32/aiVsPlayer -m \"%s\"", msg);
            system(command);
        }
        else
        {
            printf("Incorrect coordinates. Try again.\n");
        }
    }
}
