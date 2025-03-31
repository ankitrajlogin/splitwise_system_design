#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <cmath>
#include <string>
#include <stdexcept>
#include <queue>
#include <tuple>
#include <algorithm>

using namespace std;

// Utility: Round a double to two decimal places.
double roundTwoDecimals(double value) {
    return round(value * 100.0) / 100.0;
}

// Structure for a User.
struct User {
    string userId;
    string name;
    string email;
    string mobile;
    
    User() {}
    User(const string &id, const string &n, const string &e, const string &m)
        : userId(id), name(n), email(e), mobile(m) {}

public: 
    string getName(){
        return name ; 
    }

    string getEmail(){
        return email ; 
    }

    string getMobileNumber(){
        return mobile ; 
    }

    string getUserId(){
        return userId ; 
    }

    
};

// Group class: Each group has a name, a list of members (user IDs),
// and a balances map that holds pairwise balances among its members.
class Group {
private:
    string groupName;
    vector<string> members; // List of user IDs in this group.
    // Balances: balances[A][B] means B owes A an amount within the group.
    unordered_map<string, unordered_map<string, double>> balances;
    
    // Helper: Update balance for a transaction in the group.
    // When a payer pays on behalf of user 'u', then u owes payer the share.
    void updateBalance(const string &payer, const string &u, double share) {
        if (u == payer) return; // No self-transaction.
        balances[payer][u] = roundTwoDecimals(balances[payer][u] + share);
        balances[u][payer] = roundTwoDecimals(balances[u][payer] - share);
    }
    
    // Heap-based simplifyTransactions:
    // Computes net balance for each member, then uses two heaps (max-heaps)
    // to settle amounts. The heaps hold pairs: (amount, userId).
    // For creditors, amount is positive; for debtors, we store the absolute debt.
    // Returns a vector of transactions as tuples (debtor, creditor, amount)
    vector<tuple<string, string, double>> simplifyTransactions() {
        // Compute net balance for each member.
        unordered_map<string, double> net;
        for (const auto &u : members) {
            net[u] = 0.0;
        }
        for (auto &p : balances) {
            string userId = p.first;
            for (auto &q : p.second) {
                net[userId] += q.second;
            }
        }
        
        // Define comparators for max-heaps.
        auto cmp = [](const pair<string, double>& a, const pair<string, double>& b) {
            return a.second < b.second; // largest first
        };
        // Heaps for creditors and debtors.
        priority_queue<pair<string, double>, vector<pair<string, double>>, decltype(cmp)> creditors(cmp);
        priority_queue<pair<string, double>, vector<pair<string, double>>, decltype(cmp)> debtors(cmp);
        
        // Populate heaps.
        for (auto &p : net) {
            if (p.second > 0.001) { // creditor: should receive money
                creditors.push({p.first, p.second});
            } else if (p.second < -0.001) { // debtor: owes money (store as positive)
                debtors.push({p.first, -p.second});
            }
        }
        
        vector<tuple<string, string, double>> simplified;
        // While both heaps have members.
        while (!creditors.empty() && !debtors.empty()) {
            auto cred = creditors.top(); creditors.pop();
            auto debt = debtors.top(); debtors.pop();
            
            double settleAmount = min(cred.second, debt.second);
            simplified.push_back(make_tuple(debt.first, cred.first, settleAmount));
            
            cred.second -= settleAmount;
            debt.second -= settleAmount;
            
            if (cred.second > 0.001)
                creditors.push(cred);
            if (debt.second > 0.001)
                debtors.push(debt);
        }
        
        // Update internal balances with only these simplified transactions.
        for (const auto &u : members) {
            balances[u].clear();
        }
        for (auto &tx : simplified) {
            string debtor, creditor;
            double amount;
            tie(debtor, creditor, amount) = tx;
            balances[creditor][debtor] = roundTwoDecimals(amount);
            balances[debtor][creditor] = roundTwoDecimals(-amount);
        }
        
        return simplified;
    }
    
public:
    // Default constructor needed for unordered_map.
    Group() { }
    
    // Parameterized constructor.
    Group(const string &name, const vector<string> &mems) : groupName(name), members(mems) {
        // Initialize the balances map for each member.
        for (const auto &u : members) {
            balances[u] = unordered_map<string, double>();
        }
    }
    
    // Check if a user is a member of the group.
    bool isMember(const string &userId) const {
        return find(members.begin(), members.end(), userId) != members.end();
    }



