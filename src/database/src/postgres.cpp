#include "database/postgres.h"

using namespace std;

pqxx::connection DatabaseManager::connect_to_db(const string db_name, const string username,
const string password, string host, const string port)
{

   if (host == "localhost")
      host = "127.0.0.1";
   
   string connection = "dbname = " + db_name + " user = " + username + 
   " password = " + password +" hostaddr = " + host + " port = " + port;

   return pqxx::connection(connection);
}


void DatabaseManager::create_waypoint_table(pqxx::connection &db)
{
   pqxx::work worker(db);
   string query = "CREATE TABLE waypoint("  \
      "ID BIGSERIAL PRIMARY KEY  NOT NULL, " \
      "MOVE_ID SERIAL REFERENCES move(id) ON DELETE CASCADE, " \
      "X_POSITION      DOUBLE PRECISION  NOT NULL, " \
      "Y_POSITION      DOUBLE PRECISION  NOT NULL, " \
      "Z_POSITION      DOUBLE PRECISION  NOT NULL, "\
      "X_ORIENTATION  DOUBLE PRECISION NULL, " \
      "Y_ORIENTATION   DOUBLE PRECISION  NOT NULL, "\
      "Z_ORIENTATION  DOUBLE PRECISION  NOT NULL, " \
      "W_ORIENTATION  DOUBLE PRECISION  NOT NULL, "\
      "RAIL_DISPLACEMENT    INTEGER NOT NULL,"\
      "RAIL_TIME            INTEGER NOT NULL,"\
      "RAIL_SPEED           INTEGER NOT NULL,"\
      "RAIL_DIRECTION BOOLEAN NOT NULL,"\
      "DISPLACEMENT_TIME_TO_NEXT_WAYPOINT      INTEGER NOT NULL);";
   worker.exec(const_cast<char*>(query.c_str()));
   worker.commit();
}


void DatabaseManager::insert_waypoint(pqxx::connection &db, const int move_id, const double x_pos, const double y_pos,
const double z_pos, const double x_ori, const double y_ori, const double z_ori, const double w_ori, const int rail_displacement,
const int rail_time, const int rail_speed, const bool rail_direction_boolean, const int displacement_time_to_next_waypoint)
{

   string rail_bool = rail_direction_boolean ? "true" : "false";
   string query = "INSERT INTO WAYPOINT VALUES (DEFAULT, " + to_string(move_id) + ", " + to_string(x_pos) + ", " +
   to_string(y_pos) + ", " + to_string(z_pos) + ", " + to_string(x_ori) + ", " + to_string(y_ori) + ", " + to_string(z_ori) + ", "+
   to_string(w_ori) + ", " + to_string(rail_displacement) + ", " + to_string(rail_time) + ", " + to_string(rail_speed) + ", " +
   rail_bool +", " + to_string(displacement_time_to_next_waypoint) + ");";   

   pqxx::work worker(db);
   worker.exec(const_cast<char*>(query.c_str()));
   worker.commit();
}


void DatabaseManager::create_move_table(pqxx::connection &db)
{
   pqxx::work worker(db);
   
   string query = "CREATE TABLE move("  \
      "ID SERIAL PRIMARY KEY  NOT NULL, " \
      "NAME  CHAR(50) NOT NULL, " \
      "UNIQUE (NAME));";
   
   worker.exec(const_cast<char*>(query.c_str()));
   worker.commit();
}

int DatabaseManager::insert_move(pqxx::connection &db, const string name)
{

   pqxx::work worker(db);
   string query = "INSERT INTO MOVE VALUES (DEFAULT, \'" + name + "\');";

   worker.exec(const_cast<char*>(query.c_str()));
   worker.commit();

   string query_id = "SELECT id FROM move WHERE name=\'" + name + "\';";
   
   pqxx::nontransaction non_tran(db);
   pqxx::result result(non_tran.exec(const_cast<char*>(query_id.c_str())));
   pqxx::result::const_iterator c = result.begin();
   return c[0].as<long long>();

}

pqxx::result DatabaseManager::select_move(pqxx::connection &db, const string move_name)
{

   string query_id = "SELECT id FROM move WHERE name=\'" + move_name + "\';";
   char* sql_id = const_cast<char*>(query_id.c_str());

   pqxx::nontransaction non_tran(db);
   pqxx::result result_id(non_tran.exec(sql_id));
   pqxx::result::const_iterator c = result_id.begin();

   string query  = "SELECT * FROM WAYPOINT WHERE move_id =\'" + to_string(c[0].as<int>()) + "\';";
   char* sql = const_cast<char*>(query.c_str());
   pqxx::result result(non_tran.exec(sql));
   return result;

}


