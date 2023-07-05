#include <memory>
#include <functional>

#include "rclcpp/rclcpp.hpp"

#include "database/postgres.h"

#include "database_interfaces/msg/move.hpp"
#include "database_interfaces/msg/waypoint.hpp"
#include "database_interfaces/msg/log.hpp"

#include "database_interfaces/srv/add_move.hpp"
#include "database_interfaces/srv/delete_move.hpp"
#include "database_interfaces/srv/get_move.hpp"
#include "database_interfaces/srv/get_moves.hpp"
#include "database_interfaces/srv/edit_move.hpp"
#include "database_interfaces/srv/add_log.hpp"
#include "database_interfaces/srv/get_log.hpp"
#include "database_interfaces/srv/get_logs.hpp"
#include "database_interfaces/srv/delete_log.hpp"
#include "database_interfaces/srv/delete_logs.hpp"
#include "database_interfaces/srv/check_password.hpp"
#include "database_interfaces/srv/update_password.hpp"
#include "database_interfaces/srv/add_tile_layout.hpp"
#include "database_interfaces/srv/get_tile_layout.hpp"
#include "database_interfaces/srv/update_tile_layout.hpp"
#include "database_interfaces/srv/add_tile.hpp"
#include "database_interfaces/srv/get_tile.hpp"
#include "database_interfaces/srv/update_tile.hpp"
#include "database_interfaces/srv/set_language.hpp"
#include "database_interfaces/srv/get_language.hpp"

using namespace std;

pqxx::connection db_connection;

void add_move(const shared_ptr<database_interfaces::srv::AddMove::Request> request,
shared_ptr<database_interfaces::srv::AddMove::Response> response)
{
    int move_id;

    try
    {
        move_id = DatabaseManager::insert_move(db_connection, request->move.name);
    }
    catch(const pqxx::syntax_error &e)
    {
        DatabaseManager::create_move_table(db_connection);
        RCLCPP_INFO(rclcpp::get_logger("rclpcpp"), "CREATED MOVE TABLE");
        move_id = DatabaseManager::insert_move(db_connection, request->move.name);
    }
    catch(const pqxx::integrity_constraint_violation &e)
    {
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "This Move Exist so Append it");
        move_id = DatabaseManager::get_move_id(db_connection, request->move.name);

        if (move_id != -1)
        {
            response->id = -1;
            RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "TRY TO INSERT WAYPOINT DATA BUT FAILED! MOVE NAME LOST");
            return;
        }
    }

    for (int i = 0; i < request->move.waypoints.size(); i++)
	{   
		try
		{
			DatabaseManager::insert_waypoint(db_connection, move_id, request->move.waypoints[i].x_position,
			request->move.waypoints[i].y_position, request->move.waypoints[i].z_position, request->move.waypoints[i].x_orientation,
			request->move.waypoints[i].y_orientation, request->move.waypoints[i].z_orientation, request->move.waypoints[i].w_orientation,
			request->move.waypoints[i].rail_displacement, request->move.waypoints[i].rail_time,
			request->move.waypoints[i].rail_speed, request->move.waypoints[i].rail_direction,
			request->move.waypoints[i].displacement_time_to_next_waypoint);
		}
		catch(const pqxx::syntax_error &e)
		{   
			DatabaseManager::create_waypoint_table(db_connection);
			DatabaseManager::insert_waypoint(db_connection, move_id, request->move.waypoints[i].x_position,
			request->move.waypoints[i].y_position, request->move.waypoints[i].z_position, request->move.waypoints[i].x_orientation,
			request->move.waypoints[i].y_orientation, request->move.waypoints[i].z_orientation, request->move.waypoints[i].w_orientation,
			request->move.waypoints[i].rail_displacement, request->move.waypoints[i].rail_time,
			request->move.waypoints[i].rail_speed, request->move.waypoints[i].rail_direction,
			request->move.waypoints[i].displacement_time_to_next_waypoint);
		}
	}

	response->id = move_id;   
}

