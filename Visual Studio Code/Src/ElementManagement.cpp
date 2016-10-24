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

void deleteElements(){
	API_Element		element;
	elementidlist	elementsToDelete;
	GSErr err;

	char buffer[256];

	readDelimitedFrom(getClientSocket(), &elementsToDelete);

	BNZeroMemory(&element, sizeof(API_Element));

	API_Elem_Head* test;
	for (int i = 0; i < elementsToDelete.guid_size(); i++){
		element.header.guid = APIGuidFromString(elementsToDelete.guid(i).c_str());
		if (ACAPI_Element_Get(&element) == NoError){
			layermsg layerMsg;
			layerMsg.set_name(searchLayers(element.header.layer));
			bool hidden = hiddenLayer(layerMsg);
			if (hidden){
				controlLayer(layerMsg, true);
			}
			test = &element.header;
			err = ACAPI_Element_Delete(&test, 1);
			if (hidden){
				controlLayer(layerMsg, false);
			}
			if (hasError(err)){
				quit();
				return;
			}
		}
	}

}

void getWalls(){
	API_Element element;
	GSErrCode err;
	char buffer[256];
	getwallmsg msg;
	wallmsg* aux;
	pointsmessage* pts;
	polyarcsmessage* arcs;

	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_WallID, &elemList);

	for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate(); it != NULL; ++it) {
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			double x0 = element.wall.begC.x;
			double y0 = element.wall.begC.y;
			double x1 = element.wall.endC.x;
			double y1 = element.wall.endC.y;
			
			char s[256];
			APIGuid2GSGuid(element.header.guid).ConvertToString(s);
			msg.add_guid(s);

			aux = msg.add_walls();

			aux->set_bottomindex(element.header.floorInd);
			aux->set_thickness(element.wall.thickness);
			aux->set_upperindex(element.header.floorInd + 1);
			
			std::string materialName;

			if (element.wall.modelElemStructureType == API_BasicStructure){
				materialName = searchBuildingMaterialsValue(element.wall.buildingMaterial);
				aux->set_type("Basic");
			}
			else{
				materialName = searchCompositeMaterialsValue(element.wall.composite);
				aux->set_type("Composite");
			}

			if (materialName == "Not Found"){
				sprintf(buffer, "Found no material %d", element.wall.buildingMaterial);
				ACAPI_WriteReport(buffer, true);
			}
			else{
				aux->set_material(materialName);
			}

			if (element.wall.referenceLineLocation == APIWallRefLine_Center ||
				element.wall.referenceLineLocation == APIWallRefLine_CoreCenter){
				aux->set_referenceline("Center");
			}
			else if (element.wall.referenceLineLocation == APIWallRefLine_Outside ||
				element.wall.referenceLineLocation == APIWallRefLine_CoreOutside){
				aux->set_referenceline("Outside");
			}
			else if (element.wall.referenceLineLocation == APIWallRefLine_Inside ||
				element.wall.referenceLineLocation == APIWallRefLine_CoreInside){
				aux->set_referenceline("Inside");
			}
			else{
				sprintf(buffer, "Found no referenceLine %d", element.wall.buildingMaterial);
				ACAPI_WriteReport(buffer, true);
			}

			aux->set_alphaangle(element.wall.slantAlpha);
			aux->set_betaangle(element.wall.slantBeta);

			aux->set_profilename("");
			if (element.wall.profileType == APISect_Normal){
				aux->set_typeprofile("Normal");
			}
			else if (element.wall.profileType == APISect_Slanted){
				aux->set_typeprofile("Slanted");
			}
			else if (element.wall.profileType == APISect_Trapez){
				aux->set_typeprofile("DoubleSlanted");
			}
			else if (element.wall.profileType == APISect_Poly){
				aux->set_typeprofile("Poly");
				aux->set_profilename(searchProfileName(element.wall.profileAttr)); 
			}

			aux->set_height(element.wall.height);

			pts = new pointsmessage();
			pts->add_px(x0);
			pts->add_py(y0);
			pts->add_pz(0);
			pts->add_px(x1);
			pts->add_py(y1);
			pts->add_pz(0);
			aux->set_allocated_pts(pts);
			
			arcs = new polyarcsmessage();
			arcs->add_arcangle(element.wall.angle);
			arcs->add_begindex(0);
			arcs->add_endindex(1);
			aux->set_allocated_arcs(arcs);
			
			aux->set_flipped(!element.wall.flipped);

			aux->set_bottomoffset(element.wall.bottomOffset);
			
			aux->set_refoffset(element.wall.offset);

			if (element.wall.oppMat.overrideMaterial){
				aux->set_oppmat(searchOverrideMaterialsValue(element.wall.oppMat.material));
			}
			else{
				aux->set_oppmat("");
			}
			if (element.wall.refMat.overrideMaterial){
				aux->set_refmat(searchOverrideMaterialsValue(element.wall.refMat.material));
			}
			else{
				aux->set_refmat("");
			}
			if (element.wall.sidMat.overrideMaterial){
				aux->set_sidmat(searchOverrideMaterialsValue(element.wall.sidMat.material));
			}
			else{
				aux->set_sidmat("");
			}

			/*
			API_Attribute attr;
			attr.header.typeID = API_LayerID;
			attr.header.index = 1;
			err = ACAPI_Attribute_Get(&attr);
			if (hasError(err)){
				quit();
				return;
			}
			
			msgArchiCAD(attr.layer.head.name);
			aux->set_layer(attr.header.name);
			*/
			aux->set_layer(searchLayers(element.header.layer));

			GS::Array<API_Guid> windowList;
			ACAPI_Element_GetElemList(API_WindowID, &windowList);
			API_Element wElement;
			for (GS::Array<API_Guid>::ConstIterator it2 = windowList.Enumerate(); it2 != NULL; ++it2) {
				BNZeroMemory(&wElement, sizeof(API_Element));
				wElement.header.guid = *it2;
				err = ACAPI_Element_Get(&wElement);
				if (hasError(err)){
					quit();
					return;
				}
				if (wElement.window.owner == element.header.guid){
					windowmessage* wmsg = aux->add_windows();
					wmsg->set_height(wElement.window.openingBase.height);
					wmsg->set_width(wElement.window.openingBase.width);
					wmsg->set_objloc(wElement.window.objLoc);
					wmsg->set_zpos(wElement.window.lower);
					wmsg->set_guid(s);
					wmsg->set_name(searchObjectsValue(wElement.window.openingBase.libInd));
					wmsg->set_depthoffset(wElement.window.revealDepthOffset);
					wmsg->set_flipx(wElement.window.openingBase.oSide);
					wmsg->set_flipy(wElement.window.openingBase.reflected);
						
					aux->add_windoworder(0);

					API_ParamOwnerType   paramOwner;
					API_GetParamsType    getParams;

					BNZeroMemory(&paramOwner, sizeof(API_ParamOwnerType));
					paramOwner.guid = wElement.header.guid;
					paramOwner.libInd = 0;
					paramOwner.typeID = wElement.header.typeID;

					BNZeroMemory(&getParams, sizeof(API_GetParamsType));

					err = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner, NULL);
					if (err == NoError) {
						err = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, NULL);
						if (err == NoError) {
							additionalparams* wparams = new additionalparams();
							prepareParams(wparams, &paramOwner, &getParams);
							wmsg->set_allocated_params(wparams);
						}
					}
				}
			}
		}
		else{
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}
	}

	writeDelimitedTo(getClientSocket(), msg);
}

