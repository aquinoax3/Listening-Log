#include "../include/crow_all.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>

int main() {
 
 std::vector<nlohmann::json> logs;
 
 std::string filename = "logs.json";
 
 std::ifstream infile(filename);

 if (infile) {
  try {
     nlohmann::json savedLogs;
     infile >> savedLogs;

     for (const auto& entry : savedLogs) { 
       logs.push_back(entry);
     }
   }  catch (const std::exception& e) {
       std::cerr << "Error Parsing " << filename << ": " << e.what() << std::endl;
   }
 }
 
 crow::SimpleApp app;

 CROW_ROUTE(app,"/")([](){
		 return "Hello World";
	});
 
 CROW_ROUTE(app,"/logs").methods(crow::HTTPMethod::POST)([&logs](const crow::request& req){
		
		
		nlohmann::json log;

		try {
		  log = nlohmann::json::parse(req.body);
		} catch (const std::exception& e) {
		  std::cerr << "Invalid JSON format" << ": " << e.what() << std::endl;
		  return crow::response(400);
		}
		
		if (!log.contains("artist") || !log["artist"].is_string() || log["artist"].get<std::string>().empty()) {
		nlohmann::json jsonError;

		jsonError["error"] = "artist field is required";
		jsonError["details"] = "artist field is empty and it must be a string";
		jsonError["status_code"] = 400;

		crow::response res;
		res.code = 400;
		res.set_header("Content-Type", "application/json");
		res.body = jsonError.dump();

		return res;
		}

		if (!log.contains("song_title") || !log["song_title"].is_string() || log["song_title"].get<std::string>().empty()) {
		  return crow::response(400); 
		}

		if (!log.contains("album") || !log["album"].is_string() || log["album"].get<std::string>().empty()) {
		   return crow::response(400);
		}	
		
		
		logs.push_back(log); 
		
		crow::response res;
		res.code = 201;
		res.set_header("Content-Type", "application/json");		
		res.body = log.dump();

		std::ofstream file("logs.json");
		if (file) {
		  nlohmann::json allLogs = logs;
		  file << allLogs.dump();
		  std::cout << "Logs saved successfully." << std::endl;
		} else {
		  std::cerr << "Unable to write logs.json" << std::endl;
		}
		
		return res;		
	});
 

 CROW_ROUTE(app, "/logs").methods(crow::HTTPMethod::GET)([&logs](){
		crow::response res;
		res.code = 200;
		res.set_header("Content-Type", "application/json");

		nlohmann::json body = logs;
		res.body = body.dump();
		
		return res;
  	}); 
 
 app.port(18080).multithreaded().run();

}

