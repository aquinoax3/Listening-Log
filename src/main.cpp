#include "../include/crow_all.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>

int main() {
 
 std::vector<nlohmann::json> logs;

 nlohmann::json testLog;

 testLog["artist"] = "Clipse";
 testLog["song_title"] = "Mr.Metoo";
 testLog["album"] = "Lord Willing";

 logs.push_back(testLog);
  
 std::ifstream file("logs.json");

 if (file) {
  nlohmann::json savedLogs;
  file >> savedLogs;

  for (const auto& entry : savedLogs) { 
   logs.push_back(entry);
  }
  
 }
 
 crow::SimpleApp app;

 CROW_ROUTE(app,"/")([](){
		 return "Hello World";
	});
 //TODO: Create POST endpoint for log
 CROW_ROUTE(app,"/log").methods(crow::HTTPMethod::POST)([&logs](){
		crow::response res;
		res.code = 201;
		res.set_header("Content-Type", "application/json");
		
		nlohmann::json log;
		
		std::string artist;
		std::string song_title;
		std::string album;
		 		
		log["artist"] = artist;
		log["song_title"] = song_title;
		log["album"] = album;
		
		logs.push_back(log);
		res.body = log.dump();

		std::ofstream file("log.json");
		file << log.dump();
		
		return res;		
	});
 
//TODO: Create GET endpoint for log
 CROW_ROUTE(app, "/log").methods(crow::HTTPMethod::GET)([&logs](){
		crow::response res;
		res.code = 200;
		res.set_header("Content-Type", "application/json");

		nlohmann::json body = logs;
		res.body = body.dump();
		
		return res;
  	}); 
 
 app.port(18080).multithreaded().run();

}

