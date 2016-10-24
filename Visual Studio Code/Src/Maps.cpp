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
scriptMap m;

buildingMaterialMap buildingMaterials;
buildingMaterialMap compositeMaterials;
buildingMaterialMap overrideMaterials;
objectsMap objectsIds;
stringIntMap types;
stringIntMap profileTypes;
stringIntMap lineTypes;
stringIntMap layersTable;
stringStringMap parentsID;
stringCharMap libGUIDS;

short wallDefaultMaterial;
short slabDefaultMaterial;

short wallDefaultComposite;
short slabDefaultComposite;

short searchMaterials(std::string name, buildingMaterialMap map){
	buildingMaterialMap::const_iterator iter = map.find(name);
	if (iter == map.end()){
		// not found
		return -1;
	}
	else{
		return iter->second;
	}
}

short searchBuildingMaterials(std::string name){
	return searchMaterials(name, buildingMaterials);
}

short searchCompositeMaterials(std::string name){
	return searchMaterials(name, compositeMaterials);
}

short searchOverrideMaterials(std::string name){
	return searchMaterials(name, overrideMaterials);
}


int searchObjects(std::string name, objectsMap map){
	objectsMap::const_iterator iter = map.find(name);
	if (iter == map.end()){
		// not found
		return -1;
	}
	else{
		return iter->second;
	}
}

int searchObjects(std::string name){
	return searchObjects(name, objectsIds);
}


std::string searchMaterialsValue(short n, buildingMaterialMap map){
	for (buildingMaterialMap::const_iterator it = map.begin(); it != map.end(); ++it){
		if (it->second == n){
			return it->first;
		}
	}
	return "Not Found";
}

std::string searchBuildingMaterialsValue(short n){
	return searchMaterialsValue(n, buildingMaterials);
}

std::string searchCompositeMaterialsValue(short n){
	return searchMaterialsValue(n, compositeMaterials);
}

std::string searchOverrideMaterialsValue(short n){
	return searchMaterialsValue(n, overrideMaterials);
}


std::string searchObjectsValue(int n, objectsMap map){
	for (objectsMap::const_iterator it = map.begin(); it != map.end(); ++it){
		if (it->second == n){
			return it->first;
		}
	}
	return "Not Found";
}

std::string searchObjectsValue(int n){
	return searchObjectsValue(n, objectsIds);
}


int searchStringInt(std::string name, stringIntMap map){
	/*
	stringIntMap::const_iterator iter = types.find(name);
	if (iter == types.end()){
	return -1;
	}
	else{
	ACAPI_WriteReport("Found", true);
	return iter->second;
	}
	*/
	for (stringIntMap::const_iterator it = map.begin(); it != map.end(); ++it){
		if (it->first == name){
			return it->second;
		}
	}

	return -1;
}

std::string searchStringIntValue(int value, stringIntMap map){
	for (stringIntMap::const_iterator it = map.begin(); it != map.end(); ++it){
		if (it->second == value){
			return it->first;
		}
	}
	return "Not Found";
}

std::string searchProfileName(int value){
	return searchStringIntValue(value, profileTypes);
}

int searchStringIntTypes(std::string name){
	return searchStringInt(name, types);
}

int searchStringIntProfileTypes(std::string name){
	return searchStringInt(name, profileTypes);
}

int searchLineTypes(std::string name){
	return searchStringInt(name, lineTypes);
}

std::string searchLineTypes(int i){
	return searchStringIntValue(i, lineTypes);
}

int searchLayers(std::string name){
	return searchStringInt(name, layersTable);
}

std::string searchLayers(int index){
	return searchStringIntValue(index, layersTable); 
}

std::string searchStringString(std::string s, stringStringMap map){
	stringStringMap::const_iterator iter = map.find(s);
	if (iter == map.end()){
		return "";
	}
	else{
		return iter->second;
	}
}

std::string searchParentsID(std::string s){
	return searchStringString(s, parentsID);
}

char* searchLibGUIDS(std::string s){
	//return searchStringString(s, libGUIDS);
	stringCharMap::const_iterator iter = libGUIDS.find(s);
	if (iter == libGUIDS.end()){
		return "";
	}
	else{
		return iter->second;
	}
}

void profileTypesInsert(std::string name, int index){
	profileTypes.insert(std::make_pair(name, index));
}

