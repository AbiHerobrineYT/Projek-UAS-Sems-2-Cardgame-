#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <string>
#include <limits>
#include <cctype>
#include <vector>

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

bool isPowerCard(card c);
void handleActionCards(card played, int &currentIdx, int totalPlayers,
                       bool &isClockwise, string &activeColor,
                       card deck[], int &deckTop, player players[], bool &skipNext);
void startSuddenDeath(player players[], int totalplayers);
void eliminationHighest(player players[], int totalplayers);

bool exitGame = false;
int initialCardCount = 7;
int customCardCount = 0;
bool suddenDeath = false;
bool suddenDeathEnabled = true;
int rotationCount = 0;
int rotasiSebelumSD = 4;
int firstActivePlayer = 0;
int rotationCounter = 0;
bool customStackingEnabled = false;
bool drawUntilPlayableEnabled = false;
int accumulatedPenalty = 0;
string activePenaltyType = "";

enum InputAction {
    DRAW = -1,
    ESC  = -2
};

card drawFromDeck(card deck[], int &deckTop);
void addCard(player* p, card c);

void clearHand(player* p) {
    cardNode* current = p->hand;
    while (current != nullptr) {
        cardNode* nextNode = current->next;
        delete current;
        current = nextNode;
    }
    p->hand = nullptr;
    p->handSize = 0;
}

#define RESET        "\033[0m"
#define BOLD         "\033[1m"
#define RED          "\033[31m"
#define YELLOW       "\033[33m"
#define GREEN        "\033[32m"
#define BLUE         "\033[34m"
#define BRIGHT_CYAN  "\033[96m"

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
    if (isTopCard && c.color == "WILD" && c.value != "Swap")
        colorToUse = activeColor;
    return getColor(colorToUse) + BOLD + "| " + c.value + " |" + RESET;
}

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

char showDrawnCards(player* p, card drawn[], int count, bool isPenaltyDraw, bool isPlayable)
{
    if (p->isBot) return 'S';

    system("cls");
    cout << "      KAMU MENDAPATKAN\n"
         << "───────────────────────────────\n\n";

    for (int i = 0; i < count; i++) {
        Sleep(300);
        cout << "  + " << displayCard(drawn[i]) << "\n";
    }

    Sleep(500);
    cout << "\n───────────────────────────────\n";

    if (isPenaltyDraw) {
        cout << "\nTekan tombol apa saja untuk lanjut...";
        _getch();
        return 'S';
    }

    if (!isPlayable) {
        cout << "\nKartu tidak bisa dimainkan!\n";
        cout << "Tekan tombol apa saja untuk lanjut...";
        _getch();
        return 'S';
    }

    cout << "\n[ENTER] Gunakan kartu sekarang | [S] Simpan untuk nanti\n";
    while (true) {
        int key = _getch();
        if (key == 13)               return 'E';
        if (key == 's' || key == 'S') return 'S';
    }
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
    if (index < 0 || index >= p->handSize || !p->hand) return {"ERROR", "ERROR"};
    cardNode* temp = p->hand;
    for (int i = 0; i < index && temp; i++) temp = temp->next;
    return temp ? temp->data : card{"ERROR", "ERROR"};
}

void pushLog(const string& name, const card& c, bool cw,
             bool isPenalty = false, int penCount = 0, const string& extra = "") {
    logEntry e;
    e.playerName    = name;
    e.cardColor     = c.color;
    e.cardValue     = c.value;
    e.clockwise     = cw;
    e.isPenalty     = isPenalty;
    e.penaltyAmount = penCount;
    e.extra         = extra;
    moveLog.add(e);
}

