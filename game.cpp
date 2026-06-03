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
    bool eliminated = false;
};

// MOVEMENT LOG
#define MAX_LOG 15

struct logEntry {
    string playerName, cardColor, cardValue;
    bool clockwise, isPenalty;
    int penaltyAmount;
    string extra;
};

struct movementLog {
    logEntry entries[MAX_LOG];
    int head = 0, count = 0;

    void add(logEntry e) {
        entries[head] = e;
        head = (head + 1) % MAX_LOG;
        if (count < MAX_LOG) count++;
    }

    logEntry get(int i) {
        int start = (count < MAX_LOG) ? 0 : head;
        return entries[(start + i) % MAX_LOG];
    }
} moveLog;

bool exitGame = false;
int initialCardCount = 7; 
int customCardCount = 0; 
bool suddenDeath = false;
int playTurnCount = 0;
int rotationCounter = 0;
bool customStackingEnabled = false;  
int accumulatedPenalty = 0;          
string activePenaltyType = "";       

enum InputAction {
    DRAW = -1,
    ESC  = -2
};

card drawFromDeck(card deck[], int &deckTop);

bool isPowerCard(card c)
{
    return c.value == "Reverse" ||
           c.value == "Skip" ||
           c.value == "+2" ||
           c.value == "+4" ||
           c.value == "Wild" ||
           c.value == "Swap";
}

void handleActionCards(card played, int &currentIdx, int totalPlayers, 
                       bool &isClockwise, string &activeColor, 
                       card deck[], int &deckTop, player players[], bool &skipNext);

void addCard(player* p, card c);

void clearHand(player* p) { 
    cardNode* current = (*p).hand;
    while (current != nullptr) {
        cardNode* nextNode = current->next;
        delete current;
        current = nextNode;
    }
    (*p).hand = nullptr;
    (*p).handSize = 0;
}

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
    if (index < 0 || index >= (*p).handSize || !(*p).hand) return {"ERROR", "ERROR"};
    cardNode* temp = (*p).hand;
    for (int i = 0; i < index && temp; i++) temp = (*temp).next;
    return temp ? (*temp).data : card{"ERROR", "ERROR"};
}

void pushLog(const string& name, const card& c, bool cw,
             bool isPenalty = false, int penCount = 0, const string& extra = "") {
    logEntry e;
    e.playerName   = name;
    e.cardColor    = c.color;
    e.cardValue    = c.value;
    e.clockwise    = cw;
    e.isPenalty    = isPenalty;
    e.penaltyAmount = penCount;
    e.extra        = extra;
    moveLog.add(e);
}

// FITUR BARU: MENU MOVEMENT LOG FULL SCREEN
void displayLog() { 
    system("cls");
    cout << BRIGHT_CYAN << BOLD << "====================================================\n";
    cout << "                 MOVEMENT LOG (Last " << moveLog.count << ")\n";
    cout << "====================================================\n" << RESET;
    
    if (moveLog.count == 0) {
        cout << "\n  (belum ada aksi)\n\n";
    } else {
        cout << "\n";
        for (int i = 0; i < moveLog.count; i++) {
            logEntry e = moveLog.get(i);
            string dir = e.clockwise ? "CW" : "CCW";
            string colorCode = getColor(e.cardColor);

            cout << "  " << (i + 1 == moveLog.count ? ">" : " ")
                 << " [" << dir << "] "
                 << BOLD << e.playerName << RESET << ": ";
                 
            if (e.cardColor == "---") {
                cout << BOLD << e.cardValue << RESET; 
            } else {
                cout << colorCode << BOLD << e.cardColor << " " << e.cardValue << RESET;
            }

            if (e.isPenalty && e.penaltyAmount > 0)
                cout << " → +" << e.penaltyAmount << " kartu";

            if (!e.extra.empty())
                cout << " (" << e.extra << ")";

            cout << "\n";
        }
        cout << "\n";
    }
    cout << BRIGHT_CYAN << BOLD << "====================================================\n" << RESET;
    cout << "Tekan tombol apa saja untuk kembali ke permainan...";
    _getch();
}

