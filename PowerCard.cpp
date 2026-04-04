#include <iostream>
#include <string>
#include <conio.h>
#include <windows.h>

using namespace std;

// Struktur yang sama dengan game.cpp agar kompatibel
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

// Prototype fungsi pendukung dari game.cpp
void addCard(player* p, card c);
card drawFromDeck(card deck[], int &deckTop);
string getColor(string color);
string displayCard(card c);

/**
 * handleActionCards
 * Mengatur semua logika kartu sakti (Reverse, Skip, +2, +4, Wild, Swap).
 * Menggunakan Reference (&) agar perubahan nilai variabel di sini 
 * permanen pada saat kembali ke fungsi utama di game.cpp.
 */
void handleActionCards(card played, int &currentIdx, int totalPlayers, 
                       bool &isClockwise, string &activeColor, 
                       card deck[], int &deckTop, player players[],
                       bool &skipNext) 
{
    string val = played.value;
    int step = isClockwise ? 1 : -1;

    cout << "\nMengeluarkan kartu: " << displayCard(played) << " \n";

    // 1. REVERSE
    if (val == "Reverse") {
        if (totalPlayers == 2) {
            skipNext = true;
            cout << "\nHanya 2 pemain, giliran " << players[currentIdx].name << " dilewati!\n";
        } else {
        isClockwise = !isClockwise;
        cout << "\nArah putaran sekarang dibalik!\n";
        }
    }

    // 2. SKIP
    else if (val == "Skip") {
        skipNext = true; 
        cout << "\nPemain berikutnya akan dilewati!\n";
    }

    // 3. PLUS 2 (+2)
    else if (val == "+2") {
        int victimIdx = (currentIdx + step + totalPlayers) % totalPlayers;
        for (int i = 0; i < 2; i++) {
            addCard(&players[victimIdx], drawFromDeck(deck, deckTop));
        }
        skipNext = true; 
        cout << endl << players[victimIdx].name << " mengambil 2 kartu!\n";
    }

    // 4. WILD (REVISI: Ditambahkan pembersihan buffer dan feedback)
    else if (val == "Wild") {
        string choiceColor;
        if (players[currentIdx].isBot) {
            string colors[] = {"RED", "YEL", "GRN", "BLU"};
            choiceColor = colors[rand() % 4];
        } else {
            cout << "Pilih Warna Baru (R: Red, Y: Yellow, G: Green, B: Blue): ";
            // Membersihkan sisa input agar _getch() tidak terlewati
            while (_kbhit()) _getch(); 
            
            char choice = toupper(_getch());
            if (choice == 'R') choiceColor = "RED";
            else if (choice == 'Y') choiceColor = "YEL";
            else if (choice == 'G') choiceColor = "GRN";
            else choiceColor = "BLU";
        }
        activeColor = choiceColor; // Pastikan reference activeColor terupdate
        cout << "\n[!] Warna meja sekarang berubah menjadi: " << activeColor << " [!]\n";
    }

    // 5. PLUS 4 (+4) (REVISI: Logika pemilihan warna disamakan dengan Wild)
    else if (val == "+4") {
        string choiceColor;
        if (players[currentIdx].isBot) {
            string colors[] = {"RED", "YEL", "GRN", "BLU"};
            choiceColor = colors[rand() % 4];
        } else {
            cout << "Pilih Warna Baru untuk +4 (R/Y/G/B): ";
            while (_kbhit()) _getch();
            
            char choice = toupper(_getch());
            if (choice == 'R') choiceColor = "RED";
            else if (choice == 'Y') choiceColor = "YEL";
            else if (choice == 'G') choiceColor = "GRN";
            else choiceColor = "BLU";
        }
        activeColor = choiceColor;

        int victimIdx = (currentIdx + step + totalPlayers) % totalPlayers;
        for (int i = 0; i < 4; i++) {
            addCard(&players[victimIdx], drawFromDeck(deck, deckTop));
        }
        skipNext = true; 
        cout << "\n[!] Warna: " << activeColor << ". " << players[victimIdx].name << " ambil 4 kartu & dilewati!\n";
    }

    // 6. SWAP
    else if (val == "Swap") {
        int targetIdx = -1;
        if (players[currentIdx].isBot) {
            do {
                targetIdx = rand() % totalPlayers;
            } while (targetIdx == currentIdx);
        } else {
            cout << "\n--- PILIH TARGET SWAP ---\n";
            for (int i = 0; i < totalPlayers; i++) {
                if (i != currentIdx) {
                    cout << "[" << i << "] " << players[i].name << " (" << players[i].handSize << " kartu)\n";
                }
            }
            cout << "Masukkan ID Target: ";
            
            cin.clear(); 
            if (cin.peek() == '\n') cin.ignore(); 
            if (!(cin >> targetIdx)) {
                cin.clear();
                cin.ignore(1000, '\n');
            }
        }

        if (targetIdx >= 0 && targetIdx < totalPlayers && targetIdx != currentIdx) {
            cardNode* tempHand = players[currentIdx].hand;
            players[currentIdx].hand = players[targetIdx].hand;
            players[targetIdx].hand = tempHand;

            int tempSize = players[currentIdx].handSize;
            players[currentIdx].handSize = players[targetIdx].handSize;
            players[targetIdx].handSize = tempSize;

            cout << "\n[!] SWAP BERHASIL! Tangan Anda ditukar dengan " << players[targetIdx].name << "!\n";
        } else {
            cout << "\n[!] ID tidak valid atau memilih diri sendiri. Swap dibatalkan.\n";
        }
        cout << "Tekan tombol apa saja untuk lanjut...";
        _getch();
    }
    
    Sleep(1700); 
}