void displayLog() {
    system("cls");
    cout << BRIGHT_CYAN << BOLD
         << "────────────────────────────────────────────────────\n"
         << "                 MOVEMENT LOG (Last " << moveLog.count << ")\n"
         << "────────────────────────────────────────────────────\n" << RESET;

    if (moveLog.count == 0) {
        cout << "\n  (belum ada aksi)\n\n";
    } else {
        cout << "\n";
        for (int i = 0; i < moveLog.count; i++) {
            logEntry e = moveLog.get(i);
            string dir       = e.clockwise ? "CW" : "CCW";
            string colorCode = getColor(e.cardColor);

            cout << "  " << (i + 1 == moveLog.count ? ">" : " ")
                 << " [" << dir << "] "
                 << BOLD << e.playerName << RESET << ": ";

            if (e.cardColor == "---")
                cout << BOLD << e.cardValue << RESET;
            else
                cout << colorCode << BOLD << e.cardColor << " " << e.cardValue << RESET;

            if (e.isPenalty && e.penaltyAmount > 0)
                cout << " → +" << e.penaltyAmount << " kartu";

            if (!e.extra.empty())
                cout << " (" << e.extra << ")";

            cout << "\n";
        }
        cout << "\n";
    }
    cout << BRIGHT_CYAN << BOLD
         << "────────────────────────────────────────────────────\n" << RESET
         << "Tekan tombol apa saja untuk kembali ke permainan...";
    _getch();
}

void showHand(player players[], int totalplayers, int currentIdx,
              card topCard, string activeColor,
              player* p, int selected, bool isClockwise)
{
    cout << BOLD << "Kartu paling atas:\n" << RESET
         << displayCard(topCard, true, activeColor) << "\n";

    cout << BOLD << "\nList pemain:" << RESET;
    for (int i = 0; i < totalplayers; i++) {
        cout << "\n" << (i == currentIdx ? "-> " : "   ")
             << players[i].name << " (" << players[i].handSize << " kartu)";
        if (suddenDeath && players[i].eliminated)
            cout << " [ELIMINATED]";
        if (!players[i].isBot && i == currentIdx)
            cout << " <-- Kamu";
    }

    cout << BRIGHT_CYAN << BOLD << "\n\n───────────────────────────────────────────────\n" << RESET;
    cout << "\nKartu " << BOLD << p->name << RESET << ":\n";
    cardNode* temp = p->hand;
    int i = 0;
    while (temp) {
        cout << (i == selected ? " --> " : "     ") << displayCard(temp->data) << "\n";
        temp = temp->next;
        i++;
    }
    cout << BRIGHT_CYAN << BOLD << "\n───────────────────────────────────────────────\n" << RESET
         << "\nKontrol:\n"
         << "[ARROW] Pilih | [ENTER] Main | [D] Draw | [L] Log | [ESC] Keluar\n";
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
    if (topCard.value == "Swap") return true;
    if (played.color == "WILD"  ||
        played.value == "Wild"  ||
        played.value == "+4"    ||
        played.value == "Swap")
        return true;
    return (played.color == activeColor || played.value == topCard.value);
}

int botChooseCard(player* bot, card topCard, string activeColor) {
    cardNode* temp = bot->hand;
    int i = 0;
    while (temp) {
        if (isValidPlay(temp->data, topCard, activeColor)) return i;
        temp = temp->next;
        i++;
    }
    return -1;
}