void getSlabs(){
	API_Element element;
	API_ElementMemo memo;
	GSErrCode err;
	char buffer[256];
	slabrepeated slabMsg;
	API_StoryInfo storyInfo;
	storyinfo* storyInfoMsg;
	pointsmessage* points;
	intlistmsg* subpolygons;

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_SlabID, &elemList);
	for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate(); it != NULL; ++it) {

		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			err = ACAPI_Element_GetMemo(element.header.guid, &memo);
			if (err == NoError){
				points = slabMsg.add_points();
				subpolygons = slabMsg.add_subpolygons();
				for (int i = 1; i <= (element.slab.poly.nCoords - 1); i++){
					points->add_px((*memo.coords)[i].x);
					points->add_py((*memo.coords)[i].y);
					points->add_pz((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].level);
				}

				for (int i = 1; i <= element.slab.poly.nSubPolys; i++){
					subpolygons->add_ilist((*memo.pends)[i]);
				}

				storyInfoMsg = slabMsg.add_bottomlevel();
				storyInfoMsg->set_exists(true);
				storyInfoMsg->set_index(element.header.floorInd);
				storyInfoMsg->set_level((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].level);
				storyInfoMsg->set_name((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].name);

				slabMsg.add_thickness(element.slab.thickness);

				std::string materialName;

				if (element.slab.modelElemStructureType == API_BasicStructure){
					materialName = searchBuildingMaterialsValue(element.slab.buildingMaterial);
					slabMsg.add_type("Basic");
				}
				else{
					materialName = searchCompositeMaterialsValue(element.slab.composite);
					slabMsg.add_type("Composite");
				}
				if (materialName == "Not Found"){
					sprintf(buffer, "Found no material");
					ACAPI_WriteReport(buffer, true);
				}
				else{
					slabMsg.add_material(materialName);
				}
				char s[64];
				APIGuid2GSGuid(element.header.guid).ConvertToString(s);
				slabMsg.add_guid(s);

			}
		}
		else{
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}
	}
	writeDelimitedTo(getClientSocket(), slabMsg);
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

