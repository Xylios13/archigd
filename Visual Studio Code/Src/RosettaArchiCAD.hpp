#ifndef _ROSETTA_ARCHICAD_H
#define _ROSETTA_ARCHICAD_H


#undef UNICODE

#ifndef WIN32_LEAN_AND_MEAN

#define WIN32_LEAN_AND_MEAN

#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "APIEnvir.h"

#include "UniString.hpp"
#include "VectorImage.hpp"
#include "ProfileAdditionalInfo.hpp"
#include "ACAPinc.h"					// also includes APIdefs.h
#include "APICommon.h"
#include "Messages.pb.h"
#include "FileSystem.hpp"
#include "Folder.hpp"
#include <iostream>
#include <fstream>

#include <list>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message_lite.h>


typedef void(*ScriptFunction)(void);
typedef std::map <std::string, ScriptFunction> scriptMap;
typedef std::map <std::string, short> buildingMaterialMap;
typedef std::map <std::string, int> objectsMap;
typedef std::map <std::string, int> stringIntMap;
typedef std::map <std::string, std::string> stringStringMap;
typedef std::map <std::string, char*> stringCharMap;


// =============================================================================
//
// Communication
//
// =============================================================================

SOCKET getClientSocket();

SOCKET getListenSocket();

void quit();

int getUserQuit();

google::protobuf::uint32 readHdr(char *buf, const int bufSize, bool* success);

//Used to write a delimited message
//sends size followed by the message
bool writeDelimitedTo(SOCKET ClientSocket, const google::protobuf::MessageLite& message);

//Used to read a delimited message
//reads size followed by the message
bool readDelimitedFrom(SOCKET ClientSocket, google::protobuf::MessageLite* message);

//Send Element Id to Racket
void sendElementID(SOCKET ClientSocket, API_Element element, bool crash);

void sendElementID(SOCKET ClientSocket, API_Element element);

bool hasError(GSErrCode err);

int winsockcom(bool infiniteCycle);

DWORD WINAPI MyThreadFunction(LPVOID lpParam);

// =============================================================================
//
// Maps
//
// =============================================================================
short searchMaterials(std::string name, buildingMaterialMap map);

short searchBuildingMaterials(std::string name);

short searchCompositeMaterials(std::string name);

short searchOverrideMaterials(std::string name);

int searchObjects(std::string name, objectsMap map);

int searchObjects(std::string name);


std::string searchMaterialsValue(short n, buildingMaterialMap map);

std::string searchBuildingMaterialsValue(short n);

std::string searchCompositeMaterialsValue(short n);

std::string searchOverrideMaterialsValue(short n);


std::string searchObjectsValue(int n, objectsMap map);

std::string searchObjectsValue(int n);


int searchStringInt(std::string name, stringIntMap map);

std::string searchStringIntValue(int value, stringIntMap map);

std::string searchProfileName(int value);

int searchStringIntTypes(std::string name);

int searchStringIntProfileTypes(std::string name);

int searchLineTypes(std::string name);

std::string searchLineTypes(int index);

int searchLayers(std::string name);

std::string searchLayers(int index);


std::string searchStringString(std::string s, stringStringMap map);

std::string searchParentsID(std::string s);

char* searchLibGUIDS(std::string s);


void profileTypesInsert(std::string name, int index);

void layersInsert(std::string name, int index);

void addLibraryPart(API_LibPart libpart);

void setupBuildingMaterialTable();

void setupCompositeMaterialTable();

void setupOverrideMaterialTable();

void setupLinesTable();

void setupLibraryPartTable();

void setupProfileHashTable();

void setupHashTables();

void callScript(const std::string& pFunction);

// =============================================================================
//
// Auxiliar
//
// =============================================================================

void msgArchiCAD(int msg);

void msgArchiCAD(double msg);

void msgArchiCAD(std::string msg);

