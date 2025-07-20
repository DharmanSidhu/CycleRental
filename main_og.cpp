#include <iostream>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_driver.h>
#include <mysql_connection.h>

int main() {
    try {
        sql::mysql::MySQL_Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        // Connection details
        std::string host = "tcp://127.0.0.1:3306";
        std::string user = "root";
        std::string pass = "dolphin";
        std::string db = "proj";

        // Get driver instance and connect
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(host, user, pass);
        con->setSchema(db);

        // Create a statement and execute query to get all tables
        stmt = con->createStatement();
        res = stmt->executeQuery("SHOW TABLES");

        std::cout << "Tables in the database '" << db << "':" << std::endl;

        while (res->next()) {
            std::cout << "- " << res->getString(1) << std::endl;
        }

        // Cleanup
        delete res;
        delete stmt;
        delete con;
    } catch (sql::SQLException& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return 1;
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
