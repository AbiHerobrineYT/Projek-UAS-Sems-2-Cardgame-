#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <string>
#include <limits>
#include <cctype>

using namespace std;

// ==== STRUCT ====
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

bool exitGame = false;

enum InputAction {
    DRAW = -1,
    ESC  = -2
};

// ==== PROTOTYPE ====
// Penting: Pastikan parameter menggunakan Reference (&) agar perubahan nilai bersifat permanen
card drawFromDeck(card deck[], int &deckTop);
void handleActionCards(card played, int &currentIdx, int totalPlayers, 
                       bool &isClockwise, string &activeColor, 
                       card deck[], int &deckTop, player players[], bool &skipNext);
void addCard(player* p, card c);

//* ANSI CODE
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

string displayCard(card c, string activeColor = "")
{
    string colorToUse = c.color;

    // 🔥 kalau WILD, pakai warna aktif
    if (c.color == "WILD" && activeColor != "") {
        colorToUse = activeColor;
    }

    return getColor(colorToUse) + BOLD + "| " + c.value + " |" + RESET;
}

// ===================== LINKED LIST =====================
void addCard(player* p, card c) {
    cardNode* newNode = new cardNode{c, nullptr};
    if (!(*p).hand) (*p).hand = newNode;
    else {
        cardNode* temp = (*p).hand;
        while ((*temp).next) temp = (*temp).next;
        (*temp).next = newNode;
    }
    (*p).handSize++;
}

void showDrawnCards(player* p, card drawn[], int count)
{
    if ((*p).isBot) return;

    system("cls");

    cout << "====================================\n";
    cout << "        KAMU MENDAPATKAN\n";
    cout << "====================================\n\n";

    for (int i = 0; i < count; i++) {
        Sleep(250);
        cout << "  + " << displayCard(drawn[i]) << endl;
    }

    cout << "\n====================================\n";
    cout << "Tekan tombol apa saja...";
    _getch();
}

card removeCard(player* p, int index) {
    cardNode* temp = (*p).hand;
    cardNode* prev = nullptr;
    for (int i = 0; i < index; i++) {
        prev = temp;
        temp = (*temp).next;
    }
    card removed = (*temp).data;
    if (!prev) (*p).hand = (*temp).next;
    else prev->next = (*temp).next;
    delete temp;
    (*p).handSize--;
    return removed;
}

card getCard(player* p, int index) {
    cardNode* temp = (*p).hand;
    for (int i = 0; i < index; i++) temp = (*temp).next;
    return (*temp).data;
}

//* BAGIAN DISPLAY
void showHand(player players[], int totalplayers, int currentIdx,
              card topCard, string activeColor,
              player* p, int selected, bool isClockwise)
{
    cout << "---------------------------------------------\n" << endl
         << "Kartu paling atas: \n" << displayCard(topCard, activeColor) << endl;

    for (int i = 0; i < totalplayers; i++) {
        if (i == currentIdx) continue;
        cout << "\n- " << players[i].name << " (" << players[i].handSize << " kartu)\n";
    }

    cout << "\n---------------------------------------------\n";
    cardNode* temp = (*p).hand;
    int i = 0;
    cout << "\nKartu " << BOLD << (*p).name << RESET << ":\n";
    while (temp) {
        cout << (i == selected ? " --> " : "     ") << displayCard((*temp).data, activeColor) << "\n";
        temp = (*temp).next;
        i++;
    }
    cout << "\n---------------------------------------------\n"
         << "\nKontrol:"
         << "\n[ARROW] Pilih | [ENTER] Main | [D] Draw | [ESC] Keluar\n"; // sbnrnya dihapus buat saving space aja tp W/S tetep bisa
}

//* BAGIAN DECK DAN SHUFFLE
void shuffleDeck(card deck[], int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(deck[i], deck[j]);
    }
}