    void addMember(const string &newUserId) {
        // Check if the user is already a member
        if (find(members.begin(), members.end(), newUserId) != members.end()) {
            throw invalid_argument("User " + newUserId + " is already a member of the group " + groupName);
        }

        // Add the new member to the members list
        members.push_back(newUserId);

        // Initialize balances for the new member with existing members
        balances[newUserId] = unordered_map<string, double>();
        for (const auto &member : members) {
            if (member != newUserId) {
                balances[newUserId][member] = 0.0;
                balances[member][newUserId] = 0.0;
            }
        }
    }
    
    // Process an expense within the group.
    // Parameters:
    //   payer: user id of the person who paid.
    //   amount: total amount paid.
    //   involved: vector of user ids (should be a subset of group members).
    //   type: "EQUAL", "EXACT", or "PERCENT"
    //   splits: corresponding split values (for EXACT/PERCENT).
    void addExpense(const string &payer, double amount, const vector<string> &involved, 
                    const string &type, const vector<double> &splits) {
        int n = involved.size();
        // Ensure all involved users are members of this group.
        for (const auto &u : involved) {
            if (!isMember(u))
                throw invalid_argument("User " + u + " not exists in group " + groupName);
        }
        if (type == "EQUAL") {
            double share = roundTwoDecimals(amount / n);
            double totalForOthers = share * n;
            double diff = roundTwoDecimals(amount - totalForOthers);
            bool firstNonPayer = true;
            for (const auto &userId : involved) {
                if (userId == payer) continue;
                double thisShare = share;
                if (firstNonPayer) {
                    thisShare = roundTwoDecimals(share + diff);
                    firstNonPayer = false;
                }
                updateBalance(payer, userId, thisShare);
            }
        }
        else if (type == "EXACT") {
            if (splits.size() != involved.size()) {
                throw invalid_argument("Number of split amounts does not match number of users involved.");
            }
            double sum = 0;
            for (double s : splits) {
                sum += s;
            }
            if (roundTwoDecimals(sum) != roundTwoDecimals(amount)) {
                throw invalid_argument("Exact split amounts do not sum up to total amount.");
            }
            for (int i = 0; i < n; i++) {
                string userId = involved[i];
                double share = splits[i];
                if (userId == payer) continue;
                updateBalance(payer, userId, share);
            }
        }
        else if (type == "PERCENT") {
            if (splits.size() != involved.size()) {
                throw invalid_argument("Number of percentage values does not match number of users involved.");
            }
            double percentSum = 0;
            for (double p : splits) {
                percentSum += p;
            }
            if (fabs(percentSum - 100.0) > 0.001) {
                throw invalid_argument("Percentages do not sum up to 100.");
            }
            for (int i = 0; i < n; i++) {
                string userId = involved[i];
                double share = roundTwoDecimals(amount * splits[i] / 100.0);
                if (userId == payer) continue;
                updateBalance(payer, userId, share);
            }
        }
        else {
            throw invalid_argument("Invalid expense type.");
        }
    }
    
    // Show simplified balances for the group.
    // This function recalculates, simplifies the debts (using heaps), updates the internal balances map,
    // and then displays the transactions.
    void showAllBalances() {
        auto simplified = simplifyTransactions();
        if (simplified.empty()) {
            cout << "No balances" << endl;
            return;
        }
        for (auto &tx : simplified) {
            string debtor, creditor;
            double amount;
            tie(debtor, creditor, amount) = tx;
            cout << debtor << " owes " << creditor << ": " 
                 << fixed << setprecision(2) << amount << endl;
        }
    }
    
    // Show simplified balances for a single member in the group.
    void showBalanceForUser(const string &userId) {
        auto simplified = simplifyTransactions();
        bool found = false;
        for (auto &tx : simplified) {
            string debtor, creditor;
            double amount;
            tie(debtor, creditor, amount) = tx;
            if (debtor == userId || creditor == userId) {
                cout << debtor << " owes " << creditor << ": " 
                     << fixed << setprecision(2) << amount << endl;
                found = true;
            }
        }
        if (!found) {
            cout << "No balances" << endl;
        }
    }
};

// Main Expense Sharing App managing global users and groups.
class ExpenseSharingAppGlobal {
private:
    // Global users.
    unordered_map<string, User> users;
    // Groups: groupName -> Group object.
    unordered_map<string, Group> groups;
    
public:
    // Add a global user.
    void addUser(const User &user) {
        users[user.userId] = user;
    }
    