void delete_move(const shared_ptr<database_interfaces::srv::DeleteMove::Request> request,
shared_ptr<database_interfaces::srv::DeleteMove::Response> response)
{
    try
    {
        DatabaseManager::delete_move(db_connection, request->name);
        response->result = true;
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "DELETE A MOVE: %s", request->name);
    }
    catch(const pqxx::sql_error &e)
    {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "%s", e.what());
    }
}

void get_move(const shared_ptr<database_interfaces::srv::GetMove::Request> request,
shared_ptr<database_interfaces::srv::GetMove::Response> response)
{
    pqxx::result move_columns;
	try
	{
		move_columns = DatabaseManager::select_move(db_connection, request->name);
	}
	catch(const pqxx::syntax_error) // if move table does not exist
	{	
		RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Does Not Any Move Exist");
	}
	

	database_interfaces::msg::Waypoint waypoint;

	//TODO: handle exception if fields are empty
	for (pqxx::result::const_iterator c = move_columns.begin(); c != move_columns.end(); ++c)
	{	
		waypoint.id = c[0].as<int>(); // id
		waypoint.x_position = c[2].as<int>(); // ORDER
		waypoint.y_position = c[3].as<double>();  // X
		waypoint.z_position = c[4].as<double>();  // Y
		waypoint.x_orientation = c[5].as<double>(); // Z
		waypoint.y_orientation = c[6].as<double>(); // ROLL
		waypoint.z_orientation = c[7].as<double>(); // PITCH
		waypoint.w_orientation = c[8].as<double>();  // YAW
		waypoint.rail_displacement = c[9].as<int>();  // RAIL DISPLACEMENT
		waypoint.rail_time = c[10].as<int>();  // RAIL TIME
		waypoint.rail_speed = c[11].as<int>();  // RAIL SPEED
		waypoint.rail_direction = c[12].as<bool>();  // RAIL DIRECTION
		waypoint.displacement_time_to_next_waypoint = c[13].as<double>();  // DISPLACEMENT TIME TO NEXT WAYPOINT

		response->waypoints.push_back(waypoint);
	}
}

void get_moves(const shared_ptr<database_interfaces::srv::GetMoves::Request> request,
shared_ptr<database_interfaces::srv::GetMoves::Response> response)
{
    if (!request->add_waypoints)
	{	
		pqxx::result result;
		try
		{
			result = DatabaseManager::select_all_moves(db_connection);
		}
		catch(const pqxx::syntax_error)
		{
			DatabaseManager::create_move_table(db_connection);
			result = DatabaseManager::select_all_moves(db_connection);
		}

		database_interfaces::msg::Move move;

		for (pqxx::result::const_iterator c = result.begin(); c < result.end(); c++)
		{
			move.id = c[0].as<int>();
			move.name = c[1].as<string>();
			response->moves.push_back(move);
		}
	}

	else if (request->add_waypoints)
	{

		pqxx::result move_result;
		pqxx::result waypoint_result;

		try
		{
			move_result = DatabaseManager::select_all_moves(db_connection);
		}
		catch(const pqxx::syntax_error)
		{
			DatabaseManager::create_move_table(db_connection);
			RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Does Not Any Move Exist!");
		}


		database_interfaces::msg::Move move;
		database_interfaces::msg::Waypoint waypoint;

		for (pqxx::result::const_iterator m = move_result.begin(); m < move_result.end(); m++)
		{
			move.id = m[0].as<int>();
			move.name = m[1].as<string>();

			waypoint_result = DatabaseManager::select_move(db_connection, m[1].as<string>());

			for (pqxx::result::const_iterator w = waypoint_result.begin(); w != waypoint_result.end(); w++)
			{
				waypoint.id = w[0].as<int>(); // id
				waypoint.x_position = w[2].as<int>(); // ORDER
				waypoint.y_position = w[3].as<double>();  // X
				waypoint.z_position = w[4].as<double>();  // Y
				waypoint.x_orientation = w[5].as<double>(); // Z
				waypoint.y_orientation = w[6].as<double>(); // ROLL
				waypoint.z_orientation = w[7].as<double>(); // PITCH
				waypoint.w_orientation = w[8].as<double>();  // YAW
				waypoint.rail_displacement = w[9].as<int>();  // RAIL DISPLACEMENT
				waypoint.rail_time = w[10].as<int>();  // RAIL TIME
				waypoint.rail_speed = w[11].as<int>();  // RAIL SPEED
				waypoint.rail_direction = w[12].as<bool>();  // RAIL DIRECTION
				waypoint.displacement_time_to_next_waypoint = w[13].as<double>();  // DISPLACEMENT TIME TO NEXT WAYPOINT
				move.waypoints.push_back(waypoint);
			}

			response->moves.push_back(move);
		}
	}
}