void layersInsert(std::string name, int index){
	layersTable.insert(std::make_pair(name, index));
}

void addLibraryPart(API_LibPart libPart){
	objectsIds.insert(std::make_pair((const char *)GS::UniString(libPart.docu_UName).ToCStr(), libPart.index));
}

void setupBuildingMaterialTable(){
	API_Attribute attrib;
	short n;
	GSErrCode err;

	BNZeroMemory(&attrib, sizeof(API_Attribute));

	attrib.header.typeID = API_BuildingMaterialID;
	err = ACAPI_Attribute_GetNum(API_BuildingMaterialID, &n);
	for (int i = 1; i <= n && err == NoError; i++) {
		attrib.header.index = i;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			buildingMaterials.insert(std::make_pair(attrib.buildingMaterial.head.name, attrib.buildingMaterial.head.index));
		}
		if (err == APIERR_DELETED){
			err = NoError;
		}
	}
}

void setupCompositeMaterialTable(){
	API_Attribute attrib;
	short n;
	GSErrCode err;

	BNZeroMemory(&attrib, sizeof(API_Attribute));

	attrib.header.typeID = API_CompWallID;
	err = ACAPI_Attribute_GetNum(API_CompWallID, &n);
	for (int i = 1; i <= n && err == NoError; i++) {
		attrib.header.index = i;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			compositeMaterials.insert(std::make_pair(attrib.compWall.head.name, attrib.compWall.head.index));
		}
		if (err == APIERR_DELETED){
			err = NoError;
		}
	}
}

void setupOverrideMaterialTable(){
	API_Attribute attrib;
	short n;
	GSErrCode err;

	BNZeroMemory(&attrib, sizeof(API_Attribute));

	attrib.header.typeID = API_MaterialID;
	err = ACAPI_Attribute_GetNum(API_MaterialID, &n);
	for (int i = 1; i <= n && err == NoError; i++) {
		attrib.header.index = i;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			overrideMaterials.insert(std::make_pair(attrib.material.head.name, attrib.material.head.index));
		}
		if (err == APIERR_DELETED){
			err = NoError;
		}
	}
}

void setupLinesTable(){
	API_Attribute attrib;
	short n;
	GSErrCode err;

	BNZeroMemory(&attrib, sizeof(API_Attribute));

	attrib.header.typeID = API_LinetypeID;
	err = ACAPI_Attribute_GetNum(API_LinetypeID, &n);
	for (int i = 1; i <= n && err == NoError; i++) {
		attrib.header.index = i;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			lineTypes.insert(std::make_pair(attrib.linetype.head.name, attrib.linetype.head.index));
		}
		if (err == APIERR_DELETED){
			err = NoError;
		}
	}
}

void setupLibraryPartTable(){
	API_LibPart libPart;
	Int32 count;
	GSErrCode err;
	err = ACAPI_LibPart_GetNum(&count);
	if (!err) {
		for (int i = 1; i <= count; i++) {
			BNZeroMemory(&libPart, sizeof(API_LibPart));
			libPart.index = i;
			err = ACAPI_LibPart_Get(&libPart);
			if (!err) {
				if (libPart.typeID == APILib_ObjectID || libPart.typeID == APILib_DoorID){
					objectsIds.insert(std::make_pair((const char *)GS::UniString(libPart.docu_UName).ToCStr(), libPart.index));
					//(const char *)GS::UniString(libPart.ownUnID).ToCStr()
					libGUIDS.insert(std::make_pair((const char *)GS::UniString(libPart.docu_UName).ToCStr(), libPart.ownUnID));
				}
			}
			if (libPart.location != NULL)
				delete libPart.location;
		}
	}
}