//void	getRoofs(){}

void getColumns(){
	API_Element element;
	GSErrCode err;
	char buffer[256];
	columnrepeated columnMsg;
	API_StoryInfo		storyInfo;
	storyinfo* storyInfoMsg;

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_ColumnID, &elemList);
	for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate(); it != NULL; ++it) {

		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			columnMsg.add_px(element.column.origoPos.x);
			columnMsg.add_py(element.column.origoPos.y);

			storyInfoMsg = columnMsg.add_bottomlevel();
			storyInfoMsg->set_exists(true);
			storyInfoMsg->set_index(element.header.floorInd);
			storyInfoMsg->set_level((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].level);
			storyInfoMsg->set_name((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].name);

			storyInfoMsg = columnMsg.add_toplevel();
			storyInfoMsg->set_exists(true);
			storyInfoMsg->set_index(element.header.floorInd + 1);
			storyInfoMsg->set_level((*storyInfo.data)[element.header.floorInd + 1 - storyInfo.firstStory].level);
			storyInfoMsg->set_name((*storyInfo.data)[element.header.floorInd + 1 - storyInfo.firstStory].name);

			columnMsg.add_circular(element.column.circleBased);
			columnMsg.add_angle(element.column.angle);
			columnMsg.add_depth(element.column.coreDepth);
			columnMsg.add_width(element.column.coreWidth);
			columnMsg.add_slantangle(element.column.slantAngle);
			columnMsg.add_slantdirection(element.column.slantDirectionAngle);

			char s[64];
			APIGuid2GSGuid(element.header.guid).ConvertToString(s);
			columnMsg.add_guid(s);

		}
		else{
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}
	}
	//writeDelimitedTo(wallInfo, raw_out);
	writeDelimitedTo(getClientSocket(), columnMsg);
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

void getObjects(){
	API_Element element;
	GSErrCode err;
	char buffer[256];
	objectrepeated objectMsg;
	API_StoryInfo		storyInfo;
	storyinfo* storyInfoMsg;

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_ObjectID, &elemList);
	for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate(); it != NULL; ++it) {

		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {

			std::string objectName = searchObjectsValue(element.object.libInd);

			if (objectName == "Not Found"){
				sprintf(buffer, "Found no material");
				ACAPI_WriteReport(buffer, true);
			}
			else{
				objectMsg.add_name(objectName);
			}


			objectMsg.add_px(element.object.pos.x);
			objectMsg.add_py(element.object.pos.y);

			storyInfoMsg = objectMsg.add_bottomlevel();
			storyInfoMsg->set_exists(true);
			storyInfoMsg->set_index(element.header.floorInd);
			storyInfoMsg->set_level((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].level);
			storyInfoMsg->set_name((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].name);

			objectMsg.add_angle(element.object.angle);
			objectMsg.add_xratio(element.object.xRatio);
			objectMsg.add_yratio(element.object.yRatio);
			objectMsg.add_bottomoffset(element.object.level);
			if (element.header.variationID == APIVarId_SymbStair){
				objectMsg.add_stairs(true);
			}
			else{
				objectMsg.add_stairs(false);
			}

			objectMsg.add_usexyfixsize(element.object.useXYFixSize);

			char s[64];
			APIGuid2GSGuid(element.header.guid).ConvertToString(s);
			objectMsg.add_guid(s);

		}
		else{
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}
	}
	writeDelimitedTo(getClientSocket(), objectMsg);
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