void edit_move(const shared_ptr<database_interfaces::srv::EditMove::Request> request,
shared_ptr<database_interfaces::srv::EditMove::Response> response)
{
	int move_id = request->move.id;
	
	try
	{
		DatabaseManager::update_move_name(db_connection, request->move.id, request->move.name);
	}
	catch(const pqxx::syntax_error &e)
	{
		RCLCPP_WARN(rclcpp::get_logger("rclcpp"), "MOVE TABLE DOES NOT EXIST");
		DatabaseManager::create_move_table(db_connection);
		RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "CREATED MOVE TABLE");
		move_id = DatabaseManager::insert_move(db_connection, request->move.name);
		RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "ADD NEW MOVE");
	}
	catch (const pqxx::integrity_constraint_violation &e)
	{
		RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "this move`s name exist!");
		response->id = -1;
		return;
	}

	try
	{
		DatabaseManager::delete_waypoint(db_connection, request->move.id);
	}
	catch(const pqxx::syntax_error &e)
	{
		DatabaseManager::create_waypoint_table(db_connection);
	}

	for (int i = 0; i < request->move.waypoints.size(); i++)
	{   
		try
		{
			DatabaseManager::insert_waypoint(db_connection, move_id, request->move.waypoints[i].x_position,
			request->move.waypoints[i].y_position, request->move.waypoints[i].z_position, request->move.waypoints[i].x_orientation,
			request->move.waypoints[i].y_orientation, request->move.waypoints[i].z_orientation, request->move.waypoints[i].w_orientation,
			request->move.waypoints[i].rail_displacement, request->move.waypoints[i].rail_time,
			request->move.waypoints[i].rail_speed, request->move.waypoints[i].rail_direction,
			request->move.waypoints[i].displacement_time_to_next_waypoint);
		}
		catch(const pqxx::syntax_error &e)
		{   
			DatabaseManager::create_waypoint_table(db_connection);
			DatabaseManager::insert_waypoint(db_connection, move_id, request->move.waypoints[i].x_position,
			request->move.waypoints[i].y_position, request->move.waypoints[i].z_position, request->move.waypoints[i].x_orientation,
			request->move.waypoints[i].y_orientation, request->move.waypoints[i].z_orientation, request->move.waypoints[i].w_orientation,
			request->move.waypoints[i].rail_displacement, request->move.waypoints[i].rail_time,
			request->move.waypoints[i].rail_speed, request->move.waypoints[i].rail_direction,
			request->move.waypoints[i].displacement_time_to_next_waypoint);
		}
	}
	response->id = move_id;
}

void add_log(const shared_ptr<database_interfaces::srv::AddLog::Request> request,
shared_ptr<database_interfaces::srv::AddLog::Response> response)
{
	try
	{
		response->id = DatabaseManager::insert_log(db_connection, request->log.error_id,
		request->log.description, request->log.date, request->log.time);
	}
	catch(const pqxx::syntax_error& e)
	{
		DatabaseManager::create_log_table(db_connection);
		RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "CREATE LOG TABLE");
		response->id = DatabaseManager::insert_log(db_connection, request->log.error_id,
		request->log.description, request->log.date, request->log.time);
	}

}

