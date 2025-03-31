# Expense Sharing Application

## Overview

The Expense Sharing Application is a C++ console-based program designed to help groups manage and split expenses efficiently. It allows users to create groups, add members, record expenses with various splitting methods, and view simplified balances among group members.

## Features

- **User Management**: Create and manage users with unique IDs, names, emails, and mobile numbers.
- **Group Management**: Create groups, add members, and manage group compositions.
- **Expense Recording**: Add expenses to groups with flexible splitting options:
  - **EQUAL**: Split expenses equally among participants.
  - **EXACT**: Assign specific amounts to each participant.
  - **PERCENT**: Distribute expenses based on percentage allocations.
- **Balance Simplification**: Automatically simplify debts within the group to minimize the number of transactions required for settlement.
- **Balance Display**: View individual or group balances to understand who owes whom and how much.


## How to Use

### 1. Creating Users

To add a new user:
CREATE_USER <userId> <name> <email> <mobile>
*Example:*
CREATE_USER u1 Alice alice@example.com 1234567890


### 2. Creating a Group

To create a new group with specified members:
CREATE_GROUP <groupName> <numUsers> <list-of-userIds>
*Example:*
CREATE_GROUP G1 3 u1 u2 u3


### 3. Adding an Expense
To add an expense to a group:
EXPENSE <groupName> <payer> <amount> <numUsers> <list-of-userIds> <EQUAL/EXACT/PERCENT> [<splits>]

- **EQUAL**: Splits the amount equally among the specified users.
- **EXACT**: Requires exact amounts for each user in the `<splits>` section.
- **PERCENT**: Requires percentage shares for each user in the `<splits>` section.

*Examples:*

- **EQUAL Split:**
EXPENSE G1 u1 1200 3 u1 u2 u3 EQUAL



- **EXACT Split:**
EXPENSE G1 u1 1200 3 u1 u2 u3 EXACT 600 300 300


- **PERCENT Split:**
EXPENSE G1 u1 1200 3 u1 u2 u3 PERCENT 50 25 25


### 4. Viewing Balances
To view balances within a group:
SHOW <groupName> [userId]

- Omitting `[userId]` displays all balances in the group.
- Including `[userId]` displays balances related to the specified user.

*Examples:*

- **Show all balances in group G1:**
SHOW G1


- **Show balances for user u2 in group G1:**
SHOW G1 u2


## Sample Session
CREATE_USER u1 Alice alice@example.com 1234567890 CREATE_USER u2 Bob bob@example.com 2345678901 CREATE_USER u3 Charlie charlie@example.com 3456789012 CREATE_GROUP G1 3 u1 u2 u3 EXPENSE G1 u1 1200 3 u1 u2 u3 EQUAL SHOW G1


**Output:**

u2 owes u1: 400.00 u3 owes u1: 400.00


## Functionality Details

### `simplifyTransactions` Function
The `simplifyTransactions` function optimizes the debt settlement process within a group by reducing the number of transactions needed. It follows these steps:

1. **Calculate Net Balances**: Determine the net amount each member owes or is owed.
2. **Use Heaps for Optimization**:
   - **Creditors** (members who should receive money) are stored in a max-heap based on the amount they are owed.
   - **Debtors** (members who owe money) are stored in a max-heap based on the amount they owe.
3. **Settle Debts Greedily**: Match the member who owes the most with the member who is owed the most, settle the minimum of the two amounts, and update the heaps accordingly.
4. **Update Balances**: Reflect these transactions in the group's balance records.

This approach ensures that debts are settled in the fewest transactions possible, enhancing efficiency.

## Important Considerations

- **Rounding**: All monetary values are rounded to two decimal places to maintain consistency.
- **Error Handling**: The application includes checks for invalid inputs, such as non-existent users or groups, and mismatched splitting amounts.
- **Data Integrity**: The program assumes that all input data is accurate and that users adhere to the specified formats.

## Conclusion

This Expense Sharing Application provides a streamlined way for groups to manage shared expenses, ensuring clarity and fairness in financial interactions. By automating calculations and simplifying transactions, it reduces the complexity often associated with group expenditures.


