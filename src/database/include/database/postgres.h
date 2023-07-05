#ifndef POSTGRES_H
#define POSTGRES_H

#include <iostream>
#include "pqxx/pqxx"

using namespace std;

namespace DatabaseManager
{

   pqxx::connection connect_to_db(const string db_name, const string username,
   const string password, string host, const string port);

   void create_waypoint_table(pqxx::connection &db);

   void insert_waypoint(pqxx::connection &db, const int move_id, const double x_pos, const double y_pos,
const double z_pos, const double x_ori, const double y_ori, const double z_ori, const double w_ori, const int rail_displacement,
const int rail_time, const int rail_speed, const bool rail_direction_boolean, const int displacement_time_to_next_waypoint);

   void create_move_table(pqxx::connection &db);
      
   int insert_move(pqxx::connection &db, const string name);

   void delete_move(pqxx::connection &db, const string move_name);

   void delete_waypoint(pqxx::connection &db, const int move_id);

   pqxx::result select_move(pqxx::connection &db, const string move_name);

   pqxx::result select_all_moves(pqxx::connection &db);

   int get_move_id(pqxx::connection &db, const string name);

   void update_move_name(pqxx::connection &db, const int id, const string new_name);

   void create_log_table(pqxx::connection &db);

   long long insert_log(pqxx::connection &db, const int error_id, const string description,
   const string date, const string time);

   pqxx::result select_list_of_log(pqxx::connection &db, const string start_date, 
   const string end_date);

   pqxx::result select_log(pqxx::connection &db, const string date,
   const string time);

   void delete_log(pqxx::connection &db, const string date,
   const string time);

   void delete_all_log(pqxx::connection &db);

   void create_password_tables(pqxx::connection &db);

   void insert_default_passwords(pqxx::connection &db);

   bool check_password(pqxx::connection &db, const string pass, const string lvl);

   bool update_password(pqxx::connection &db, const string old_password,
   const string new_password, const string lvl);

   void create_tile_layout_table(pqxx::connection &db);

   int insert_tile_layout(pqxx::connection &db, const string style, const int move_id);

   std::string get_tile_layout(pqxx::connection &db, const int move_id);

   bool update_tile_layout(pqxx::connection &db, const string new_style, const int move_id);

   void create_tile_table(pqxx::connection &db);

   int insert_tile(pqxx::connection &db, const int size, const int thickness, const int move_id);

   int get_tile_size(pqxx::connection &db, const int move_id);

   int get_tile_thickness(pqxx::connection &db, const int move_id);

   bool update_tile(pqxx::connection &db, const int size, const int thickness, const int move_id);

   void create_language_table(pqxx::connection &db);

   bool update_language_settings(pqxx::connection &db, string lang);

   string get_language_settings(pqxx::connection &db);

}

#endif