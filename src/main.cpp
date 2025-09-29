#include "../include/crow_all.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>


auto makeErrorResponse(std::string field, std::string details){
  nlohmann::json jsonError;

  jsonError["error"] = field + " field is required.";
  jsonError["details"] = details;
  jsonError["status_code"] = 400;

  crow::response res;
  res.code = 400;
  res.set_header("Content-Type", "application/json");
  res.body = jsonError.dump();

  return res;
}

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
		  return makeErrorResponse("artist","artist field is empty and it must be a string");
		
		}

		if (!log.contains("song_title") || !log["song_title"].is_string() || log["song_title"].get<std::string>().empty()) {
		  return makeErrorResponse("song_title", "song title field is empty and it must be a string"); 
		}

		if (!log.contains("album") || !log["album"].is_string() || log["album"].get<std::string>().empty()) { 
		  return makeErrorResponse("album", "album field is empty and it must be a string");
		}	
		
		
		logs.push_back(log); 
	
		nlohmann::json success = {
			{"message", "Log created successfully"},
			{"log", log}
		};	
		
		crow::response res;
		res.code = 201;
		res.set_header("Content-Type", "application/json");		
		res.body = success.dump(4);

		std::ofstream file("logs.json", std::ios::trunc);
		if (file) {
		  nlohmann::json allLogs = logs;
		  file << allLogs.dump(4);
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
		
		nlohmann::json success = {
		  {"message", "Request succeeded"},
		  {"logs", body}
		};

		res.body = success.dump(4);
		
		return res;
  	}); 
 
 app.port(18080).multithreaded().run();

}

