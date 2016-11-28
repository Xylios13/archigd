#include "RosettaArchiCAD.hpp"
/*
#undef UNICODE

#ifndef WIN32_LEAN_AND_MEAN

#define WIN32_LEAN_AND_MEAN

#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "APIEnvir.h"

#include "ACAPinc.h"					// also includes APIdefs.h
#include "APICommon.h"
#include "Messages.pb.h"
#include "RosettaArchiCAD.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message_lite.h>
*/
//TODO ELIMINATE THIS PLAGUE!!!
int currentLevel = 0;

int getCurrentLevel(){
	return currentLevel;
}

void checkStory(){
	API_StoryInfo		storyInfo;
	GSErrCode			err;
	storyinfo			storyInfoMsg;
	char buffer[256];

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}
	storyInfoMsg.set_exists(true);
	storyInfoMsg.set_index(currentLevel);
	storyInfoMsg.set_level((*storyInfo.data)[currentLevel - storyInfo.firstStory].level);
	storyInfoMsg.set_name((*storyInfo.data)[currentLevel - storyInfo.firstStory].name);

	writeDelimitedTo(getClientSocket(), storyInfoMsg);

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

void checkStoryAbove(){
	API_StoryInfo		storyInfo;
	GSErrCode			err;
	storymsg			storyMsg;
	storyinfo			storyInfoMsg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &storyMsg);

	storyInfoMsg.set_exists(false);
	storyInfoMsg.set_index(0);
	storyInfoMsg.set_level(0);
	storyInfoMsg.set_name("");

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	storyInfoMsg.set_level(storyMsg.height() + (*storyInfo.data)[currentLevel].level);


	for (int i = storyInfo.lastStory - storyInfo.firstStory; i >= 0; i--) {
		//sprintf(buffer, "level: %f height: %f", (*storyInfo.data)[i].level, storyMsg.height());
		//ACAPI_WriteReport(buffer, true);
		if ((*storyInfo.data)[i].level == (storyMsg.height() + (*storyInfo.data)[currentLevel].level)){
			storyInfoMsg.set_exists(true);
			storyInfoMsg.set_index((*storyInfo.data)[i].index);
			storyInfoMsg.set_level((*storyInfo.data)[i].level);
			storyInfoMsg.set_name((*storyInfo.data)[i].name);
			break;
		}
	}

	//int numberOfStories = BMhGetSize(reinterpret_cast<GSHandle> (storyInfo.data)) / Sizeof32(API_StoryType) - 1;
	//for (int i = 0; i < numberOfStories; i++){}

	writeDelimitedTo(getClientSocket(), storyInfoMsg);


	BMKillHandle(reinterpret_cast<GSHandle*> (&storyInfo.data));
	return;
}

void checkStoryBelow(){
	API_StoryInfo		storyInfo;
	GSErrCode			err;
	storymsg			storyMsg;
	storyinfo			storyInfoMsg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &storyMsg);

	storyInfoMsg.set_exists(false);
	storyInfoMsg.set_index(0);
	storyInfoMsg.set_level(0);
	storyInfoMsg.set_name("");

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}
	storyInfoMsg.set_level((*storyInfo.data)[currentLevel - storyInfo.firstStory].level - storyMsg.height());

	for (int i = storyInfo.lastStory - storyInfo.firstStory; i >= 0; i--) {
		if ((*storyInfo.data)[i].level == ((*storyInfo.data)[currentLevel - storyInfo.firstStory].level - storyMsg.height())){
			storyInfoMsg.set_exists(true);
			storyInfoMsg.set_index((*storyInfo.data)[i].index);
			storyInfoMsg.set_level((*storyInfo.data)[i].level);
			storyInfoMsg.set_name((*storyInfo.data)[i].name);
			break;
		}
	}

	writeDelimitedTo(getClientSocket(), storyInfoMsg);

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