void setupProfileHashTable(){
	API_Attribute		attrib;
	//API_AttributeDefExt	defs;
	short				count;
	GSErrCode			err = NoError;
	char buffer[256];

	BNZeroMemory(&attrib, sizeof(API_Attribute));

	err = ACAPI_Attribute_GetNum(API_ProfileID, &count);
	if (err != NoError){
		return;
	}

	for (int i = 1; i <= count; i++) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.header.typeID = API_ProfileID;
		attrib.header.index = i;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			profileTypes.insert(std::make_pair(attrib.header.name, attrib.header.index));
			/*
			err = ACAPI_Attribute_GetDefExt(API_ProfileID, attrib.header.index, &defs);
			if (err == APIERR_BADID) {
			BNZeroMemory(&defs, sizeof(API_AttributeDefExt));
			err = NoError;
			}
			if (err == NoError) {
			profileTypes.insert(std::make_pair(attrib.header.name, attrib.header.index));
			ACAPI_WriteReport(attrib.header.name, true);
			}
			ACAPI_DisposeAttrDefsHdlsExt(&defs);
			*/
		}
	}

	return;
}

void setupLayersTable(){
	API_Attribute		attrib;
	//API_AttributeDefExt	defs;
	short				count;
	GSErrCode			err = NoError;
	char buffer[256];

	BNZeroMemory(&attrib, sizeof(API_Attribute));

	err = ACAPI_Attribute_GetNum(API_LayerID, &count);
	if (err != NoError){
		return;
	}

	for (int i = 1; i <= count; i++) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.header.typeID = API_LayerID;
		attrib.header.index = i;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			layersTable.insert(std::make_pair(attrib.layer.head.name, attrib.layer.head.index));
			/*
			err = ACAPI_Attribute_GetDefExt(API_ProfileID, attrib.header.index, &defs);
			if (err == APIERR_BADID) {
			BNZeroMemory(&defs, sizeof(API_AttributeDefExt));
			err = NoError;
			}
			if (err == NoError) {
			profileTypes.insert(std::make_pair(attrib.header.name, attrib.header.index));
			ACAPI_WriteReport(attrib.header.name, true);
			}
			ACAPI_DisposeAttrDefsHdlsExt(&defs);
			*/
		}
	}

	return;
}

