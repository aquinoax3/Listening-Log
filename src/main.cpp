#include "../include/crow_all.h"
#include "../include/json.hpp"
#include <iostream>


int main() {
 
 nlohmann::json doc;

 std::string logData = {R"({
	"artist": "Frank Ocean", 
	"song_title": "Lost",
	"album": "Channel Orange"
	})"};
 
 doc = nlohmann::json::parse(logData);
 
 crow::SimpleApp app;

 CROW_ROUTE(app,"/")([](){
		 return "Hello World";
	});
 //TODO: Create POST endpoint for log
 //CROW_ROUTE(app,"/log")([](){
		 
//		 });
 
//TODO: Create GET endpoint for log
 CROW_ROUTE(app, "/log")([&doc](){
		crow::response res;
		res.code = 200;
		res.set_header("Content-Type", "application/json");
		res.body = doc.dump();
		return res;
  	}); 
 app.port(18080).multithreaded().run();

}

