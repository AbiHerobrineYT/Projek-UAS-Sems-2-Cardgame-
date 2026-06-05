#include "akun.h"
#include <fstream>
#include <sstream>

int friendGraph[10][10] = {0}; 

void loadGraph() {
    ifstream file("friends.csv");
    if (!file.is_open()) return;
    string line;
    int r = 0;
    while (getline(file, line) && r < 10) {
        stringstream ss(line);
        string token;
        int c = 0;
        while (getline(ss, token, ',') && c < 10) {
            friendGraph[r][c] = stoi(token);
            c++;
        }
        r++;
    }
    file.close();
}

void simpanGraph() {
    ofstream file("friends.csv");
    for (int r = 0; r < 10; r++) {
        for (int c = 0; c < 10; c++) {
            file << friendGraph[r][c] << (c == 9 ? "" : ",");
        }
        file << endl;
    }
    file.close();
}

int loadAkun(Akun akun[], int maxSize) {
    ifstream file("akun.csv");
    string line;
    int count = 0;

    if(getline(file, line)) {
        while (getline(file, line) && count < maxSize) {
            stringstream ss(line);
            string token; 
            getline(ss, akun[count].username, ',');
            getline(ss, akun[count].password, ',');
            
            string tmp;
            getline(ss, tmp, ','); akun[count].wins = stoi(tmp);
            getline(ss, tmp, ','); akun[count].losses = stoi(tmp);
            getline(ss, tmp, ','); akun[count].games = stoi(tmp);
            count++;
        }
    }
    file.close();
    
    loadGraph();
    return count;
}

void simpanAkun(Akun akun[], int count) {
    ofstream file("akun.csv");
    file << "username,password,wins,losses,games" << endl;
    for (int i = 0; i < count; i++) {
        file << akun[i].username << ","
             << akun[i].password << ","
             << akun[i].wins << ","
             << akun[i].losses << ","
             << akun[i].games << endl;
    }
    file.close();
    
    simpanGraph();
}

int loginAkun(Akun akun[], int count, string username, string password) {
    for (int i = 0; i< count; i++) {
        if (akun[i].username == username && akun[i].password == password) {
            return i;
        }
    }
    return -1;
}

bool daftarAkun(Akun akun[], int &count, string username, string password) {
    for (int i = 0; i < count; i++) {
        if (akun[i].username == username) return false;
    }
    
    akun[count].username = username;
    akun[count].password = password;
    akun[count].wins = 0;
    akun[count].losses = 0;
    akun[count].games = 0;

    for(int i=0; i<10; i++) {
        friendGraph[count][i] = 0;
        friendGraph[i][count] = 0;
    }
    
    count++;
    simpanAkun(akun, count);
    return true;
}

void updateStats(Akun akun[], int count, string username, bool menang) {
    for (int i = 0; i < count; i++) {
        if (akun[i].username == username) {
            akun[i].games++;
            if (menang) akun[i].wins++;
            else akun[i].losses++;
            simpanAkun(akun, count);
            return;
        }
    }
}