#include <iostream>
#include <string>
using namespace std;

struct card 
{
    string color;
    string value;
};

void createDeck(card kartu[], int &deckSize)
{
    string colors[] = {"RED", "YEL", "GRN", "BLU"};
    int index = 0;

    int repeatNumber = 6;
    int repeatAction = 4;
    int repeatWild = 3;

    for (int c = 0; c < 4; c++)
    {
        for (int r = 0; r < repeatNumber; r++)
        {
            kartu[index++] = {colors[c], "0"};
        }

        for (int i = 1; i <= 9; i++)
        {
            for (int r = 0; r < repeatNumber; r++)
            {
                kartu[index++] = {colors[c], to_string(i)};
            }
        }

        string actionCard[] = {"Reverse", "Skip", "+2"};
        for (int n = 0; n < 3; n++)
        {
            for (int r = 0; r < repeatAction; r++)
            {
                kartu[index++] = {colors[c], actionCard[n]};
            }
        }
    }

    string wildCard[] = {"+4", "Swap", "Wild"};
    for (int p = 0; p < 3; p++)
    {
        for (int r = 0; r < repeatWild; r++)
        {
            kartu[index++] = {"WILD", wildCard[p]};
        }
    }

    deckSize = index;
}