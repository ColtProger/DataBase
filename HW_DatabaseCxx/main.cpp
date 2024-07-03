#include<iostream>
#include <pqxx/pqxx>
#include <Windows.h>
#pragma execution_character_set("utf-8")

class ClientsDB {
public:
//pqxx::connection conn;

	ClientsDB() {};
	
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


void AddClient(pqxx::connection& conn)
{
	std::string reply;

	std::cout << "Добавить клиента (да, нет)? ";
	std::cin >> reply;

	while ((reply == "да") || (reply == "lf")) {

		std::string newName, newSurname, newEmail;
		std::cout << "Введите имя, фамилию и email: " << std::endl;
		std::cin >> newName >> newSurname >> newEmail;

		conn.prepare("insert_client", "INSERT INTO Client(name, surname, email) VALUES($1, $2, $3)");
		pqxx::work tx{ conn };

		tx.exec_prepared("insert_client", tx.esc(newName), tx.esc(newSurname), tx.esc(newEmail));

		tx.commit();

		std::cout << "Добавить клиента (да, нет)? ";
		std::cin >> reply;
	};

}

void CahngeClient(pqxx::connection& conn)
{
	std::string reply;

	std::cout << "Изменить данные клиента (да, нет)? ";
	std::cin >> reply;
	
	while ((reply == "да") || (reply == "lf")) {

		int point;
		std::cout << "Введите изменяемое поле (1 - имя, 2 - фамилию или 3 - email:) " << std::endl;
		std::cin >> point;

		std::string changable;
		std::cout << "Введите id  клиента: " << std::endl;
		std::cin >> changable;

		std::string newdata;
		std::cout << "Введите новое значение: " << std::endl;
		std::cin >> newdata;


		pqxx::work tx{ conn };
		if (point == 1) {
			tx.exec("UPDATE Client SET  name =  '" + tx.esc(newdata) + "' where id = '" + tx.esc(changable) + "' ");
		}
		if (point == 2) {
			tx.exec("UPDATE Client SET  surname =  '" + tx.esc(newdata) + "' where id =  '" + tx.esc(changable) + "' ");
		}
		if (point == 3) {
			tx.exec("UPDATE Client SET  email =  '" + tx.esc(newdata) + "' where id =  '" + tx.esc(changable) + "' ");
		}


		tx.commit();

		std::cout << "Внести еще изменения (да, нет)? ";
		std::cin >> reply;
	};
}

void DropClient(pqxx::connection& conn)
{
	std::string reply;

	std::cout << "Удалить данные клиента (да, нет)? ";
	std::cin >> reply;

	while ((reply == "да") || (reply == "lf")) {

		int cId;
		std::cout << "Введите id  клиента: " << std::endl;
		std::cin >> cId;

	    pqxx::work tx{ conn };
		

		for (auto [phone_id] : tx.query<int>("SELECT pc.phone_id FROM phonenumbers AS pn "
			"LEFT JOIN clientphone AS pc ON pc.phone_id = pn.id "
			"LEFT JOIN client AS cl ON cl.id = pc.client_id "
			"WHERE client_id = '" + std::to_string(cId) + "' "
			"ORDER BY client_id "))
		{
			tx.exec("DELETE FROM clientphone USING phonenumbers "
				"WHERE phone_id = phonenumbers.id AND phonenumbers.id = '" + std::to_string(phone_id) + "' ");

			tx.exec("DELETE FROM phonenumbers "
				"WHERE id = '" + std::to_string(phone_id) + "' ");
		}


		tx.exec("DELETE FROM Client "
			"WHERE id =  '" + std::to_string(cId) + "' ");
	   
		tx.commit();

		std::cout << "Удалить еще? (да, нет)? ";
		std::cin >> reply;
	};
}


void FindClient(pqxx::connection& conn)
{
	std::string reply;

	std::cout << "Найти клиента (да, нет)? ";
	std::cin >> reply;

	while ((reply == "да") || (reply == "lf")) { 

		std::string info;
		
	std::cout << "Введите информацию: " << std::endl;
		std::cin >> info;
	    
	    
		pqxx::work tx{ conn };

		for (auto [id, name, sn, email, phone] :
			tx.query<int, std::string, std::string, std::string,std::string>("SELECT "
			   "cl.id, cl.name, cl.surname, cl.email, pn.phonenumber FROM client AS cl "
			   "LEFT JOIN clientphone AS pc ON pc.client_id = cl.id "
			   "LEFT JOIN phonenumbers AS pn ON pn.id = pc.phone_id "
			   "WHERE name = '" + tx.esc(info) + "' OR surname = '" + tx.esc(info) + "' "
			   "OR email = '" + tx.esc(info) + "'  OR phonenumber = '" + tx.esc(info) + "' "))
		{
			std::cout <<id <<". " << name << " "<< sn << '\n';
			std::cout << "Email: " << email << '\n';
			std::cout << "phone:  " << phone << '\n';
		}

		std::cout << "Найти еще (да, нет)? ";
		std::cin >> reply;
	};
}

void AddPhonenumber(pqxx::connection& conn)
{

	std::string reply;

	std::cout << "Добавить номер клиента (да, нет)? ";
	std::cin >> reply;

	while ((reply == "да") || (reply == "lf")) {

		int cId;
		std::cout << "Введите id клиента: " << std::endl;
		std::cin >> cId;

		std::string phone;
		std::cout << "Введите телефонный номер клиента: " << std::endl;
		std::cin >> phone;

		pqxx::work tx{ conn };

		tx.exec("INSERT INTO phonenumbers (phonenumber) "
			"VALUES('" + tx.esc(phone) + "')");

		int phone_id = tx.query_value<int>("SELECT id FROM phonenumbers WHERE phonenumber = '" + tx.esc(phone) + "'");

		tx.exec("INSERT INTO ClientPhone (client_id, phone_id) "
			"VALUES('" + std::to_string(cId) + "', '" + std::to_string(phone_id) + "')");

		tx.commit();

		std::cout << "Добавить еще номер клиента (да, нет)? ";
		std::cin >> reply;

	};
}

void RemovePhonenumber(pqxx::connection& conn)
{
	std::string reply;
	std::cout << "Удалить номер (да, нет)? ";
	std::cin >> reply;

	while ((reply == "да") || (reply == "lf")) {
		int cId;
		std::cout << "Введите id клиента: " << std::endl;
		std::cin >> cId;

		pqxx::work tx{ conn };

		int count = 0;
		std::cout << "Номера телефонов клиента: " << std::endl;
		for (auto [phonenumber, phone_id] : tx.query<std::string, int>("SELECT pn.phonenumber, pc.phone_id FROM phonenumbers AS pn "
			"LEFT JOIN clientphone AS pc ON pc.phone_id = pn.id "
			"LEFT JOIN client AS cl ON cl.id = pc.client_id "
			"WHERE client_id = '" + std::to_string(cId) + "' "
			"ORDER BY client_id "))
		{
			std::cout << phone_id << ".  " << phonenumber << "\n";
			count++;
		}

		if (count > 0) {
			int phone_id;
			std::cout << "Укажите id номера для удаления: " << std::endl;
			std::cin >> phone_id;

			std::cout << "Номер будет безвозвратно удален из базы. Продолжить?" << "\n";
			std::string repl;
			std::cin >> repl;
			if ((repl == "да") || (repl == "lf")) {
				tx.exec("DELETE FROM clientphone USING phonenumbers "
					"WHERE phone_id = phonenumbers.id AND phonenumbers.id = '" + std::to_string(phone_id) + "' ");

				tx.exec("DELETE FROM phonenumbers "
					"WHERE id =  '" + std::to_string(phone_id) + "' ");
			};
		}
		else {
			std::cout << "У клиента нет номеров в базе данных" << std::endl;
		}
		tx.commit();

		std::cout << "Удалить еще номера (да, нет)? ";
		std::cin >> reply;
	};

}




};
int main() { 
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	//setlocale(LC_ALL, "Russian");

	std::string reply;
	std::string DB_info = "host=localhost "
		"port=5432 "
		"dbname=postgres "
		"user=postgres "
		"password=2612";
	
	try {
		pqxx::connection conn(DB_info);
		ClientsDB DB;
		DB.CreateDB(conn);
		
		int reply;
		std::cout <<"Выберите дейстие: " << std::endl;
		std::cout << "1. Добавить клиента " << std::endl;
		std::cout << "2. Добавить телефон для существующего клиента " << std::endl;
		std::cout << "3. Изменить данные о клиенте" << std::endl;
		std::cout << "4. Удалить телефон для существующего клиента" << std::endl;
		std::cout << "5. Удалить существующего клиента " << std::endl;
		std::cout << "6. Найти клиента по его данным (имени, фамилии, email-у или телефону)" << std::endl;
		std::cout << "7.Завершить сессию " << std::endl;
		std::cin >> reply;

		while (reply != 7) {
			if (reply == 1) DB.AddClient(conn);
			if (reply == 2) DB.AddPhonenumber(conn);
			if (reply == 3) DB.CahngeClient(conn);
			if (reply == 4) DB.RemovePhonenumber(conn);
			if (reply == 5) DB.DropClient(conn);
			if (reply == 6) DB.FindClient(conn);

			std::cout << "Выберите дейстие: " << std::endl;
			std::cout << "1. Добавить клиента " << std::endl;
			std::cout << "2. Добавить телефон для существующего клиента " << std::endl;
			std::cout << "3. Изменить данные о клиенте" << std::endl;
			std::cout << "4. Удалить телефон для существующего клиента" << std::endl;
			std::cout << "5. Удалить существующего клиента " << std::endl;
			std::cout << "6. Найти клиента по его данным (имени, фамилии, email-у или телефону)" << std::endl;
			std::cout << "7.Завершить сессию " << std::endl;
			std::cin >> reply;
		}
		
		
			//DB.AddClient(conn);

			
		
		 //DB.CahngeClient(conn);
		//DB.AddPhonenumber(conn);
		//DB.RemovePhonenumber(conn);
		//DB.DropClient(conn);
		//DB.FindClient(conn);
	}

	catch (pqxx::sql_error e)
	{
		std::cout << e.what() << std::endl;
	}

	

	


return 0; 

}