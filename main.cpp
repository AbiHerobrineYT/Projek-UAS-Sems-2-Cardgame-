#include <iostream>
#include <conio.h>
#include <windows.h>
#include "akun.h"
using namespace std;

struct card {
    string color;
    string value;
};

extern bool exitGame;

void createDeck(card deck[], int &deckSize);
string startGame(card deck[], int deckSize, int botAmount, int humanAmount, string humanNames[]);

void startMenu()
{
    cout << "\n────────────────── UNO GAME ───────────────────\n" << endl
         << "\n          Press any button to Play        \n" << endl
         << "\n          ESC to Exit...        " << endl
         << "\n───────────────────────────────────────────────\n" << endl;
}

Akun accounts[10];
int accountCount = 0;
int loggedInIndex = -1;

string inputText(string prompt) {
    string result = "";
    cout << prompt;
    char ch;
    while (true) {
        ch = _getch();
        if (ch == 13) {
            break;
        } else if (ch == 8) {
            if (!result.empty()) {
                result.pop_back();
                cout << "\b \b";
            }
        } else if (isprint(ch) && result.length() < 15) {
            result += ch;
            cout << ch;
        }
    }
    cout << endl;
    return result;
}

bool loginMenu() {
    accountCount = loadAkun(accounts, 10);
    int select = 1;

    while (true) {
        system("cls");
        cout << "\n──────────── LOGIN / REGISTER ────────────\n\n";
        cout << (select == 1 ? " --> " : "     ") << "Login\n\n";
        cout << (select == 2 ? " --> " : "     ") << "Register\n\n";
        cout << "  Tekan ESC untuk Keluar...\n";
        cout << "──────────────────────────────────────────\n";

        int key = _getch();
        if (key == 27) return false;

        if (key == 0 || key == 224) {
            int arrow = _getch();
            if (arrow == 72 && select > 1) select--;
            else if (arrow == 80 && select < 2) select++;
        } 
        else if (key == 13) {
            if (select == 1) {
                system("cls");
                cout << "\n──────────── LOGIN ────────────\n\n";
                string user = inputText("Username: ");
                string pass = inputText("Password: ");

                int idx = loginAkun(accounts, accountCount, user, pass);
                if (idx == -1) {
                    cout << "\nUsername atau password salah!\n";
                    Sleep(1500);
                } else {
                    loggedInIndex = idx;
                    cout << "\nSelamat datang, " << accounts[idx].username << "!\n";
                    cout << "Games: " << accounts[idx].games
                         << " | Wins: " << accounts[idx].wins
                         << " | Losses: " << accounts[idx].losses << "\n";
                    Sleep(1800);
                    return true;
                }
            }
            else if (select == 2) {
                system("cls");
                cout << "\n──────────── REGISTER ────────────\n\n";
                string user = inputText("Username baru: ");
                string pass = inputText("Password baru: ");

                bool ok = daftarAkun(accounts, accountCount, user, pass);
                if (!ok) {
                    cout << "\nUsername sudah dipakai!\n";
                } else {
                    cout << "\nAkun berhasil dibuat! Silakan login.\n";
                }
                Sleep(1500);
            }
        }
    }
}