string pickColor(bool isBot) {
    if (isBot) {
        string colors[] = {"RED", "YEL", "GRN", "BLU"};
        return colors[rand() % 4];
    }

    const string colorNames[]  = {"RED", "YEL", "GRN", "BLU"};
    const string colorLabels[] = {"RED (Merah)", "YEL (Kuning)", "GRN (Hijau)", "BLU (Biru)"};
    int sel = 0;

    while (true) {
        system("cls");
        cout << "\n──────────── PILIH WARNA ────────────\n\n";
        for (int i = 0; i < 4; i++) {
            cout << (sel == i ? " --> " : "     ")
                 << getColor(colorNames[i]) << BOLD << colorLabels[i] << RESET << "\n\n";
        }
        cout << "─────────────────────────────────────\n";

        int key = _getch();
        if (key == 224 || key == 0) {
            int arrow = _getch();
            if (arrow == 72 && sel > 0) sel--;
            else if (arrow == 80 && sel < 3) sel++;
        } else if (key == 13) {
            return colorNames[sel];
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
        if (key == 27)                   return ESC;
        else if (key == 224 || key == 0) {
            int arrow = _getch();
            if (arrow == 72 && selected > 0)               selected--;
            if (arrow == 80 && selected < p->handSize - 1) selected++;
        }
        else if (key == 'w' || key == 'W') { if (selected > 0) selected--; }
        else if (key == 'l' || key == 'L') { displayLog(); }
        else if (key == 13) {
            card c = getCard(p, selected);
            if (!isValidPlay(c, topCard, activeColor)) {
                cout << RED << "\nKartu tidak cocok! Tekan tombol apa saja...\n" << RESET;
                Sleep(1000);
                continue;
            }
            return selected;
        }
        else if (key == 'd' || key == 'D') return DRAW;
    }
}

bool stackingMenu(bool hasResponseCard, int accPenalty, const string& penType,
                  player* current, int &targetIdx,
                  int currentIdx, int totalPlayers,
                  card deck[], int &deckTop,
                  card &topCard, string &activeColor,
                  bool &isClockwise, bool &skipNext,
                  player players[])
{
    while (true) {
        string opsi[3];
        int jumlahOpsi = 0;

        if (hasResponseCard) {
            opsi[jumlahOpsi++] = "Tumpuk Kartu " + penType + " (lempar ke pemain berikutnya)";
            opsi[jumlahOpsi++] = "Pasrah (Tarik " + to_string(accPenalty) + " Kartu)";
        } else {
            opsi[jumlahOpsi++] = "Pasrah (Tarik " + to_string(accPenalty) + " Kartu)";
        }
        opsi[jumlahOpsi++] = "Lihat Movement Log";

        int sel = 0;
        while (true) {
            system("cls");
            cout << "────────────────────────────────────────────────────\n"
                 << " WARNING: KAMU TERKENA SERANGAN " << penType << "!\n"
                 << " Total hukuman: " << accPenalty << " kartu\n"
                 << "────────────────────────────────────────────────────\n\n";

            if (!hasResponseCard) {
                if (!customStackingEnabled)
                    cout << "House Rule Stacking MATI. Kamu tidak bisa menumpuk.\n\n";
                else
                    cout << "Kamu tidak memiliki kartu " << penType << " untuk ditumpuk.\n\n";
            }

            for (int i = 0; i < jumlahOpsi; i++)
                cout << (sel == i ? " --> " : "     ") << opsi[i] << "\n\n";

            cout << "─────────────────────────────────────────\n";

            int key = _getch();
            if (key == 224 || key == 0) {
                int arrow = _getch();
                if (arrow == 72 && sel > 0) sel--;
                else if (arrow == 80 && sel < jumlahOpsi - 1) sel++;
            } else if (key == 13) {
                break;
            }
        }

        string chosen = opsi[sel];

        if (chosen.find("Lihat") != string::npos) {
            displayLog();
            continue;
        }

        if (chosen.find("Tumpuk") != string::npos) return true;

        return false;
    }
}

void houseRulesMenu(int totalplayers) {

    int defaultRotasi = max(4, 10 - totalplayers);

    customStackingEnabled    = false;
    suddenDeathEnabled       = true;
    drawUntilPlayableEnabled = false;
    rotasiSebelumSD          = defaultRotasi;
    customCardCount          = initialCardCount;

    const int RULE_COUNT = 5;

    const string ruleNames[RULE_COUNT] = {
        "Stacking Power Cards",
        "Sudden Death",
        "Rotasi Sebelum Sudden Death",
        "Draw Until Playable",
        "Jumlah Kartu Awal"
    };

    const string ruleDesc[RULE_COUNT] = {
        "Pemain bisa tumpuk +2/+4 ke pemain berikutnya",
        "Setelah N rotasi, pemain tereliminasi tiap ronde",
        "Jumlah rotasi penuh sebelum Sudden Death aktif",
        "Draw terus sampai dapat kartu yang bisa dimainkan",
        "Jumlah kartu awal per pemain (maks. 15)"
    };

    const bool isToggle[RULE_COUNT] = { true, true, false, true, false };

    bool* boolPtr[RULE_COUNT] = {
        &customStackingEnabled,
        &suddenDeathEnabled,
        nullptr,
        &drawUntilPlayableEnabled,
        nullptr
    };

    int* intPtr[RULE_COUNT] = {
        nullptr, nullptr,
        &rotasiSebelumSD,
        nullptr,
        &customCardCount
    };

    const int intMin[RULE_COUNT] = { 0, 0, 2, 0, 1  };
    const int intMax[RULE_COUNT] = { 0, 0, 20, 0, 15 };

    const string intPrompt[RULE_COUNT] = {
        "", "", "Jumlah rotasi (min 2, maks 20): ", "", "Jumlah kartu per pemain (1-15): "
    };

    int sel = 0;

    while (true) {
        system("cls");

        cout << BRIGHT_CYAN << BOLD
             << "\n──────────────────────────────────────────\n"
             << "               HOUSE RULES SETUP\n"
             << "──────────────────────────────────────────\n" << RESET
             << "\n  [↑↓] Pilih   [ENTER] Toggle/Edit   [ESC] Mulai Permainan\n"
             << "\n";

        for (int i = 0; i < RULE_COUNT; i++) {
            bool isSel = (i == sel);

            bool disabled = (i == 2 && !suddenDeathEnabled);

            if (isSel)
                cout << BRIGHT_CYAN << BOLD << " >";
            else
                cout << "  ";

            if (disabled)
                cout << RESET << "  " << "\033[90m" << ruleNames[i]; // abu-abu
            else
                cout << RESET << "  " << BOLD << ruleNames[i];

            cout << RESET;

            if (isToggle[i]) {
                bool val = *(boolPtr[i]);
                if (val)
                    cout << "  " << GREEN << BOLD << "[ ON  ]" << RESET;
                else
                    cout << "  " << RED   << BOLD << "[ OFF ]" << RESET;
            } else {
                if (disabled)
                    cout << "  " << "\033[90m" << "[ " << *(intPtr[i]) << " ]" << RESET;
                else
                    cout << "  " << YELLOW << BOLD << "[ " << *(intPtr[i]) << " ]" << RESET;
            }

            if (disabled)
                cout << "\n     " << "\033[90m" << ruleDesc[i] << RESET << "\n\n";
            else
                cout << "\n     " << ruleDesc[i] << "\n\n";
        }

        cout << BRIGHT_CYAN << BOLD
             << "──────────────────────────────────────────\n" << RESET
             << "  Tekan [ESC] untuk mulai permainan\n";

        int key = _getch();

        if (key == 27) break;

        if (key == 224 || key == 0) {
            int arrow = _getch();
            if (arrow == 72 && sel > 0)                sel--;
            else if (arrow == 80 && sel < RULE_COUNT - 1) sel++;
        }
        else if (key == 13) {
            if (sel == 2 && !suddenDeathEnabled) continue;

            if (isToggle[sel]) {
                *(boolPtr[sel]) = !(*(boolPtr[sel]));
            } else {
                system("cls");
                cout << "\n" << BOLD << ruleNames[sel] << RESET
                     << "\n" << ruleDesc[sel] << "\n\n"
                     << intPrompt[sel];

                int val;
                if (!(cin >> val)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    val = *(intPtr[sel]);
                } else {
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }

                if (val < intMin[sel]) val = intMin[sel];
                if (val > intMax[sel]) val = intMax[sel];
                *(intPtr[sel]) = val;
            }
        }
    }
}

void playTurn(player players[], int totalplayers, int &currentIdx,
              card &topCard, string &activeColor,
              card deck[], int &deckTop, bool &isClockwise)
{
    while (players[currentIdx].eliminated) {
        int step = isClockwise ? 1 : -1;
        currentIdx = (currentIdx + step + totalplayers) % totalplayers;
    }

    player* current = &players[currentIdx];
    bool skipNext   = false;

    if (accumulatedPenalty > 0) {
        bool hasResponseCard = false;
        int  responseIdx     = -1;

        if (customStackingEnabled) {
            cardNode* checkNode = current->hand;
            int idx = 0;
            while (checkNode) {
                if (checkNode->data.value == activePenaltyType) {
                    hasResponseCard = true;
                    responseIdx     = idx;
                    break;
                }
                idx++;
                checkNode = checkNode->next;
            }
        }

        if (current->isBot) {
            if (hasResponseCard) {
                card played = removeCard(current, responseIdx);
                topCard     = played;
                if (played.color != "WILD") activeColor = played.color;
                else activeColor = pickColor(true);

                cout << "\n[STACK!] BOT " << current->name
                     << " menumpuk dengan " << activePenaltyType << "!\n";
                Sleep(1500);

                handleActionCards(played, currentIdx, totalplayers,
                                  isClockwise, activeColor, deck, deckTop, players, skipNext);
            } else {
                cout << "\n[!] BOT " << current->name
                     << " tidak bisa membalas! Menarik " << accumulatedPenalty << " kartu...\n";
                Sleep(2000);
                for (int i = 0; i < accumulatedPenalty; i++)
                    addCard(current, drawFromDeck(deck, deckTop));
                accumulatedPenalty = 0;
                activePenaltyType  = "";
            }

            int step = isClockwise ? 1 : -1;
            currentIdx = (currentIdx + step + totalplayers) % totalplayers;
            return;
        }

        else {
            bool mauTumpuk = stackingMenu(
                hasResponseCard, accumulatedPenalty, activePenaltyType,
                current, responseIdx,
                currentIdx, totalplayers,
                deck, deckTop,
                topCard, activeColor,
                isClockwise, skipNext,
                players
            );

            if (mauTumpuk && hasResponseCard) {
                card played = removeCard(current, responseIdx);
                topCard     = played;
                if (played.color != "WILD") activeColor = played.color;
                else activeColor = pickColor(false);

                handleActionCards(played, currentIdx, totalplayers,
                                  isClockwise, activeColor, deck, deckTop, players, skipNext);

                int step = isClockwise ? 1 : -1;
                currentIdx = (currentIdx + step + totalplayers) % totalplayers;
                return;
            }

            cout << "\n\nKamu menarik " << accumulatedPenalty << " kartu.\n";
            for (int i = 0; i < accumulatedPenalty; i++)
                addCard(current, drawFromDeck(deck, deckTop));
            Sleep(1500);

            accumulatedPenalty = 0;
            activePenaltyType  = "";

            int step = isClockwise ? 1 : -1;
            currentIdx = (currentIdx + step + totalplayers) % totalplayers;
            return;
        }
    }

    int humanCount = 0;
    for (int i = 0; i < totalplayers; i++)
        if (!players[i].isBot) humanCount++;

    if (!current->isBot && humanCount > 1) {
        system("cls");
        cout << "\n───────────────────────────────────────────────\n"
             << "             GILIRAN: " << BOLD << current->name << RESET << "\n"
             << "───────────────────────────────────────────────\n"
             << "  Mohon oper perangkat ke " << BOLD << current->name << RESET << ".\n\n"
             << "  Tekan [ENTER] jika " << current->name << " sudah siap...";
        while (_getch() != 13);
    }

    system("cls");
    cout << "\nGiliran: " << BOLD << current->name << RESET
         << (current->isBot ? " (BOT)" : "") << "\n";

    int chosenIdx;

    if (current->isBot) {
        Sleep(1700);
        chosenIdx = botChooseCard(current, topCard, activeColor);

        if (chosenIdx == -1) {
            cout << "\n" << current->name << " menarik kartu...\n";
            Sleep(800);

            if (drawUntilPlayableEnabled) {
                while (true) {
                    card drawn = drawFromDeck(deck, deckTop);
                    addCard(current, drawn);
                    cout << current->name << " mendapatkan kartu...\n";
                    Sleep(600);
                    if (isValidPlay(drawn, topCard, activeColor)) {
                        cout << current->name << " langsung memainkan kartu!\n";
                        Sleep(800);
                        card toPlay = removeCard(current, current->handSize - 1);
                        pushLog(current->name, toPlay, isClockwise, false, 0, "DRAW & PLAY");
                        topCard = toPlay;
                        if (toPlay.color != "WILD") activeColor = toPlay.color;
                        handleActionCards(toPlay, currentIdx, totalplayers,
                                          isClockwise, activeColor, deck, deckTop, players, skipNext);

                        if (current->handSize == 1) {
                            cout << BOLD << YELLOW << "\nUNO!\n" << RESET;
                            if (moveLog.count > 0) {
                                int li = (moveLog.head - 1 + MAX_LOG) % MAX_LOG;
                                moveLog.entries[li].extra += (moveLog.entries[li].extra.empty() ? "" : " ") + string("UNO!");
                            }
                            Sleep(1000);
                        }
                        if (current->handSize == 0) return;
                        break;
                    }
                    if (deckTop < 0) {
                        pushLog(current->name, {"---", "DRAW"}, isClockwise, false, 0, "DRAW CARD");
                        cout << current->name << " tidak bisa main (deck habis).\n";
                        Sleep(1000);
                        break;
                    }
                }
            } else {
                card drawn = drawFromDeck(deck, deckTop);
                cout << current->name << " mendapatkan kartu...\n";
                Sleep(800);

                if (isValidPlay(drawn, topCard, activeColor)) {
                    cout << current->name << " langsung memainkan kartu!\n";
                    Sleep(800);
                    pushLog(current->name, drawn, isClockwise, false, 0, "DRAW & PLAY");
                    topCard = drawn;
                    if (drawn.color != "WILD") activeColor = drawn.color;
                    handleActionCards(drawn, currentIdx, totalplayers,
                                      isClockwise, activeColor, deck, deckTop, players, skipNext);

                    if (current->handSize == 1) {
                        cout << BOLD << YELLOW << "\nUNO!\n" << RESET;
                        if (moveLog.count > 0) {
                            int li = (moveLog.head - 1 + MAX_LOG) % MAX_LOG;
                            moveLog.entries[li].extra += (moveLog.entries[li].extra.empty() ? "" : " ") + string("UNO!");
                        }
                        Sleep(1000);
                    }
                    if (current->handSize == 0) return;
                } else {
                    addCard(current, drawn);
                    pushLog(current->name, {"---", "DRAW"}, isClockwise, false, 0, "DRAW CARD");
                    cout << current->name << " menyimpan kartu.\n";
                    Sleep(1000);
                }
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

        if (chosenIdx == ESC) { exitGame = true; return; }

        if (chosenIdx == DRAW) {

            if (drawUntilPlayableEnabled) {
                vector<card> drawnCards;
                bool playable = false;

                while (true) {
                    card drawn = drawFromDeck(deck, deckTop);
                    drawnCards.push_back(drawn);
                    addCard(current, drawn);

                    if (isValidPlay(drawn, topCard, activeColor)) {
                        playable = true;
                        break;
                    }
                    if (deckTop < 0) break;
                }

                system("cls");
                cout << "      KAMU MENDAPATKAN\n"
                     << "───────────────────────────────\n\n";
                for (int i = 0; i < (int)drawnCards.size(); i++) {
                    cout << "  + " << displayCard(drawnCards[i]) << "\n";
                    Sleep(200);
                }
                cout << "\n───────────────────────────────\n";

                if (playable) {
                    cout << "\nKartu terakhir bisa dimainkan!\n"
                         << "[ENTER] Mainkan  |  [S] Simpan\n";
                    while (true) {
                        int k = _getch();
                        if (k == 13) {
                            card toPlay = removeCard(current, current->handSize - 1);
                            pushLog(current->name, toPlay, isClockwise, false, 0, "DRAW & PLAY");
                            topCard = toPlay;
                            if (toPlay.color != "WILD") activeColor = toPlay.color;
                            handleActionCards(toPlay, currentIdx, totalplayers,
                                              isClockwise, activeColor, deck, deckTop, players, skipNext);
                            break;
                        }
                        if (k == 's' || k == 'S') {
                            pushLog(current->name, {"---", "DRAW"}, isClockwise, false, 0, "DRAW CARD");
                            break;
                        }
                    }
                } else {
                    cout << "\nDeck habis. Semua kartu disimpan.\n";
                    pushLog(current->name, {"---", "DRAW"}, isClockwise, false, 0, "DRAW CARD");
                    Sleep(1200);
                }

            } else {
                card drawn    = drawFromDeck(deck, deckTop);
                bool playable = isValidPlay(drawn, topCard, activeColor);
                char choice   = showDrawnCards(current, &drawn, 1, false, playable);

                if (choice == 'E' && playable) {
                    pushLog(current->name, drawn, isClockwise, false, 0, "DRAW & PLAY");
                    topCard = drawn;
                    if (drawn.color != "WILD") activeColor = drawn.color;
                    handleActionCards(drawn, currentIdx, totalplayers,
                                      isClockwise, activeColor, deck, deckTop, players, skipNext);
                } else {
                    addCard(current, drawn);
                    pushLog(current->name, {"---", "DRAW"}, isClockwise, false, 0, "DRAW CARD");
                }
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
    topCard     = played;
    if (played.color != "WILD") activeColor = played.color;

    {
        string extra = ""; bool isPen = false; int penCount = 0;
        if      (played.value == "+2")      { isPen = true; penCount = 2; extra = "+2 PENALTY"; }
        else if (played.value == "+4")      { isPen = true; penCount = 4; extra = "+4 PENALTY"; }
        else if (played.value == "Skip")    extra = "SKIP";
        else if (played.value == "Reverse") extra = "REVERSE";
        else if (played.value == "Wild")    extra = "WILD";
        else if (played.value == "Swap")    extra = "SWAP";
        pushLog(current->name, played, isClockwise, isPen, penCount, extra);
    }

    handleActionCards(played, currentIdx, totalplayers,
                      isClockwise, activeColor, deck, deckTop, players, skipNext);

    if (current->handSize == 1) {
        cout << BOLD << YELLOW << "\nUNO!\n" << RESET;
        if (moveLog.count > 0) {
            int li = (moveLog.head - 1 + MAX_LOG) % MAX_LOG;
            moveLog.entries[li].extra += (moveLog.entries[li].extra.empty() ? "" : " ") + string("UNO!");
        }
        Sleep(1000);
    }

    if (current->handSize == 0) return;

    int step = isClockwise ? 1 : -1;
    currentIdx = (currentIdx + step + totalplayers) % totalplayers;

    if (skipNext) {
        cout << "\n[!] Giliran " << players[currentIdx].name << " DILEWATI!\n";
        currentIdx = (currentIdx + step + totalplayers) % totalplayers;
        Sleep(1700);
    }
}

string startGame(card deck[], int deckSize, int botAmount, int humanAmount, string namaAkun[])
{
    srand(time(0));
    shuffleDeck(deck, deckSize);

    int totalplayers = botAmount + humanAmount;
    if (totalplayers > 4) totalplayers = 4;

    player players[4];
    bool isClockwise   = true;
    suddenDeath        = false;
    rotationCount      = 0;
    rotationCounter    = 0;
    accumulatedPenalty = 0;
    activePenaltyType  = "";

    moveLog.count = 0;
    moveLog.head  = 0;

    for (int i = 0; i < humanAmount; i++) {
        players[i].name       = namaAkun[i];
        players[i].isBot      = false;
        players[i].hand       = nullptr;
        players[i].handSize   = 0;
        players[i].eliminated = false;
    }

    for (int i = 0; i < botAmount; i++) {
        players[humanAmount + i].name       = "Bot " + to_string(i + 1);
        players[humanAmount + i].isBot      = true;
        players[humanAmount + i].hand       = nullptr;
        players[humanAmount + i].handSize   = 0;
        players[humanAmount + i].eliminated = false;
    }

    houseRulesMenu(totalplayers);

    int deckTop = deckSize - 1;
    for (int r = 0; r < customCardCount; r++)
        for (int i = 0; i < totalplayers; i++)
            addCard(&players[i], drawFromDeck(deck, deckTop));

    card topCard;
    while (true) {
        if (deckTop < 0) break;
        topCard = drawFromDeck(deck, deckTop);
        if (!isPowerCard(topCard)) break;
    }

    string activeColor  = topCard.color;
    int currentIdx      = 0;
    firstActivePlayer   = 0;

    while (true)
    {
        int alive = 0, winner = -1;
        for (int i = 0; i < totalplayers; i++) {
            if (!players[i].eliminated) { alive++; winner = i; }
        }

        if (alive <= 1) {
            system("cls");
            cout << "\n────────────────────────────────────\n"
                 << "               GAME OVER\n"
                 << "────────────────────────────────────\n\n";
            string finalWinner = "DRAW";
            if (winner != -1 && alive == 1) {
                cout << players[winner].name << " adalah pemenangnya!\n";
                finalWinner = players[winner].name;
            } else {
                cout << "Semua pemain tereliminasi! DRAW\n";
            }
            _getch();
            for (int i = 0; i < totalplayers; i++) clearHand(&players[i]);
            return finalWinner;
        }

        while (players[currentIdx].eliminated) {
            int step = isClockwise ? 1 : -1;
            currentIdx = (currentIdx + step + totalplayers) % totalplayers;
        }

        int idxSebelumTurn = currentIdx;

        playTurn(players, totalplayers, currentIdx, topCard, activeColor, deck, deckTop, isClockwise);

        if (suddenDeathEnabled && !suddenDeath && idxSebelumTurn == firstActivePlayer) {
            rotationCount++;

            int sisaRotasi = rotasiSebelumSD - rotationCount;
            if (sisaRotasi > 0 && sisaRotasi <= 3) {
                cout << "\n──────────────────────────────────\n"
                     << sisaRotasi << " Rotasi Lagi Sebelum Sudden Death!!\n"
                     << "────────────────────────────────────\n";
                Sleep(1200);
            }

            if (rotationCount >= rotasiSebelumSD) {
                startSuddenDeath(players, totalplayers);

                for (int i = 0; i < totalplayers; i++) {
                    if (!players[i].eliminated) {
                        firstActivePlayer = i;
                        break;
                    }
                }
            }
        }

        if (suddenDeath) {
            rotationCounter++;
            if (rotationCounter >= totalplayers) {
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
                cout << BRIGHT_CYAN << BOLD
                     << "───────────────────────────────────────────────\n" << RESET
                     << "               " << BOLD << YELLOW << "UNO GAME!!" << RESET << "\n"
                     << BRIGHT_CYAN << BOLD
                     << "───────────────────────────────────────────────\n" << RESET
                     << "\n\n          PEMENANG: " << players[i].name << "\n\n\n"
                     << "Tekan tombol apa saja untuk kembali ke menu...";
                _getch();
                for (int j = 0; j < totalplayers; j++) clearHand(&players[j]);
                return players[i].name;
            }
        }
    }

    for (int i = 0; i < totalplayers; i++) clearHand(&players[i]);
    return "EXIT";
}