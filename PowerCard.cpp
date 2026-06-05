#include <iostream>
#include <windows.h>
#include <conio.h>
#include <string>
#include <climits>

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
    bool eliminated = false;
};

#define RESET        "\033[0m"
#define BOLD         "\033[1m"
#define RED          "\033[31m"
#define YELLOW       "\033[33m"
#define GREEN        "\033[32m"
#define BLUE         "\033[34m"
#define BRIGHT_CYAN  "\033[96m"

string getColor(string color);
string displayCard(card c, bool isTopCard = false, string activeColor = "");
void   addCard(player* p, card c);
card   drawFromDeck(card deck[], int &deckTop);
string pickColor(bool isBot);

extern bool   suddenDeath;
extern bool   customStackingEnabled;
extern int    accumulatedPenalty;
extern string activePenaltyType;

bool isPowerCard(card c)
{
    return c.value == "Reverse" ||
           c.value == "Skip"    ||
           c.value == "+2"      ||
           c.value == "+4"      ||
           c.value == "Wild"    ||
           c.value == "Swap";
}

void startSuddenDeath(player players[], int totalplayers)
{
    suddenDeath = true;
    for (int i = 0; i < totalplayers; i++) {
        cardNode* curr = players[i].hand;
        cardNode* prev = nullptr;
        while (curr) {
            if (isPowerCard(curr->data)) {
                cardNode* del = curr;
                if (prev == nullptr) {
                    players[i].hand = curr->next;
                    curr = players[i].hand;
                } else {
                    prev->next = curr->next;
                    curr = prev->next;
                }
                delete del;
                players[i].handSize--;
            } else {
                prev = curr;
                curr = curr->next;
            }
        }
    }

    system("cls");
    cout << "\n──────────────────────────────────\n"
         << "        SUDDEN DEATH AKTIF\n"
         << "────────────────────────────────────\n"
         << "\nPower Card DESTROYED!\n"
         << "Setiap rotasi pemain dengan\n"
         << "kartu terbanyak akan dieliminasi.\n\n"
         << "Tekan tombol apa saja...";
    _getch();
}

void eliminationHighest(player players[], int totalplayers)
{
    system("cls");
    int maxCard = -1;
    cout << "\n──────────────────────────────────\n"
         << "          SUDDEN DEATH!!\n"
         << "────────────────────────────────────\n";

    for (int i = 0; i < totalplayers; i++) {
        if (!players[i].eliminated && players[i].handSize > maxCard)
            maxCard = players[i].handSize;
    }

    for (int i = 0; i < totalplayers; i++) {
        if (!players[i].eliminated && players[i].handSize == maxCard) {
            players[i].eliminated = true;
            cout << "\n" << players[i].name << " Tereliminasi!\n";
            Sleep(2000);
        }
    }
}