static bool	 Do_LibpartSearchByUnID(API_ElemTypeID	 typeID, API_AddParType  ***markAddPars, Int32 *origLibInd)
{
	Int32                addParNum;
	API_AddParType       **addPars = NULL;
	double               a, b;
	GSErrCode            err = NoError;
	API_LibPart			 libpart;
	char				 unid1[128];
	char				 unid2[128];
	char				 unid3[128];
	char				 unid4[128];

	switch (typeID) {
	case API_DetailID:
		strcpy(unid1, "{73B06DCB-55C0-4573-8635-8439A8DC3FAE}-{120765D3-2EB2-4CE8-8B65-5C8BEA1FD9DF}");
		strcpy(unid2, "{D8E3B58C-1B61-4660-B556-E59CA780A217}-{4BC17A74-4707-475C-B8C3-4AAEB5E66BE9}");
		strcpy(unid3, "{2C409D23-C2A3-4446-85AE-FFFB501C6F72}-{6F776393-7555-4983-986E-2AF112F75DB2}");	//in ArchiCAD.dll
		strcpy(unid4, "");
		break;
	case API_WorksheetID:
		strcpy(unid1, "{84E14FDD-FE29-497C-9C9F-6BCE7286F0A6}-{FE0304BA-5512-4D12-B6D9-B2CC8D873300}");	// Basic Worksheet Marker
		strcpy(unid2, "");																					// Simple Worksheet Marker
		strcpy(unid3, "");
		strcpy(unid4, "");
		break;
	case API_CutPlaneID:
		strcpy(unid1, "{0650F435-2C8A-4182-B311-271E890AD1AC}-{B6BC3B82-F221-47C2-8D29-8C32B30064E1}");	// Basic Section Marker
		strcpy(unid2, "{6DD7E0E5-C35D-44C6-9C6D-D9090C5C2FBE}-{D05C595C-0842-492D-B979-6D5930FF7094}");	// Basic Section-Elev Marker
		strcpy(unid3, "{A533987C-D38E-4F01-A790-5ACC99A18DCA}-{99992154-2372-4781-B0B1-8E788632014E}");	// Section-Elevation Marker - 70
		strcpy(unid4, "");
		break;
	case API_ElevationID:
		strcpy(unid1, "{8ABB49D8-B53D-4D92-88E6-7467DC20D491}-{055E1FBF-5FE0-426B-9289-16FDB143491B}");	// Basic Section Marker
		strcpy(unid2, "");
		strcpy(unid3, "");
		strcpy(unid4, "");
		break;
	case API_InteriorElevationID:
		strcpy(unid1, "{510BC02A-85AE-4598-8EC6-2F333C26EC5D}-{7F67D33C-66B7-4576-9F45-A69F20AB3D2D}");	// Built-in IE Marker
		strcpy(unid2, "{724B7CEC-92C1-41A3-883E-3100D6F274DB}-{D38663B1-5C13-4288-B951-60D118E429C2}");	// Basic Section-Elev Marker
		strcpy(unid3, "");
		strcpy(unid4, "");
		break;
	case API_DoorID:
		strcpy(unid1, "{292E664B-B671-48AD-BA71-3EF25461DA99}-{311E9D20-7449-4F5E-BB78-43C00FDDE7E5}");
		strcpy(unid2, "{50DC8FE6-BF72-DB44-8B8C-9BC7B68508F7}-{7A1DD325-2EDD-464B-9D11-0BE5021FBF42}");	//in ArchiCAD.dll
		strcpy(unid3, "{036F7C0F-7E36-47B1-8B70-D0CD051029B6}-{867B3F0C-22B5-4ECB-A905-2ADDFCFC1D90}");	//in ArchiCAD.dll
		strcpy(unid4, "");
		break;
	case API_WindowID:
		strcpy(unid1, "{036F7C0F-7E36-47B1-8B70-D0CD051029B6}-{867B3F0C-22B5-4ECB-A905-2ADDFCFC1D90}");
		strcpy(unid2, "{C651965D-E5DA-4920-926D-F873108FEB7F}-{287E3C61-0B8C-4082-BE2A-6AF18E824A7E}");	//in ArchiCAD.dll
		strcpy(unid3, "{8410F22D-3B0E-6943-B4F2-EDDBAA8C91D9}-{C8F52822-AA23-49CD-A256-B67B5A084CDA}");	//in ArchiCAD.dll
		strcpy(unid4, "");
		break;
	case API_SkylightID:
		strcpy(unid1, "{0654C626-430D-11DF-85ED-521F56D89593}-{4CBB1F66-5DD9-45ED-9E5A-ED0DB90E6648}");
		strcpy(unid2, "{4E82DDBC-2782-4A64-9E4F-64C0E772B794}-{46FF0DEF-0F75-42F6-B5DD-6F62913FC6A5}");	//in ArchiCAD.dll
		strcpy(unid3, "{58BDEC28-D93F-4262-95CE-D537B32C7BE6}-{3C3B52C1-432A-4E22-ABD9-F57A4600E8B7}");	//in ArchiCAD.dll
		strcpy(unid4, "");
		break;
	default:
		break;
	}

	BNZeroMemory(&libpart, sizeof(libpart));
	strcpy(libpart.ownUnID, unid1);
	err = ACAPI_LibPart_Search(&libpart, false);
	if (err == NoError && *origLibInd != libpart.index) {
		err = ACAPI_LibPart_GetParams(libpart.index, &a, &b, &addParNum, &addPars);
		if (err == NoError) {
			delete libpart.location;
			*origLibInd = libpart.index;
			*markAddPars = addPars;
			return true;
		}
	}

	BNZeroMemory(&libpart, sizeof(libpart));
	strcpy(libpart.ownUnID, unid2);
	err = ACAPI_LibPart_Search(&libpart, false);
	if (err == NoError && *origLibInd != libpart.index) {
		err = ACAPI_LibPart_GetParams(libpart.index, &a, &b, &addParNum, &addPars);
		if (err == NoError) {
			delete libpart.location;
			*origLibInd = libpart.index;
			*markAddPars = addPars;
			return true;
		}
	}

	BNZeroMemory(&libpart, sizeof(libpart));
	strcpy(libpart.ownUnID, unid3);
	err = ACAPI_LibPart_Search(&libpart, false);
	if (err == NoError && *origLibInd != libpart.index) {
		err = ACAPI_LibPart_GetParams(libpart.index, &a, &b, &addParNum, &addPars);
		if (err == NoError) {
			delete libpart.location;
			*origLibInd = libpart.index;
			*markAddPars = addPars;
			return true;
		}
	}

	BNZeroMemory(&libpart, sizeof(libpart));
	strcpy(libpart.ownUnID, unid4);
	err = ACAPI_LibPart_Search(&libpart, false);
	if (err == NoError && *origLibInd != libpart.index) {
		err = ACAPI_LibPart_GetParams(libpart.index, &a, &b, &addParNum, &addPars);
		if (err == NoError) {
			delete libpart.location;
			*origLibInd = libpart.index;
			*markAddPars = addPars;
			return true;
		}
	}

	markAddPars = NULL;

	return false;
}	// Do_LibpartSearchByUnID