void getRoofs(){
	API_Element element;
	API_ElementMemo memo;
	GSErrCode err;
	char buffer[256];
	roofrepeated roofMsg;
	API_StoryInfo storyInfo;
	storyinfo* storyInfoMsg;
	pointsmessage* points;
	intlistmsg* subpolygons;

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_RoofID, &elemList);
	for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate(); it != NULL; ++it) {

		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			err = ACAPI_Element_GetMemo(element.header.guid, &memo);
			if (err == NoError){
				points = roofMsg.add_points();
				subpolygons = roofMsg.add_subpolygons();
				for (int i = 1; i <= element.roof.u.planeRoof.poly.nCoords; i++){
					points->add_px((*memo.coords)[i].x);
					points->add_py((*memo.coords)[i].y);
					points->add_pz((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].level);
				}

				for (int i = 1; i <= element.roof.u.planeRoof.poly.nSubPolys; i++){
					subpolygons->add_ilist((*memo.pends)[i]);
				}

				storyInfoMsg = roofMsg.add_bottomlevel();
				storyInfoMsg->set_exists(true);
				storyInfoMsg->set_index(element.header.floorInd);
				storyInfoMsg->set_level((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].level);
				storyInfoMsg->set_name((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].name);

				roofMsg.add_height(element.roof.shellBase.level);
				roofMsg.add_thickness(element.slab.thickness);

				std::string materialName;

				if (element.roof.shellBase.modelElemStructureType == API_BasicStructure){
					materialName = searchBuildingMaterialsValue(element.roof.shellBase.buildingMaterial);
					roofMsg.add_type("Basic");
				}
				else{
					materialName = searchCompositeMaterialsValue(element.roof.shellBase.composite);
					roofMsg.add_type("Composite");
				}
				if (materialName == "Not Found"){
					sprintf(buffer, "Found no material");
					ACAPI_WriteReport(buffer, true);
				}
				else{
					roofMsg.add_material(materialName);
				}
				char s[64];
				APIGuid2GSGuid(element.header.guid).ConvertToString(s);
				roofMsg.add_guid(s);

			}
		}
		else{
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}
	}
	writeDelimitedTo(getClientSocket(), roofMsg);
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

//TODO
void getMeshes(){


}
//TODO more gets!

void getLines(){
	API_Element element;
	GSErrCode err;
	char buffer[256];
	getlinesmsg msg;
	linemsg* lineMsg;
	pointsmessage* pts;

	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_LineID, &elemList);

	for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate(); it != NULL; ++it) {
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get(&element);
		if (hasError(err)){
			quit();
			return;
		}

		char s[64];
		APIGuid2GSGuid(element.header.guid).ConvertToString(s);
		msg.add_guids(s);

		lineMsg = msg.add_lines();

		pts = new pointsmessage();

		pts->add_px(element.line.begC.x);
		pts->add_py(element.line.begC.y);
		pts->add_pz(0);
		pts->add_px(element.line.endC.x);
		pts->add_py(element.line.endC.y);
		pts->add_pz(0);
		lineMsg->set_allocated_pts(pts);

	}

	writeDelimitedTo(getClientSocket(), msg);
}