void handleActionCards(card played, int &currentIdx, int totalPlayers,
                       bool &isClockwise, string &activeColor,
                       card deck[], int &deckTop, player players[], bool &skipNext)
{
    string val = played.value;
    int step   = isClockwise ? 1 : -1;

    cout << "\nMengeluarkan kartu: " << displayCard(played, true, activeColor) << "\n";
    Sleep(800);

    if (val == "Reverse") {
        if (totalPlayers == 2) {
            skipNext = true;
            cout << "\nHanya 2 pemain — Reverse berlaku sebagai Skip!\n";
        } else {
            isClockwise = !isClockwise;
            cout << "\nArah putaran sekarang dibalik!\n";
        }
        Sleep(1200);
    }

    else if (val == "Skip") {
        skipNext = true;
        cout << "\nPemain berikutnya akan dilewati!\n";
        Sleep(1200);
    }

    else if (val == "+2") {
        if (customStackingEnabled) {
            accumulatedPenalty += 2;
            activePenaltyType   = "+2";
            cout << "\nHukuman ditumpuk! Total sekarang: " << accumulatedPenalty << " kartu\n";
        } else {
            int victimIdx = (currentIdx + step + totalPlayers) % totalPlayers;
            Sleep(1000);
            cout << players[victimIdx].name << " harus mengambil 2 kartu!\n";
            Sleep(800);
            card drawn[2];
            for (int i = 0; i < 2; i++) {
                drawn[i] = drawFromDeck(deck, deckTop);
                addCard(&players[victimIdx], drawn[i]);
            }
        }
        skipNext = true;
    }

    else if (val == "Wild") {
        activeColor = pickColor(players[currentIdx].isBot);
        cout << "\n# Warna meja sekarang: "
             << getColor(activeColor) << BOLD << activeColor << RESET << "\n";
        Sleep(1000);
    }

    else if (val == "+4") {
        activeColor = pickColor(players[currentIdx].isBot);

        if (customStackingEnabled) {
            accumulatedPenalty += 4;
            activePenaltyType   = "+4";
            cout << "\n# Warna berubah ke: "
                 << getColor(activeColor) << BOLD << activeColor << RESET << "\n";
            cout << "Hukuman ditumpuk! Total: " << accumulatedPenalty << " kartu\n";
        } else {
            int victimIdx = (currentIdx + step + totalPlayers) % totalPlayers;
            cout << "\n# Warna berubah ke: "
                 << getColor(activeColor) << BOLD << activeColor << RESET << "\n";
            Sleep(800);
            cout << players[victimIdx].name << " harus mengambil 4 kartu!\n";
            Sleep(1000);
            card drawn[4];
            for (int i = 0; i < 4; i++) {
                drawn[i] = drawFromDeck(deck, deckTop);
                addCard(&players[victimIdx], drawn[i]);
            }
        }
        skipNext = true;
    }

    else if (val == "Swap") {
        int targetIdx = -1;

        if (players[currentIdx].isBot) {
            int minCards = INT_MAX;
            for (int i = 0; i < totalPlayers; i++) {
                if (i == currentIdx) continue;
                if (players[i].handSize < minCards) {
                    minCards  = players[i].handSize;
                    targetIdx = i;
                }
            }
        } else {
            int selected = 0;
            for (int i = 0; i < totalPlayers; i++) {
                if (i != currentIdx) { selected = i; break; }
            }

            while (true) {
                system("cls");
                cout << "─────────────── PILIH TARGET SWAP ──────────────\n\n";
                for (int i = 0; i < totalPlayers; i++) {
                    if (i == currentIdx) continue;
                    cout << (i == selected ? " --> " : "     ")
                         << players[i].name
                         << " (" << players[i].handSize << " kartu)\n\n";
                }
                cout << "────────────────────────────────────────────────\n"
                     << "[ARROW] Pilih | [ENTER] Konfirmasi\n";

                int key = _getch();
                if (key == 224 || key == 0) {
                    int arrow = _getch();
                    if (arrow == 72) {
                        do {
                            selected--;
                            if (selected < 0) selected = totalPlayers - 1;
                        } while (selected == currentIdx);
                    } else if (arrow == 80) {
                        do {
                            selected++;
                            if (selected >= totalPlayers) selected = 0;
                        } while (selected == currentIdx);
                    }
                } else if (key == 13) {
                    targetIdx = selected;
                    break;
                }
            }
        }

        cardNode* tempHand       = players[currentIdx].hand;
        players[currentIdx].hand = players[targetIdx].hand;
        players[targetIdx].hand  = tempHand;

        int tempSize                 = players[currentIdx].handSize;
        players[currentIdx].handSize = players[targetIdx].handSize;
        players[targetIdx].handSize  = tempSize;

        system("cls");
        cout << "\n[#] SWAP BERHASIL!\n\n"
             << players[currentIdx].name << " bertukar kartu dengan "
             << players[targetIdx].name << "!\n\n"
             << "Tekan tombol apa saja untuk lanjut...";
        _getch();
    }

    Sleep(500);
}