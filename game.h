#ifndef GAME_H
#define GAME_H

#include <string>
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


void addCard(player* p, card c);
card removeCard(player* p, int index);
void showHand(player* p);
void changeCardColor(player* p, int index, string newColor);
card getCard(player* p, int index);


card drawFromDeck(card deck[], int &deckTop);


bool isValidPlay(card played, card topCard, string activeColor);


void rotateHands(player players[], int totalplayers);
void swapHands(player* a, player* b);
string chooseColor(bool isBot);
int chooseSwapTarget(player players[], int totalplayers, int currentIdx, bool isBot);


int botChooseCard(player* bot, card topCard, string activeColor);


bool playTurn(player players[], int totalplayers, int currentIdx,
              card &topCard, string &activeColor,
              card deck[], int &deckTop,
              int &direction, bool &skipNext);

void startGame(card deck[], int deckSize, int botAmount);
void createDeck(card deck[], int &deckSize); 

#endif