void createStory(){
	API_StoryInfo		storyInfo;
	GSErrCode			err;
	storymsg			storyMsg;
	API_StoryCmdType	storyCmd;
	storyinfo			storyInfoMsg;
	bool				newHeight = true;
	char buffer[256];
	double				diff = 10000000;
	double				currentDiff = 0;
	readDelimitedFrom(getClientSocket(), &storyMsg);

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	int numberOfStories = BMhGetSize(reinterpret_cast<GSHandle> (storyInfo.data)) / Sizeof32(API_StoryType) - 1;

	for (int i = 0; i < numberOfStories; i++){
		//sprintf(buffer, "level: %f height: %f", (*storyInfo.data)[i].level, storyMsg.height());
		//ACAPI_WriteReport(buffer, true);
		double doublediff = (*storyInfo.data)[i].level - storyMsg.height();
		//Originally 1 and -1 for range
		if (doublediff < 0.05 && doublediff > -0.05){
			newHeight = false;
			currentLevel = (*storyInfo.data)[i].index;
			break;
		}

		if (storyMsg.height() >= 0){
			currentDiff = storyMsg.height() - (*storyInfo.data)[i].level;
		}
		else{
			if ((*storyInfo.data)[i].level <= 0){
				currentDiff = abs(storyMsg.height()) - abs((*storyInfo.data)[i].level);
			}
			else{
				currentDiff = diff;
			}
		}
		if (currentDiff >= 0 && currentDiff < diff){
			diff = currentDiff;
			currentLevel = (*storyInfo.data)[i].index;
		}

	}

	if (newHeight){
		BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
		bool hasUpperLevel = false;
		double upperLevelHeight = 0;
		if (currentLevel + 1 < numberOfStories){
			hasUpperLevel = true;
			upperLevelHeight = (*storyInfo.data)[currentLevel + 1 - storyInfo.firstStory].level;
		}
		storyCmd.index = currentLevel;
		storyCmd.height = abs(storyMsg.height() - (*storyInfo.data)[currentLevel - storyInfo.firstStory].level);
		for (int i = 0; i < storyMsg.name().size(); i++){
			storyCmd.uName[i] = storyMsg.name().c_str()[i];
		}
		if (storyMsg.height() >= 0){
			storyCmd.action = APIStory_InsAbove;
			currentLevel++;
		}
		else{
			storyCmd.action = APIStory_InsBelow;
			currentLevel--;
		}

		err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
		if (hasError(err)){
			quit();
			return;
		}

		err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
		if (err != NoError) {
			ErrorBeep("APIEnv_GetStorySettingsID", err);
			return;
		}
		numberOfStories = BMhGetSize(reinterpret_cast<GSHandle> (storyInfo.data)) / Sizeof32(API_StoryType) - 1;
		
		if (hasUpperLevel){
			storyCmd.action = APIStory_SetHeight;
			storyCmd.index = currentLevel;
			storyCmd.height = upperLevelHeight - storyMsg.height();
			//msgArchiCAD(storyCmd.height);
			err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
			if (hasError(err)){
				quit();
				return;
			}
		}
		/*
		for (int i = 0; i < numberOfStories; i++){
			double doublediff = (*storyInfo.data)[i].level - storyMsg.height();
			if (doublediff < 0.1 && doublediff > -0.1){
				
				if (i == (numberOfStories - 1)){
					break;
				}
				else{	
					storyCmd.action = APIStory_SetHeight;
					storyCmd.index = currentLevel;
					storyCmd.height = (*storyInfo.data)[currentLevel + 1 - storyInfo.firstStory].level - (2 * storyMsg.height());
					msgArchiCAD(storyCmd.height);
					err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
					if (hasError(err)){
						quit();
						return;
					}
				}
				
			}
		}*/
	}
	
	storyInfoMsg.set_exists(true);
	storyInfoMsg.set_index(currentLevel);
	storyInfoMsg.set_level((*storyInfo.data)[currentLevel - storyInfo.firstStory].level);
	storyInfoMsg.set_name("Story");

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));

	writeDelimitedTo(getClientSocket(), storyInfoMsg);
	/*
	storyCmd.action = APIStory_GoTo;
	storyCmd.index = currentLevel;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
	ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
	*/
}