int	pointRelation(float x1, float x2);

void multTMX(double *resMat, double *aMatrix);

void multMatrix(double* a, double* b, double* r);

static GSErrCode GetLocation(IO::Location*& loc, bool useEmbeddedLibrary){
	GS::Array<API_LibraryInfo>	libInfo;
	loc = NULL;

	GSErrCode err = NoError;

	if (useEmbeddedLibrary) {
		Int32 embeddedLibraryIndex = -1;
		// get embedded library location
		if (ACAPI_Environment(APIEnv_GetLibrariesID, &libInfo, &embeddedLibraryIndex) == NoError && embeddedLibraryIndex >= 0) {
			try {
				loc = new IO::Location(libInfo[embeddedLibraryIndex].location);
			}
			catch (std::bad_alloc&) {
				return APIERR_MEMFULL;
			}

			if (loc != NULL) {
				IO::Location ownFolderLoc(*loc);
				ownFolderLoc.AppendToLocal(IO::Name("LibPart_Test Library"));
				err = IO::fileSystem.CreateFolder(ownFolderLoc);
				if (err == NoError || err == IO::FileSystem::TargetExists)
					loc->AppendToLocal(IO::Name("LibPart_Test Library"));
			}
		}
	}
	else {
		// register our own folder and create the library part in it
		if (ACAPI_Environment(APIEnv_GetLibrariesID, &libInfo) == NoError) {
			IO::Location folderLoc;
			API_SpecFolderID specID = API_UserDocumentsFolderID;
			ACAPI_Environment(APIEnv_GetSpecFolderID, &specID, &folderLoc);
			folderLoc.AppendToLocal(IO::Name("LibPart_Test Library"));
			IO::Folder destFolder(folderLoc, IO::Folder::Create);
			if (destFolder.GetStatus() != NoError || !destFolder.IsWriteable())
				return APIERR_GENERAL;

			loc = new IO::Location(folderLoc);

			for (UInt32 ii = 0; ii < libInfo.GetSize(); ii++) {
				if (folderLoc == libInfo[ii].location)
					return NoError;
			}

			try {
				API_LibraryInfo		li;
				li.location = folderLoc;

				libInfo.Push(li);
			}
			catch (const GS::OutOfMemoryException&) {
				DBBREAK_STR("Not enough memory");
				return APIERR_MEMFULL;
			}

			ACAPI_Environment(APIEnv_SetLibrariesID, &libInfo);
		}
	}

	return NoError;
}