void DatabaseManager::delete_move(pqxx::connection &db, const string move_name)
{
   pqxx::work worker(db);
   string sql = "DELETE FROM move WHERE name = \'" + move_name + "\';" ;
   worker.exec(const_cast<char*>(sql.c_str()));
   worker.commit();
}


void DatabaseManager::delete_waypoint(pqxx::connection &db, const int move_id)
{
   pqxx::work worker(db);
   string sql = "DELETE FROM waypoint WHERE move_id = \'" + to_string(move_id) + "\';" ;
   worker.exec(const_cast<char*>(sql.c_str()));
   worker.commit();
}


pqxx::result DatabaseManager::select_all_moves(pqxx::connection &db)
{

   string query  = "SELECT * FROM move;";
   char* sql = const_cast<char*>(query.c_str());
   pqxx::nontransaction non_tran(db);
   pqxx::result result(non_tran.exec(sql));
   return result;

}


int DatabaseManager::get_move_id(pqxx::connection &db, const string name)
{
   string query = "SELECT id FROM move WHERE name=\'" + name + "\';";
   char* sql = const_cast<char*>(query.c_str());

   pqxx::nontransaction non_tran(db);
   pqxx::result result(non_tran.exec(sql));
   pqxx::result::const_iterator c = result.begin();

   try
   {
      return c[0].as<int>();
   }
   catch(const std::exception)
   {
      return -1;
   }
   
}


void DatabaseManager::update_move_name(pqxx::connection &db, const int id, const string new_name)
{
   
   pqxx::work worker(db);
   string query = "UPDATE move SET name = \'" + new_name + "\' WHERE id = " + to_string(id) +";";
   char* sql = const_cast<char*>(query.c_str());
   worker.exec(sql);
   worker.commit();

}

void DatabaseManager::create_log_table(pqxx::connection &db)
{

   pqxx::work worker(db);
   string sql = "CREATE TABLE log("  \
      "ID BIGSERIAL PRIMARY KEY  NOT NULL, " \
      "ERROR_ID INTEGER NOT NULL, " \
      "DESCRIPTION TEXT NOT NULL, " \
      "DATE date NOT NULL, " \
      "TIME time NOT NULL);";
  
   worker.exec(const_cast<char*>(sql.c_str()));
   worker.commit();

}


long long DatabaseManager::insert_log(pqxx::connection &db,
const int error_id, const string description, const string date, const string time)
{

   string error = to_string(error_id);

   pqxx::work worker(db);
   string query = "INSERT INTO log VALUES (DEFAULT," + error +
   ", \'" + description +"\'" + ", \'" + date + "\'" + ", \'" + time + "\'"  ");";
   char* sql = const_cast<char*>(query.c_str());
   worker.exec(sql);
   worker.commit();

   query = "SELECT id FROM log WHERE DATE=\'" + date + "\'" + 
   "AND TIME = " + "\'" + time + "\'" + ";";
   sql = const_cast<char*>(query.c_str());

   pqxx::nontransaction non_tran(db);
   pqxx::result result(non_tran.exec(sql));
   pqxx::result::const_iterator c = result.begin();
   return c[0].as<long long>();

}

pqxx::result DatabaseManager::select_list_of_log(pqxx::connection &db,
const string start_date, const string end_date)
{
   
   string query;
   query = "SELECT * FROM log WHERE date >= \'" + start_date + 
   "\' And date <= \'" + end_date + "\';";
   char* sql = const_cast<char*>(query.c_str());
   pqxx::nontransaction non_tran(db);
   pqxx::result result(non_tran.exec(sql));
   return result;

}

pqxx::result DatabaseManager::select_log(pqxx::connection &db, const string date,
const string time)
{

   string query;
   query = "SELECT * FROM log WHERE date = \'" + date + 
   "\' And time = \'" + time + "\';";
   char* sql = const_cast<char*>(query.c_str());
   pqxx::nontransaction non_tran(db);
   pqxx::result result(non_tran.exec(sql));
   return result;

}


void DatabaseManager::delete_log(pqxx::connection &db, const string date,
const string time)
{
   string query;
   query = "DELETE FROM log WHERE date = \'" + date + 
   "\' And time = \'" + time + "\';";
   pqxx::work worker(db);
   worker.exec(const_cast<char*>(query.c_str()));
   worker.commit();
}


