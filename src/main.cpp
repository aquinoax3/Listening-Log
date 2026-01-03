#include "../include/crow_all.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <random>


std::optional<std::pair<std::string, std::string>> validateLogData(const nlohmann::json& log) {
	
	std::pair<std::string, std::string> result;	
	
	if (!log.contains("artist")) {
	  result.first =  "artist";
	  result.second =  "Missing required field";
	  
	  return result;
	}

	if (!log["artist"].is_string()) {
	  result.first =  "artist";
	  result.second =  "Must be a string";
	  
	  return result;

	}

	if (log["artist"].get<std::string>().empty()) {  
  	  result.first =  "artist";
	  result.second =  "Cannot be empty";
	  
	  return result;
	
	}
	
	//TODO: Add the remaining fields to be checked
	if (!log.contains("song_title")) {
	  result.first =  "song_title";
	  result.second =  "Missing required field";
	
	  return result;
        }

      	if (!log["song_title"].is_string()) {
	  result.first =  "song_title";
	  result.second =  "Must be a string";
	
	  return result;

        }

      	if (log["song_title"].get<std::string>().empty()) {  
	  result.first =  "song_title";
	  result.second =  "Cannot be empty";
	
	  return result;
      
      	}
      
	if (!log.contains("album")) {
	  result.first =  "album";
	  result.second =  "Missing required field";
	  
	  return result;
	}

	if (!log["album"].is_string()) {
	  result.first =  "album";
	  result.second =  "Must be a string";
	  
	  return result;

	}

	if (log["album"].get<std::string>().empty()) {  
  	  result.first =  "album";
	  result.second =  "Cannot be empty";
	  
	  return result;
	
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

std::string generateId(std::mt19937 generator) {

  std::string timestamp = utcTimestamp();

  std::uniform_int_distribution<int> dist(1,9);
  
  std::string randomNumber = std::to_string(dist(generator));

  return timestamp + "-" + randomNumber; 
}

int main() {
 
 std::vector<nlohmann::json> logs;
 
 std::string filename = "logs.json";
 
 std::ifstream infile(filename);
 
 // Get random number from hardware to seed
 std::random_device rd;

 // random generator
 std::mt19937 gen(rd());

 std::string testId = generateId(gen);
 std::cout << "Random Test Id: " << testId << std::endl;

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
		
		std::optional<std::pair<std::string, std::string>> validationError =  validateLogData(log);
	
		if (validationError) {
		  return makeErrorResponse(validationError->first, validationError->second);
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