card drawFromDeck(card deck[], int &deckTop) {
    if (deckTop < 0) return {"WILD", "Wild"}; 
    return deck[deckTop--];
}

//* VALIDASI KARTU
bool isValidPlay(card played, card topCard, string activeColor)
{
    if (topCard.value == "Swap")
        return true;

    if (played.color == "WILD" || 
        played.value == "Wild" || 
        played.value == "+4" || 
        played.value == "Swap")
        return true;

    return (played.color == activeColor || played.value == topCard.value);
}

//* BOT AI
int botChooseCard(player* bot, card topCard, string activeColor) {
    cardNode* temp = (*bot).hand;
    int i = 0;
    while (temp) {
        if (isValidPlay((*temp).data, topCard, activeColor)) return i;
        temp = (*temp).next;
        i++;
    }
    return -1;
}

// ===================== INPUT =====================
int arrowSelect(player players[], int totalplayers, int currentIdx,
                player* p, card topCard, string activeColor, bool isClockwise)
{
    int selected = 0;
    while (true) {
        system("cls");
        showHand(players, totalplayers, currentIdx, topCard, activeColor, p, selected, isClockwise);

        int key = _getch();
        if (key == 27) {
            return ESC;
        }
        else if (key == 224) { 
            int arrow = _getch();
            if (arrow == 72 && selected > 0) selected--; 
            if (arrow == 80 && selected < (*p).handSize - 1) selected++; 
        }
        else if (key == 'w' || key == 'W') {
            if (selected > 0) selected--;
        }
        else if (key == 's' || key == 'S') {
            if (selected < (*p).handSize - 1) selected++;
        }
        else if (key == 13) { 
            card c = getCard(p, selected);
            if (!isValidPlay(c, topCard, activeColor)) {
                cout << RED << "Kartu tidak cocok!" << RESET << "\n";
                Sleep(1000);
                continue;
            }
            return selected;
        }
        else if (key == 'd' || key == 'D') return DRAW; 
    }
}

// ===================== TURN =====================
void playTurn(player players[], int totalplayers, int &currentIdx,
              card &topCard, string &activeColor,
              card deck[], int &deckTop, bool &isClockwise) // Reference isClockwise
{
    player* current = &players[currentIdx];
    bool skipNext = false; 

    system("cls");
    cout << "\nGiliran: " << BOLD << (*current).name << RESET << ((*current).isBot ? " (BOT)" : "") << "\n";

    int chosenIdx;
    if ((*current).isBot) {
        Sleep(1700);
        chosenIdx = botChooseCard(current, topCard, activeColor);
        if (chosenIdx == -1) {
            cout << endl << (*current).name << " menarik kartu...\n";

            card drawn = drawFromDeck(deck, deckTop);
            addCard(current, drawn);

            Sleep(1700);
            currentIdx = (currentIdx + (isClockwise ? 1 : -1) + totalplayers) % totalplayers;
            return;
        }
    } else {
        chosenIdx = arrowSelect(players, totalplayers, currentIdx, current, topCard, activeColor, isClockwise);
        if (chosenIdx == ESC) {
            exitGame = true;
            return;
        }
        if (chosenIdx == DRAW) {
            card drawn = drawFromDeck(deck, deckTop);
            addCard(current, drawn);

            card temp[1] = { drawn };
            showDrawnCards(current, temp, 1);

            currentIdx = (currentIdx + (isClockwise ? 1 : -1) + totalplayers) % totalplayers;
            return;
        }
        // if (chosenIdx == -1) {
        //     addCard(current, drawFromDeck(deck, deckTop));
        //     currentIdx = (currentIdx + (isClockwise ? 1 : -1) + totalplayers) % totalplayers;
        //     return;
        // }
    }

    card played = removeCard(current, chosenIdx);
    topCard = played;
    if (played.color != "WILD") activeColor = played.color;

    // Memproses efek kartu sakti
    handleActionCards(played, currentIdx, totalplayers, isClockwise, 
                      activeColor, deck, deckTop, players, skipNext);

    if ((*current).handSize == 1) { cout << BOLD << YELLOW << "\nUNO!\n" << RESET; Sleep(1000); }
    if ((*current).handSize == 0) return; 

    // Update Giliran Berdasarkan Arah (Step)
    int step = isClockwise ? 1 : -1;
    currentIdx = (currentIdx + step + totalplayers) % totalplayers;

    // Jika Skip aktif (dari +2, +4, atau Skip card)
    if (skipNext) {
        cout << "\n[!] Giliran " << players[currentIdx].name << " DILEWATI!\n";
        currentIdx = (currentIdx + step + totalplayers) % totalplayers;
        Sleep(1700);
    }
}

