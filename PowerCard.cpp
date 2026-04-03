#include <iostream>
#include <string>
#include <conio.h>
#include <windows.h>

using namespace std;

// Struktur yang sama dengan game.cpp agar kompatibel
struct card {
    string color;
    string value;
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

/**
 * handleActionCards
 * Mengatur semua logika kartu sakti (Reverse, Skip, +2, +4, Wild, Swap).
 * Menggunakan Reference (&) agar perubahan nilai variabel di sini 
 * permanen pada saat kembali ke fungsi utama di game.cpp.
 */
void handleActionCards(card played, int &currentIdx, int totalPlayers, 
                       bool &isClockwise, string &activeColor, 
                       card deck[], int &deckTop, player players[]) 
{
    string val = played.value;
    int step = isClockwise ? 1 : -1;

    cout << "\n[!] MENGAKTIFKAN EFEK: " << val << " [!]\n";

    // 1. REVERSE: Membalikkan arah permainan
    if (val == "Reverse") {
        isClockwise = !isClockwise;
        cout << "Arah putaran sekarang dibalik!\n";
    }

    // 2. SKIP: Melompati giliran pemain berikutnya
    else if (val == "Skip") {
        currentIdx = (currentIdx + (isClockwise ? 1 : -1) + totalPlayers) % totalPlayers;
        cout << "Player " << players[currentIdx].name << " dilewati!\n";
    }

    // 3. PLUS 2 (+2): Korban ambil 2 kartu dan dilewati
    else if (val == "+2") {
        int victimIdx = (currentIdx + (isClockwise ? 1 : -1) + totalPlayers) % totalPlayers;
        for (int i = 0; i < 2; i++) {
            addCard(&players[victimIdx], drawFromDeck(deck, deckTop));
        }
        currentIdx = victimIdx; // Menetapkan index ke korban agar di main di-skip
        cout << players[victimIdx].name << " kena +2 dan giliran dilewati!\n";
    }

    // 4. WILD: Pemain aktif memilih warna baru
    else if (val == "Wild") {
        if (players[currentIdx].isBot) {
            string colors[] = {"RED", "YEL", "GRN", "BLU"};
            activeColor = colors[rand() % 4];
        } else {
            cout << "Pilih Warna Baru (R: Red, Y: Yellow, G: Green, B: Blue): ";
            char choice = toupper(_getch());
            if (choice == 'R') activeColor = "RED";
            else if (choice == 'Y') activeColor = "YEL";
            else if (choice == 'G') activeColor = "GRN";
            else activeColor = "BLU";
        }
        cout << "Warna meja sekarang: " << activeColor << "!\n";
    }

    // 5. PLUS 4 (+4): Pilih warna & korban ambil 4 kartu + dilewati
    else if (val == "+4") {
        if (players[currentIdx].isBot) {
            string colors[] = {"RED", "YEL", "GRN", "BLU"};
            activeColor = colors[rand() % 4];
        } else {
            cout << "Pilih Warna Baru untuk +4 (RED/YEL/GRN/BLU): ";
            cin >> activeColor;
        }

        int victimIdx = (currentIdx + (isClockwise ? 1 : -1) + totalPlayers) % totalPlayers;
        for (int i = 0; i < 4; i++) {
            addCard(&players[victimIdx], drawFromDeck(deck, deckTop));
        }
        currentIdx = victimIdx;
        cout << "Warna: " << activeColor << ". " << players[victimIdx].name << " kena +4 dan dilewati!\n";
    }

    // 6. SWAP: Menukar seluruh isi tangan dengan pemain lain
    else if (val == "Swap") {
        int targetIdx;
        if (players[currentIdx].isBot) {
            targetIdx = (currentIdx + 1) % totalPlayers;
        } else {
            cout << "Masukkan ID Player target (0-" << totalPlayers-1 << "): ";
            cin >> targetIdx;
        }

        if (targetIdx >= 0 && targetIdx < totalPlayers && targetIdx != currentIdx) {
            // TUKAR POINTER HAND (Materi Pointer & Linked List)
            cardNode* tempHand = players[currentIdx].hand;
            players[currentIdx].hand = players[targetIdx].hand;
            players[targetIdx].hand = tempHand;

            // Tukar data ukuran tangan
            int tempSize = players[currentIdx].handSize;
            players[currentIdx].handSize = players[targetIdx].handSize;
            players[targetIdx].handSize = tempSize;

            cout << "SWAP BERHASIL! Tangan Anda ditukar dengan " << players[targetIdx].name << "!\n";
        }
    }
    Sleep(1500); 
}