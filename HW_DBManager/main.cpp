#include<iostream>
#include <pqxx/pqxx> 
#include <Windows.h>
#pragma execution_character_set("utf-8")


class ClientDB {
private: 
	//pqxx::connection conn;

public:
	
ClientDB() {};

void CreateTable(pqxx::connection& conn, std::string query)
{
	pqxx::transaction t{ conn };
	t.exec(query);
	t.commit();
}

void CreateDB(pqxx::connection& conn)
{
	std::string query_table1 = "CREATE TABLE IF NOT EXISTS Client  ("
		"id SERIAL  PRIMARY KEY, "
		"name VARCHAR(60), "
		"surname VARCHAR(60), "
		"email VARCHAR(60) "
		");  ";

	std::string query_table2 = "CREATE TABLE IF NOT EXISTS phonenumbers  ("
		"id SERIAL  PRIMARY KEY, "
		"phonenumber VARCHAR(12) "
		");  ";

	std::string query_table3 = "CREATE TABLE IF NOT EXISTS ClientPhone  ("
		"client_id INTEGER REFERENCES Client(id), "
		"phone_id INTEGER REFERENCES phonenumbers(id), "
		"CONSTRAINT cp PRIMARY KEY(client_id, phone_id) "
		");  ";


	CreateTable(conn, query_table1);
	CreateTable(conn, query_table2);
	CreateTable(conn, query_table3);
}

int AddClient(pqxx::connection& conn, const std::string& firstName, const std::string& lastName, const std::string& email)
{
	//conn.prepare("insert_client", "INSERT INTO Client(name, surname, email) VALUES($1, $2, $3)");
	
	pqxx::work tx{ conn };

	tx.exec("INSERT INTO Client (name, surname, email) "
		" VALUES ('" + tx.esc(firstName) + "', '" + tx.esc(lastName) + "', '" + tx.esc(email) + "') ");

	int cl_id = tx.query_value<int>("SELECT id FROM Client "
		"WHERE name = '" + tx.esc(firstName) + "' AND  surname = '" + tx.esc(lastName) + "' AND email = '" + tx.esc(email) + "'");
	
	tx.commit();
	return cl_id;

}

void addPhoneNumber(pqxx::connection& conn, int clientId, const std::string& phoneNumber){

	pqxx::work tx{ conn };

	tx.exec("INSERT INTO phonenumbers (phonenumber) "
		"VALUES('" + tx.esc(phoneNumber) + "')");

	int phone_id = tx.query_value<int>("SELECT id FROM phonenumbers "
		"WHERE phonenumber = '" + tx.esc(phoneNumber) + "' ");

	tx.exec("INSERT INTO ClientPhone (client_id, phone_id) "
		"VALUES('" + std::to_string(clientId) + "', '" + std::to_string(phone_id) + "')");

	tx.commit();
	}

void updateClient(pqxx::connection& conn,  int clientId, const std::string& field, const std::string& newdata) {

	
	//std::string changable;
	pqxx::work tx{ conn };
	if (field == "name") {
		tx.exec("UPDATE Client SET  name =  '" + tx.esc(newdata) + "' where id ='" + std::to_string(clientId) + "' ");
	}
	if (field == "surname") {
		tx.exec("UPDATE Client SET  surname =  '" + tx.esc(newdata) + "' where id =  '" + std::to_string(clientId) + "' ");
	}
	if (field == "email") {
		tx.exec("UPDATE Client SET  email =  '" + tx.esc(newdata) + "' where id = '" + std::to_string(clientId) + "' ");
	}
	tx.commit();
}

void removeClient(pqxx::connection& conn, int clientId) {

	pqxx::work tx{ conn };


	for (auto [phone_id] : tx.query<int>("SELECT pc.phone_id FROM phonenumbers AS pn "
		"LEFT JOIN clientphone AS pc ON pc.phone_id = pn.id "
		"LEFT JOIN client AS cl ON cl.id = pc.client_id "
		"WHERE client_id = '" + std::to_string(clientId) + "' "
		"ORDER BY client_id "))
	{
		tx.exec("DELETE FROM clientphone USING phonenumbers "
			"WHERE phone_id = phonenumbers.id AND phonenumbers.id = '" + std::to_string(phone_id) + "' ");

		tx.exec("DELETE FROM phonenumbers "
			"WHERE id = '" + std::to_string(phone_id) + "' ");
	}


	tx.exec("DELETE FROM Client "
		"WHERE id =  '" + std::to_string(clientId) + "' ");

	tx.commit();
}

void findClient(pqxx::connection& conn, const std::string& info) {

	pqxx::work tx{ conn };

	for (auto [id, name, sn, email, phone] :
		tx.query<int, std::string, std::string, std::string, std::string>("SELECT "
			"cl.id, cl.name, cl.surname, cl.email, pn.phonenumber FROM client AS cl "
			"LEFT JOIN clientphone AS pc ON pc.client_id = cl.id "
			"LEFT JOIN phonenumbers AS pn ON pn.id = pc.phone_id "
			"WHERE name = '" + tx.esc(info) + "' OR surname = '" + tx.esc(info) + "' "
			"OR email = '" + tx.esc(info) + "'  OR phonenumber = '" + tx.esc(info) + "' "))
	{
		std::cout << id << ". " << name << " " << sn << '\n';
		std::cout << "Email: " << email << '\n';
		std::cout << "phone:  " << phone << '\n';
	}

}

};


int main() {
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	//setlocale(LC_ALL, "Russian");

	try {
		pqxx::connection conn("host=localhost "
			"port=5432 "
			"dbname=postgres "
			"user=postgres "
			"password=2612");
		
		ClientDB DB;
		DB.CreateDB(conn);

		
		/*DB.AddClient(conn, "Ivan", "Ivanov", "vanya@mail.ru");
		DB.AddClient(conn, "Petr", "Romanov", "petrI@yandex.ru");
		DB.AddClient(conn, "Jenny", "Lopes", "lopespopes@gmail.com");
		DB.AddClient(conn, "Till", "Schweiger", "hanne@dm.dm");
		DB.AddClient(conn, "Philya", "Kirkorov", "netadver@yandex.ru");
		DB.AddClient(conn, "Sidor", "Sidorov", "sidr@gmail.com");

		DB.addPhoneNumber(conn, 1, "998877");
		DB.addPhoneNumber(conn, 1, "003366");
		DB.addPhoneNumber(conn, 2, "445566"); 
        DB.addPhoneNumber(conn, 3, "123456");
		DB.addPhoneNumber(conn, 5, "112233");
		DB.addPhoneNumber(conn, 6, "111222");
		DB.addPhoneNumber(conn, 6, "654321");*/


		DB.updateClient(conn, 5, "name", "Timur");
		DB.updateClient(conn, 5, "surname", "iEgoKomanda");

		DB.removeClient(conn, 5);
		/*DB.AddClient(conn, "Till", "Schweiger", "hanne@dm.dm");

		DB.findClient(conn, "Ivan");

		DB.findClient(conn, "123456");

		DB.findClient(conn, "petrI@yandex.ru");*/

		//следующее поле вызывает исключение так как нет привязанног номера
		//DB.findClient(conn, "hanne@dm.dm");


	} catch (pqxx::sql_error e) 
	{ 
		std::cout << e.what() << std::endl; 
	}

return 0; 
}