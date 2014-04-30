#include "rapidjson\document.h"
#include "rapidjson\stringbuffer.h"
#include "rapidjson\filestream.h";
#include "rapidjson\prettywriter.h";
#include "rapidjson\rapidjson.h";
#include "rapidjson\reader.h";
#include "rapidjson\writer.h";
#include "Object.h"
#include <vector>
#include <glm/gtc/type_ptr.hpp>

class gameState
{
public:
	gameState();
	~gameState();
	void addObject(Object*);
	void removeObject(int);
	std::vector<Object*> getObjectByType(string);
	std::string getJSONStringFull();
	std::string getJSONStringModified();
	std::vector<Object*> parseJSONString(std::string);
	std::string getPosString(std::vector<std::pair<string, mat4>>*);
	std::vector<std::pair<string, mat4> >* parsePosString(std::string);

private:
	std::vector<Object*> objects;
	std::vector<int> openIndices;

};

gameState::gameState()
{

}

gameState::~gameState()
{
}

std::string getJSONStringFull(){

}
std::vector<Object* > gameState::parseJSONString(std::string str){
	rapidjson::Document parsedFromString;
	parsedFromString.SetObject();
	parsedFromString.Parse<0>(str.c_str());
}

std::string gameState::getPosString(std::vector<std::pair<string, mat4>>* v){
	rapidjson::Document fromScratch;
	fromScratch.SetObject();
	rapidjson::Document::AllocatorType& allocator = fromScratch.GetAllocator();
	rapidjson::Value array(rapidjson::kArrayType);
	std::string temp;
	for (int h = 0; h < v->size(); ++h){
		rapidjson::Value array(rapidjson::kArrayType);
		for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j){
			temp = "";
			temp += std::to_string(v->at(h).second[i][j]);
			array.PushBack(v->at(h).second[i][j], allocator);
		}
		fromScratch.AddMember(v->at(h).first.c_str(), array, allocator);
	}

	rapidjson::StringBuffer strbuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
	fromScratch.Accept(writer);
	return strbuf.GetString();

}

std::vector<std::pair<string, mat4> >* gameState::parsePosString(std::string str){
	rapidjson::Document parsedFromString;
	parsedFromString.SetObject();
	parsedFromString.Parse<0>(str.c_str());
	std::vector<std::pair<string, mat4> >* pos = new std::vector<std::pair<string, mat4> >;
	if (!parsedFromString["player1"].IsNull()){
		float nums[16] = { 0.0 };
		int k = 0;
		for (int i = 0; i < 4; ++i){
			for (int j = 0; j < 4; ++j){
				nums[i * 4 + j] = (float)parsedFromString["player1"][k].GetDouble();
				k++;
			}
		}

		glm::mat4 bbb = (glm::make_mat4(nums));
		pos->push_back(std::make_pair("player1", bbb));
	}
	if (!parsedFromString["player2"].IsNull()){
		float nums[16] = { 0.0 };
		int k = 0;
		for (int i = 0; i < 4; ++i){
			for (int j = 0; j < 4; ++j){
				nums[i * 4 + j] = (float)parsedFromString["player2"][k].GetDouble();
				k++;
			}
		}

		glm::mat4 bbb = (glm::make_mat4(nums));
		pos->push_back(std::make_pair("player2", bbb));
	}
	if (!parsedFromString["player3"].IsNull()){
		float nums[16] = { 0.0 };
		int k = 0;
		for (int i = 0; i < 4; ++i){
			for (int j = 0; j < 4; ++j){
				nums[i * 4 + j] = (float)parsedFromString["player3"][k].GetDouble();
				k++;
			}
		}

		glm::mat4 bbb = (glm::make_mat4(nums));
		pos->push_back(std::make_pair("player3", bbb));
	}
	if (!parsedFromString["player4"].IsNull()){
		float nums[16] = { 0.0 };
		int k = 0;
		for (int i = 0; i < 4; ++i){
			for (int j = 0; j < 4; ++j){
				nums[i * 4 + j] = (float)parsedFromString["player4"][k].GetDouble();
				k++;
			}
		}

		glm::mat4 bbb = (glm::make_mat4(nums));
		pos->push_back(std::make_pair("player4", bbb));
	}

	return pos;
}

void gameState::addObject(Object* obj){
	if (openIndices.size() == 0){
		objects.push_back(obj);
		obj->setGameStateIndex(objects.size() - 1);
	}
	else {
		objects[openIndices.back()] = obj;
		obj->setGameStateIndex(openIndices.back());
		openIndices.pop_back();
	}

}

void gameState::removeObject(int i){
	delete objects[i];
	objects[i] = NULL;
	openIndices.push_back(i);
}

std::vector<Object*> gameState::getObjectByType(string type){
	std::vector<Object*> res;
	for (int i = 0; i < objects.size(); ++i)
	if (objects[i]->getType() == type)
		res.push_back(objects[i]);

	return res;
}