/*
void upperLevel(){
	API_StoryInfo		storyInfo;
	GSErrCode			err;
	upperlevelmsg		upperLevelMsg;
	API_StoryCmdType	storyCmd;
	storyinfo			storyInfoMsg;
	bool				newHeight = true;
	double				diff = 10000000;
	double				currentDiff = 0;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &upperLevelMsg);

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	int numberOfStories = BMhGetSize(reinterpret_cast<GSHandle> (storyInfo.data)) / Sizeof32(API_StoryType) - 1;
	double absoluteHeight = upperLevelMsg.height() + (*storyInfo.data)[upperLevelMsg.index()].level;
	//currentLevel = upperLevelMsg.index();
	for (int i = (upperLevelMsg.index() - storyInfo.firstStory); i < numberOfStories; i++){
		//if ((*storyInfo.data)[i].level == absoluteHeight){
		double doublediff = (*storyInfo.data)[i].level - absoluteHeight;
		if (doublediff < 0.1 && doublediff > -0.1){
			newHeight = false;
			currentLevel = (*storyInfo.data)[i].index;
			break;
		}
		currentDiff = absoluteHeight - (*storyInfo.data)[i].level;

		if (currentDiff >= 0 && currentDiff < diff){
			diff = currentDiff;
			currentLevel = (*storyInfo.data)[i].index;
		}
	}
	std::string name = "Story";
	
	if (newHeight){
		BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
		storyCmd.index = currentLevel;
		//storyCmd.height = abs(upperLevelMsg.height() - (*storyInfo.data)[currentLevel - storyInfo.firstStory].level);
		storyCmd.height = diff;
		strcpy(storyCmd.name, "Story");
		storyCmd.action = APIStory_InsAbove;
		currentLevel++;

		err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
		if (hasError(err)){
			quit();
			return;
		}

		err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
		if (err != NoError) {
			ErrorBeep("APIEnv_GetStorySettingsID", err);
			return;
		}
		numberOfStories = BMhGetSize(reinterpret_cast<GSHandle> (storyInfo.data)) / Sizeof32(API_StoryType) - 1;

		for (int i = 0; i < numberOfStories; i++){
			double doublediff = (*storyInfo.data)[i].level - upperLevelMsg.height();
			if (doublediff < 0.1 && doublediff > -0.1){
				if (i == (numberOfStories - 1)){
					break;
				}
				else{
					storyCmd.action = APIStory_SetHeight;
					storyCmd.index = currentLevel;
					storyCmd.height = (*storyInfo.data)[currentLevel + 1 - storyInfo.firstStory].level - (2 * upperLevelMsg.height());
					err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
					if (hasError(err)){
						quit();
						return;
					}
				}
			}
		}

	}
	//int upperlevelindex = currentLevel + 1;
	storyInfoMsg.set_exists(true);
	storyInfoMsg.set_index(currentLevel);
	storyInfoMsg.set_level((*storyInfo.data)[currentLevel - storyInfo.firstStory].level);
	storyInfoMsg.set_name("Story");

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));

	writeDelimitedTo(getClientSocket(), storyInfoMsg);

}
*/