//* GAME START
void startGame(card deck[], int deckSize, int botAmount) {
    srand(time(0));
    shuffleDeck(deck, deckSize);
    // string namaBot[] = {"Alfan", "Iren", "Hima", "Ahnaf", "Rahel"};
    // int jumlahNamaBot = sizeof(namaBot) / sizeof(namaBot[0]);

    int totalplayers = botAmount + 1;
    player players[4];
    bool isClockwise = true; 

    string inputNama;
    bool valid = false;

    do {
        inputNama = "";
        cout << "Nama Anda: ";

        char ch;
        while (true) {
            ch = _getch();

            if (ch == 13) break;

            else if (ch == 8) {
                if (!inputNama.empty()) {
                    inputNama.pop_back();
                    cout << "\b \b";
                }
            }

            else if (isprint(ch)) {
                if (inputNama.length() < 15) {
                    inputNama += ch;
                    cout << ch;
                }
            }
        }

        cout << endl;

        bool hanyaSpasi = true;
        for (char c : inputNama) {
            if (!isspace(c)) {
                hanyaSpasi = false;
                break;
            }
        }

        if (inputNama.empty() || hanyaSpasi) {
            cout << RED << "Nama tidak boleh kosong!\n" << RESET;
            Sleep(1000);
        }
        else {
            valid = true;
        }

    } while (!valid);

    players[0].name = inputNama;
    players[0].isBot = false;

    // for (int i = jumlahNamaBot - 1; i > 0; i--) { //? ngacak nama bot biar gak bingugn di UI
    //     int j  = rand() % (i + 1);
    //     swap(namaBot[i], namaBot[j]);
    // }

    for (int i = 1; i <= botAmount; i++) {
        players[i].name = "Bot " + to_string(i);
        players[i].isBot = true;
    }

    int deckTop = deckSize - 1;
    for (int r = 0; r < 7; r++)
        for (int i = 0; i < totalplayers; i++)
            addCard(&players[i], drawFromDeck(deck, deckTop));

    card topCard;
    while (true) {
        topCard = drawFromDeck(deck, deckTop);

        if (topCard.value != "Reverse" &&
            topCard.value != "Skip" &&
            topCard.value != "+2" &&
            topCard.value != "+4" &&
            topCard.value != "Wild" &&
            topCard.value != "Swap") {
            break;
        }
    }
    string activeColor = topCard.color;

    int currentIdx = 0;
    while (true) {
        playTurn(players, totalplayers, currentIdx, topCard, activeColor, deck, deckTop, isClockwise);

        if (exitGame) {
            cout << "\n\nAkan keluar segera...";
            Sleep(1700);
            break;
        }

        for (int i = 0; i < totalplayers; i++) {
            if (players[i].handSize == 0) {
                system("cls");
                cout << "------------------------------------\n"
                     << "         " << BOLD << YELLOW << "UNO GAME!!" << RESET << "\n"
                     << "------------------------------------\n"
                     << "\n\n\n\n          PEMENANG: " << players[i].name << "\n\n\n"
                     << "Tekan tombol apa saja untuk kembali ke menu";
                     _getch();
                return;
            }
        }
    }
}