void DatabaseManager::delete_all_log(pqxx::connection &db)
{
   string query;
   query = "Delete FROM log;";
   pqxx::work worker(db);
   worker.exec(const_cast<char*>(query.c_str()));
   worker.commit();
}


void DatabaseManager::create_password_tables(pqxx::connection &db)
{

   string query;

   query = "CREATE TABLE passwords(" \
   "level_1 TEXT DEFAULT '0000', " \
   "level_2 TEXT DEFAULT '0000'," \
   "level_3 TEXT DEFAULT '0000')";

   pqxx::work worker(db);
   worker.exec(const_cast<char*>(query.c_str()));
   worker.commit();

}

void DatabaseManager::insert_default_passwords(pqxx::connection &db)
{
   
   string query;
   query = "INSERT INTO passwords VALUES(DEFAULT, DEFAULT, DEFAULT);";
   pqxx::work worker(db);
   worker.exec(const_cast<char*>(query.c_str()));
   worker.commit();

}

bool DatabaseManager::check_password(pqxx::connection &db, const string pass, const string lvl)
{

   string query = "SELECT * FROM passwords;";

   char* sql = const_cast<char*>(query.c_str());
   pqxx::nontransaction non_tran(db);
   pqxx::result result(non_tran.exec(sql));
   pqxx::result::const_iterator c = result.begin();

   string pass_lvl_1, pass_lvl_2, pass_lvl_3;

   pass_lvl_1 = c[0].as<string>();
   pass_lvl_2 = c[1].as<string>();
   pass_lvl_2 = c[2].as<string>();

   if (lvl == "1" && pass == pass_lvl_1)
      return true;

   if (lvl == "2" && pass == pass_lvl_2)
      return true;
   
   if (lvl == "3" && pass == pass_lvl_2)
      return true;

   return false;
}


bool DatabaseManager::update_password(pqxx::connection &db, const string old_password,
const string new_password, const string lvl)
{

   string query;
   
   if (lvl == "1")
   {
      query = "UPDATE passwords SET level_1 = '" + new_password + 
      "' WHERE level_1 = '" + old_password + "' ;";
   }
   
   if (lvl == "2")
   {
      query = "UPDATE passwords SET level_2 = '" + new_password + 
      "' WHERE level_2 = '" + old_password + "' ;";
   }

   if (lvl == "3")
   {
      query = "UPDATE passwords SET level_3 = '" + new_password + 
      "' WHERE level_3 = '" + old_password + "' ;";
   }

   else
      return false;

   try
   {
      pqxx::work worker(db);
      worker.exec(const_cast<char*>(query.c_str()));
      worker.commit();
      return true;
   }
   catch(const std::exception)
   {
      // ERROR
   }
   

   return false;

}

void DatabaseManager::create_tile_layout_table(pqxx::connection &db)
{
   string query = "CREATE TABLE tile_layout("\
   "id SERIAL PRIMARY KEY NOT NULL,"\
   "style TEXT NOT NULL,"\
   "MOVE_ID SERIAL REFERENCES move(id) ON DELETE CASCADE," \
   "UNIQUE (MOVE_ID));";

   pqxx::work worker(db);
   worker.exec(const_cast<char*>(query.c_str()));
   worker.commit();
}


int DatabaseManager::insert_tile_layout(pqxx::connection &db, const string style, const int move_id)
{
   string query = "INSERT INTO tile_layout VALUES(DEFAULT, '" + style + "', " + to_string(move_id) + ");";

   pqxx::work worker(db);
   worker.exec(const_cast<char*>(query.c_str()));
   worker.commit();
   
   string query_id = "SELECT id FROM tile_layout WHERE move_id= " + to_string(move_id) + " ;";

   pqxx::nontransaction non_tran(db);
   pqxx::result result_id(non_tran.exec(const_cast<char*>(query_id.c_str())));
   pqxx::result::const_iterator c = result_id.begin();

   return c[0].as<int>();
}

std::string DatabaseManager::get_tile_layout(pqxx::connection &db, const int move_id)
{
   
   string query = "SELECT style FROM tile_layout WHERE move_id =  " + to_string(move_id) + ";";

   pqxx::nontransaction non_tran(db);
   pqxx::result result_id(non_tran.exec(const_cast<char*>(query.c_str())));
   pqxx::result::const_iterator c = result_id.begin();

   try
   {
      return c[0].as<string>();
   }
   catch(const std::exception)
   {
      return "";
   }
}


