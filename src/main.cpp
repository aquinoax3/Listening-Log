#include "../include/crow_all.h"


int main() {

 crow::SimpleApp app;

 CROW_ROUTE(app,"/")([](){
		 return "Hello World";
	});
 //TODO: Create POST endpoint for log
 //TODO: Create GET endpoint for log
  
 app.port(18080).multithreaded().run();

}

