#include "../include/crow_all.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>

std::optional<std::string> validateLogData(const nlohmann::json& log) {
	if (!log.contains("artist") || !log["artist"].is_string() || log["artist"].get<std::string>().empty()) {  
	  return "Invalid or missing artist";	
	}

	if (!log.contains("song_title") || !log["song_title"].is_string() || log["song_title"].get<std::string>().empty()) {
	  return "Invalid or missing song title";
	}

	if (!log.contains("album") || !log["album"].is_string() || log["album"].get<std::string>().empty()) { 
	  return "Invalid or missing album";
	}
	
	return std::nullopt;	
}


std::string utcTimestamp() {

   // Get the current time point from the system clock
    std::chrono::time_point now = std::chrono::system_clock::now();

    // Convert the time point to a std::time_t for printing (pre-C++20)
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // Convert to UTC
    tm* utc = std::gmtime(&now_c);

    // Format time as ISO 8601
    std::stringstream ss;
    ss << std::put_time(utc, "%Y-%m-%dT%H:%M:%SZ");

    std::string timestamp = ss.str();

    return timestamp;

}


crow::response makeErrorResponse(const std::string& field, const std::string& details){
   
  nlohmann::json metadata = {
    {"status", "error"},
    {"timestamp", utcTimestamp()} 
  };

  nlohmann::json jsonError;
  
  jsonError["metadata"] = metadata;
  
  jsonError["data"]["error"] = {
    {"field", field},
    {"details", details}
  };
  
   
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
		
		std::optional<std::string> validationError =  validateLogData(log);
	
		if (validationError) {
		  return makeErrorResponse("validation", *validationError);
		}	
		
		
		logs.push_back(log);

		nlohmann::json metadata = {
		  {"status", "success"},
		  {"timestamp", utcTimestamp()}
		}; 
		
		nlohmann::json data = {
		  {"log", log}
		};
			
		nlohmann::json success = {
			{"metadata", metadata},
			{"data", data}
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
		
		nlohmann::json data = {
		  {"log", body}
		};
		
		nlohmann::json metadata = {
		  {"status", "success"},
		  {"timestamp", utcTimestamp()}
		};
		
		
		nlohmann::json success = {
		  {"metadata", metadata},
		  {"data", data}
		};	

		res.body = success.dump(4);
		
		return res;
  	}); 
 
 app.port(18080).multithreaded().run();

}