void get_log(const shared_ptr<database_interfaces::srv::GetLog::Request> request,
shared_ptr<database_interfaces::srv::GetLog::Response> response)
{

	try
	{
		pqxx::result row_log = DatabaseManager::select_log(db_connection, request->date, request->time);

		for (pqxx::result::const_iterator c = row_log.begin(); c < row_log.end(); c++)
		{
			response->log.error_id = c[1].as<int>();
			response->log.description = c[2].as<string>();
			response->log.date = c[3].as<string>();
			response->log.time = c[4].as<string>();
		}
	}
	catch(const pqxx::syntax_error &e)
	{
		RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), e.what());
	}

}

void get_logs(const shared_ptr<database_interfaces::srv::GetLogs::Request> request,
shared_ptr<database_interfaces::srv::GetLogs::Response> response)
{
	pqxx::result row_logs = DatabaseManager::select_list_of_log(db_connection, request->start_date, request->end_date);

	database_interfaces::msg::Log log;

	for (pqxx::result::const_iterator c = row_logs.begin(); c != row_logs.end(); c++)
	{
		log.error_id = c[1].as<int>();
		log.description = c[2].as<string>();
		log.date= c[3].as<string>();
		log.time = c[4].as<string>();
		response->logs.push_back(log);
	}

}

void delete_log(const shared_ptr<database_interfaces::srv::DeleteLog::Request> request,
shared_ptr<database_interfaces::srv::DeleteLog::Response> response)
{
	try
	{
		DatabaseManager::delete_log(db_connection, request->date, request->time);
		response->result = true;
	}
	catch(const pqxx::syntax_error& e)
	{
		RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), e.what());
		response->result = false;
	}
	
}

void delete_all_logs(const shared_ptr<database_interfaces::srv::DeleteLogs::Request> request,
shared_ptr<database_interfaces::srv::DeleteLogs::Response> response)
{
	try
	{
		DatabaseManager::delete_all_log(db_connection);
		response->result = true;
	}
	catch(const pqxx::syntax_error &e)
	{
		RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), e.what());
		response->result = false;
	}
	
}


void check_password(const shared_ptr<database_interfaces::srv::CheckPassword::Request> request,
shared_ptr<database_interfaces::srv::CheckPassword::Response> response)
{
	try
	{
		response->correct = DatabaseManager::check_password(db_connection, request->password, request->level);
	}
	catch(const pqxx::syntax_error& e)
	{	
		RCLCPP_WARN(rclcpp::get_logger("rclcpp"),
		"PASSWORD TABLES IS REMOVED SO WE CREATE IT AGAIN AND FOR THIS ALL PASSWORDS RESET FACTORIES");
		DatabaseManager::create_password_tables(db_connection);
		DatabaseManager::insert_default_passwords(db_connection);

		response->correct = DatabaseManager::check_password(db_connection, request->password, request->level);
	}
	
}


void update_password(const shared_ptr<database_interfaces::srv::UpdatePassword::Request> request,
shared_ptr<database_interfaces::srv::UpdatePassword::Response> &response)
{
	try
	{
		if (DatabaseManager::check_password(db_connection, request->old_password, request->level))
		{
			response->result = DatabaseManager::update_password(db_connection, request->old_password,
			request->new_password, request->level);
		}
		else
			response->result = false;
	}
	catch(const pqxx::syntax_error)
	{
		RCLCPP_WARN(rclcpp::get_logger("rclcpp"), "PASSWORD TABLES IS REMOVED SO WE CREATE IT AGAIN AND FOR THIS ALL PASSWORDS RESET FACTORIES");
		DatabaseManager::create_password_tables(db_connection);
		DatabaseManager::insert_default_passwords(db_connection);

		if (DatabaseManager::check_password(db_connection, request->old_password, request->level))
		{
			response->result = DatabaseManager::update_password(db_connection, request->old_password,
			request->new_password, request->level);
		}
		else
			response->result = false;

	}
}


