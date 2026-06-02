#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <string>
#include <limits>
#include <cctype>

using namespace std;

// STRUCT
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
    int hp = 3;
    bool eliminated = false;
};

bool exitGame = false;
int initialCardCount = 7; //default system card
int customCardCount = 0; //jumlah kartu custom yang dimasukkan user 
bool suddenDeath = false;
int suddenDeathLimit = 0;
int playTurnCount = 0;
int rotationCounter = 0;

enum InputAction {
    DRAW = -1,
    ESC  = -2
};

card drawFromDeck(card deck[], int &deckTop);
void handleActionCards(card played, int &currentIdx, int totalPlayers, 
                       bool &isClockwise, string &activeColor, 
                       card deck[], int &deckTop, player players[], bool &skipNext);
void addCard(player* p, card c);

// ANSI CODE
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define BRIGHT_CYAN    "\033[96m" 

string getColor(string color) {
    if (color == "RED") return RED;
    if (color == "YEL") return YELLOW;
    if (color == "GRN") return GREEN;
    if (color == "BLU") return BLUE;
    return RESET;
}

string displayCard(card c, bool isTopCard = false, string activeColor = "")
{
    string colorToUse = c.color;

    if (isTopCard && c.color == "WILD" && c.value != "Swap") {
        colorToUse = activeColor;
    }

    return getColor(colorToUse) + BOLD + "| " + c.value + " |" + RESET;
}

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

