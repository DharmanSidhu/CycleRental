#include <iostream>
#include <string>
#include <vector>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <mysql_driver.h>
#include <mysql_connection.h>

// Utility function prototypes
bool login(sql::Connection* con, int& userId);
void showTables(sql::Connection* con);
void showTableContents(sql::Connection* con, const std::string& tableName);
void showOverdueUsers(sql::Connection* con);
void showAvailableCycles(sql::Connection* con);
void showActiveReservations(sql::Connection* con);
void printMenu();

int main() {
    const std::string host = "tcp://127.0.0.1:3306";
    const std::string user = "root";
    const std::string pass = "dolphin";
    const std::string db = "proj";
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect(host, user, pass));
        con->setSchema(db);

        // logging in
        int userId = -1;
        if (!login(con.get(), userId)) {
            std::cout << "Incorrect Login. Exiting.\n";
            return 1;
        }

        // if credentials correct
        int choice;
        do {
            printMenu();
            std::cin >> choice;
            std::cin.ignore();
            switch(choice) {
                case 1:
                    showTables(con.get());
                    break;
                case 2: {
                    std::string table;
                    std::cout << "Enter table name: ";
                    std::getline(std::cin, table);
                    showTableContents(con.get(), table);
                    break;
                }
                case 3:
                    showOverdueUsers(con.get());
                    break;
                case 4:
                    showAvailableCycles(con.get());
                    break;
                case 5:
                    showActiveReservations(con.get());
                    break;
                case 9:
                    std::cout << "Exiting.\n";
                    break;
                default:
                    std::cout << "Invalid choice. Try again.\n";
            }
        } while (choice != 9);
    } catch (sql::SQLException& e) {
        std::cerr << "SQL Error: " << e.what() << std::endl;
    }
    return 0;
}

bool login(sql::Connection* con, int& userId) {
    int inputUid;
    std::string inputEmail;
    std::cout<<"UID: ";
    std::cin >> inputUid;
    std::cin.ignore();
    std::cout<<"email: ";
    std::getline(std::cin, inputEmail);

    std::unique_ptr<sql::PreparedStatement> pstmt(
    con->prepareStatement("SELECT user_id, name FROM user WHERE user_id = ? AND email = ?"));
    pstmt->setInt(1, inputUid);
    pstmt->setString(2, inputEmail);

    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    if (res->next()) {
        userId = res->getInt("user_id"); // or you can query for the name for greetings
        std::string userName = res->getString("name");
        // std::cout << "Login successful. Welcome user #" << userId << "!\n";
        std::cout << "Login successful. Welcome, " << userName << "!\n";
        return true;
    } else {
        std::cout << "Invalid user ID or email.\n";
        return false;
    }
} // returns true if login succeeded


void printMenu() {
    std::cout << "\n===== CRMS CLI Menu =====\n";
    std::cout << "1. List all tables\n";
    std::cout << "2. View table contents\n";
    std::cout << "3. Query overdue users\n";
    std::cout << "4. Show available cycles\n";
    std::cout << "5. Show active reservations\n";
    std::cout << "6. View profile (PENDING)\n";
    std::cout << "7. Rent a cycle (PENDING)\n";
    std::cout << "8. Return a cycle (PENDING)\n";
    std::cout << "9. Exit\n";
    std::cout << "Enter your choice: ";
}

void showTables(sql::Connection* con) {
    std::unique_ptr<sql::Statement> stmt(con->createStatement());
    std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SHOW TABLES"));
    std::cout << "\nTables in database:\n";
    while (res->next())
        std::cout << "- " << res->getString(1) << std::endl;
}

void showTableContents(sql::Connection* con, const std::string& tableName) {
    std::unique_ptr<sql::Statement> stmt(con->createStatement());
    std::string query = "SELECT * FROM " + tableName;
    try {
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));
        sql::ResultSetMetaData* meta = res->getMetaData();
        int numCols = meta->getColumnCount();
        // Print header
        for (int i = 1; i <= numCols; ++i)
            std::cout << meta->getColumnName(i) << '\t';
        std::cout << std::endl;
        // Print rows
        while (res->next()) {
            for (int i = 1; i <= numCols; ++i)
                std::cout << res->getString(i) << '\t\t';
            std::cout << std::endl;
        }
    } catch (sql::SQLException& e) {
        std::cerr << "Table not found or SQL error: " << e.what() << std::endl;
    }
}

void showOverdueUsers(sql::Connection* con) {
    std::unique_ptr<sql::Statement> stmt(con->createStatement());
    std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(
        "SELECT user_id, name, email FROM user WHERE overdue=TRUE"));
    std::cout << "\nOverdue Users:\n";
    while (res->next())
        std::cout << res->getInt("user_id") << '\t' << res->getString("name")
                  << '\t' << res->getString("email") << '\n';
}

void showAvailableCycles(sql::Connection* con) {
    std::unique_ptr<sql::Statement> stmt(con->createStatement());
    std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(
        "SELECT * FROM cycle WHERE status='available'"));
    std::cout << "\nAvailable Cycles:\n";
    while (res->next())
        std::cout << res->getInt("cycle_id") << '\t' << res->getString("status") << '\n';
}

void showActiveReservations(sql::Connection* con) {
    std::unique_ptr<sql::Statement> stmt(con->createStatement());
    std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(
        "SELECT c.cycle_id, u.user_id, u.name "
        "FROM cycle c JOIN reserves r ON c.cycle_id = r.cycle_id "
        "JOIN user u ON u.user_id = r.user_id "
        "WHERE c.status = 'rented'"));
    std::cout << "\nActive Reservations:\n";
    while (res->next())
        std::cout << "Cycle: " << res->getInt("cycle_id")
                  << " | User: " << res->getInt("user_id")
                  << " | Name: " << res->getString("name") << '\n';
}