void friendMenu() {
    if (loggedInIndex == -1) {
        cout << "\n[!] Fitur Teman hanya untuk akun terdaftar!\n"; Sleep(1500);
        return;
    }
    
    int select = 1; // Tracker untuk posisi arrow
    while (true) {
        system("cls");
        cout << "\n────────────── FRIEND MENU ──────────────\n\n";
        cout << (select == 1 ? " --> " : "     ") << "Daftar Teman Anda\n\n";
        cout << (select == 2 ? " --> " : "     ") << "Tambah Teman (Cari Akun)\n\n";
        cout << (select == 3 ? " --> " : "     ") << "Cek Permintaan Pertemanan\n\n";
        cout << "  Tekan ESC untuk Kembali...\n";
        cout << "─────────────────────────────────────────\n";
        
        int key = _getch();
        if (key == 27) break;
        
        if (key == 0 || key == 224) {
            int arrow = _getch();
            if (arrow == 72 && select > 1) select--;
            else if (arrow == 80 && select < 3) select++;
        }
        else if (key == 13) {
            if (select == 1) {
                system("cls");
                cout << "=== DAFTAR TEMAN ANDA ===\n\n";
                bool ada = false;
                int displayIndex = 1;
                for(int i = 0; i < accountCount; i++) {
                    if (friendGraph[loggedInIndex][i] == 1) {
                        cout << "[" << displayIndex << "] " << accounts[i].username << "\n";
                        ada = true;
                        displayIndex++;
                    }
                }
                if (!ada) cout << "Anda belum memiliki teman.\n";
                cout << "\nTekan tombol apa saja untuk kembali..."; _getch();
            }
            else if (select == 2) {
                system("cls");
                cout << "=== DAFTAR AKUN TERSEDIA ===\n\n";
                for(int i = 0; i < accountCount; i++) {
                    if (i != loggedInIndex) {
                        cout << "[" << (i + 1) << "] " << accounts[i].username;
                        if (friendGraph[loggedInIndex][i] == 1) cout << " (Berteman)";
                        else if (friendGraph[loggedInIndex][i] == 2) cout << " (Pending)";
                        cout << "\n";
                    }
                }
                cout << "\nMasukkan nomor akun untuk di-Add (atau -1 untuk batal): ";
                int t; cin >> t;
                t = t - 1;

                if (t >= 0 && t < accountCount && t != loggedInIndex && friendGraph[loggedInIndex][t] == 0) {
                    friendGraph[loggedInIndex][t] = 2;
                    simpanGraph();
                    cout << "\nPermintaan terkirim ke " << accounts[t].username << "!\n";
                }
                Sleep(1500);
            }
            else if (select == 3) {
                system("cls");
                cout << "=== PERMINTAAN TERTUNDA ===\n\n";
                bool ada = false;
                for(int i = 0; i < accountCount; i++) {
                    if (friendGraph[i][loggedInIndex] == 2) {
                        cout << "Akun: " << accounts[i].username << " ingin berteman dengan Anda.\n";
                        cout << "Terima? (Y/N): ";
                        char c = _getch(); cout << c << "\n";
                        if (c == 'y' || c == 'Y') {
                            friendGraph[i][loggedInIndex] = 1;
                            friendGraph[loggedInIndex][i] = 1;
                            simpanGraph();
                            cout << "Pertemanan diterima!\n";
                        } else {
                            friendGraph[i][loggedInIndex] = 0;
                            simpanGraph();
                            cout << "Ditolak.\n";
                        }
                        ada = true; Sleep(1000);
                    }
                }
                if (!ada) { cout << "Tidak ada permintaan.\n"; Sleep(1500); }
            }
        }
    }
}

void modeMenu(int select)
{
    cout << "\n───────────────── UNO GAME MENU ─────────────────\n" << endl;
    cout << (select == 1 ? " --> " : "     ") << "[ 1 ] Singleplayer (vs Bot)\n\n";
    cout << (select == 2 ? " --> " : "     ") << "[ 2 ] Multiplayer (Pass n Play)\n\n";
    cout << (select == 3 ? " --> " : "     ") << "[ 3 ] Friend List\n\n";
    cout << (select == 4 ? " --> " : "     ") << "[ 4 ] Logout\n\n";
    cout << "\n         Tekan ESC untuk Keluar Game...        " << endl;
    cout << "─────────────────────────────────────────────────\n" << endl;
}

void singleplayerBotMenu(int select)
{
    cout << "\n────────────── PILIH JUMLAH LAWAN (BOT) ──────────────\n" << endl;
    for (int i = 1; i <= 3; i++) {
        if (select == i) {
            cout << " -->       --- " << i << " Bot ---\n" << endl;
        } else {
            cout << "           --- " << i << " Bot ---\n" << endl;
        }
    }
    cout << "\n         Tekan ESC untuk Kembali...        " << endl;
}