void add_tile_layout(const shared_ptr<database_interfaces::srv::AddTileLayout::Request> request,
shared_ptr<database_interfaces::srv::AddTileLayout::Response> response)
{
	int move_id;

	try
	{
		try
		{
			move_id = DatabaseManager::get_move_id(db_connection, request->move_name);
			if (move_id == -1)
			{
				response->id = -1;
				RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "This Move Name Does Not Exist!");
			}
		}
		catch(const pqxx::syntax_error)
		{
			response->id = -1;
			RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Move Table Does Not Exist!");
		}

		response->id = DatabaseManager::insert_tile_layout(db_connection, request->style, move_id);
	}
	catch(const pqxx::syntax_error) // -> if tile_layout doese not exist!
	{
		DatabaseManager::create_tile_layout_table(db_connection);
		response->id = DatabaseManager::insert_tile_layout(db_connection, request->style, move_id);
	}
	catch(const pqxx::integrity_constraint_violation) // -> if this move id used for another tile_layout
	{
		RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "THE MOVE ID MUST BE UNIQUE!");
		response->id = -1;
	}

}


void get_tile_layout(const shared_ptr<database_interfaces::srv::GetTileLayout::Request> request,
shared_ptr<database_interfaces::srv::GetTileLayout::Response> response)
{
	int move_id;

	try
	{
		move_id = DatabaseManager::get_move_id(db_connection, request->move_name);
		if (move_id == -1)
		{
			RCLCPP_WARN(rclcpp::get_logger("rclcpp"), "This Move Does Not Exist");
			response->style = "";
		}
		response->style = DatabaseManager::get_tile_layout(db_connection, move_id);
	}
	catch(const pqxx::syntax_error)
	{
		RCLCPP_WARN(rclcpp::get_logger("rclcpp"), "MOVE TABLE DOES NOT EXIST!");
		response->style = "";
	}

} 

void update_tile_layout(const shared_ptr<database_interfaces::srv::UpdateTileLayout::Request> request,
shared_ptr<database_interfaces::srv::UpdateTileLayout::Response> response)
{	
	int move_id;
	try
	{
		move_id = DatabaseManager::get_move_id(db_connection, request->move_name);
		if (move_id == -1)
		{
			RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "THIS MOVE DOES NOT EXIST!");
			response->result = false;
		}
		else
		{
			response->result = DatabaseManager::update_tile_layout(db_connection, request->new_style,
			move_id);
		}
	}
	catch(const pqxx::syntax_error)
	{
		RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "THIS MOVE DOES NOT EXIST!");
		DatabaseManager::create_move_table(db_connection);
		response->result = false;
	}
	catch(const pqxx::integrity_constraint_violation) 
	{
		RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "THE MOVE ID MUST BE UNIQUE!");
		response->result = false;
	}

}


void add_tile(const shared_ptr<database_interfaces::srv::AddTile::Request> request,
shared_ptr<database_interfaces::srv::AddTile::Response> response)
{
	int move_id;
	try
	{	
		try
		{
			move_id = DatabaseManager::get_move_id(db_connection, request->move_name);
		}
		catch(const pqxx::syntax_error) // IF Move Table Doese Not Exist
		{
			response->id = -1;
			RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "MOVE TABLE DOES NOT EXIST");
		}
		
		if (move_id == -1)
		{
			response->id = -1;
			RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "MOVE TABLE DOES NOT EXIST");
		}

		response->id = DatabaseManager::insert_tile(db_connection, request->size, request->thickness, move_id);

	}
	catch(const pqxx::syntax_error) // IF Tile Table Doese Not Exist
	{
		DatabaseManager::create_tile_table(db_connection);
		response->id = DatabaseManager::insert_tile(db_connection, request->size, request->thickness, move_id);
	}
	catch(const pqxx::integrity_constraint_violation) 
	{
		RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "THE MOVE ID MUST BE UNIQUE!");
		response->id = -1;
	}
}

