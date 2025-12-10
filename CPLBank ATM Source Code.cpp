#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <cctype>
#include <ctime>
#include <vector>
#include <limits>
using namespace std;

// golbal
string accountsFile = "Accounts.txt"; // all accounts stored here
int MAX_ACCOUNTS = 50; //array capacity.

// time
int currentAccountIndex = -1;    //whihc account from array is logged in.
// derived filenames will be set at login

// structure and array
struct Account {
    string cardNo;
    string pin;
    double balance;
};

//array for simplicity.
Account accountList[50];
int accountCount = 0; // how many accounts loaded into array

// mini statement in memory
vector<string> miniStatement;

// Fucntion Prototype
string CurrentDateTime();
void PauseAndReturn();
bool LoadAccounts();
void SaveAccounts(); // save full array back to Accounts.txt
int FindAccountIndex(const string &card);

// history or receipt
string HistoryFileFor(const string &card);
string ReceiptFileFor(const string &card);
void AddToHistoryFile(const string &card, const string &action);
void PrintReceiptFile(const string &card, const string &action, double amount);

// main features
bool Login();
void ShowMainMenu();
void Withdraw();
void Deposit();
void CheckBalance();
void ChangePIN();
void Transfer();
void ViewHistory();
void MobileTopUp();
void BillPayment();
void FastCash();

// Main
int main() {
    // load accounts into array
    if (!LoadAccounts()){
        // if cannot load, create the file with a default account
        ofstream create(accountsFile.c_str(), ios::out);
        if (create.is_open()){
            // default sample accounts (cardNo PIN balance)
            create << "1234567890 2203 10000.00" << endl;
            create << "9988776655 1111 5000.00" << endl;
            create << "1122334455 3333 7500.00" << endl;
            create.close();
            // try loading again
            LoadAccounts();
        }
    }

    // login first
    if (!Login()){
        // login failed or account locked
        return 0;
    }

    // main menu loop
    ShowMainMenu();

    // save all accounts before exiting
    SaveAccounts();

    return 0;
}

// ======================= FUNCTION DEFINITIONS =======================

// get current timestamp string
string CurrentDateTime(){
    time_t now = time(0);
    tm *lt = localtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
    return string(buf);
}

// pause wrapper
void PauseAndReturn(){
    cout << "\nPress any key to return to menu..." << endl;
    system("pause");
}

// LoadAccounts reads Accounts.txt and fills accountList[] with card and pin and balance
bool LoadAccounts(){
    ifstream fin(accountsFile.c_str());
    if (!fin.is_open()) return false;

    accountCount = 0;
    string c, p;
    double b;
    while (fin >> c >> p >> b){
        if (accountCount < MAX_ACCOUNTS){
            accountList[accountCount].cardNo = c;
            accountList[accountCount].pin = p;
            accountList[accountCount].balance = b;
            accountCount++;
        } else {
            // ignore extra lines beyond capacity
            break;
        }
    }
    fin.close();
    return true;
}

// SaveAccounts writes all accountList entries back to Accounts.txt
void SaveAccounts(){
    ofstream fout(accountsFile.c_str(), ios::out | ios::trunc);
    if (!fout.is_open()){
        cout << "Error: cannot save accounts file." << endl;
        return;
    }
    for (int i = 0; i < accountCount; ++i){
        fout << accountList[i].cardNo << " "
             << accountList[i].pin << " "
             << fixed << setprecision(2) << accountList[i].balance << endl;
    }
    fout.close();
}

// find index by card number, return -1 if not found
int FindAccountIndex(const string &card){
    for (int i = 0; i < accountCount; ++i){
        if (accountList[i].cardNo == card) return i;
    }
    return -1;
}

// file names helpers
string HistoryFileFor(const string &card){
    return card + "_history.txt";
}
string ReceiptFileFor(const string &card){
    return card + "_receipt.txt";
}

// append line to history file and keep mini-statement in memory
void AddToHistoryFile(const string &card, const string &action){
    string hist = HistoryFileFor(card);
    ofstream hout(hist.c_str(), ios::app);
    if (hout.is_open()){
        hout << "[" << CurrentDateTime() << "] " << action << endl;
        hout.close();
    }
    // keep in-memory small mini-statement
    miniStatement.push_back("[" + CurrentDateTime() + "] " + action);
    if (miniStatement.size() > 10) miniStatement.erase(miniStatement.begin());
}