void showHand(player players[], int totalplayers, int currentIdx,
              card topCard, string activeColor,
              player* p, int selected, bool isClockwise)
{
    cout << BOLD << "Kartu paling atas: \n" << RESET << displayCard(topCard, true, activeColor) << endl;

    for (int i = 0; i < totalplayers; i++) {
        

        if (i == currentIdx) {
            cout << BOLD << "\nList pemain:" << RESET;
            cout << "\n- " << players[i].name << " (" << players[i].handSize << " kartu)";
            if (suddenDeath && players[i].eliminated == true)
            {
                cout << " [ELIMINATED]";
            }
            if (!players[i].isBot && i == currentIdx) cout << " <-- Kamu";
            cout << RESET;
        }
        else
        {
            cout << "\n- " << players[i].name << " (" << players[i].handSize << " kartu)";
            if (suddenDeath && players[i].eliminated == true)
            {
                cout << " [ELIMINATED]";
            }
        }
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
    // DITAMBAHKAN KONTROL [L] Log
    cout << BRIGHT_CYAN << BOLD << "\n───────────────────────────────────────────────\n" << RESET
         << "\nKontrol:"
         << "\n[ARROW] Pilih | [ENTER] Main | [D] Draw | [L] Log | [ESC] Keluar\n";
}

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
    for (int i = 0; i < totalplayers; i++)
    {
        cardNode* curr = players[i].hand;
        cardNode* prev = nullptr;
        while (curr)
        {
            if (isPowerCard(curr->data))
            {
                cardNode* del = curr;

                if (prev == nullptr)
                {
                    players[i].hand = curr->next;
                    curr = players[i].hand;
                }
                else
                {
                    prev->next = curr->next;
                    curr = prev->next;
                }

                delete del;
                players[i].handSize--;
            }
            else 
            {
                prev = curr;
                curr = curr->next;
            }
        }
    }

    system("cls");

    cout << "\n====================================";
    cout << "\n        SUDDEN DEATH AKTIF";
    cout << "\n====================================";

    cout << "\nPower Card DESTROYED!";
    cout << "\nSetiap rotasi pemain dengan";
    cout << "\nkartu terbanyak akan dieliminasi.";

    cout << "\n\nTekan tombol apa saja...";
    _getch();
}

