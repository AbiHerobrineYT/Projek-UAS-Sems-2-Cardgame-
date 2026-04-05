#include <iostream>
#include <conio.h>
#include <windows.h>
using namespace std;

struct card {
    string color;
    string value;
};

extern bool exitGame;

// prototype
void createDeck(card deck[], int &deckSize);
void startGame(card deck[], int deckSize, int botAmount);

void startMenu()
{
    cout << "\n────────────────── UNO GAME ───────────────────\n" << endl
         << "\n          Press any button to Play        \n" << endl
         << "\n          ESC to Exit...        " << endl
         << "\n───────────────────────────────────────────────\n" << endl;
}

void mainMenu(int mainMenuSelect)
{
        cout << "\n────────────── Opponent ──────────────\n" << endl;
        
        if (mainMenuSelect == 1){
            cout << " -->       --- 1 Bot ---\n"<< endl
                 << "           --- 2 Bot ---\n"<< endl
                 << "           --- 3 Bot ---\n"<< endl
                 << "\n         ESC to Back...        " << endl;
        }
        else if (mainMenuSelect == 2){
            cout << "           --- 1 Bot ---\n"<< endl
                 << " -->       --- 2 Bot ---\n"<< endl
                 << "           --- 3 Bot ---\n"<< endl
                 << "\n         ESC to Back...        " << endl;
        }
        else if (mainMenuSelect == 3){
            cout << "           --- 1 Bot ---\n"<< endl;
            cout << "           --- 2 Bot ---\n"<< endl;
            cout << " -->       --- 3 Bot ---\n"<< endl;
            cout << "\n         ESC to Back...        " << endl;
    }
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    int input;

    while (true)
    {
        system("cls");
        startMenu();

        input = _getch();

        if (input == 27) 
            break;

        int select = 1;

        while (true)
        {
            system("cls");
            mainMenu(select);

            input = _getch();

            if (input == 27)
                break;

            if (input == 0 || input == 224)
            {
                int arrow = _getch(); 

                if (arrow == 72 && select > 1)
                    select--;
                else if (arrow == 80 && select < 3)
                    select++;
            }
            else if (input == 13)
            {
                system("cls");
                cout << "Kamu pilih " << select << " bot!\n";

                card deck[600];
                int deckSize;

                createDeck(deck, deckSize);
                startGame(deck, deckSize, select);

                exitGame = false;
                break;
            }
        }
    }

    return 0;
}