void upperLevel(){
	API_StoryInfo		storyInfo;
	GSErrCode			err;
	upperlevelmsg		upperLevelMsg;
	API_StoryCmdType	storyCmd;
	storyinfo			storyInfoMsg;
	bool				newHeight = true;
	double				diff = 10000000;
	double				currentDiff = 0;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &upperLevelMsg);

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	int numberOfStories = BMhGetSize(reinterpret_cast<GSHandle> (storyInfo.data)) / Sizeof32(API_StoryType) - 1;
	double absoluteHeight = upperLevelMsg.height() + (*storyInfo.data)[upperLevelMsg.index()].level;
	//currentLevel = upperLevelMsg.index();
	for (int i = (upperLevelMsg.index() - storyInfo.firstStory); i < numberOfStories; i++){
		//if ((*storyInfo.data)[i].level == absoluteHeight){
		double doublediff = (*storyInfo.data)[i].level - absoluteHeight;
		if (doublediff < 0.1 && doublediff > -0.1){
			newHeight = false;
			currentLevel = (*storyInfo.data)[i].index;
			break;
		}
		currentDiff = absoluteHeight - (*storyInfo.data)[i].level;

		if (currentDiff >= 0 && currentDiff < diff){
			diff = currentDiff;
			currentLevel = (*storyInfo.data)[i].index;
		}
	}
	std::string name = "Story";
	/*
	for (int i = 0; i < numberOfStories; i++){
	//sprintf(buffer, "level: %f height: %f", (*storyInfo.data)[i].level, storyMsg.height());
	//ACAPI_WriteReport(buffer, true);
	if (((*storyInfo.data)[i].level + pastLevelsHeights) == upperLevelMsg.height()){
	newHeight = false;
	currentLevel = (*storyInfo.data)[i].index;
	break;
	}
	pastLevelsHeights += (*storyInfo.data)[i].level;

	currentDiff = upperLevelMsg.height() - (*storyInfo.data)[i].level;

	if (currentDiff >= 0 && currentDiff < diff){
	diff = currentDiff;
	currentLevel = (*storyInfo.data)[i].index;
	}

	}
	*/
	if (newHeight){
		BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
		storyCmd.index = currentLevel;
		//storyCmd.height = abs(upperLevelMsg.height() - (*storyInfo.data)[currentLevel - storyInfo.firstStory].level);
		storyCmd.height = diff;
		storyCmd.uName[0] = 'S';
		storyCmd.uName[1] = 't';
		storyCmd.uName[2] = 'o';
		storyCmd.uName[3] = 'r';
		storyCmd.uName[4] = 'y';
		storyCmd.action = APIStory_InsAbove;
		currentLevel++;

		err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
		if (hasError(err)){
			quit();
			return;
		}

		err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
		if (err != NoError) {
			ErrorBeep("APIEnv_GetStorySettingsID", err);
			return;
		}
		numberOfStories = BMhGetSize(reinterpret_cast<GSHandle> (storyInfo.data)) / Sizeof32(API_StoryType) - 1;

		for (int i = 0; i < numberOfStories; i++){
			double doublediff = (*storyInfo.data)[i].level - upperLevelMsg.height();
			if (doublediff < 0.1 && doublediff > -0.1){
				if (i == (numberOfStories - 1)){
					break;
				}
				else{
					storyCmd.action = APIStory_SetHeight;
					storyCmd.index = currentLevel;
					storyCmd.height = (*storyInfo.data)[currentLevel + 1 - storyInfo.firstStory].level - (2 * upperLevelMsg.height());
					err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
					if (hasError(err)){
						quit();
						return;
					}
				}
			}
		}

	}
	//int upperlevelindex = currentLevel + 1;
	storyInfoMsg.set_exists(true);
	storyInfoMsg.set_index(currentLevel);
	storyInfoMsg.set_level((*storyInfo.data)[currentLevel - storyInfo.firstStory].level);
	storyInfoMsg.set_name("Story");

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));

	writeDelimitedTo(getClientSocket(), storyInfoMsg);
	/*
	storyCmd.action = APIStory_GoTo;
	storyCmd.index = currentLevel - 1;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
	ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
	*/

}

void createStoryAbove(){
	API_StoryCmdType	storyCmd;
	GSErrCode			err;
	storymsg			storyMsg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &storyMsg);

	BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
	storyCmd.action = APIStory_InsAbove;
	storyCmd.index = currentLevel;
	storyCmd.height = storyMsg.height();

	for (int i = 0; i < storyMsg.name().size(); i++){
		storyCmd.uName[i] = storyMsg.name().c_str()[i];
	}

	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
	currentLevel++;
	/*
	storyCmd.action = APIStory_GoTo;
	storyCmd.index = currentLevel;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
	ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
	*/
}

void createStoryBelow(){
	API_StoryCmdType	storyCmd;
	GSErrCode			err;
	storymsg			storyMsg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &storyMsg);

	BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
	storyCmd.action = APIStory_InsBelow;
	//storyCmd.index = (short)(storyInfo.firstStory - storyInfo.lastStory);
	storyCmd.index = currentLevel;
	storyCmd.height = storyMsg.height();

	for (int i = 0; i < storyMsg.name().size(); i++){
		storyCmd.uName[i] = storyMsg.name().c_str()[i];
	}

	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);

		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}
	currentLevel--;
	/*
	storyCmd.action = APIStory_GoTo;
	storyCmd.index = currentLevel;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
	ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
	*/
}

void chooseStory(){
	doublemessage		doubleMsg;
	API_StoryCmdType	storyCmd;
	GSErrCode			err;

	readDelimitedFrom(getClientSocket(), &doubleMsg);

	storyCmd.action = APIStory_GoTo;
	storyCmd.index = (int)doubleMsg.d();
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
	else{
		currentLevel = (int)doubleMsg.d();
	}
}