void multiplayerBotMenu(int select, int maxBot)
{
    cout << "\n──────────────── TAMBAH BOT (OPSIONAL) ────────────────\n" << endl;
    for (int i = 0; i <= maxBot; i++) {
        if (select == i) {
            cout << " -->       --- " << i << " Bot ---\n" << endl;
        } else {
            cout << "           --- " << i << " Bot ---\n" << endl;
        }
    }
    cout << "\n         Tekan ESC untuk Kembali...        " << endl;
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    while (true)
    {
        bool loggedIn = loginMenu();
        if (!loggedIn) return 0;

        string currentAccountName = accounts[loggedInIndex].username;
        int input;
        int modeSelect = 1;
        bool sessionActive = true;

        while (sessionActive)
        {
            system("cls");
            cout << "\nUser Saat Ini: " << currentAccountName << "\n";
            modeMenu(modeSelect);

            input = _getch();

            if (input == 27)
                return 0;

            if (input == 0 || input == 224)
            {
                int arrow = _getch();
                if (arrow == 72 && modeSelect > 1)
                    modeSelect--;
                else if (arrow == 80 && modeSelect < 4)
                    modeSelect++;
            }
            else if (input == 13)
            {
                if (modeSelect == 4)
                {
                    loggedInIndex = -1;
                    sessionActive = false;
                    cout << "\nLogging out...\n";
                    Sleep(1000);
                }
                else if (modeSelect == 3) {
                    friendMenu();
                }
                else if (modeSelect == 1)
                {
                    int botSelect = 1;
                    bool backToMainMenu = false;

                    while (true)
                    {
                        system("cls");
                        singleplayerBotMenu(botSelect);

                        input = _getch();

                        if (input == 27) {
                            backToMainMenu = true;
                            break;
                        }

                        if (input == 0 || input == 224)
                        {
                            int arrow = _getch();
                            if (arrow == 72 && botSelect > 1)
                                botSelect--;
                            else if (arrow == 80 && botSelect < 3)
                                botSelect++;
                        }
                        else if (input == 13)
                        {
                            system("cls");
                            cout << "Memulai Singleplayer: 1 Pemain Manusia & " << botSelect << " Bot...\n";
                            Sleep(1500);

                            card deck[600];
                            int deckSize;

                            createDeck(deck, deckSize);

                            string pNames[4];
                            pNames[0] = currentAccountName;
                            
                            string pemenang = startGame(deck, deckSize, botSelect, 1, pNames);

                            if (pemenang != "EXIT") {
                                bool isMenang = (pemenang == currentAccountName);
                                updateStats(accounts, accountCount, currentAccountName, isMenang);
                            }
                            break;
                        }
                    }
                    if (backToMainMenu) continue;
                    if (exitGame) return 0;
                }

                else if (modeSelect == 2)
                {
                    int totalTeman = 0;
                    int daftarIdTeman[10]; 
                    for(int i = 0; i < accountCount; i++) {
                        if (friendGraph[loggedInIndex][i] == 1) {
                            daftarIdTeman[totalTeman] = i;
                            totalTeman++;
                        }
                    }

                    if (totalTeman < 1) {
                        cout << "\n[!] Anda butuh minimal 1 Teman (saling Add) untuk bermain Multiplayer!\n"; 
                        Sleep(2500);
                        continue;
                    }

                    int maxPlayersAllowed = totalTeman + 1;
                    if (maxPlayersAllowed > 4) maxPlayersAllowed = 4;

                    system("cls");
                    cout << "\n────────────── JUMLAH PEMAIN MANUSIA ──────────────\n\n";
                    cout << "Pilih jumlah pemain manusia (2 sampai " << maxPlayersAllowed << "): ";
                    int humanSelect;
                    cin >> humanSelect;
                    
                    if (humanSelect < 2 || humanSelect > maxPlayersAllowed) {
                        cout << "Input tidak valid!\n"; Sleep(1500); 
                        continue;
                    }

                    string pNames[4];
                    pNames[0] = currentAccountName;

                    for (int p = 1; p < humanSelect; p++) {
                        system("cls");
                        cout << "Invite Teman untuk Slot Player " << (p+1) << ":\n\n";
                        for(int i = 0; i < totalTeman; i++) {
                            cout << "[" << (i + 1) << "] " << accounts[daftarIdTeman[i]].username << "\n";
                        }
                        cout << "\nKetik angka teman yang ingin di-invite: ";
                        int pick; cin >> pick;
                        pick = pick - 1;
                        
                        if (pick >= 0 && pick < totalTeman) {
                            pNames[p] = accounts[daftarIdTeman[pick]].username;
                        } else {
                            cout << "Salah input, coba lagi!\n"; Sleep(1000);
                            p--;
                        }
                    }

                    int maxBotAllowed = 4 - humanSelect;
                    int botSelect = 0;
                    if (maxBotAllowed > 0)
                    {
                        while (true)
                        {
                            system("cls");
                            multiplayerBotMenu(botSelect, maxBotAllowed);
                            input = _getch();
                            if (input == 27) {
                                botSelect = -1;
                                break;
                            }
                            if (input == 0 || input == 224) {
                                int arrow = _getch();
                                if (arrow == 72 && botSelect > 0) botSelect--;
                                else if (arrow == 80 && botSelect < maxBotAllowed) botSelect++;
                            }
                            else if (input == 13) {
                                break;
                            }
                        }
                    }

                    if (botSelect == -1) continue;

                    system("cls");
                    cout << "Memulai Multiplayer: " << humanSelect << " Pemain & " << botSelect << " Bot!\n";
                    cout << "Pemain: \n";
                    for(int i = 0; i < humanSelect; i++) cout << "- " << pNames[i] << "\n";
                    Sleep(2000);

                    card deck[600];
                    int deckSize;
                    createDeck(deck, deckSize);
                    
                    string pemenang = startGame(deck, deckSize, botSelect, humanSelect, pNames);

                    if (pemenang != "EXIT") {
                        for (int i = 0; i < humanSelect; i++) {
                            bool isMenang = (pNames[i] == pemenang);
                            updateStats(accounts, accountCount, pNames[i], isMenang);
                        }
                    }

                    if (exitGame) return 0; // Langsung tutup game jika diperintahkan keluar
                }
            }
        }
    }

    return 0;
}