// write a receipt entry for user
void PrintReceiptFile(const string &card, const string &action, double amount){
    string rfile = ReceiptFileFor(card);
    ofstream rout(rfile.c_str(), ios::app);
    if (rout.is_open()){
        rout << "===================================" << endl;
        rout << "          CPLBANK RECEIPT          " << endl;
        rout << "Date/Time: " << CurrentDateTime() << endl;
        rout << "Action: " << action << endl;
        rout << "Amount: RM " << fixed << setprecision(2) << amount << endl;
        rout << "Remaining Balance: RM " << fixed << setprecision(2) << accountList[currentAccountIndex].balance << endl;
        rout << "===================================" << endl << endl;
        rout.close();
    }
}

// login.
bool Login(){
    int attempts = 0;
    string enteredCard, enteredPin;

    while (attempts < 3){
        system("cls");
        cout << "===================================" << endl;
        cout << "         WELCOME TO CPLBANK        " << endl;
        cout << "===================================" << endl;
        cout << "Enter Card Number: ";
        cin >> enteredCard;
        cout << "Enter PIN: ";
        cin >> enteredPin;

        // basic format check (digits only)
        bool card_ok = true;
        for (char ch : enteredCard) if (!isdigit(ch)) card_ok = false;
        bool pin_ok = true;
        for (char ch : enteredPin) if (!isdigit(ch)) pin_ok = false;

        if (!card_ok || !pin_ok){
            cout << "\nInvalid format. Card and PIN must be digits only." << endl;
            attempts++;
            PauseAndReturn();
            continue;
        }

        int idx = FindAccountIndex(enteredCard);
        if (idx >= 0 && accountList[idx].pin == enteredPin){
            currentAccountIndex = idx;
            // clear statement for user and load
            miniStatement.clear();
            // set derived filenames if needed
            cout << "\nLogin successful!" << endl;
            AddToHistoryFile(accountList[currentAccountIndex].cardNo, "Login successful");
            PauseAndReturn();
            return true;
        } else {
            attempts++;
            cout << "\nWrong card or PIN. Attempts left: " << (3 - attempts) << endl;
            AddToHistoryFile(enteredCard, "Failed login attempt");
            PauseAndReturn();
        }
    }

    cout << "\nAccount locked after 3 wrong attempts. Please visit branch." << endl;
    return false;
}

// mian menu
void ShowMainMenu(){
    int choice = -1;
    while (true){
        system("cls");
        cout << "==========================================" << endl;
        cout << "           CPLBANK MAIN MENU              " << endl;
        cout << "==========================================" << endl;
        cout << " Logged in: " << accountList[currentAccountIndex].cardNo << endl;
        cout << " Balance: RM " << fixed << setprecision(2) << accountList[currentAccountIndex].balance << endl;
        cout << "------------------------------------------" << endl;
        cout << "1. Withdraw" << endl;
        cout << "2. Check Balance" << endl;
        cout << "3. Change PIN" << endl;
        cout << "4. Transfer Funds" << endl;
        cout << "5. Deposit Cash" << endl;
        cout << "6. Mobile Top-up" << endl;
        cout << "7. Bill Payment" << endl;
        cout << "8. Fast Cash" << endl;
        cout << "9. Transaction History / Mini-statement" << endl;
        cout << "0. Logout / Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        if (cin.fail()){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Enter numbers only." << endl;
            PauseAndReturn();
            continue;
        }

        switch (choice){
            case 1: Withdraw(); break;
            case 2: CheckBalance(); break;
            case 3: ChangePIN(); break;
            case 4: Transfer(); break;
            case 5: Deposit(); break;
            case 6: MobileTopUp(); break;
            case 7: BillPayment(); break;
            case 8: FastCash(); break;
            case 9: ViewHistory(); break;
            case 0:
                // save and exit to login / program end
                SaveAccounts();
                cout << "Logging out... Thank you for using CPLBANK." << endl;
                PauseAndReturn();
                return;
            default:
                cout << "Invalid choice. Try again." << endl;
                PauseAndReturn();
        }
    }
}

// TRANSACTION

