#ifndef MODELTRACKER_H
#define MODELTRACKER_H
#include <string>
#include  <map>

extern class Model;
namespace ModelTracker
{
extern std::map<std::string, Model*> models;
	
}


#endif;