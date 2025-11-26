#include "../include/crow_all.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>


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

std::optional<std::string> validateMetaData(const nlohmann::json& metadata) {
	if (!metadata.contains("artist") || !metadata["artist"].is_string() || metadata["artist"].get<std::string>().empty()) {  
	  return "Invalid or missing artist";	
	}

	if (!metadata.contains("song_title") || !metadata["song_title"].is_string() || metadata["song_title"].get<std::string>().empty()) {
	  return "Invalid or missing song title";
	}

	if (!metadata.contains("album") || !metadata["album"].is_string() || metadata["album"].get<std::string>().empty()) { 
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


    std::cout << "Timestamp : " << timestamp << std::endl;

    // Print the current time in a human-readable format
    //std::cout << "Current time: " << std::ctime(&now_c) << std::endl;

   return timestamp;

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
		
		std::optional<std::string> validationError =  validateMetaData(log);
	
	//TO DO: Figure out how to handle validation error, redefine res schema now that metadata is included	
		//if (validationError) {
		  //return makeErrorResponse(validationError);
		//}	
		
		
		logs.push_back(log);

		nlohmann::json metadata = {
		  {"status", "success"},
		  {"timestamp", utcTimestamp()}
		}; 
		
		nlohmann::json log = {
		  {"log", log}
		};
			
		nlohmann::json success = {
			{"metadata", metadata},
			{"data", log}
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