void get_tile(const shared_ptr<database_interfaces::srv::GetTile::Request> request,
shared_ptr<database_interfaces::srv::GetTile::Response> response)
{

	int move_id;
	
	try
	{	
		try
		{
			move_id = DatabaseManager::get_move_id(db_connection, request->move_name);
		}
		catch(const pqxx::syntax_error) // IF Move Table Doese Not Exist
		{
			response->size = -1;
			response->thickness = -1;
			RCLCPP_ERROR(rclcpp::get_logger("rclpcpp"), "MOVE TABLE DOES NOT EXIST");
		}
		
		if (move_id == -1)
		{
			response->size = -1;
			response->thickness = -1;
			RCLCPP_ERROR(rclcpp::get_logger("rclpcpp"), "MOVE TABLE DOES NOT EXIST");
		}

		// thickness and size is be -1 if it is null
		response->size = DatabaseManager::get_tile_size(db_connection, move_id);
		response->thickness = DatabaseManager::get_tile_thickness(db_connection, move_id);
	}
	catch(const pqxx::syntax_error)
	{
		RCLCPP_ERROR(rclcpp::get_logger("rclpcpp"), "TILE TABLE DOES NOT EXIST!");
		response->size = -1;
		response->thickness = -1;
	}
}

void update_tile(const shared_ptr<database_interfaces::srv::UpdateTile::Request> request,
shared_ptr<database_interfaces::srv::UpdateTile::Response> response)
{
	int move_id;

	try
	{	
		try
		{
			move_id = DatabaseManager::get_move_id(db_connection, request->move_name);
		}
		catch(const pqxx::syntax_error) // IF Move Table Doese Not Exist
		{
			response->result = false;
			RCLCPP_ERROR(rclcpp::get_logger("rclpcpp"), "MOVE TABLE DOES NOT EXIST");
		}

		if (move_id == -1)
		{
			response->result = false;
			RCLCPP_ERROR(rclcpp::get_logger("rclpcpp"), "MOVE TABLE DOES NOT EXIST");
		}

		response->result = DatabaseManager::update_tile(db_connection, request->size, request->thickness,
		move_id);
	}
	catch(const pqxx::syntax_error) // IF Tile Table Doese Not Exist
	{
		response->result = false;
		RCLCPP_ERROR(rclcpp::get_logger("rclpcpp"), "MOVE TILE DOES NOT EXIST");
	}
	catch(const pqxx::integrity_constraint_violation) 
	{
		RCLCPP_ERROR(rclcpp::get_logger("rclpcpp"), "THE MOVE ID MUST BE UNIQUE!");
		response->result = false;
	}
}


void change_language_setting(const shared_ptr<database_interfaces::srv::SetLanguage::Request> request,
shared_ptr<database_interfaces::srv::SetLanguage::Response> response)
{
	try
	{
		response->result = DatabaseManager::update_language_settings(db_connection, request->lanuage);
		RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "THE SETTING LANGUAGE CHANGED TO : %s", request->lanuage);
	}
	catch (const pqxx::undefined_table)
	{
		try
		{
			DatabaseManager::create_language_table(db_connection);
			RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "CREATED language_settings TABLE");
			response->result = DatabaseManager::update_language_settings(db_connection, request->lanuage);
			RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "THE SETTING LANGUAGE CHANGED TO : %s", request->lanuage);
		}
		catch (const std::exception &e)
		{
			RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), e.what());
		}
	}
	catch (const std::exception &e)
	{
		RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), e.what());
	}
}