void getPolyLines(){
	API_Element element;
	API_ElementMemo memo;
	API_StoryInfo storyInfo;
	GSErrCode err;
	char buffer[256];
	getpolylinesmsg msg;
	polylinemsg* polylineMsg;
	pointsmessage* pts;
	polyarcsmessage* arcs;

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_PolyLineID, &elemList);

	for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate(); it != NULL; ++it) {
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get(&element);
		if (hasError(err)){
			quit();
			return;
		}
		err = ACAPI_Element_GetMemo(element.header.guid, &memo);
		if (hasError(err)){
			quit();
			return;
		}

		char s[64];
		APIGuid2GSGuid(element.header.guid).ConvertToString(s);
		msg.add_guids(s);

		polylineMsg = msg.add_polylines();

		pts = new pointsmessage();
		for (int i = 1; i <= element.polyLine.poly.nCoords; i++){
			pts->add_px((*memo.coords)[i].x);
			pts->add_py((*memo.coords)[i].y);
			pts->add_pz((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].level);
		}
		polylineMsg->set_allocated_pts(pts);
		
		arcs = new polyarcsmessage();
		int arcscounter = 1;
		for (int i = 0; i < element.polyLine.poly.nArcs; i++){
			for (; arcscounter < (*memo.parcs)[i].begIndex; arcscounter++){
				arcs->add_arcangle(0);
				arcs->add_begindex(arcscounter);
				arcs->add_endindex(arcscounter + 1);
			}
			arcs->add_arcangle((*memo.parcs)[i].arcAngle);
			arcs->add_begindex((*memo.parcs)[i].begIndex);
			arcs->add_endindex((*memo.parcs)[i].endIndex);
			arcscounter++;
		}
		polylineMsg->set_allocated_arcs(arcs);
	}

	writeDelimitedTo(getClientSocket(), msg);
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

void selectElement(){
	API_ElemTypeID		typeID;
	API_Guid			guid;
	API_Element			element;
	API_ElementMemo		memo;
	API_Coord3D			c3;
	GSErrCode			err = NoError;
	elementid			eleMsg;
	char buffer[256];
	while (!ClickAnElem("Click an element", API_ZombieElemID, NULL, &typeID, &guid, &c3)){
	}
	/*
	if (!ClickAnElem("Click an element", API_ZombieElemID, NULL, &typeID, &guid, &c3)) {
	WriteReport_Alert("Please click an element");
	return;
	}
	*/
	BNZeroMemory(&element, sizeof(API_Element));
	element.header.typeID = typeID;
	element.header.guid = guid;
	err = ACAPI_Element_Get(&element);
	if (err != NoError) {
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		return;
	}

	sendElementID(getClientSocket(), element);

}

void highlightElementByID(){
	GSErrCode err;
	API_NeigID neigID;
	API_Element element;
	elementidlist eleMsg;
	API_StoryCmdType	storyCmd;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &eleMsg);

	Int32 nItem = eleMsg.guid_size();
	bool add = true;

	//Clear all selected elements
	err = ACAPI_Element_Select(NULL, 0, add);

	API_Neig** neigHdl = reinterpret_cast<API_Neig**> (BMAllocateHandle(nItem * sizeof(API_Neig), ALLOCATE_CLEAR, 0));

	API_Neig neig;
	for (int i = 0; i < nItem; i++){
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = APIGuidFromString(eleMsg.guid(i).c_str());
		err = ACAPI_Element_Get(&element);
		if (err != NoError) {
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
			return;
		}
		err = ACAPI_Goodies(APIAny_ElemTypeToNeigID, (void*)element.header.typeID, &neigID);

		(*neigHdl)[i].neigID = neigID;
		(*neigHdl)[i].guid = element.header.guid;
		(*neigHdl)[i].flags = API_NeigFlg_Normal;
		(*neigHdl)[i].elemPartType = APINeigElemPart_None;
	}

	//Add the current element to the selection
	err = ACAPI_Element_Select(neigHdl, nItem, add);
	if (err != NoError) {
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		return;
	}

	BMhKill((GSHandle*)&neigHdl);

	/*
	* In order for the GUI update, we need to create an element, and then
	* delete it.
	* By doing this we can have the selected element highlighted
	*/

	API_Element		wallElement;
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	BNZeroMemory(&wallElement, sizeof(API_Element));

	wallElement.header.typeID = API_WallID;
	wallElement.header.layer = 1;

	err = ACAPI_Element_GetDefaults(&wallElement, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	err = ACAPI_Element_Create(&wallElement, &memo);

	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);

	API_Elem_Head* test;
	test = &wallElement.header;
	ACAPI_Element_Delete(&test, 1);

}