char showDrawnCards(player* p, card drawn[], int count, bool isPenaltyDraw, bool isPlayable)
{
    if ((*p).isBot) return 'S';

    system("cls");

    cout << "      KAMU MENDAPATKAN\n"
         << "───────────────────────────────\n\n";

    for (int i = 0; i < count; i++) {
        Sleep(250);
        cout << "  + " << displayCard(drawn[i]) << endl;
    }
    cout << "\n\n───────────────────────────────\n";

    if (isPenaltyDraw) {
        cout << "\n====================================\n";
        cout << "Tekan tombol apa saja...";
        _getch();
        return 'S';
    }

    if (!isPlayable) {

        cout << "\n====================================\n";
        cout << "Kartu tidak bisa dimainkan!\n";
        cout << "Tekan tombol apa saja...";
        _getch();
        return 'S';
    }
        cout << "\n====================================\n";
        cout << "[ENTER] Gunakan kartu sekarang | [S] Simpan untuk nanti\n";

    while (true) {
        int key = _getch();

        if (key == 13) return 'E';
        if (key == 's' || key == 'S') return 'S';
    }
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

// Bagian Display
void showHand(player players[], int totalplayers, int currentIdx,
              card topCard, string activeColor,
              player* p, int selected, bool isClockwise)
{
    cout << BOLD << "Kartu paling atas: \n" << RESET << displayCard(topCard, true, activeColor) << endl;

    for (int i = 0; i < totalplayers; i++) {
        if (players[i].eliminated) continue; // BIAR TIDAK ERROR: Jangan tampilkan jika sudah tereliminasi

        if (i == currentIdx)
        cout << BOLD << "\nList pemain:" << RESET
             << "\n- " << players[i].name << " (" << players[i].handSize << " kartu) <-- Kamu" << RESET;
    else
        cout << "\n- " << players[i].name << " (" << players[i].handSize << " kartu)";
    }

    cout << BRIGHT_CYAN << BOLD << "\n\n───────────────────────────────────────────────\n" << RESET;
    cardNode* temp = (*p).hand;
    int i = 0;
    cout << "\nKartu " << BOLD << (*p).name << RESET << ":\n";
    while (temp) {
        cout << (i == selected ? " --> " : "     ") << displayCard((*temp).data) << "\n";
        temp = (*temp).next;
        i++;
    }
    cout << BRIGHT_CYAN << BOLD << "\n───────────────────────────────────────────────\n" << RESET
         << "\nKontrol:"
         << "\n[ARROW] Pilih | [ENTER] Main | [D] Draw | [ESC] Keluar\n"; // sbnrnya dihapus buat saving space aja tp W/S tetep bisa
}

//Bagian Shuffle
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

// Sistem validasi kartu
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

// Algoritma basic bot AI
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

void startSuddenDeath(player players[], int totalplayers)
{
    suddenDeath = true;
    suddenDeathLimit = 0;
    for (int i = 0; i < totalplayers; i++)
    {
        players[i].hp = 3;

        if (players[i].handSize > suddenDeathLimit) 
        {
            suddenDeathLimit = players[i].handSize;
        }
    }
    system("cls");

    cout << "\n====================================\n";
    cout << "         SUDDEN DEATH AKTIF!\n";
    cout << "====================================\n";

    cout << "\nHP semua pemain = 3";
    cout << "\nBatas kartu awal = " << suddenDeathLimit << endl;

    cout << "\nTekan tombol apa saja...";
    _getch();
}

void checkSuddenDeath(player players[], int totalplayers)
{
    if (suddenDeathLimit > 0)
    {
        suddenDeathLimit--;
    }

    cout << "\n====================================\n";
    cout << "SUDDEN DEATH!!\n";
    cout << "Batas kartu sekarang: "
         << suddenDeathLimit << endl;
    cout << "====================================\n";
    for (int i = 0; i < totalplayers; i++)
    {
        if (players[i].eliminated) continue;

        if (players[i].handSize > suddenDeathLimit)
        {
            players[i].hp--;

            cout << "\n"
                 << players[i].name
                 << " menerima 1 damage!";

            cout << "\nHP tersisa: "
                 << players[i].hp
                 << endl;
            
            if (players[i].hp <= 0)
            {
                players[i].eliminated = true;

                cout << "\n"
                     << players[i].name
                     << " TERELIMINASI!\n";
            }
        }

    }

}

// Bagian kontrols (inputan)
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

// Bagian turn
void playTurn(player players[], int totalplayers, int &currentIdx,
              card &topCard, string &activeColor,
              card deck[], int &deckTop, bool &isClockwise)
{
    player* current = &players[currentIdx];
    bool skipNext = false; 

    int humanCount = 0;
    for (int i = 0; i < totalplayers; i++) {
        if (!players[i].isBot) humanCount++;
    }

    if (!(*current).isBot && humanCount > 1) {
        system("cls");
        cout << "\n───────────────────────────────────────────────\n";
        cout << "             GILIRAN: " << BOLD << (*current).name << RESET << "\n";
        cout << "───────────────────────────────────────────────\n";
        cout << "  Mohon oper perangkat ke " << BOLD << (*current).name << RESET << ".\n";
        cout << "\n\n";
        cout << "  Tekan [ENTER] jika " << (*current).name << " sudah siap...";
        while (_getch() != 13);
    }

    system("cls");
    cout << "\nGiliran: " << BOLD << (*current).name << RESET 
         << ((*current).isBot ? " (BOT)" : "") << "\n";

    int chosenIdx;

    if ((*current).isBot) {
        Sleep(1700);
        chosenIdx = botChooseCard(current, topCard, activeColor);

        if (chosenIdx == -1) {
            cout << endl << (*current).name << " menarik kartu...\n";
            Sleep(800);

            card drawn = drawFromDeck(deck, deckTop);

            cout << (*current).name << " mendapatkan kartu...\n";
            Sleep(800);

            if (isValidPlay(drawn, topCard, activeColor)) {
                cout << (*current).name << " langsung memainkan kartu!\n";
                Sleep(800);

                topCard = drawn;
                if (drawn.color != "WILD")
                    activeColor = drawn.color;

                handleActionCards(drawn, currentIdx, totalplayers,
                                  isClockwise, activeColor,
                                  deck, deckTop, players, skipNext);

                if ((*current).handSize == 1) {
                    cout << BOLD << YELLOW << "\nUNO!\n" << RESET;
                    Sleep(1000);
                }

                if ((*current).handSize == 0) return;

                int step = isClockwise ? 1 : -1;
                currentIdx = (currentIdx + step + totalplayers) % totalplayers;

                if (skipNext) {
                    cout << "\n[!] Giliran " << players[currentIdx].name << " DILEWATI!\n";
                    currentIdx = (currentIdx + step + totalplayers) % totalplayers;
                    Sleep(1700);
                }

                return;
            }
            else {
                addCard(current, drawn);
                cout << (*current).name << " menyimpan kartu.\n";
                Sleep(1000);
            }

            int step = isClockwise ? 1 : -1;
            currentIdx = (currentIdx + step + totalplayers) % totalplayers;

            if (skipNext) {
                cout << "\n[!] Giliran " << players[currentIdx].name << " DILEWATI!\n";
                currentIdx = (currentIdx + step + totalplayers) % totalplayers;
                Sleep(1700);
            }

            return;
        }
    }

    else {
        chosenIdx = arrowSelect(players, totalplayers, currentIdx,
                                current, topCard, activeColor, isClockwise);

        if (chosenIdx == ESC) {
            exitGame = true;
            return;
        }

        if (chosenIdx == DRAW) {
            card drawn = drawFromDeck(deck, deckTop);

            bool playable = isValidPlay(drawn, topCard, activeColor);
            char choice = showDrawnCards(current, &drawn, 1, false, playable);;

            if (choice == 'E' && isValidPlay(drawn, topCard, activeColor)) {
                topCard = drawn;

                if (drawn.color != "WILD")
                    activeColor = drawn.color;

                handleActionCards(drawn, currentIdx, totalplayers,
                                  isClockwise, activeColor,
                                  deck, deckTop, players, skipNext);
            }
            else {
                addCard(current, drawn);
            }

            int step = isClockwise ? 1 : -1;
            currentIdx = (currentIdx + step + totalplayers) % totalplayers;

            if (skipNext) {
                cout << "\n[!] Giliran " << players[currentIdx].name << " DILEWATI!\n";
                currentIdx = (currentIdx + step + totalplayers) % totalplayers;
                Sleep(1700);
            }

            return;
        }
    }

    card played = removeCard(current, chosenIdx);

    topCard = played;
    if (played.color != "WILD")
        activeColor = played.color;

    handleActionCards(played, currentIdx, totalplayers,
                      isClockwise, activeColor,
                      deck, deckTop, players, skipNext);

    if ((*current).handSize == 1) {
        cout << BOLD << YELLOW << "\nUNO!\n" << RESET;
        Sleep(1000);
    }

    if ((*current).handSize == 0) return;

    int step = isClockwise ? 1 : -1;
    currentIdx = (currentIdx + step + totalplayers) % totalplayers;

    if (skipNext) {
        cout << "\n[!] Giliran " << players[currentIdx].name << " DILEWATI!\n";
        currentIdx = (currentIdx + step + totalplayers) % totalplayers;
        Sleep(1700);
    }
}

// Game Start
void startGame(card deck[], int deckSize, int botAmount, int humanAmount, string namaAkun[])
{
    srand(time(0));
    shuffleDeck(deck, deckSize);

    int totalplayers = botAmount + humanAmount;
    player players[4];
    bool isClockwise = true; 
    suddenDeath = false;
    suddenDeathLimit = 0;   
    playTurnCount = 0;
    rotationCounter = 0;

    for (int i = 0; i < humanAmount; i++) {
        string inputNama;
        bool valid = false;

        // do {
        //     inputNama = "";
        //     if (humanAmount > 1) {
        //         cout << "Nama Pemain " << (i + 1) << ": ";
        //     } else {
        //         cout << "Nama Anda: ";
        //     }

        //     char ch;
        //     while (true) {
        //         ch = _getch();

        //         if (ch == 13) break;

        //         else if (ch == 8) {
        //             if (!inputNama.empty()) {
        //                 inputNama.pop_back();
        //                 cout << "\b \b";
        //             }
        //         }

        //         else if (isprint(ch)) {
        //             if (inputNama.length() < 15) {
        //                 inputNama += ch;
        //                 cout << ch;
        //             }
        //         }
        //     }

        //     cout << endl;

        //     bool hanyaSpasi = true;
        //     for (char c : inputNama) {
        //         if (!isspace(c)) {
        //             hanyaSpasi = false;
        //             break;
        //         }
        //     }

        //     if (inputNama.empty() || hanyaSpasi) {
        //         cout << RED << "Nama tidak boleh kosong!\n" << RESET;
        //         Sleep(1000);
        //     }
        //     else {
        //         valid = true;
        //     }

        // } while (!valid);

        // players[i].name = inputNama;
        // players[i].isBot = false;
    }

    players[0].name = namaAkun[0];  // pakai username dari akun
    players[0].isBot = false;

    // BIAR TIDAK ERROR: Inisialisasi otomatis jika pemain manusia lebih dari 1 (karena menu input di atas mati)
    for (int i = 1; i < humanAmount; i++) {
        players[i].name = "Pemain " + to_string(i + 1);
        players[i].isBot = false;
    }

    for (int i = 0; i < botAmount; i++) {
        players[humanAmount + i].name = "Bot " + to_string(i + 1);
        players[humanAmount + i].isBot = true;
    }

    system("cls"); 
    cout << "=========================================\n";
    cout << "          PENGATURAN HOUSE RULES         \n";
    cout << "=========================================\n";
    cout << "Gunakan Aturan Jumlah Kartu Kustom?\n";
    cout << "[1] Ya (Input Manual)\n";
    cout << "[2] Tidak (Gunakan Default Sistem: " << initialCardCount << " Kartu)\n"; 
    cout << "Pilihan: ";
    int pilihanRule;
    cin >> pilihanRule;

    if (pilihanRule == 1) {
        cout << "Masukkan jumlah kartu awal per pemain: ";
        cin >> customCardCount;
        if (customCardCount <= 0) {
            customCardCount = initialCardCount; 
        }
    } else {
        customCardCount = initialCardCount; 
    }

    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    system("cls"); 

    int deckTop = deckSize - 1;
    for (int r = 0; r < customCardCount; r++) 
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
        while (true) 
        {
            // BIAR TIDAK ERROR: Lewati pemain yang sudah tereliminasi di mode Sudden Death
            while (players[currentIdx].eliminated) {
                int step = isClockwise ? 1 : -1;
                currentIdx = (currentIdx + step + totalplayers) % totalplayers;
            }

            int alive = 0;
            int winner = -1;
            playTurn(players, totalplayers, currentIdx, topCard, activeColor, deck, deckTop, isClockwise);
            
            playTurnCount++;

            if (!suddenDeath && playTurnCount >= totalplayers * 10)
            {
                startSuddenDeath(players, totalplayers);
            }

            if (suddenDeath)
            {
                rotationCounter++;

                if (rotationCounter >= totalplayers)
                {
                    rotationCounter = 0;
                    checkSuddenDeath(players, totalplayers);
                }
            }
            
            if (exitGame) {
                cout << "\n\nAkan keluar segera...";
                Sleep(1700);
                break;
            }

            for (int i = 0; i < totalplayers; i++) {
                if (players[i].handSize == 0 && !players[i].eliminated) {
                    system("cls");
                    cout << BRIGHT_CYAN << BOLD << "───────────────────────────────────────────────\n" << RESET
                        << "               " << BOLD << YELLOW << "UNO GAME!!" << RESET << "\n"
                        << BRIGHT_CYAN << BOLD << "───────────────────────────────────────────────\n" << RESET
                        << "\n\n          PEMENANG: " << players[i].name << "\n\n\n"
                        << "\n\n\nTekan tombol apa saja untuk kembali ke menu";

                    _getch();
                    return;
                }
            }

            for (int i = 0; i < totalplayers; i++)
            {
                if (!players[i].eliminated)
                {
                    alive++;
                    winner = i;
                }
            }

            // == 1
            if (suddenDeath && alive <= 1)
            {
                system("cls");

                cout << "\n====================================\n";
                cout << "PEMENANG SUDDEN DEATH\n";
                cout << "====================================\n";

                if (winner != -1 && alive == 1) {
                    cout << "\n" << players[winner].name << " adalah pemenangnya!\n";
                } else {
                    cout << "\nSemua pemain tereliminasi! Game DRAW!\n";
                }

                _getch();
                return;
            }
        }
}