void get_language_setting(const shared_ptr<database_interfaces::srv::GetLanguage::Request> request,
shared_ptr<database_interfaces::srv::GetLanguage::Response> &response)
{
	try
	{
		response->language = DatabaseManager::get_language_settings(db_connection);
	}
	catch(const std::exception &e)
	{
		RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), e.what());
	}
}

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::Node::SharedPtr node = rclcpp::Node::make_shared("database_node");
    
    rclcpp::Service<database_interfaces::srv::AddMove>::SharedPtr add_move_service =
    node->create_service<database_interfaces::srv::AddMove>("add_move", &add_move);

    rclcpp::Service<database_interfaces::srv::DeleteMove>::SharedPtr delete_move_service =
    node->create_service<database_interfaces::srv::DeleteMove>("delete_move", &delete_move);

    rclcpp::Service<database_interfaces::srv::GetMove>::SharedPtr get_move_service =
    node->create_service<database_interfaces::srv::GetMove>("get_move", &get_move);

    rclcpp::Service<database_interfaces::srv::GetMoves>::SharedPtr get_moves_service = 
    node->create_service<database_interfaces::srv::GetMoves>("get_moves", &get_moves);

	rclcpp::Service<database_interfaces::srv::EditMove>::SharedPtr edit_move_service = 
    node->create_service<database_interfaces::srv::EditMove>("edit_move", &edit_move);

	rclcpp::Service<database_interfaces::srv::AddLog>::SharedPtr add_log_service = 
    node->create_service<database_interfaces::srv::AddLog>("add_log", &add_log);

	rclcpp::Service<database_interfaces::srv::GetLog>::SharedPtr get_log_service = 
    node->create_service<database_interfaces::srv::GetLog>("get_log", &get_log);

	rclcpp::Service<database_interfaces::srv::GetLogs>::SharedPtr get_logs_service = 
    node->create_service<database_interfaces::srv::GetLogs>("get_logs", &get_logs);

	rclcpp::Service<database_interfaces::srv::DeleteLog>::SharedPtr delete_log_service = 
    node->create_service<database_interfaces::srv::DeleteLog>("delete_log", &delete_log);

	rclcpp::Service<database_interfaces::srv::DeleteLogs>::SharedPtr delete_all_logs_service = 
    node->create_service<database_interfaces::srv::DeleteLogs>("delete_all_logs", &delete_all_logs);

	rclcpp::Service<database_interfaces::srv::CheckPassword>::SharedPtr check_password_service =
	node->create_service<database_interfaces::srv::CheckPassword>("check_password", &check_password);

	rclcpp::Service<database_interfaces::srv::UpdatePassword>::SharedPtr update_password_service =
	node->create_service<database_interfaces::srv::UpdatePassword>("update_password", &update_password);

	rclcpp::Service<database_interfaces::srv::AddTileLayout>::SharedPtr add_tile_layout_service =
	node->create_service<database_interfaces::srv::AddTileLayout>("add_tile_layout", &add_tile_layout);

	rclcpp::Service<database_interfaces::srv::GetTileLayout>::SharedPtr get_tile_layout_service =
	node->create_service<database_interfaces::srv::GetTileLayout>("get_tile_layout", &get_tile_layout);

	rclcpp::Service<database_interfaces::srv::UpdateTileLayout>::SharedPtr update_tile_layout_service =
	node->create_service<database_interfaces::srv::UpdateTileLayout>("update_tile_layout", &update_tile_layout);

	rclcpp::Service<database_interfaces::srv::AddTile>::SharedPtr add_tile_service =
	node->create_service<database_interfaces::srv::AddTile>("add_tile", &add_tile);

	rclcpp::Service<database_interfaces::srv::GetTile>::SharedPtr get_tile_service =
	node->create_service<database_interfaces::srv::GetTile>("get_tile", &get_tile);

	rclcpp::Service<database_interfaces::srv::UpdateTile>::SharedPtr update_tile_service =
	node->create_service<database_interfaces::srv::UpdateTile>("update_tile", &update_tile);

	rclcpp::Service<database_interfaces::srv::SetLanguage>::SharedPtr set_language_service =
	node->create_service<database_interfaces::srv::SetLanguage>("set_language", &change_language_setting);

	rclcpp::Service<database_interfaces::srv::GetLanguage>::SharedPtr get_language_service =
	node->create_service<database_interfaces::srv::GetLanguage>("get_language", &get_language_setting);

    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Database Server Ready");
    
    rclcpp::spin(node);
    rclcpp::shutdown();

    return 0;
}