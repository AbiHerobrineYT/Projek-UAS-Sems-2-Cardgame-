#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include "Game.h"
using namespace std;

// ===================== ANSI COLOR =====================
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"

string getColor(string color) {
    if (color == "RED") return RED;
    if (color == "YEL") return YELLOW;
    if (color == "GRN") return GREEN;
    if (color == "BLU") return BLUE;
    return RESET;
}

string displayCard(card c) {
    return getColor(c.color) + BOLD + "| " + c.value + " |" + RESET;
}

// ===================== LINKED LIST =====================
void addCard(player* p, card c) {
    cardNode* newNode = new cardNode{c, nullptr};

    if (!p->hand) p->hand = newNode;
    else {
        cardNode* temp = p->hand;
        while (temp->next) temp = temp->next;
        temp->next = newNode;
    }
    p->handSize++;
}

card removeCard(player* p, int index) {
    cardNode* temp = p->hand;
    cardNode* prev = nullptr;

    for (int i = 0; i < index; i++) {
        prev = temp;
        temp = temp->next;
    }

    card removed = temp->data;
    if (!prev) p->hand = temp->next;
    else prev->next = temp->next;

    delete temp;
    p->handSize--;
    return removed;
}

card getCard(player* p, int index) {
    cardNode* temp = p->hand;
    for (int i = 0; i < index; i++) temp = temp->next;
    return temp->data;
}

void showHand(player* p, int selected) {
    cardNode* temp = p->hand;
    int i = 0;

    cout << "\nKartu: " << BOLD << p->name << RESET << "\n";
    while (temp) {
        cout << (i == selected ? " --> " : "     ")
             << displayCard(temp->data) << "\n";
        temp = temp->next;
        i++;
    }

    cout << "\n[ARROW] Pilih | [ENTER] Main | [D] Draw\n";
}

// ===================== SHUFFLE =====================
void shuffleDeck(card deck[], int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(deck[i], deck[j]);
    }
}

// ===================== DECK =====================
card drawFromDeck(card deck[], int &deckTop) {
    if (deckTop < 0) return {"RED", "0"}; // fallback
    return deck[deckTop--];
}

// ===================== VALIDASI =====================
bool isValidPlay(card played, card topCard, string activeColor) {
    if (played.color == activeColor) return true;
    if (played.value == topCard.value) return true;
    return false;
}

// ===================== BOT =====================
int botChooseCard(player* bot, card topCard, string activeColor) {
    cardNode* temp = bot->hand;
    int i = 0;

    while (temp) {
        if (isValidPlay(temp->data, topCard, activeColor))
            return i;
        temp = temp->next;
        i++;
    }
    return -1;
}

// ===================== INPUT =====================
int arrowSelect(player* p, card topCard, string activeColor) {
    int selected = 0;

    while (true) {
        system("cls");
        showHand(p, selected);

        int key = _getch();

        if (key == 224) {
            int arrow = _getch();
            if (arrow == 72 && selected > 0) selected--;
            if (arrow == 80 && selected < p->handSize - 1) selected++;
        }
        else if (key == 13) {
            card c = getCard(p, selected);
            if (!isValidPlay(c, topCard, activeColor)) {
                cout << "Kartu tidak valid!\n";
                Sleep(800);
                continue;
            }
            return selected;
        }
        else if (key == 'd' || key == 'D') return -1;
    }
}

// ===================== TURN =====================
void playTurn(player players[], int totalplayers, int &currentIdx,
              card &topCard, string &activeColor,
              card deck[], int &deckTop)
{
    player* current = &players[currentIdx];

    system("cls");
    cout << "\nGiliran: " << current->name;
    if (current->isBot) cout << " (BOT)";
    cout << "\nTop: " << displayCard(topCard);
    cout << " | Warna: " << getColor(activeColor) << activeColor << RESET << "\n";

    int chosenIdx;

    if (current->isBot) {
        Sleep(1200);
        chosenIdx = botChooseCard(current, topCard, activeColor);

        if (chosenIdx == -1) {
            addCard(current, drawFromDeck(deck, deckTop));
            cout << "BOT draw...\n";
            Sleep(1200);
            currentIdx = (currentIdx + 1) % totalplayers;
            return;
        }
    } else {
        chosenIdx = arrowSelect(current, topCard, activeColor);

        if (chosenIdx == -1) {
            addCard(current, drawFromDeck(deck, deckTop));
            currentIdx = (currentIdx + 1) % totalplayers;
            return;
        }
    }

    card played = removeCard(current, chosenIdx);
    topCard = played;
    activeColor = played.color;

    if (current->handSize == 1) {
        cout << "UNO!\n";
        Sleep(1000);
    }

    currentIdx = (currentIdx + 1) % totalplayers;
}

// ===================== GAME =====================
void startGame(card deck[], int deckSize, int botAmount) {
    srand(time(0));
    shuffleDeck(deck, deckSize);

    int totalplayers = botAmount + 1;
    player players[4];

    cout << "Nama kamu: ";
    cin >> players[0].name;
    players[0].hand = nullptr;
    players[0].handSize = 0;
    players[0].isBot = false;

    for (int i = 1; i <= botAmount; i++) {
        players[i].name = "Bot " + to_string(i);
        players[i].hand = nullptr;
        players[i].handSize = 0;
        players[i].isBot = true;
    }

    int deckTop = deckSize - 1;

    // bagi kartu
    for (int r = 0; r < 7; r++)
        for (int i = 0; i < totalplayers; i++)
            addCard(&players[i], drawFromDeck(deck, deckTop));

    // kartu awal
    card topCard = drawFromDeck(deck, deckTop);
    string activeColor = topCard.color;

    int currentIdx = 0;

    while (true) {
        playTurn(players, totalplayers, currentIdx,
                 topCard, activeColor,
                 deck, deckTop);

        for (int i = 0; i < totalplayers; i++) {
            if (players[i].handSize == 0) {
                system("cls");
                cout << players[i].name << " MENANG!\n";
                return;
            }
        }
    }
}