//TODO: See more file type that are define in API_FileOpenPars, fileTypeID
void openFile(){
	GSErrCode err;
	char buffer[256];
	API_FileOpenPars openPars;

	openmessage pathMsg;

	readDelimitedFrom(getClientSocket(), &pathMsg);

	BNZeroMemory(&openPars, sizeof(API_FileOpenPars));
	std::string extension = pathMsg.extension();

	if (extension == "pln"){
		openPars.fileTypeID = APIFType_PlanFile;
	}
	else if (extension == "pdf") {
		openPars.fileTypeID = APIFType_PdfFile;
	}
	else if (extension == "ifc"
		|| extension == "ifcxml"
		|| extension == "ifczip") {
		openPars.fileTypeID = APIFType_IfcFile;
	}
	else {
		openPars.fileTypeID = APIFType_None;
	}

	openPars.useStoredLib = true;

	IO::Location* folderLoc = new IO::Location(pathMsg.path().c_str());

	openPars.file = folderLoc;

	err = ACAPI_Automate(APIDo_OpenID, &openPars, NULL);

	delete openPars.file;

	if (err != NoError){
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		return;
	}

}

void elementRefresh(API_Element* element){
	char buffer[256];
	GSErrCode err;
	API_DatabaseInfo dbInfo;
	API_DatabaseInfo floorInfo;
	floorInfo.typeID = APIWind_FloorPlanID;
	floorInfo.index = 0;
	ACAPI_Database(APIDb_GetCurrentDatabaseID, &dbInfo);
	ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &floorInfo);

	API_Element mask;
	ACAPI_ELEMENT_MASK_CLEAR(mask);
	err = ACAPI_Element_Change(element, &mask, NULL, 0, true);

	if (err != NoError){
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &dbInfo);
}

void elementChange(API_Element* element, API_ElementMemo* memo, API_Element* mask){
	char buffer[256];
	GSErrCode err;
	API_DatabaseInfo dbInfo;
	API_DatabaseInfo floorInfo;
	floorInfo.typeID = APIWind_FloorPlanID;
	floorInfo.index = 0;
	ACAPI_Database(APIDb_GetCurrentDatabaseID, &dbInfo);
	ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &floorInfo);

	err = ACAPI_Element_Change(element, mask, memo, APIMemoMask_Polygon, true);

	if (err != NoError){
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &dbInfo);
}

void internalGroupElements(elementidlist msg){
	GSErrCode err;
	API_Guid groupID;

	GS::Array<API_Guid>* elemGuids = new GS::Array<API_Guid>(msg.guid_size());
	
	for (int i = 0; i < elemGuids->GetCapacity(); i++){
		GS::Guid guid;

		guid.ConvertFromString(msg.guid(i).c_str());
		elemGuids->Insert(i, GSGuid2APIGuid(guid));
	}

	err = ACAPI_ElementGroup_Create((*elemGuids), &groupID);
	delete(elemGuids);
	if (hasError(err)){
		quit();
		return;
	}

	API_Element el;
	el.header.guid = groupID;
	sendElementID(getClientSocket(), el);
}

void groupElements(){
	elementidlist msg;
	readDelimitedFrom(getClientSocket(), &msg);
	internalGroupElements(msg);
}

void deleteAllElements(){
	GSErrCode err;
	GS::Array<API_Guid> elemList;

	err = ACAPI_Element_GetElemList(API_ZombieElemID, &elemList);
	if (hasError(err)){
		quit();
		return;
	}
	if (elemList.GetSize() > 0){
		int size = elemList.GetSize(); 
		API_Elem_Head* headList = new API_Elem_Head[size];
		API_Elem_Head* toDelete;
		for (int i = 0; i < size; i++){
			API_Element el;
			el.header.guid = elemList.Get(i);
			headList[i] = el.header;
			/*
			err = ACAPI_Element_Get(&el);
			toDelete = &el.header;
			ACAPI_Element_Delete(&toDelete, 1);
			if (hasError(err)){
				quit();
				return;
			}
			*/
		}
		ACAPI_Element_Delete(&headList, size);
		if (hasError(err)){
			quit();
			return;
		}
	}
	
}