bool DatabaseManager::update_tile_layout(pqxx::connection &db, const string new_style, const int move_id)
{

   string query = "UPDATE tile_layout SET style = '" + new_style +"' WHERE move_id = "\
   + to_string(move_id) + ";";


   pqxx::work worker(db);

   try
   {
      worker.exec(const_cast<char*>(query.c_str()));
      worker.commit();
      return true;
   }
   catch(const std::exception)
   {
      return false;
   }

}


void DatabaseManager::create_tile_table(pqxx::connection &db)
{

   string query = "CREATE TABLE tile(" \
   "id SERIAL PRIMARY KEY, " \
   "size SMALLSERIAL NOT NULL," \
   "thickness SMALLSERIAL NOT NULL," \
   "move_id SERIAL REFERENCES move(id) ON DELETE CASCADE," \
   "UNIQUE (MOVE_ID));";

   pqxx::work worker(db);
   worker.exec(const_cast<char*>(query.c_str()));
   worker.commit();

}

int DatabaseManager::insert_tile(pqxx::connection &db, const int size, const int thickness, const int move_id)
{

   string query = "INSERT INTO tile VALUES(DEFAULT, " + to_string(size) + ", " + to_string(thickness) +
   ", " + to_string(move_id) + ");";

   pqxx::work worker(db);
   
   worker.exec(const_cast<char*>(query.c_str()));
   worker.commit();

   string query_id = "SELECT id FROM tile WHERE move_id =  " + to_string(move_id) + ";";

   pqxx::nontransaction non_tran(db);
   pqxx::result result_id(non_tran.exec(const_cast<char*>(query_id.c_str())));
   pqxx::result::const_iterator c = result_id.begin();

   return c[0].as<int>();

}

int DatabaseManager::get_tile_size(pqxx::connection &db, const int move_id)
{

   string query = "SELECT size FROM tile WHERE move_id = " + to_string(move_id) + ";";

   pqxx::nontransaction non_tran(db);
   pqxx::result result_id(non_tran.exec(const_cast<char*>(query.c_str())));
   pqxx::result::const_iterator c = result_id.begin();

   try
   {
      return c[0].as<int>();
   }
   catch(const std::exception) // if c[0] is null
   {
      return -1;
   }
}

int DatabaseManager::get_tile_thickness(pqxx::connection &db, const int move_id)
{

   string query = "SELECT thickness FROM tile WHERE move_id = " + to_string(move_id) + ";";

   pqxx::nontransaction non_tran(db);
   pqxx::result result_id(non_tran.exec(const_cast<char*>(query.c_str())));
   pqxx::result::const_iterator c = result_id.begin();

   try
   {
      return c[0].as<int>();
   }
   catch(const std::exception) // if c[0] is null
   {
      return -1;
   }

}


bool DatabaseManager::update_tile(pqxx::connection &db, const int size, const int thickness, const int move_id)
{

   string query = "UPDATE tile SET size = " + to_string(size) +", thickness = " + to_string(thickness) +\
   " WHERE move_id = " + to_string(move_id) + ";";


   pqxx::work worker(db);

   try
   {
      worker.exec(const_cast<char*>(query.c_str()));
      worker.commit();
      return true;
   }
   catch(const std::exception)
   {
      return false;
   }

}

void DatabaseManager::create_language_table(pqxx::connection &db)
{
   string query = "CREATE TABLE language_settings(" \
   "id SERIAL PRIMARY KEY, " \
   "language text NOT NULL DEFAULT 'english');";

   pqxx::work worker(db);
   worker.exec(const_cast<char*>(query.c_str()));

   string insert_defualt_lang_query = "INSERT INTO language_settings(id, language) VALUES(1, 'english');";

   try
   {
      worker.exec(const_cast<char*>(insert_defualt_lang_query.c_str()));
      worker.commit();
   }
   catch(const std::exception)
   {
      //pass
   }
}


bool DatabaseManager::update_language_settings(pqxx::connection &db, string lang)
{

   string query = "UPDATE language_settings SET language = '" + lang + "' WHERE ID = 1;";

   pqxx::work worker(db);

   try
   {
      worker.exec(const_cast<char*>(query.c_str()));
      worker.commit();
      return true;
   }
   catch(const std::exception &e)
   {
      return false;
   }
}

string DatabaseManager::get_language_settings(pqxx::connection &db)
{
   string query = "SELECT language FROM language_settings WHERE id = 1";

   pqxx::nontransaction non_tran(db);
   pqxx::result result_id(non_tran.exec(const_cast<char*>(query.c_str())));
   pqxx::result::const_iterator c = result_id.begin();

   try
   {
      return c[0].as<string>();
   }
   catch(const std::exception)
   {
      return "";
   }
}