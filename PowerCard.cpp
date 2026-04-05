#include <iostream>
#include <string>
#include <conio.h>
#include <windows.h>
#include <limits>

using namespace std;

struct card {
    string color, value;
};

struct cardNode {
    card data;
    cardNode* next;
};

struct player {
    string name;
    cardNode* hand = nullptr;
    int handSize = 0;
    bool isBot;
};

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"

void addCard(player* p, card c);
card drawFromDeck(card deck[], int &deckTop);
string getColor(string color);
string displayCard(card c, bool isTopCard = false, string activeColor = "");
void showDrawnCards(player* p, card drawn[], int count);

void handleActionCards(card played, int &currentIdx, int totalPlayers, 
                       bool &isClockwise, string &activeColor, 
                       card deck[], int &deckTop, player players[],
                       bool &skipNext) 
{
    string val = played.value;
    int step = isClockwise ? 1 : -1;

    cout << "\nMengeluarkan kartu: " << displayCard(played, true, activeColor) << " \n";

    if (val == "Reverse") {
        if (totalPlayers == 2) {
            skipNext = true;
            cout << "\nHanya 2 pemain.\n";
        } else {
        isClockwise = !isClockwise;
        cout << "\nArah putaran sekarang dibalik!\n";
        }
    }

    else if (val == "Skip") {
        skipNext = true; 
        cout << "\nPemain berikutnya akan dilewati!\n";
    }

    else if (val == "+2") {
        int victimIdx = (currentIdx + step + totalPlayers) % totalPlayers;

        card drawn[2];

        for (int i = 0; i < 2; i++) {
            drawn[i] = drawFromDeck(deck, deckTop);
            addCard(&players[victimIdx], drawn[i]);
        }

        cout << endl << players[victimIdx].name << " mengambil 2 kartu!\n";

        Sleep(1500);

        showDrawnCards(&players[victimIdx], drawn, 2);

        skipNext = true; 
    }

    else if (val == "Wild") {
        string choiceColor;
        if (players[currentIdx].isBot) {
            string colors[] = {"RED", "YEL", "GRN", "BLU"};
            choiceColor = colors[rand() % 4];
        } else {
            cout << "\nPilih Warna Baru (R/Y/G/B): ";
            while (_kbhit()) _getch(); 
            
            char choice = toupper(_getch());
            if (choice == 'R') choiceColor = "RED";
            else if (choice == 'Y') choiceColor = "YEL";
            else if (choice == 'G') choiceColor = "GRN";
            else choiceColor = "BLU";
        }
        activeColor = choiceColor;
        cout << "\n# Warna meja sekarang berubah menjadi: " << getColor(activeColor) << BOLD << activeColor << RESET << "\n";
    }

    else if (val == "+4") {
        string choiceColor;

        if (players[currentIdx].isBot) {
            string colors[] = {"RED", "YEL", "GRN", "BLU"};
            choiceColor = colors[rand() % 4];
        } else {
            cout << "\nPilih Warna Baru untuk +4 (R/Y/G/B): ";
            while (_kbhit()) _getch();

            char choice = toupper(_getch());
            if (choice == 'R') choiceColor = "RED";
            else if (choice == 'Y') choiceColor = "YEL";
            else if (choice == 'G') choiceColor = "GRN";
            else choiceColor = "BLU";
        }

        activeColor = choiceColor;

        int victimIdx = (currentIdx + step + totalPlayers) % totalPlayers;

        Sleep(1500);

        card drawn[4];

        for (int i = 0; i < 4; i++) {
            drawn[i] = drawFromDeck(deck, deckTop);
            addCard(&players[victimIdx], drawn[i]);
        }

        cout << "\n# Warna: " << getColor(activeColor) << BOLD << activeColor << RESET << ". " << players[victimIdx].name << " ambil 4 kartu & dilewati!\n";

        showDrawnCards(&players[victimIdx], drawn, 4);

        skipNext = true; 
    }

    else if (val == "Swap") {
        int targetIdx = -1;

        if (players[currentIdx].isBot) {
            do {
                targetIdx = rand() % totalPlayers;
            } while (targetIdx == currentIdx);
        } 
        else {
            int selected = 0;

            if (selected == currentIdx) selected++;

            while (true) {
                system("cls");

                cout << "─────────────── PILIH TARGET SWAP ──────────────\n\n";

                for (int i = 0; i < totalPlayers; i++) {
                    if (i == currentIdx) continue;

                    if (i == selected)
                        cout << " > ";
                    else
                        cout << "   ";

                    cout << "[" << i << "] "
                        << players[i].name
                        << " (" << players[i].handSize << " kartu)\n";
                }

                int key = _getch();

                if (key == 224) {
                    key = _getch();

                    if (key == 72) {
                        do {
                            selected--;
                            if (selected < 0) selected = totalPlayers - 1;
                        } while (selected == currentIdx);
                    }

                    else if (key == 80) {
                        do {
                            selected++;
                            if (selected >= totalPlayers) selected = 0;
                        } while (selected == currentIdx);
                    }
                }
                else if (key == 13) {
                    targetIdx = selected;
                    break;
                }
            }
        }

        cardNode* tempHand = players[currentIdx].hand;
        players[currentIdx].hand = players[targetIdx].hand;
        players[targetIdx].hand = tempHand;

        int tempSize = players[currentIdx].handSize;
        players[currentIdx].handSize = players[targetIdx].handSize;
        players[targetIdx].handSize = tempSize;

        cout << "\n[#] SWAP BERHASIL! ";
                Sleep(1000);
        cout << "\n\n" << players[currentIdx].name 
             << " bertukar kartu dengan "
             << players[targetIdx].name << "!\n";

        cout << "\nTekan tombol apa saja untuk lanjut...";
        _getch();

    }

    Sleep(1700);
}