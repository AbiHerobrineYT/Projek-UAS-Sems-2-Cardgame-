#include <iostream>
#include <string>
using namespace std;

struct card 
{
    string color;
    string value;
};

void createDeck (card kartu[], int &deckSize)
{
    string colors[] = {"RED", "YEL", "GRN", "BLU"};
    int index = 0;

    for (int c = 0; c<4; c++)
    {   
        kartu[index++] = {colors[c], "0"};
        for (int i = 1; i<= 9;i++)
        {
            kartu[index++] = {colors[c], to_string(i)};
            kartu[index++] = {colors[c], to_string(i)};
        }
        
        string actionCard[] = {"Reverse","Skip","+2"};

        for (int n = 0; n < 3; n++)
        {
            kartu[index++] = {colors[c], actionCard[n]};
            kartu[index++] = {colors[c], actionCard[n]};
        }
    }
    
    string wildCard[] = {"+4","Swap","Wild"};
        for (int o =0; o<4;o++)
        {
        for (int p = 0; p<3; p++)
            {
                kartu[index++] = {"HTM", wildCard[p]};
            }
        }
        deckSize = index;
}

// main()
// {
//     return 0;
// }