void setupHashTables(){
	API_Attribute attrib;
	short n;
	API_LibPart libPart;
	Int32 count;
	GSErrCode err;

	setupBuildingMaterialTable();

	setupCompositeMaterialTable();

	setupOverrideMaterialTable();

	setupLinesTable();

	setupLibraryPartTable();

	setupProfileHashTable();

	setupLayersTable();

	if (searchLayers("Default Layer") == -1){
		layermsg msg;
		msg.set_name("Default Layer");
		internalCreateLayer(msg);
	}

	wallDefaultMaterial = searchMaterials("GENERIC - STRUCTURAL", buildingMaterials);
	slabDefaultMaterial = searchMaterials("GENERIC - INTERNAL CLADDING", buildingMaterials);

	wallDefaultComposite = searchMaterials("Generic Wall/Shell", compositeMaterials);
	slabDefaultComposite = searchMaterials("Generic Slab/Roof", compositeMaterials);

	m.insert(std::make_pair("Wall", &createWall));
	m.insert(std::make_pair("NewWall", &createNewWall));
	m.insert(std::make_pair("MultiWall", &createMultiWall));
	m.insert(std::make_pair("Door", &createDoor));
	m.insert(std::make_pair("Window", &createWindow));
	m.insert(std::make_pair("Line", &createLine));
	m.insert(std::make_pair("PolyLine", &createPolyLine));
	m.insert(std::make_pair("Spline", &createSpline));
	m.insert(std::make_pair("Circle", &createCircle));
	m.insert(std::make_pair("Arc", &createArc));
	m.insert(std::make_pair("Sphere", &createSphere));
	m.insert(std::make_pair("Cylinder", &createCylinder));
	m.insert(std::make_pair("ComplexShell", &createComplexShell));
	m.insert(std::make_pair("SimpleShell", &createSimpleShell));
	m.insert(std::make_pair("Shell", &createShell));
	m.insert(std::make_pair("RotateShell", &rotateShell));
	m.insert(std::make_pair("TranslateShell", &translateShell));
	m.insert(std::make_pair("Hole", &createHoleInShell));
	m.insert(std::make_pair("CurtainWall", &createNewCurtainWall));
	m.insert(std::make_pair("CWTransform", &transformCW));
	m.insert(std::make_pair("Slab", &createSlab));
	m.insert(std::make_pair("NewSlab", &createNewSlab));
	m.insert(std::make_pair("HoleSlab", &createHole));
	m.insert(std::make_pair("WallsSlab", &createWallsFromSlab));
	m.insert(std::make_pair("CWallsSlab", &createCWallsFromSlab));
	m.insert(std::make_pair("ColumnsSlab", &createColumnsFromSlab));
	m.insert(std::make_pair("AddArcs", &addArcsToElement));
	m.insert(std::make_pair("Translate", &translateElement));
	m.insert(std::make_pair("RotateZ", &rotateElementZ));
	m.insert(std::make_pair("Mirror", &mirrorElement));
	m.insert(std::make_pair("Trim", &trimElement));
	m.insert(std::make_pair("IntersectWall", &intersectWall));
	m.insert(std::make_pair("DestructiveIntersectWall", &destructiveIntersectWall));
	m.insert(std::make_pair("Column", &createColumn));
	m.insert(std::make_pair("NewColumn", &createNewColumn));
	m.insert(std::make_pair("Beam", &createBeam));
	m.insert(std::make_pair("Object", &createObject));
	m.insert(std::make_pair("LibraryPart", &createLibraryPart));
	m.insert(std::make_pair("Stairs", &createStairs));
	m.insert(std::make_pair("Roof", &createRoof));
	m.insert(std::make_pair("NewRoof", &createNewRoof));
	m.insert(std::make_pair("PolyRoof", &createPolyRoof));
	m.insert(std::make_pair("Mesh", &createMesh));
	m.insert(std::make_pair("Morph", &createMorph));
	m.insert(std::make_pair("MorphTrans", &changeMorphTrans));
	m.insert(std::make_pair("RevShell", &createRevolvedShell));
	m.insert(std::make_pair("ExtShell", &createExtrudedShell));
	m.insert(std::make_pair("ApplyMatrix", &applyMatrixMorph));
	m.insert(std::make_pair("Box", &createBox));
	m.insert(std::make_pair("Profile", &createProfile));
	m.insert(std::make_pair("AddToProfile", &addHatchWholeProfile));
	m.insert(std::make_pair("Layer", &createLayer));
	m.insert(std::make_pair("LayerElem", &attributeLayerToElement));
	m.insert(std::make_pair("HideLayer", &hideLayer));
	m.insert(std::make_pair("ShowLayer", &showLayer));
	m.insert(std::make_pair("Story", &createStory));
	m.insert(std::make_pair("StoryAbove", &createStoryAbove));
	m.insert(std::make_pair("StoryBelow", &createStoryBelow));
	m.insert(std::make_pair("CheckStory", &checkStory));
	m.insert(std::make_pair("CheckStoryAbove", &checkStoryAbove));
	m.insert(std::make_pair("CheckStoryBelow", &checkStoryBelow));
	m.insert(std::make_pair("ChooseStory", &chooseStory));
	m.insert(std::make_pair("UpperLevel", &upperLevel));
	m.insert(std::make_pair("DeleteStories", &deleteStories));
	m.insert(std::make_pair("Delete", &deleteElements));
	m.insert(std::make_pair("Test", &testFunction));
	m.insert(std::make_pair("WriteMaterialsFile", &writeMaterialsWall));
	//m.insert(std::make_pair("Refresh", &refresh3DView));
	m.insert(std::make_pair("RefreshOn", &turnRefreshOn));
	m.insert(std::make_pair("RefreshOff", &turnRefreshOff));
	m.insert(std::make_pair("quit", &quit));
	m.insert(std::make_pair("Quit", &quit));

	m.insert(std::make_pair("SelectElement", &selectElement));
	m.insert(std::make_pair("Highlight", &highlightElementByID));

	m.insert(std::make_pair("2D", &view2D));
	m.insert(std::make_pair("3D", &view3D));

	m.insert(std::make_pair("Render", &render));
	m.insert(std::make_pair("Group", &groupElements));
	m.insert(std::make_pair("DeleteAll", &deleteAllElements));

	m.insert(std::make_pair("OpenFile", &openFile));

	m.insert(std::make_pair("GetLevels", &getLevels));
	m.insert(std::make_pair("GetWalls", &getWalls));
	m.insert(std::make_pair("GetSlabs", &getSlabs));
	m.insert(std::make_pair("GetColumns", &getColumns));
	m.insert(std::make_pair("GetObjects", &getObjects));
	m.insert(std::make_pair("GetRoofs", &getRoofs));
	m.insert(std::make_pair("GetLines", &getLines));
	m.insert(std::make_pair("GetPolyLines", &getPolyLines));
	m.insert(std::make_pair("HoleTest", &createHoleTest));

	//Setup Types map

	types.insert(std::make_pair("Zombie", API_ZombieLibID));
	types.insert(std::make_pair("Spec", APILib_SpecID));
	types.insert(std::make_pair("Window", APILib_WindowID));
	types.insert(std::make_pair("Door", APILib_DoorID));
	types.insert(std::make_pair("Object", APILib_ObjectID));
	types.insert(std::make_pair("Lamp", APILib_LampID));
	types.insert(std::make_pair("Room", APILib_RoomID));
	types.insert(std::make_pair("Property", APILib_PropertyID));
	types.insert(std::make_pair("PlanSign", APILib_PlanSignID));
	types.insert(std::make_pair("Label", APILib_LabelID));
	types.insert(std::make_pair("Macro", APILib_MacroID));
	types.insert(std::make_pair("Pict", APILib_PictID));
	types.insert(std::make_pair("ListScheme", APILib_ListSchemeID));
	types.insert(std::make_pair("Skylight", APILib_SkylightID));

	//Setup ParentsID map

	parentsID.insert(std::make_pair("Root", "{F938E33A-329D-4A36-BE3E-85E126820996}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("ModelElement", "{103E8D2C-8230-42E1-9597-46F84CCE28C0}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("Light", "{51A838E5-0F40-4F6D-A658-CF2845581749}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("Opening", "{6ACDA889-69B2-4EC5-936C-CB1DA7032A92}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("WallOpening", "{F6AE9687-2BC7-4D47-88C9-8F793E1DE2D6}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("WallDoor", "{11E85B84 - 8DD1 - 491B - A2FE - 337454A91545}-{00000000 - 0000 - 0000 - 0000 - 000000000000}"));
	parentsID.insert(std::make_pair("WallWindow", "{4ABD0A6E-634B-4931-B3AA-9BEE01F35CDF}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("PlanSign", "{4FD10D67-2F29-4844-A65A-6597589B0CB5}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("ZoneStamp", "{A73622DE-E40A-4E63-ADB7-52D13422BD06}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("Label", "{BDB8C3EE-4019-46C8-91D0-7A8DE0A5EC6D}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("Properties", "{8F157ABA-E5C9-48B6-9DCE-68F0F02A808E}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("WallEnd", "{0F059DC2-6053-11D7-9084-000393ABEA8E}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("Stair", "{57B7C584-5C0D-11D6-A0D8-036F034B6792}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("Marker", "{B3A106BF-6277-456E-8657-65B5C2D1C315}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("DetailMarker", "{BEE8F3CB-CF52-4029-9602-9B449461EF64}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("SectionMarker", "{930D0E3F-8BD2-4A54-A1F0-DAC3DB4531D4}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("WindowMarker", "{9C0F3C78-D2CB-4528-ADA2-703C8A09FC1A}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("EmptyDoor", "{80E9BA91-1494-4E24-85A7-A092857FB5C8}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("EmptyWindow", "{9647310C-5BD5-4874-8C3F-FEE520EC1820}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("BasicStamp", "{7E21C4B2-6113-4CDA-96C2-E658F17D9CA6}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("BasicSectMarker", "{E56F96A9-D028-477C-A21D-AC1225B15BDD}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("BasicDetailMarker", "{0585F813-1620-476A-A6B4-9EAD75FC9F17}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("BasicWindowMarker", "{036F7C0F-7E36-47B1-8B70-D0CD051029B6}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("SectionMarker70", "{13639454-0C08-4B20-94DA-8C9A5241DD56}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("WindowMarker70", "{C9A7085A-956D-4A59-A4F0-85B71215144A}-{00000000-0000-0000-0000-000000000000}"));
	parentsID.insert(std::make_pair("Railing", "{09BEB35E-7BF8-448C-B408-AC04D159B0D1}-{63BC5095-58DC-489D-ADBC-3376F29547AC}"));

}

void callScript(const std::string& pFunction){
	scriptMap::const_iterator iter = m.find(pFunction);
	if (iter == m.end()){
		// not found - quit
		quit();
	}
	else{
		(*iter->second)();
	}

}