void Withdraw(){
    system("cls");
    cout << "===== WITHDRAW MONEY =====" << endl;
    cout << "Enter amount to withdraw (RM): ";
    double amount;
    cin >> amount;
    if (cin.fail()){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input." << endl; PauseAndReturn(); return; }

    if (amount <= 0){
        cout << "Invalid amount!" << endl;
        PauseAndReturn();
        return;
    }
    if (amount > accountList[currentAccountIndex].balance - 10.0){
        cout << "Insufficient balance! Keep at least RM10." << endl;
        PauseAndReturn();
        return;
    }

    // dispense logic (simple)
    int remaining = (int) amount;
    int n100 = remaining / 100; remaining %= 100;
    int n50  = remaining / 50;  remaining %= 50;
    int n10  = remaining / 10;  remaining %= 10;
    if (remaining != 0){
        cout << "ATM can only dispense multiples of RM10. Enter multiples of 10." << endl;
        PauseAndReturn();
        return;
    }

    accountList[currentAccountIndex].balance -= amount;
    SaveAccounts();
    AddToHistoryFile(accountList[currentAccountIndex].cardNo, "Withdraw RM " + to_string((int)amount));
    PrintReceiptFile(accountList[currentAccountIndex].cardNo, "Withdraw", amount);

    cout << "Withdrawal successful!" << endl;
    cout << "Dispensed: " << n100 << " x RM100, " << n50 << " x RM50, " << n10 << " x RM10" << endl;
    cout << "Remaining Balance: RM " << fixed << setprecision(2) << accountList[currentAccountIndex].balance << endl;
    PauseAndReturn();
}

void Deposit(){
    system("cls");
    cout << "===== DEPOSIT MONEY =====" << endl;
    cout << "Enter amount to deposit (RM): ";
    double amount;
    cin >> amount;
    if (cin.fail()){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input." << endl; PauseAndReturn(); return; }

    if (amount <= 0){
        cout << "Invalid amount!" << endl;
        PauseAndReturn();
        return;
    }

    accountList[currentAccountIndex].balance += amount;
    SaveAccounts();
    AddToHistoryFile(accountList[currentAccountIndex].cardNo, "Deposit RM " + to_string((int)amount));
    PrintReceiptFile(accountList[currentAccountIndex].cardNo, "Deposit", amount);

    cout << "Deposit successful!" << endl;
    cout << "New Balance: RM " << fixed << setprecision(2) << accountList[currentAccountIndex].balance << endl;
    PauseAndReturn();
}

void CheckBalance(){
    system("cls");
    cout << "=========== ACCOUNT BALANCE ===========" << endl;
    cout << "Card No: " << accountList[currentAccountIndex].cardNo << endl;
    cout << "Available Balance: RM " << fixed << setprecision(2) << accountList[currentAccountIndex].balance << endl;
    cout << "=======================================" << endl;
    AddToHistoryFile(accountList[currentAccountIndex].cardNo, "Balance Check");
    PauseAndReturn();
}

void ChangePIN(){
    system("cls");
    cout << "========== CHANGE PIN ==========" << endl;
    string newPin, confirm;
    cout << "Enter new 4-digit PIN: ";
    cin >> newPin;
    if (newPin.length() != 4){ cout << "PIN must be 4 digits. Aborted." << endl; PauseAndReturn(); return; }
    for (char ch : newPin) if (!isdigit(ch)){ cout << "PIN must be numeric. Aborted." << endl; PauseAndReturn(); return; }

    cout << "Confirm new PIN: ";
    cin >> confirm;
    if (newPin != confirm){ cout << "PINs do not match. Aborted." << endl; PauseAndReturn(); return; }

    accountList[currentAccountIndex].pin = newPin;
    SaveAccounts();
    AddToHistoryFile(accountList[currentAccountIndex].cardNo, "PIN Changed");
    PrintReceiptFile(accountList[currentAccountIndex].cardNo, "Change PIN", 0.0);

    cout << "PIN changed successfully." << endl;
    PauseAndReturn();
}

void Transfer(){
    system("cls");
    cout << "============= TRANSFER MONEY =============" << endl;
    cout << "Enter recipient account number: ";
    string dest;
    cin >> dest;
    int destIdx = FindAccountIndex(dest);
    if (destIdx < 0){
        cout << "Recipient not found." << endl;
        PauseAndReturn();
        return;
    }

    cout << "Enter amount to transfer (RM): ";
    double amount; cin >> amount;
    if (cin.fail()){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input." << endl; PauseAndReturn(); return; }

    if (amount <= 0){
        cout << "Invalid amount!" << endl; PauseAndReturn(); return;
    }
    if (amount > accountList[currentAccountIndex].balance - 10.0){
        cout << "Insufficient balance! Keep at least RM10." << endl; PauseAndReturn(); return;
    }

    accountList[currentAccountIndex].balance -= amount;
    accountList[destIdx].balance += amount;
    SaveAccounts();

    AddToHistoryFile(accountList[currentAccountIndex].cardNo, "Transfer to " + dest + " RM " + to_string((int)amount));
    PrintReceiptFile(accountList[currentAccountIndex].cardNo, "Transfer", amount);
    // also add history to recipient file
    AddToHistoryFile(accountList[destIdx].cardNo, "Received RM " + to_string((int)amount) + " from " + accountList[currentAccountIndex].cardNo);

    cout << "Transfer successful! Remaining Balance: RM " << fixed << setprecision(2) << accountList[currentAccountIndex].balance << endl;
    PauseAndReturn();
}

void ViewHistory(){
    system("cls");
    cout << "=========== TRANSACTION HISTORY (mini) ===========" << endl;
    if (miniStatement.empty()){
        cout << "No recent transactions." << endl;
    } else {
        for (size_t i = 0; i < miniStatement.size(); ++i){
            cout << miniStatement[i] << endl;
        }
    }

    cout << "\n-- Full history from file --" << endl;
    string hist = HistoryFileFor(accountList[currentAccountIndex].cardNo);
    ifstream hin(hist.c_str());
    if (!hin.is_open()){
        cout << "No history file yet." << endl;
    } else {
        string line;
        while (getline(hin, line)) cout << line << endl;
        hin.close();
    }
    cout << "==============================================" << endl;
    PauseAndReturn();
}

// simple mobile top-up
void MobileTopUp(){
    system("cls");
    cout << "========= MOBILE TOP-UP =========" << endl;
    cout << "Enter mobile number (without +60): ";
    string mobile; cin >> mobile;
    cout << "Enter amount (RM): ";
    double amt; cin >> amt;
    if (cin.fail()){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid." << endl; PauseAndReturn(); return; }
    if (amt <= 0){ cout << "Invalid amount." << endl; PauseAndReturn(); return; }
    if (amt > accountList[currentAccountIndex].balance - 10.0){ cout << "Not enough balance." << endl; PauseAndReturn(); return; }

    accountList[currentAccountIndex].balance -= amt;
    SaveAccounts();
    AddToHistoryFile(accountList[currentAccountIndex].cardNo, "MobileTopUp " + mobile + " RM " + to_string((int)amt));
    PrintReceiptFile(accountList[currentAccountIndex].cardNo, "Mobile Top-up", amt);

    cout << "Top-up successful. New balance: RM " << fixed << setprecision(2) << accountList[currentAccountIndex].balance << endl;
    PauseAndReturn();
}

// simple bill payment
void BillPayment(){
    system("cls");
    cout << "========= BILL PAYMENT =========" << endl;
    cout << "1. TNB (Electricity)\n2. Water\n0. Cancel\nChoose: ";
    int sel; cin >> sel;
    if (sel == 0) return;
    cout << "Enter reference: ";
    string ref; cin >> ref;
    cout << "Enter amount (RM): ";
    double amt; cin >> amt;
    if (cin.fail()){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid." << endl; PauseAndReturn(); return; }
    if (amt <= 0){ cout << "Invalid amount." << endl; PauseAndReturn(); return; }
    if (amt > accountList[currentAccountIndex].balance - 10.0){ cout << "Not enough balance." << endl; PauseAndReturn(); return; }

    accountList[currentAccountIndex].balance -= amt;
    SaveAccounts();
    AddToHistoryFile(accountList[currentAccountIndex].cardNo, "BillPayment ref:" + ref + " RM " + to_string((int)amt));
    PrintReceiptFile(accountList[currentAccountIndex].cardNo, "Bill Payment", amt);

    cout << "Bill paid. New balance: RM " << fixed << setprecision(2) << accountList[currentAccountIndex].balance << endl;
    PauseAndReturn();
}

void FastCash(){
    system("cls");
    cout << "========= FAST CASH =========" << endl;
    cout << "1. RM100\n2. RM300\n3. RM500\n0. Cancel\nChoose: ";
    int sel; cin >> sel;
    double amt = 0;
    if (sel == 1) amt = 100;
    else if (sel == 2) amt = 300;
    else if (sel == 3) amt = 500;
    else return;

    if (amt > accountList[currentAccountIndex].balance - 10.0){
        cout << "Not enough balance." << endl; PauseAndReturn(); return;
    }

    accountList[currentAccountIndex].balance -= amt;
    SaveAccounts();
    AddToHistoryFile(accountList[currentAccountIndex].cardNo, "FastCash RM " + to_string((int)amt));
    PrintReceiptFile(accountList[currentAccountIndex].cardNo, "Fast Cash", amt);

    cout << "Fast cash dispensed. New balance: RM " << fixed << setprecision(2) << accountList[currentAccountIndex].balance << endl;
    PauseAndReturn();
}

//program finally finished.