void handleParams(additionalparams msg, API_ParamOwnerType* paramOwner, API_GetParamsType* getParams);

void prepareParams(additionalparams* msg, API_ParamOwnerType* paramOwner, API_GetParamsType* getParams);

void populateMemo(API_ElementMemo *memo, pointsmessage pts, polyarcsmessage arcs);

void writeMaterialsWall();

void turnRefreshOn();

void turnRefreshOff();

bool getVisualFeedback();

void render();

int getLayerNumber(std::string name);

void groupElements();

void deleteAllElements();

void holeMemo(int oldSize, int oldPends, int oldArcSize, pointsmessage pointsMsg, polyarcsmessage polyarcsMsg, API_ElementMemo oldMemo, API_ElementMemo* memo, int* newSize, int* newPends, int* newArcSize);
// =============================================================================
//
// Geometry
//
// =============================================================================

void createCircle();

void createLine();

void createPolyLine();

void createSpline();

void createArc();

void createSphere();

void createCylinder();

void createMorph();

void testcreateMorph();

void createBox();

void changeMorphTrans();

void applyMatrixMorph();

//----------- Shells

void createSimpleShell();

void createShell();

void createComplexShell();

void createRevolvedShell();

void createExtrudedShell();

void rotateShell();

void translateShell();

void createHoleInShell();

// =============================================================================
//
// Objects
//
// =============================================================================

void createWall();

void createNewWall();

void createPolyWall();

void createMultiWall();

void createDoor();

void internalcreateWindow(windowmessage windowMsg);

void createWindow();

void createNewCurtainWall();

void transformCW();

void addArcsToElement();

void createColumn();

void createNewColumn();

void createBeam();

void createSlab();

void createNewSlab();

void createWallsFromSlab();

void createCWallsFromSlab();

void createColumnsFromSlab();

void createObject();

void createLibraryPart();

void createRoof();

void createNewRoof();

void createPolyRoof();

void createHole();

void createHoleTest();

void createStairs();

void createMesh();

void translateElement();

void rotateElementZ();

void trimElement();

void intersections(API_Coord* polygon, int nPoints, API_Coord beg, API_Coord end, std::list<API_Coord>* intersections);

void intersectWall();

void destructiveIntersectWall();

void mirrorElement();

//----------- Profiles

void buildProfileDescription(profilemsg msg, VectorImage* image);

void modifyProfileDescription(profilemsg msg, VectorImage& previousImage, VectorImage* image);

void createProfile();

void addHatchWholeProfile();

//----------- Layers

void internalCreateLayer(layermsg msg);

void createLayer();

void attributeLayerToElement();

void controlLayer(layermsg msg, bool show);

void hideLayer();

void showLayer();

bool hiddenLayer(layermsg msg);

void testFunction();

//----------- Camera

void createCamera();

// =============================================================================
//
// Stories
//
// =============================================================================

int getCurrentLevel();

void checkStory();

void checkStoryAbove();

void checkStoryBelow();

void createStory();

void upperLevel();

void createStoryAbove();

void createStoryBelow();

void chooseStory();

void deleteStories();

void getLevels();

void view2D();

void view3D();

void refresh3DView();

// =============================================================================
//
// ElementManagement
//
// =============================================================================

void deleteElements();

void getWalls();

void getSlabs();

void getColumns();

void getObjects();

void getRoofs();

void getMeshes();

void getLines();

void getPolyLines();

void selectElement();

void highlightElementByID();

void openFile();

void elementRefresh(API_Element* element);

void elementChange(API_Element* element, API_ElementMemo* memo, API_Element* mask);

#endif