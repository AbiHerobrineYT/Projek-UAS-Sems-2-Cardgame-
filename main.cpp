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

int arrowSelect(const string items[], int count, const string &title) {
    int select = 0;
    while (true) {
        system("cls");
        cout << "\n" << title << "\n\n";
        for (int i = 0; i < count; i++) {
            cout << (select == i ? " --> " : "     ") << items[i] << "\n\n";
        }
        cout << "  Tekan ESC untuk Kembali...\n";
        cout << string(title.length() + 4, '-') << "\n";

        int key = _getch();
        if (key == 27) return -1;
        if (key == 0 || key == 224) {
            int arrow = _getch();
            if (arrow == 72 && select > 0) select--;
            else if (arrow == 80 && select < count - 1) select++;
        } else if (key == 13) {
            return select;
        }
    }
}

bool loginMenu() {
    accountCount = loadAkun(accounts, 10);
    int select = 0;

    while (true) {
        system("cls");
        cout << "\n──────────── LOGIN / REGISTER ────────────\n\n";
        cout << (select == 0 ? " --> " : "     ") << "Login\n\n";
        cout << (select == 1 ? " --> " : "     ") << "Register\n\n";
        cout << "  Tekan ESC untuk Keluar...\n\n";
        cout << "──────────────────────────────────────────\n";

        int key = _getch();
        if (key == 27) return false;

        if (key == 0 || key == 224) {
            int arrow = _getch();
            if (arrow == 72 && select > 0) select--;
            else if (arrow == 80 && select < 1) select++;
        }
        else if (key == 13) {
            if (select == 0) {
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
            else if (select == 1) {
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

    int select = 0;
    while (true) {
        system("cls");
        cout << "\n────────────── FRIEND MENU ──────────────\n\n";
        cout << (select == 0 ? " --> " : "     ") << "Daftar Teman Anda\n\n";
        cout << (select == 1 ? " --> " : "     ") << "Tambah Teman (Cari Akun)\n\n";
        cout << (select == 2 ? " --> " : "     ") << "Cek Permintaan Pertemanan\n\n";
        cout << "  Tekan ESC untuk Kembali...\n";
        cout << "─────────────────────────────────────────\n";

        int key = _getch();
        if (key == 27) break;

        if (key == 0 || key == 224) {
            int arrow = _getch();
            if (arrow == 72 && select > 0) select--;
            else if (arrow == 80 && select < 2) select++;
        }
        else if (key == 13) {
            if (select == 0) {
                system("cls");
                cout << "──── DAFTAR TEMAN ANDA ────\n\n";
                bool ada = false;
                for(int i = 0; i < accountCount; i++) {
                    if (friendGraph[loggedInIndex][i] == 1) {
                        cout << "  " << accounts[i].username << "\n";
                        ada = true;
                    }
                }
                if (!ada) cout << "Anda belum memiliki teman.\n";
                cout << "\nTekan tombol apa saja untuk kembali..."; _getch();
            }
            else if (select == 1) {
                // Buat list akun yang bisa di-add
                string namaAkun[10];
                int idxAkun[10];
                int total = 0;
                for(int i = 0; i < accountCount; i++) {
                    if (i != loggedInIndex) {
                        string label = accounts[i].username;
                        if (friendGraph[loggedInIndex][i] == 1) label += " (Berteman)";
                        else if (friendGraph[loggedInIndex][i] == 2) label += " (Pending)";
                        namaAkun[total] = label;
                        idxAkun[total] = i;
                        total++;
                    }
                }

                if (total == 0) {
                    system("cls");
                    cout << "\nTidak ada akun lain tersedia.\n";
                    Sleep(1500);
                    continue;
                }

                int pick = arrowSelect(namaAkun, total, "──── TAMBAH TEMAN ────");
                if (pick != -1) {
                    int t = idxAkun[pick];
                    if (friendGraph[loggedInIndex][t] == 0) {
                        friendGraph[loggedInIndex][t] = 2;
                        simpanGraph();
                        cout << "\nPermintaan terkirim ke " << accounts[t].username << "!\n";
                    } else {
                        cout << "\nSudah berteman atau permintaan sudah dikirim.\n";
                    }
                    Sleep(1500);
                }
            }
            else if (select == 2) {
                system("cls");
                cout << "──── PERMINTAAN TERTUNDA ────\n\n";
                bool ada = false;
                for(int i = 0; i < accountCount; i++) {
                    if (friendGraph[i][loggedInIndex] == 2) {
                        system("cls");
                        cout << "──── PERMINTAAN PERTEMANAN ────\n\n";
                        cout << accounts[i].username << " ingin berteman dengan Anda.\n\n";
                        cout << (1 == 1 ? "" : "");

                        string pilihanArr[2] = {"Terima", "Tolak"};
                        int pilihan = 0;
                        while (true) {
                            system("cls");
                            cout << "──── PERMINTAAN PERTEMANAN ────\n\n";
                            cout << accounts[i].username << " ingin berteman dengan Anda.\n\n";
                            cout << (pilihan == 0 ? " --> " : "     ") << "Terima\n\n";
                            cout << (pilihan == 1 ? " --> " : "     ") << "Tolak\n\n";
                            cout << "─────────────────────────────────────────\n";

                            int k = _getch();
                            if (k == 0 || k == 224) {
                                int arrow = _getch();
                                if (arrow == 72 && pilihan > 0) pilihan--;
                                else if (arrow == 80 && pilihan < 1) pilihan++;
                            } else if (k == 13) {
                                break;
                            }
                        }

                        if (pilihan == 0) {
                            friendGraph[i][loggedInIndex] = 1;
                            friendGraph[loggedInIndex][i] = 1;
                            simpanGraph();
                            cout << "Pertemanan diterima!\n";
                        } else {
                            friendGraph[i][loggedInIndex] = 0;
                            simpanGraph();
                            cout << "Ditolak.\n";
                        }
                        ada = true;
                        Sleep(1000);
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
    cout << (select == 0 ? " --> " : "     ") << "Singleplayer (vs Bot)\n\n";
    cout << (select == 1 ? " --> " : "     ") << "Multiplayer (Pass n Play)\n\n";
    cout << (select == 2 ? " --> " : "     ") << "Friend List\n\n";
    cout << (select == 3 ? " --> " : "     ") << "Logout\n\n";
    cout << "\n         Tekan ESC untuk Keluar Game...        " << endl;
    cout << "─────────────────────────────────────────────────\n" << endl;
}

void singleplayerBotMenu(int select)
{
    cout << "\n────────────── PILIH JUMLAH LAWAN (BOT) ──────────────\n" << endl;
    for (int i = 0; i < 3; i++) {
        cout << (select == i ? " -->       --- " : "           --- ") << (i + 1) << " Bot ---\n" << endl;
    }
    cout << "\n         Tekan ESC untuk Kembali...        " << endl;
}

void multiplayerBotMenu(int select, int maxBot)
{
    cout << "\n──────────────── TAMBAH BOT (OPSIONAL) ────────────────\n" << endl;
    for (int i = 0; i <= maxBot; i++) {
        cout << (select == i ? " -->       --- " : "           --- ") << i << " Bot ---\n" << endl;
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
        int modeSelect = 0;
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
                if (arrow == 72 && modeSelect > 0)
                    modeSelect--;
                else if (arrow == 80 && modeSelect < 3)
                    modeSelect++;
            }
            else if (input == 13)
            {
                if (modeSelect == 3)
                {
                    loggedInIndex = -1;
                    sessionActive = false;
                    cout << "\nLogging out...\n";
                    Sleep(1000);
                }
                else if (modeSelect == 2) {
                    friendMenu();
                }
                else if (modeSelect == 0)
                {
                    int botSelect = 0;
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
                            if (arrow == 72 && botSelect > 0)
                                botSelect--;
                            else if (arrow == 80 && botSelect < 2)
                                botSelect++;
                        }
                        else if (input == 13)
                        {
                            system("cls");
                            cout << "Memulai Singleplayer: 1 Pemain Manusia & " << (botSelect + 1) << " Bot...\n";
                            Sleep(1500);

                            card deck[600];
                            int deckSize;

                            createDeck(deck, deckSize);

                            string pNames[4];
                            pNames[0] = currentAccountName;

                            string pemenang = startGame(deck, deckSize, botSelect + 1, 1, pNames);

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

                else if (modeSelect == 1)
                {
                    // Kumpulkan daftar teman
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

                    // Pilih jumlah pemain manusia pakai arrow key
                    int jumlahOpsi = maxPlayersAllowed - 1; // opsi: 2..maxPlayersAllowed
                    string opsiPemain[3];
                    for (int i = 0; i < jumlahOpsi; i++) {
                        opsiPemain[i] = to_string(i + 2) + " Pemain Manusia";
                    }

                    int humanPick = arrowSelect(opsiPemain, jumlahOpsi, "────────── JUMLAH PEMAIN MANUSIA ──────────");
                    if (humanPick == -1) continue;
                    int humanSelect = humanPick + 2;

                    string pNames[4];
                    pNames[0] = currentAccountName;

                    // Pilih teman untuk setiap slot pakai arrow key
                    bool batalInvite = false;
                    for (int p = 1; p < humanSelect; p++) {
                        string namaOpsi[10];
                        int idxOpsi[10];
                        int totalOpsi = 0;
                        for(int i = 0; i < totalTeman; i++) {
                            // Hindari duplikat pemain yang sudah dipilih
                            bool sudahDipilih = false;
                            for (int x = 1; x < p; x++) {
                                if (pNames[x] == accounts[daftarIdTeman[i]].username) {
                                    sudahDipilih = true;
                                    break;
                                }
                            }
                            if (!sudahDipilih) {
                                namaOpsi[totalOpsi] = accounts[daftarIdTeman[i]].username;
                                idxOpsi[totalOpsi] = i;
                                totalOpsi++;
                            }
                        }

                        if (totalOpsi == 0) {
                            cout << "\nTidak cukup teman tersedia untuk slot ini.\n";
                            Sleep(1500);
                            batalInvite = true;
                            break;
                        }

                        string judulInvite = "Invite Teman untuk Slot Player " + to_string(p + 1);
                        int pick = arrowSelect(namaOpsi, totalOpsi, judulInvite);
                        if (pick == -1) { batalInvite = true; break; }
                        pNames[p] = namaOpsi[pick];
                    }

                    if (batalInvite) continue;

                    // Pilih jumlah bot pakai arrow key
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
                    cout << "Pemain:\n";
                    for(int i = 0; i < humanSelect; i++) cout << "  " << pNames[i] << "\n";
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

                    if (exitGame) return 0;
                }
            }
        }
    }

    return 0;
}