    // Create a new group.
    void createGroup(const string &groupName, const vector<string> &userIds) {
        // Ensure each user exists globally.
        for (const auto &uid : userIds) {
            if (users.find(uid) == users.end()) {
                throw invalid_argument("User " + uid + " not exists.");
            }
        }
        groups[groupName] = Group(groupName, userIds);
    }

    void addUserToGroup(const string &groupName, const string &userId) {
        // Check if the group exists
        if (groups.find(groupName) == groups.end()) {
            throw invalid_argument("Group " + groupName + " does not exist.");
        }

        // Check if the user exists globally
        if (users.find(userId) == users.end()) {
            throw invalid_argument("User " + userId + " does not exist.");
        }

        // Add the user to the group
        groups[groupName].addMember(userId);
    }
    
    // Process an expense within a group.
    void addExpense(const string &groupName, const string &payer, double amount, 
                    const vector<string> &involved, const string &type, const vector<double> &splits) {
        if (groups.find(groupName) == groups.end())
            throw invalid_argument("Group " + groupName + " not exists.");
        groups[groupName].addExpense(payer, amount, involved, type, splits);
    }
    
    // Show all balances for a given group.
    void showAllBalances(const string &groupName) {
        cout << "------------------------------------------------" << endl; 
        cout << "Group Name : " << groupName << endl; 
        if (groups.find(groupName) == groups.end()) {
            cout << "Group " << groupName << " not exists." << endl;
            return;
        }
        groups[groupName].showAllBalances();
        cout << "------------------------------------------------" << endl; 
        cout << endl ; 
    }
    
    // Show balances for a single user within a group.
    void showBalanceForUser(const string &groupName, const string &userId) {
        cout << "------------------------------------------------" << endl; 
        cout << "Group Name : " << groupName << endl; 

        if (groups.find(groupName) == groups.end()) {
            cout << "Group " << groupName << " not exists." << endl;
            return;
        }
        groups[groupName].showBalanceForUser(userId);

        cout << "------------------------------------------------" << endl;
        cout << endl;  
    }
};


int main() {
    ExpenseSharingAppGlobal app;
    // Commands supported:
    // CREATE_USER <userId> <name> <email> <mobile>
    // CREATE_GROUP <groupName> <numUsers> <list-of-userIds>
    // EXPENSE <groupName> <payer> <amount> <numUsers> <list-of-userIds> <EQUAL/EXACT/PERCENT> <optional splits>
    // SHOW <groupName> [userId]
    
    string line;
    while(getline(cin, line)) {
        if(line.empty()) continue;
        istringstream iss(line);
        string command;
        iss >> command;
        
        try {
            if (command == "CREATE_USER") {
                string uid, name, email, mobile;
                iss >> uid >> name >> email >> mobile;
                app.addUser(User(uid, name, email, mobile));
            }
            else if (command == "CREATE_GROUP") {
                string groupName;
                int numUsers;
                iss >> groupName >> numUsers;
                vector<string> groupUsers;
                for (int i = 0; i < numUsers; i++) {
                    string uid;
                    iss >> uid;
                    groupUsers.push_back(uid);
                }
                app.createGroup(groupName, groupUsers);
            }
            // ADD_USER_TO_GROUP <groupName> <userId>
            else if(command == "ADD_USER_TO_GROUP") {
                string groupName, userId;
                iss >> groupName >> userId;
                app.addUserToGroup(groupName, userId);
            }
            else if (command == "EXPENSE") {
                string groupName;
                iss >> groupName;
                string payer;
                double amount;
                int numUsers;
                iss >> payer >> amount >> numUsers;
                vector<string> involved;
                for (int i = 0; i < numUsers; i++) {
                    string uid;
                    iss >> uid;
                    involved.push_back(uid);
                }
                string expenseType;
                iss >> expenseType;
                vector<double> splits;
                if (expenseType == "EXACT" || expenseType == "PERCENT") {
                    for (int i = 0; i < numUsers; i++) {
                        double val;
                        iss >> val;
                        splits.push_back(val);
                    }
                }
                app.addExpense(groupName, payer, amount, involved, expenseType, splits);
            }
            else if (command == "SHOW") {
                string groupName;
                iss >> groupName;
                string userId;
                if (iss >> userId) {
                    app.showBalanceForUser(groupName, userId);
                } else {
                    app.showAllBalances(groupName);
                }
            }
        } catch (const exception &ex) {
            cout << ex.what() << endl;
        }
    }
    
    return 0;
}