void deleteStories(){
	API_StoryInfo		storyInfo;
	API_StoryCmdType	storyCmd;
	GSErrCode			err;
	char buffer[256];

	BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));

	/*
	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	storyCmd.action = APIStory_InsAbove;
	storyCmd.index = storyInfo.lastStory;
	storyCmd.height = 10;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
	*/

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	int numberOfStories = BMhGetSize(reinterpret_cast<GSHandle> (storyInfo.data)) / Sizeof32(API_StoryType) - 1;

	storyCmd.action = APIStory_Delete;
	storyCmd.dispOnSections = false;
	storyCmd.dontRebuild = true;
	for (int i = 0; i < numberOfStories; i++){
		storyCmd.index = (*storyInfo.data)[i].index;
		msgArchiCAD((*storyInfo.data)[i].index);

		err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
		if (hasError(err)){
			quit();
			return;
		}
	}
	
	/*
	storyCmd.action = APIStory_Delete;
	for (int i = numberOfStories - 1; i >= 0; i--){
		storyCmd.index = (*storyInfo.data)[i].index;
		err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
		if (hasError(err)){
			quit();
			return;
		}
	}
	*/
	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (hasError(err)) {
		quit();
		return;
	}

	//sprintf(buffer, "index %d", (*storyInfo.data)[0].index);
	//ACAPI_WriteReport(buffer, true);

	currentLevel = 0;
	/*
	storyCmd.action = APIStory_GoTo;
	storyCmd.index = currentLevel;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
	ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
	*/
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));

	//ACAPI_CloseUndoableSession();

	//err = ACAPI_OpenUndoableSession("Geometry Test -- Create elements");
	//if (hasError(err)) {
	//	quit();
	//	return;
	//}
}

void getLevels(){
	API_StoryInfo		storyInfo;
	GSErrCode			err;
	levelrepeated		levelsMsg;
	storyinfo*			storyInfoMsg;
	char buffer[256];

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}
	for (int i = 0; i <= storyInfo.lastStory - storyInfo.firstStory; i++){
		//for (int i = storyInfo.lastStory - storyInfo.firstStory; i >= 0; i--) {
		//sprintf(buffer, "level: %f height: %f", (*storyInfo.data)[i].level, storyMsg.height());
		//ACAPI_WriteReport(buffer, true);
		storyInfoMsg = levelsMsg.add_levels();
		storyInfoMsg->set_exists(true);
		storyInfoMsg->set_index((*storyInfo.data)[i].index);
		storyInfoMsg->set_level((*storyInfo.data)[i].level);
		storyInfoMsg->set_name((*storyInfo.data)[i].name);
	}

	writeDelimitedTo(getClientSocket(), levelsMsg);

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

void view2D(){
	/*
	API_StoryCmdType	storyCmd;
	GSErrCode			err;
	char buffer[256];
	API_DatabaseInfo floorInfo;
	floorInfo.typeID = APIWind_FloorPlanID;
	floorInfo.index = 0;
	ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &floorInfo);

	
	storyCmd.action = APIStory_GoTo;
	storyCmd.index = 0;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (hasError(err)){
		quit();
		return;
	}
	*/

	API_StoryCmdType	storyCmd;
	API_WindowInfo      windowInfo;
	API_DatabaseInfo    planDB;
	API_Element         element;
	GS::Array<API_Guid> elemList;
	GSErrCode           err;

	BNZeroMemory(&planDB, sizeof(API_DatabaseInfo));
	planDB.typeID = APIWind_FloorPlanID;

	ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &planDB, NULL);

	BNZeroMemory(&windowInfo, sizeof(API_WindowInfo));
	windowInfo.typeID = APIWind_FloorPlanID;
	windowInfo.databaseUnId = planDB.databaseUnId;
	err = ACAPI_Automate(APIDo_ChangeWindowID, &windowInfo, NULL);
	if (hasError(err)){
		msgArchiCAD("Here");
		quit();
		return;
	}

	storyCmd.action = APIStory_GoTo;
	storyCmd.index = 0;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (hasError(err)){
		quit();
		return;
	}
}

void view3D(){
	GSErrCode err;
	char buffer[256];

	err = ACAPI_Automate(APIDo_ShowAllIn3DID, NULL, NULL);
	if (err != NoError) {
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		return;
	}
}

void refresh3DView(){
	GSErrCode		err;
	char buffer[256];

	//err = ACAPI_OpenUndoableSession("Geometry Test -- Create elements");

	err = ACAPI_Database(APIDb_RebuildCurrentDatabaseID);
	if (err != NoError){
		sprintf(buffer, ErrID_To_Name(err));
		//ACAPI_WriteReport(buffer, true);
	}

	//ACAPI_CloseUndoableSession();
}