void eliminationHighest(player players[], int totalplayers)
{
    system("cls");
    int maxCard = -1;
    cout << "\n====================================\n";
    cout << "             SUDDEN DEATH!!             ";
    cout << "\n====================================\n";
    for (int i = 0; i < totalplayers; i++)
    {
        if (players[i].eliminated) continue;

        if (players[i].handSize > maxCard) maxCard = players[i].handSize;
    }

    for (int i = 0; i < totalplayers; i++)
    {
        if (players[i].eliminated == true) continue;

        if (players[i].handSize == maxCard) 
        {
            players[i].eliminated = true;

            cout << "\n" << players[i].name << " Tereliminasi!";
            Sleep(2000);
        }
    }
}

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
        else if (key == 'l' || key == 'L') {  // TANGKAP INPUT 'L' UNTUK LOG
            displayLog();
            continue; // Ulangi loop untuk memunculkan showHand lagi
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
    while (players[currentIdx].eliminated)
    {
        int step = isClockwise ? 1 : -1;

        currentIdx = (currentIdx + step + totalplayers) % totalplayers;
    }
    player* current = &players[currentIdx];
    bool skipNext = false; 

    // === TAMBAHKAN BLOK STACKING INI ===
    if (accumulatedPenalty > 0) {
        bool hasResponseCard = false;
        int targetIdx = -1;

        if (customStackingEnabled) {
            cardNode* checkNode = current->hand;
            int responseIdx = 0;
            while (checkNode) {
                if (checkNode->data.value == activePenaltyType) {
                    hasResponseCard = true;
                    targetIdx = responseIdx;
                    break;
                }
                responseIdx++;
                checkNode = checkNode->next;
            }
        }

        // --- BOT LOGIC ---
        if (current->isBot) {
            if (hasResponseCard) {
                card played = removeCard(current, targetIdx);
                topCard = played;
                if (played.color != "WILD") activeColor = played.color;
                else {
                    string botColors[] = {"RED", "YEL", "GRN", "BLU"};
                    activeColor = botColors[rand() % 4];
                }
                
                cout << "\n[STACK!] BOT " << current->name << " MENUMPUK dengan " << activePenaltyType << "!\n";
                handleActionCards(played, currentIdx, totalplayers, isClockwise, activeColor, deck, deckTop, players, skipNext);
            } else {
                cout << "\n[!] BOT " << current->name << " tidak bisa membalas! Menarik " << accumulatedPenalty << " kartu...\n";
                Sleep(2000);
                for (int i = 0; i < accumulatedPenalty; i++) addCard(current, drawFromDeck(deck, deckTop));
                accumulatedPenalty = 0; activePenaltyType = "";
            }
            
            // Geser giliran bot
            int step = isClockwise ? 1 : -1;
            currentIdx = (currentIdx + step + totalplayers) % totalplayers;
            return; 
        } 
        // --- HUMAN LOGIC ---
        else {
            while (true) {
                system("cls");
                cout << "====================================================\n";
                cout << " WARNING: KAMU TERKENA SERANGAN " << activePenaltyType << "!\n";
                cout << " Total kartu hukuman saat ini: " << accumulatedPenalty << " kartu.\n";
                cout << "====================================================\n\n";

                if (hasResponseCard) {
                    cout << "Kamu memiliki kartu " << activePenaltyType << " untuk ditumpuk (Stack)!\n";
                    cout << "[1] Tumpuk Kartu (Lempar ancaman ke pemain berikutnya)\n";
                    cout << "[2] Pasrah (Tarik " << accumulatedPenalty << " Kartu)\n";
                    cout << "[3] Lihat Movement Log\n"; // TAMBAHAN OPSI LOG
                    cout << "Pilihan: ";
                    int pil; cin >> pil;
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (pil == 1) {
                        card played = removeCard(current, targetIdx);
                        topCard = played;
                        if (played.color != "WILD") activeColor = played.color;
                        else {
                            cout << "\nPilih Warna Baru:\n1. RED\n2. YELLOW\n3. GREEN\n4. BLUE\nPilihan: ";
                            int cChoice; cin >> cChoice;
                            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            if (cChoice == 1) activeColor = "RED";
                            else if (cChoice == 2) activeColor = "YEL";
                            else if (cChoice == 3) activeColor = "GRN";
                            else activeColor = "BLU";
                        }
                        
                        handleActionCards(played, currentIdx, totalplayers, isClockwise, activeColor, deck, deckTop, players, skipNext);

                        int step = isClockwise ? 1 : -1;
                        currentIdx = (currentIdx + step + totalplayers) % totalplayers;
                        return;
                    } 
                    else if (pil == 3) {
                        displayLog();
                        continue; // Kembali untuk me-redraw layar warning
                    }
                    else if (pil == 2) {
                        break; // Keluar dari while untuk menarik kartu
                    }
                } else {
                    if (!customStackingEnabled) {
                        cout << "House Rule Stacking MATI! Kamu dilarang menumpuk kartu penalti.\n";
                    } else {
                        cout << "Kamu tidak memiliki kartu balasan " << activePenaltyType << " di tangan...\n";
                    }
                    cout << "[1] Pasrah (Tarik " << accumulatedPenalty << " Kartu)\n";
                    cout << "[2] Lihat Movement Log\n"; // TAMBAHAN OPSI LOG
                    cout << "Pilihan: ";
                    int pil; cin >> pil;
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    
                    if (pil == 2) {
                        displayLog();
                        continue;
                    }
                    break; // Keluar dari while untuk menarik kartu
                }
            } // end while

            // Jika pasrah / tidak bisa menumpuk
            for (int i = 0; i < accumulatedPenalty; i++) addCard(current, drawFromDeck(deck, deckTop));
            cout << "\n\nKamu menarik " << accumulatedPenalty << " kartu.\n";
            Sleep(1500);

            accumulatedPenalty = 0; activePenaltyType = "";

            int step = isClockwise ? 1 : -1;
            currentIdx = (currentIdx + step + totalplayers) % totalplayers;
            return;
        }
    }

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

                pushLog((*current).name, drawn, isClockwise, false, 0, "DRAW & PLAY");

                topCard = drawn;
                if (drawn.color != "WILD")
                    activeColor = drawn.color;

                handleActionCards(drawn, currentIdx, totalplayers,
                                  isClockwise, activeColor,
                                  deck, deckTop, players, skipNext);

                if ((*current).handSize == 1) {
                    cout << BOLD << YELLOW << "\nUNO!\n" << RESET;
                    if (moveLog.count > 0) { 
                        int lastIdx = (moveLog.head - 1 + MAX_LOG) % MAX_LOG;
                        if (moveLog.entries[lastIdx].extra.empty())
                            moveLog.entries[lastIdx].extra = "UNO!";
                        else
                            moveLog.entries[lastIdx].extra += " UNO!";
                    }
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
                pushLog((*current).name, {"---", "DRAW"}, isClockwise, false, 0, "DRAW CARD");
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
                pushLog((*current).name, drawn, isClockwise, false, 0, "DRAW & PLAY");
                
                topCard = drawn;

                if (drawn.color != "WILD")
                    activeColor = drawn.color;

                handleActionCards(drawn, currentIdx, totalplayers,
                                  isClockwise, activeColor,
                                  deck, deckTop, players, skipNext);
            }
            else {
                addCard(current, drawn);
                pushLog((*current).name, {"---", "DRAW"}, isClockwise, false, 0, "DRAW CARD");
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

    {
        string extra = "";
        bool isPen = false; int penCount = 0;
        if (played.value == "+2") { isPen = true; penCount = 2; extra = "+2 PENALTY"; }
        else if (played.value == "+4") { isPen = true; penCount = 4; extra = "+4 PENALTY"; }
        else if (played.value == "Skip")    extra = "SKIP";
        else if (played.value == "Reverse") extra = "REVERSE";
        else if (played.value == "Wild")    extra = "WILD";
        else if (played.value == "Swap")    extra = "SWAP";
        pushLog((*current).name, played, isClockwise, isPen, penCount, extra);
    }

    handleActionCards(played, currentIdx, totalplayers,
                      isClockwise, activeColor,
                      deck, deckTop, players, skipNext);

    if ((*current).handSize == 1) {
        cout << BOLD << YELLOW << "\nUNO!\n" << RESET;
        if (moveLog.count > 0) { 
            int lastIdx = (moveLog.head - 1 + MAX_LOG) % MAX_LOG;
            if (moveLog.entries[lastIdx].extra.empty())
                moveLog.entries[lastIdx].extra = "UNO!";
            else
                moveLog.entries[lastIdx].extra += " UNO!";
        }
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
string startGame(card deck[], int deckSize, int botAmount, int humanAmount, string namaAkun[])
{
    srand(time(0));
    shuffleDeck(deck, deckSize);

    int totalplayers = botAmount + humanAmount;
    if (totalplayers > 4) totalplayers = 4;

    player players[4];
    bool isClockwise = true; 
    suddenDeath = false;   
    playTurnCount = 0;
    rotationCounter = 0;

    moveLog.count = 0; 
    moveLog.head = 0;

    for (int i = 0; i < humanAmount; i++) {
        string inputNama;
        bool valid = false;
    }

    players[0].name = namaAkun[0];  
    players[0].isBot = false;
    players[0].hand = nullptr;
    players[0].handSize = 0;
    players[0].eliminated = false;

    // BIAR TIDAK ERROR: Inisialisasi otomatis jika pemain manusia lebih dari 1
    for (int i = 1; i < humanAmount; i++) {
        players[i].name = "Pemain " + to_string(i + 1);
        players[i].isBot = false;
        players[i].hand = nullptr;
        players[i].handSize = 0;
        players[i].eliminated = false;
    }

    for (int i = 0; i < botAmount; i++) {
        players[humanAmount + i].name = "Bot " + to_string(i + 1);
        players[humanAmount + i].isBot = true;
        players[humanAmount + i].hand = nullptr;
        players[humanAmount + i].handSize = 0;
        players[humanAmount + i].eliminated = false;
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
    
    cout << "\n2. Stacking Power Cards?\n";
    cout << "[1] Ya\n";
    cout << "[2] Tidak\n";
    cout << "Pilihan: ";
    int pilihanStacking; cin >> pilihanStacking;
    customStackingEnabled = (pilihanStacking == 1);
    
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    system("cls"); 
    
    int deckTop = deckSize - 1;
    for (int r = 0; r < customCardCount; r++) 
    for (int i = 0; i < totalplayers; i++)
    addCard(&players[i], drawFromDeck(deck, deckTop));
    
    card topCard;
    
    while (true) {
        if (deckTop < 0) break;
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
    int tempHandSize = players[0].handSize;
    string activeColor = topCard.color;
    int currentIdx = 0;
        
    while (true) 
    {
        int alive = 0;
        int winner = -1;

        for (int i = 0; i < totalplayers; i++) {
            if (!players[i].eliminated) {
                alive++;
                winner = i;
            }
        }

        if (alive <= 1) {
            system("cls");
            cout << "\n====================================\n";
            cout << "               GAME OVER";
            cout << "\n====================================\n";

            string finalWinner = "DRAW";
            if (winner != -1 && alive == 1) {
                cout << "\n" << players[winner].name << " adalah pemenangnya!\n";
                finalWinner = players[winner].name;
            } 
            else {
                cout << "\nSemua pemain tereliminasi! Game" << finalWinner << "\n";
            }

            _getch();
            for (int i = 0; i < totalplayers; i++) clearHand(&players[i]);
            return finalWinner;
        }

        while (players[currentIdx].eliminated) {
            int step = isClockwise ? 1 : -1;
            currentIdx = (currentIdx + step + totalplayers) % totalplayers;
        }

        playTurn(players, totalplayers, currentIdx, topCard, activeColor, deck, deckTop, isClockwise);
            
        playTurnCount++;
        int pengurangTurn = 5;
        int turnToSD =(totalplayers * 2) - playTurnCount;

    if (!suddenDeath &&
        turnToSD > 0 &&
        turnToSD <= 4)
    {
        cout << "\n====================================\n";
        cout << turnToSD
            << " Turn Lagi Sebelum Sudden Death!!";
        cout << "\n====================================\n";

        Sleep(1000);
    }

        if (!suddenDeath && playTurnCount >= totalplayers * 2)
        {
            startSuddenDeath(players,totalplayers);
        }

        if (suddenDeath)
        {
            rotationCounter++;

            if (rotationCounter >= totalplayers)
            {
                rotationCounter = 0;
                eliminationHighest(players, totalplayers);
            }
        }
            
        if (exitGame) {
            cout << "\n\nAkan keluar segera...";
            Sleep(1700);
            for (int i = 0; i < totalplayers; i++) clearHand(&players[i]);
            return "EXIT";
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
                for (int j = 0; j < totalplayers; j++) clearHand(&players[j]);
                return players[i].name; 
            }
        }
    }

    for (int i = 0; i < totalplayers; i++) clearHand(&players[i]);
    return "EXIT"; 
}