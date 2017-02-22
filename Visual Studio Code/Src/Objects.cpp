#include "RosettaArchiCAD.hpp"



//TODO ELIMINATE THIS currentlevel PLAGUE!!!

void createWall(){
	API_Element		wallElement;
	API_ElementMemo memo;
	API_StoryInfo	storyInfo;
	GSErrCode		err;
	wallmessage		wallMsg;
	elementid		wallId;
	short			material = 0;
	bool			crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &wallMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	BNZeroMemory(&wallElement, sizeof(API_Element));

	wallElement.header.typeID = API_WallID;
	wallElement.header.layer = 1;

	err = ACAPI_Element_GetDefaults(&wallElement, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	if (wallMsg.type() == "BasicStructure"){
		material = searchBuildingMaterials(wallMsg.material());
		wallElement.wall.buildingMaterial = material;
		wallElement.wall.modelElemStructureType = API_BasicStructure;
		if (wallMsg.thickness() > 0){
			wallElement.wall.thickness = wallMsg.thickness();
		}
	}
	else if (wallMsg.type() == "CompositeStructure"){
		material = searchCompositeMaterials(wallMsg.material());
		wallElement.wall.modelElemStructureType = API_CompositeStructure;
		wallElement.wall.composite = material;
	}

	if (material == -1){
		crash = true;
	}

	if (wallMsg.has_bottomstory()){
		wallElement.header.floorInd = wallMsg.bottomstory();
		wallElement.wall.relativeTopStory = wallMsg.topstory() - wallMsg.bottomstory();
		wallElement.wall.topOffset = 0;
		err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
		if (err != NoError) {
			ErrorBeep("APIEnv_GetStorySettingsID", err);
			crash = true;
		}
		else{
			wallElement.wall.height = (*storyInfo.data)[wallMsg.topstory() - storyInfo.firstStory].level;
			wallElement.wall.topOffset = (*storyInfo.data)[wallMsg.topstory() - storyInfo.firstStory].level;
		}
		wallElement.wall.height = 0;

		BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
	}
	else{
		wallElement.header.floorInd = getCurrentLevel();
		wallElement.wall.height = wallMsg.height();
	}


	wallElement.wall.begC.x = wallMsg.p0x();
	wallElement.wall.begC.y = wallMsg.p0y();

	wallElement.wall.endC.x = wallMsg.p1x();
	wallElement.wall.endC.y = wallMsg.p1y();



	wallElement.wall.angle = wallMsg.angle();

	wallElement.wall.bottomOffset = wallMsg.bottom();



	if (wallMsg.referenceline() == "Center"){
		wallElement.wall.referenceLineLocation = APIWallRefLine_Center;
	}
	else if (wallMsg.referenceline() == "Outside"){
		wallElement.wall.referenceLineLocation = APIWallRefLine_Outside;
	}
	else if (wallMsg.referenceline() == "Inside"){
		wallElement.wall.referenceLineLocation = APIWallRefLine_Inside;
	}


	//Some other values
	//wallElement.wall.displayOption = API_Standard;
	//wallElement.wall.bottomOffset = 0.0;
	//wallElement.wall.poly.nCoords = 4;
	//wallElement.wall.thickness1 = 0.3;

	//wallElement.wall.profileType = APISect_Slanted;
	//wallElement.wall.profileType = APISect_Poly;
	//wallElement.wall.slantAlpha = 90.0 * DEGRAD;
	//wallElement.wall.slantBeta = 120.0 * DEGRAD;

	/*
	wallElement.wall.refMat.material = 5;
	wallElement.wall.refMat.overrideMaterial = true;
	wallElement.wall.oppMat.material = 5;
	wallElement.wall.oppMat.overrideMaterial = true;
	wallElement.wall.sidMat.material = 5;
	wallElement.wall.sidMat.overrideMaterial = true;
	*/

	//wallElement.hotlink.hotlinkNodeGuid = wallElement.header.guid;
	//double* tmx = wallElement.hotlink.transformation.tmx;
	//tmx[3] = 10.0;


	//Create the wall
	err = ACAPI_Element_Create(&wallElement, &memo);
	sendElementID(getClientSocket(), wallElement, crash);

	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);

}

void createNewWall(){
	API_Element		wallElement;
	API_ElementMemo memo;
	API_StoryInfo	storyInfo;
	GSErrCode		err;
	wallmsg			wallMsg;
	elementid		wallId;
	elementidlist	elementIDList;
	pointsmessage	pointsMsg;
	API_Guid emptyGuid;
	short			material = 0;
	bool			crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &wallMsg);

	BNZeroMemory(&wallElement, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	wallElement.header.typeID = API_WallID;
	
	emptyGuid = wallElement.header.guid;

	err = ACAPI_Element_GetDefaults(&wallElement, &memo);

	if (hasError(err)){
		quit();
		return;
	}
	
	wallElement.header.layer = getLayerNumber(wallMsg.layer());

	short overrideMat;
	overrideMat = searchOverrideMaterials(wallMsg.refmat());
	if (overrideMat == -1){
		wallElement.wall.refMat.overrideMaterial = false;
	}
	else{
		wallElement.wall.refMat.overrideMaterial = true;
		wallElement.wall.refMat.material = overrideMat;
	}
	
	overrideMat = searchOverrideMaterials(wallMsg.oppmat());
	if (overrideMat == -1){
		wallElement.wall.oppMat.overrideMaterial = false;
	}
	else{
		wallElement.wall.oppMat.overrideMaterial = true;
		wallElement.wall.oppMat.material = overrideMat;
	}
	
	overrideMat = searchOverrideMaterials(wallMsg.sidmat());
	if (overrideMat == -1){
		wallElement.wall.sidMat.overrideMaterial = false;
	}
	else{
		wallElement.wall.sidMat.overrideMaterial = true;
		wallElement.wall.sidMat.material = overrideMat;
	}

	wallElement.header.floorInd = wallMsg.bottomindex();

	if (wallMsg.has_flipped()){
		wallElement.wall.flipped = wallMsg.flipped();
	}
	else{
		wallElement.wall.flipped = false;
	}

	wallElement.wall.bottomOffset = wallMsg.bottomoffset();

	if (wallMsg.type() == "Basic"){
		material = searchBuildingMaterials(wallMsg.material());
		wallElement.wall.buildingMaterial = material;
		wallElement.wall.modelElemStructureType = API_BasicStructure;
		if (wallMsg.thickness() > 0){
			wallElement.wall.thickness = wallMsg.thickness();
		}
	}
	else if (wallMsg.type() == "Composite"){
		material = searchCompositeMaterials(wallMsg.material());
		wallElement.wall.modelElemStructureType = API_CompositeStructure;
		wallElement.wall.composite = material;
		if (wallMsg.thickness() != 0.3){
			msgArchiCAD("Composite Material do not support thickness!");
			quit();
		}
	}


	if (material == -1){
		crash = true;
	}

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);

	/*
	wallElement.wall.height = 0;
	for (int i = wallMsg.bottomindex() + 1; i <= wallMsg.upperindex(); i++){
	wallElement.wall.height += (*storyInfo.data)[i].level;
	}
	*/

	//if (!wallMsg.toplinked()){
	//	wallElement.wall.relativeTopStory = 0;
	//}

	if (wallMsg.has_height()){
		wallElement.wall.height = wallMsg.height();
	}
	else{
		wallElement.wall.height = (*storyInfo.data)[wallMsg.upperindex()].level - (*storyInfo.data)[wallMsg.bottomindex()].level;
	}

	if (wallMsg.referenceline() == "Center"){
		wallElement.wall.referenceLineLocation = APIWallRefLine_Center;
	}
	else if (wallMsg.referenceline() == "Outside"){
		wallElement.wall.referenceLineLocation = APIWallRefLine_Outside;
	}
	else if (wallMsg.referenceline() == "Inside"){
		wallElement.wall.referenceLineLocation = APIWallRefLine_Inside;
	}

	//Check if the wall uses a profile
	if (wallMsg.profilename() != ""){
		int profileIndex = searchStringIntProfileTypes(wallMsg.profilename());
		if (profileIndex == -1){
			crash = true;
		}
		else {
			wallElement.wall.modelElemStructureType = API_ProfileStructure;
			wallElement.wall.profileAttr = profileIndex;
			wallElement.wall.profileType = APISect_Poly;
		}
	}
	//if it is not a profile, it can be Normal, Slanted or DoubleSlanted
	else {
		if (wallMsg.typeprofile() == "Normal"){
			wallElement.wall.profileType = APISect_Normal;
		}
		else if (wallMsg.typeprofile() == "Slanted"){
			wallElement.wall.profileType = APISect_Slanted;
		}
		else if (wallMsg.typeprofile() == "DoubleSlanted"){
			wallElement.wall.profileType = APISect_Trapez;
		}
	}

	//readDelimitedFrom(getClientSocket(), &pointsMsg);
	pointsMsg = wallMsg.pts();
	for (int i = 0; i < pointsMsg.px_size() - 1; i++){

		//Beginning Point
		wallElement.wall.begC.x = pointsMsg.px(i);
		wallElement.wall.begC.y = pointsMsg.py(i);
		//Ending Point
		wallElement.wall.endC.x = pointsMsg.px(i + 1);
		wallElement.wall.endC.y = pointsMsg.py(i + 1);
		
		if (i < wallMsg.arcs().arcangle_size()){
			wallElement.wall.angle = wallMsg.arcs().arcangle(i);
			if (wallElement.wall.angle < 0){
				//Beginning Point
				wallElement.wall.begC.x = pointsMsg.px(i + 1);
				wallElement.wall.begC.y = pointsMsg.py(i + 1);
				//Ending Point
				wallElement.wall.endC.x = pointsMsg.px(i);
				wallElement.wall.endC.y = pointsMsg.py(i);

				wallElement.wall.flipped = !wallElement.wall.flipped;
			}
		}
		else{
			wallElement.wall.angle = 0;
		}

		//////

		wallElement.wall.slantAlpha = wallMsg.alphaangle();
		wallElement.wall.slantBeta = wallMsg.betaangle();

		wallElement.wall.offset = wallMsg.refoffset();
		//Create the wall
		err = ACAPI_Element_Create(&wallElement, &memo);

		if (hasError(err)){
			quit();
			return;
		}

		//Convert Element Id from GuId to char*
		GS::UniString guidString = APIGuidToString(wallElement.header.guid);
		char s[64];
		APIGuid2GSGuid(wallElement.header.guid).ConvertToString(s);
		elementIDList.add_guid(s);
		for (int j = 0; j < wallMsg.windoworder_size(); j++){
			if (wallMsg.windoworder(j) == i){
				windowmessage wmsg = wallMsg.windows(j);
				wmsg.set_guid(s);
				internalcreateWindow(wmsg);
			}
		}
	}
	elementIDList.set_crashmaterial(crash);
	//Sending element Id
	writeDelimitedTo(getClientSocket(), elementIDList);

	ACAPI_DisposeElemMemoHdls(&memo);
	/*
	wallElement.wall.begC.x = wallMsg.p0x();
	wallElement.wall.begC.y = wallMsg.p0y();

	wallElement.wall.endC.x = wallMsg.p1x();
	wallElement.wall.endC.y = wallMsg.p1y();

	wallElement.wall.angle = wallMsg.angle();

	//Create the wall
	err = ACAPI_Element_Create(&wallElement, &memo);
	sendElementID(wallElement, crash);

	if (err != NoError){
	ErrorBeep("ACAPI_Element_Create", err);
	sprintf(buffer, ErrID_To_Name(err));
	ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);
	*/
}

void createPolyWall(){
	API_Element		wallElement;
	API_ElementMemo memo;
	GSErrCode		err;
	wallmessage		wallMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage	polyarcsMsg;
	elementid		wallId;
	API_StoryCmdType s;
	char buffer[256];


	readDelimitedFrom(getClientSocket(), &wallMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&wallElement, sizeof(API_Element));

	readDelimitedFrom(getClientSocket(), &pointsMsg);
	readDelimitedFrom(getClientSocket(), &polyarcsMsg);


	wallElement.header.typeID = API_WallID;
	wallElement.header.layer = 1;
	wallElement.header.floorInd = getCurrentLevel();

	//Beginning Point
	//wallElement.wall.begC.x = wallMsg.p0x();
	//wallElement.wall.begC.y = wallMsg.p0y();
	//Ending Point
	//wallElement.wall.endC.x = wallMsg.p1x();
	//wallElement.wall.endC.y = wallMsg.p1y();
	//Height
	wallElement.wall.height = wallMsg.height();
	//Thickness
	wallElement.wall.thickness = wallMsg.thickness();
	//Angle
	wallElement.wall.angle = wallMsg.angle();

	wallElement.wall.bottomOffset = wallMsg.bottom();
	//Some other values
	//wallElement.wall.displayOption = API_Standard;
	//wallElement.wall.bottomOffset = 0.0;
	//wallElement.wall.poly.nCoords = 4;
	//wallElement.wall.thickness1 = 0.3;


	//wallElement.wall.profileType = APISect_Slanted;
	wallElement.wall.slantAlpha = 90.0 * DEGRAD;
	wallElement.wall.slantBeta = 120.0 * DEGRAD;

	wallElement.wall.profileType = APISect_Poly;


	wallElement.wall.type = APIWtyp_Poly;
	//wallElement.wall.type = APIWtyp_Trapez;
	wallElement.wall.polyCanChange = true;
	wallElement.wall.poly.nCoords = (pointsMsg.px_size() * 2) + 1;
	//wallElement.wall.poly.nSubPolys = 1;
	//wallElement.wall.poly.nArcs = 2;

	memo.coords = (API_Coord**)BMAllocateHandle((wallElement.wall.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);

	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((wallElement.wall.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));

	//memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((cWallMsg.numarcs() + 1) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	//memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((cWallMsg.numarcs()) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	//memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((2) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	/*
	memo.bezierDirs = reinterpret_cast<API_SplineDir**> (BMAllocateHandle(2 * sizeof(API_SplineDir), ALLOCATE_CLEAR, 0));
	(*memo.bezierDirs)[1].lenPrev = 1.0;
	(*memo.bezierDirs)[1].lenNext = 1.0;
	(*memo.bezierDirs)[1].dirAng = 45.0 * DEGRAD;
	*/

	int index = 1;

	//Out Line
	for (int i = 0; i < pointsMsg.px_size(); i++){

		(*memo.coords)[index].x = pointsMsg.px(i);
		(*memo.coords)[index].y = pointsMsg.py(i);

		(*memo.coords)[wallElement.wall.poly.nCoords - 1 - i].x = pointsMsg.px(i);
		(*memo.coords)[wallElement.wall.poly.nCoords - 1 - i].y = pointsMsg.py(i);

		index++;
	}

	double bpx, bpy, epx, epy, dx, dy, theta;

	int endOfOutLine = index;

	//In Line
	for (int i = 0; i < pointsMsg.px_size() - 1; i++){

		bpx = (*memo.coords)[endOfOutLine - 1 - i].x;
		bpy = (*memo.coords)[endOfOutLine - 1 - i].y;
		epx = (*memo.coords)[endOfOutLine - 2 - i].x;
		epy = (*memo.coords)[endOfOutLine - 2 - i].y;

		dx = epx - bpx;
		dy = epy - bpy;
		//theta = atan(dy / dx);
		theta = atan2(dy, dx);


		(*memo.coords)[index].x += wallElement.wall.thickness * sin(theta);
		(*memo.coords)[index].y += wallElement.wall.thickness * -cos(theta);

		if (index < wallElement.wall.poly.nCoords){
			(*memo.coords)[index + 1].x += wallElement.wall.thickness * sin(theta);
			(*memo.coords)[index + 1].y += wallElement.wall.thickness * -cos(theta);
		}

		sprintf(buffer, "index: %d x: %f y: %f index2: %d x2: %f y2: %f", index, (*memo.coords)[index].x, (*memo.coords)[index].y, index + 1, (*memo.coords)[index + 1].x, (*memo.coords)[index + 1].y);
		ACAPI_WriteReport(buffer, true);

		index++;
	}


	(*memo.coords)[wallElement.wall.poly.nCoords] = (*memo.coords)[1];

	(*memo.pends)[1] = wallElement.wall.poly.nCoords;

	//Create the wall
	err = ACAPI_Element_Create(&wallElement, &memo);

	// sendElementID(wallElement);

	//if there is an error a circle will be created
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);

}

void createMultiWall(){
	API_Element		wallElement;
	API_ElementMemo memo;
	GSErrCode		err;
	wallmessage		wallMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage	polyarcsMsg;
	elementidlist	elementIDList;
	short			material = 0;
	bool			crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &wallMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	BNZeroMemory(&wallElement, sizeof(API_Element));
	wallElement.header.typeID = API_WallID;
	wallElement.header.layer = 1;


	err = ACAPI_Element_GetDefaults(&wallElement, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	wallElement.header.floorInd = getCurrentLevel();

	wallElement.wall.bottomOffset = wallMsg.bottom();

	//Height
	wallElement.wall.height = wallMsg.height();
	//Thickness
	wallElement.wall.thickness = wallMsg.thickness();

	if (wallMsg.referenceline() == "Center"){
		wallElement.wall.referenceLineLocation = APIWallRefLine_Center;
	}
	else if (wallMsg.referenceline() == "Outside"){
		wallElement.wall.referenceLineLocation = APIWallRefLine_Outside;
	}
	else if (wallMsg.referenceline() == "Inside"){
		wallElement.wall.referenceLineLocation = APIWallRefLine_Inside;
	}

	if (wallMsg.type() == "BasicStructure"){
		material = searchBuildingMaterials(wallMsg.material());
		wallElement.wall.buildingMaterial = material;
		wallElement.wall.modelElemStructureType = API_BasicStructure;
		if (wallMsg.thickness() > 0){
			wallElement.wall.thickness = wallMsg.thickness();
		}
	}
	else if (wallMsg.type() == "CompositeStructure"){
		material = searchCompositeMaterials(wallMsg.material());
		wallElement.wall.modelElemStructureType = API_CompositeStructure;
		wallElement.wall.composite = material;
		if (wallMsg.thickness() != 0.3){
			msgArchiCAD("Composite Material do not support thickness!");
			quit();
		}
	}

	if (material == -1){
		crash = true;
	}

	readDelimitedFrom(getClientSocket(), &pointsMsg);
	readDelimitedFrom(getClientSocket(), &polyarcsMsg);


	for (int i = 0; i < pointsMsg.px_size() - 1; i++){

		//Beginning Point
		wallElement.wall.begC.x = pointsMsg.px(i);
		wallElement.wall.begC.y = pointsMsg.py(i);
		//Ending Point
		wallElement.wall.endC.x = pointsMsg.px(i + 1);
		wallElement.wall.endC.y = pointsMsg.py(i + 1);

		//Angle
		if (i < polyarcsMsg.arcangle_size()){
			wallElement.wall.angle = polyarcsMsg.arcangle(i);
		}
		else {
			wallElement.wall.angle = 0.0;
		}

		//Create the wall
		err = ACAPI_Element_Create(&wallElement, &memo);

		if (err != NoError){
			ErrorBeep("ACAPI_Element_Create", err);
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}

		//Convert Element Id from GuId to char*
		GS::UniString guidString = APIGuidToString(wallElement.header.guid);
		char s[64];
		APIGuid2GSGuid(wallElement.header.guid).ConvertToString(s);
		elementIDList.add_guid(s);


	}
	elementIDList.set_crashmaterial(crash);
	//Sending element Id
	writeDelimitedTo(getClientSocket(), elementIDList);

	ACAPI_DisposeElemMemoHdls(&memo);

}

void createDoor(){
	API_Element			doorElement, wallElement;
	API_ElementMemo		memo;
	//API_LibPart			libPart;
	GSErrCode			err;
	doormessage			doorMsg;
	API_SubElemMemoMask	marker;
	API_AddParType		**markAddPars;
	API_Coord			beginWall, endWall;
	double				wallLength;
	double				doorStartingPoint;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &doorMsg);

	BNZeroMemory(&doorElement, sizeof(API_Element));
	BNZeroMemory(&wallElement, sizeof(API_Element));
	BNZeroMemory(&marker, sizeof(API_SubElemMemoMask));
	doorElement.header.typeID = API_DoorID;
	doorElement.header.layer = getLayerNumber(doorMsg.layer());
	
	marker.subType = APISubElemMemoMask_MainMarker;

	err = ACAPI_Element_GetDefaultsExt(&doorElement, &memo, 1UL, &marker);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memo);
		ACAPI_DisposeElemMemoHdls(&marker.memo);
		return;
	}

	int libID = searchObjects(doorMsg.name());
	if (libID == -1){
		crash = true;
	}
	else{
		doorElement.door.openingBase.libInd = libID;
		marker.subElem.object.libInd = libID;
	}

	if (!Do_LibpartSearchByUnID(API_DoorID, &markAddPars, &marker.subElem.object.libInd)) {
		ACAPI_DisposeElemMemoHdls(&memo);
		ACAPI_DisposeElemMemoHdls(&marker.memo);
		return;
	}
	marker.memo.params = markAddPars;
	marker.subElem.object.pen = 166;
	marker.subElem.object.useObjPens = true;

	GS::Guid wallGuid;

	wallGuid.ConvertFromString(doorMsg.guid().c_str());

	doorElement.door.objLoc = doorMsg.objloc();

	doorElement.door.lower = doorMsg.zpos();

	if (doorMsg.fixpoint() == -1 || doorMsg.fixpoint() == 0 || doorMsg.fixpoint() == 1){
		doorElement.door.fixPoint = doorMsg.fixpoint();
	}
	else{
		msgArchiCAD("No door fixpoint matches the given value. Give -1 for Beg, 0 for Center, 1 for End");
	}

	//libPart.index = libID;

	//if (ACAPI_LibPart_Search(&libPart, false) == NoError){
		//TODO
	//	doorElement.door.openingBase.height = 0;
	//}

	

	if (doorMsg.hole()){
		doorElement.door.openingBase.libInd = 6;
	}

	doorElement.door.openingBase.reflected = doorMsg.flipy();
	doorElement.door.openingBase.oSide = doorMsg.flipx();
	doorElement.door.revealDepthOffset = doorMsg.depthoffset();
	doorElement.door.owner = GSGuid2APIGuid(wallGuid);

	wallElement.header.typeID = API_WallID;
	wallElement.header.guid = GSGuid2APIGuid(wallGuid);

	if (ACAPI_Element_Get(&wallElement) == NoError){
		doorElement.header.floorInd = wallElement.header.floorInd;
	}

	API_ParamOwnerType   paramOwner;
	API_GetParamsType    getParams;
	BNZeroMemory(&paramOwner, sizeof(API_ParamOwnerType));
	paramOwner.libInd = doorElement.door.openingBase.libInd;

	BNZeroMemory(&getParams, sizeof(API_GetParamsType));

	err = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner, NULL);
	if (err == NoError) {
		err = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, NULL);
		if (err == NoError) {
			handleParams(doorMsg.params(), &paramOwner, &getParams);
			memo.params = getParams.params;
		}
	}

	int numParams = BMhGetSize(reinterpret_cast<GSHandle> (memo.params)) / Sizeof32(API_AddParType) - 1;
	int found = 0;
	for (int j = 0; j < numParams; j++){
		if ((std::string)(*memo.params)[j].name == "A"){
			doorElement.door.openingBase.width = (*memo.params)[j].value.real;
			found++;
		}
		else if ((std::string)(*memo.params)[j].name == "B"){
			doorElement.door.openingBase.height = (*memo.params)[j].value.real;
			found++;
		}
		if (found == 2){
			break;
		}
	}

	if (doorMsg.height() != -10000){
		doorElement.door.openingBase.height = doorMsg.height();
	}

	if (doorMsg.width() != -10000){
		doorElement.door.openingBase.width = doorMsg.width();
	}

	/*
	API_Coord c2, begC;
	c2.x = 1.0;
	c2.y = 1.0;
	begC.x = 0.0;
	begC.y = 0.0;
	doorElement.door.objLoc = DistCPtr(&c2, &begC);
	*/
	//err = ACAPI_Element_CreateExt(&doorElement, &memo, 1UL, &marker);
	err = ACAPI_Element_Create(&doorElement, &memo);
	if (err != NoError){
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}
	sendElementID(getClientSocket(), doorElement, crash);

	//required to force update on width/height
	elementRefresh(&doorElement);

	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&marker.memo);
}

void oldcreateWindow(){

	API_Element			windowElement, wallElement;
	API_ElementMemo		memo;
	GSErrCode			err;
	windowmessage		windowMsg;
	API_SubElemMemoMask	marker;
	API_AddParType		**markAddPars;
	API_Coord			beginWall, endWall;
	double				wallLength;
	double				windowStartingPoint;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &windowMsg);

	BNZeroMemory(&windowElement, sizeof(API_Element));
	BNZeroMemory(&wallElement, sizeof(API_Element));
	BNZeroMemory(&marker, sizeof(API_SubElemMemoMask));
	windowElement.header.typeID = API_WindowID;
	windowElement.header.layer = 1;
	windowElement.header.floorInd = getCurrentLevel();

	marker.subType = APISubElemMemoMask_MainMarker;

	err = ACAPI_Element_GetDefaults(&windowElement, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}
	/*
	err = ACAPI_Element_GetDefaultsExt(&windowElement, &memo, 1UL, &marker);
	if (err != NoError) {
	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&marker.memo);
	return;
	}
	*/
	int libID = searchObjects(windowMsg.name());
	if (libID == -1){
		crash = true;
	}
	else{
		windowElement.window.openingBase.libInd = libID;
		marker.subElem.object.libInd = libID;
	}

	if (!Do_LibpartSearchByUnID(API_WindowID, &markAddPars, &marker.subElem.object.libInd)) {
		ACAPI_DisposeElemMemoHdls(&memo);
		ACAPI_DisposeElemMemoHdls(&marker.memo);
		return;
	}
	marker.memo.params = markAddPars;
	marker.subElem.object.pen = 166;
	marker.subElem.object.useObjPens = true;

	GS::Guid wallGuid;

	wallGuid.ConvertFromString(windowMsg.guid().c_str());

	windowStartingPoint = windowMsg.objloc();

	windowElement.window.lower = windowMsg.zpos();

	windowElement.window.objLoc = windowStartingPoint;

	windowElement.window.openingBase.width = windowMsg.width();
	windowElement.window.openingBase.height = windowMsg.height();

	//Keeping this for future reference of how to get an element given an ID
	/*
	BNZeroMemory(&wallElement, sizeof(API_Element));
	wallElement.header.guid = APIGuidFromString(windowMsg.guid().c_str());
	if (ACAPI_Element_Get(&wallElement) == NoError) {
	beginWall = wallElement.wall.begC;
	endWall = wallElement.wall.endC;
	wallLength = DistCPtr(&endWall, &beginWall);
	}

	*/

	windowElement.window.owner = GSGuid2APIGuid(wallGuid);

	wallElement.header.typeID = API_WallID;
	wallElement.header.guid = GSGuid2APIGuid(wallGuid);

	if (ACAPI_Element_Get(&wallElement) == NoError){
		windowElement.header.floorInd = wallElement.header.floorInd;
	}

	API_ParamOwnerType   paramOwner;
	API_GetParamsType    getParams;
	BNZeroMemory(&paramOwner, sizeof(API_ParamOwnerType));
	paramOwner.libInd = windowElement.window.openingBase.libInd;

	BNZeroMemory(&getParams, sizeof(API_GetParamsType));

	err = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner, NULL);
	if (err == NoError) {
		err = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, NULL);
		if (err == NoError) {
			handleParams(windowMsg.params(), &paramOwner, &getParams);
			memo.params = getParams.params;
		}
	}
	/*
	API_Coord c2, begC;
	c2.x = 1.0;
	c2.y = 1.0;
	begC.x = 0.0;
	begC.y = 0.0;
	windowElement.window.objLoc = DistCPtr(&c2, &begC);
	*/

	err = ACAPI_Element_Create(&windowElement, &memo);
	//err = ACAPI_Element_CreateExt(&windowElement, &memo, 1UL, &marker);

	if (err != NoError){
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}
	sendElementID(getClientSocket(), windowElement, crash);

	//required to force update on width/height
	elementRefresh(&windowElement);

	ACAPI_Goodies(APIAny_CloseParametersID, NULL, NULL);

	ACAPI_DisposeAddParHdl(&getParams.params);
	//ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&marker.memo);
}

void internalcreateWindow(windowmessage windowMsg){
	API_Element			windowElement, wallElement;
	API_ElementMemo		memo;
	GSErrCode			err;
	API_SubElemMemoMask	marker;
	API_AddParType		**markAddPars;
	API_Coord			beginWall, endWall;
	double				wallLength;
	double				windowStartingPoint;
	bool crash = false;
	char buffer[256];


	BNZeroMemory(&windowElement, sizeof(API_Element));
	BNZeroMemory(&wallElement, sizeof(API_Element));
	BNZeroMemory(&marker, sizeof(API_SubElemMemoMask));
	windowElement.header.typeID = API_WindowID;
	windowElement.header.layer = getLayerNumber(windowMsg.layer());
	windowElement.header.floorInd = getCurrentLevel();

	marker.subType = APISubElemMemoMask_MainMarker;

	err = ACAPI_Element_GetDefaults(&windowElement, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	/*
	err = ACAPI_Element_GetDefaultsExt(&windowElement, &memo, 1UL, &marker);
	if (err != NoError) {
	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&marker.memo);
	return;
	}
	*/
	int libID = searchObjects(windowMsg.name());
	if (libID == -1){
		crash = true;
	}
	else{
		windowElement.window.openingBase.libInd = libID;
		marker.subElem.object.libInd = libID;
	}

	if (!Do_LibpartSearchByUnID(API_WindowID, &markAddPars, &marker.subElem.object.libInd)) {
		ACAPI_DisposeElemMemoHdls(&memo);
		ACAPI_DisposeElemMemoHdls(&marker.memo);
		return;
	}

	marker.memo.params = markAddPars;
	marker.subElem.object.pen = 166;
	marker.subElem.object.useObjPens = true;

	GS::Guid wallGuid;

	wallGuid.ConvertFromString(windowMsg.guid().c_str());

	windowStartingPoint = windowMsg.objloc();

	windowElement.window.lower = windowMsg.zpos();

	windowElement.window.objLoc = windowStartingPoint;
	
	windowElement.window.revealDepthOffset = windowMsg.depthoffset();
	
	windowElement.window.openingBase.oSide = windowMsg.flipx();
	windowElement.window.openingBase.reflected = windowMsg.flipy();

	//Keeping this for future reference of how to get an element given an ID
	/*
	BNZeroMemory(&wallElement, sizeof(API_Element));
	wallElement.header.guid = APIGuidFromString(windowMsg.guid().c_str());
	if (ACAPI_Element_Get(&wallElement) == NoError) {
	beginWall = wallElement.wall.begC;
	endWall = wallElement.wall.endC;
	wallLength = DistCPtr(&endWall, &beginWall);
	}

	*/

	windowElement.window.owner = GSGuid2APIGuid(wallGuid);

	

	wallElement.header.typeID = API_WallID;
	wallElement.header.guid = GSGuid2APIGuid(wallGuid);

	if (ACAPI_Element_Get(&wallElement) == NoError){
		windowElement.header.floorInd = wallElement.header.floorInd;
	}

	API_ParamOwnerType   paramOwner;
	API_GetParamsType    getParams;
	BNZeroMemory(&paramOwner, sizeof(API_ParamOwnerType));
	paramOwner.libInd = windowElement.window.openingBase.libInd;

	BNZeroMemory(&getParams, sizeof(API_GetParamsType));

	err = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner, NULL);
	if (err == NoError) {
		err = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, NULL);
		if (err == NoError) {
			handleParams(windowMsg.params(), &paramOwner, &getParams);
			memo.params = getParams.params;
		}
	}
	/*
	API_Coord c2, begC;
	c2.x = 1.0;
	c2.y = 1.0;
	begC.x = 0.0;
	begC.y = 0.0;
	windowElement.window.objLoc = DistCPtr(&c2, &begC);
	*/

	int numParams = BMhGetSize(reinterpret_cast<GSHandle> (memo.params)) / Sizeof32(API_AddParType) - 1;
	int found = 0;
	for (int j = 0; j < numParams; j++){
		if ((std::string)(*memo.params)[j].name == "A"){
			windowElement.window.openingBase.width = (*memo.params)[j].value.real;
			found++;
		}
		else if ((std::string)(*memo.params)[j].name == "B"){
			windowElement.window.openingBase.height = (*memo.params)[j].value.real;
			found++;
		}
		if (found == 2){
			break;
		}
	}

	if (windowMsg.width() != -10000){
		windowElement.window.openingBase.width = windowMsg.width();
	}

	if (windowMsg.height() != -10000){
		windowElement.window.openingBase.height = windowMsg.height();
	}

	err = ACAPI_Element_Create(&windowElement, &memo);
	//err = ACAPI_Element_CreateExt(&windowElement, &memo, 1UL, &marker);

	if (err != NoError){
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}
	sendElementID(getClientSocket(), windowElement, crash);

	//required to force update on width/height
	elementRefresh(&windowElement);

	ACAPI_Goodies(APIAny_CloseParametersID, NULL, NULL);

	ACAPI_DisposeAddParHdl(&getParams.params);
	//ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&marker.memo);
}

void createWindow(){
	windowmessage		windowMsg;
	readDelimitedFrom(getClientSocket(), &windowMsg);
	internalcreateWindow(windowMsg);
}

void createNewCurtainWall(){
	API_Element			curtainWallElement;
	API_ElementMemo		memo;
	API_StoryInfo	storyInfo;
	GSErrCode			err;
	curtainwallmsg		msg;
	char buffer[256];
	API_Guid emptyGuid;


	BNZeroMemory(&curtainWallElement, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	curtainWallElement.header.typeID = API_CurtainWallID;

	emptyGuid = curtainWallElement.header.guid;

	err = ACAPI_Element_GetDefaults(&curtainWallElement, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	readDelimitedFrom(getClientSocket(), &msg);

	curtainWallElement.header.layer = getLayerNumber(msg.layer());
	curtainWallElement.header.floorInd = msg.bottomindex();
	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);

	curtainWallElement.curtainWall.offset = msg.offset();

	if (msg.has_height()){
		curtainWallElement.curtainWall.height = msg.height();
	}
	else{
		curtainWallElement.curtainWall.height = (*storyInfo.data)[msg.upperindex()].level - (*storyInfo.data)[msg.bottomindex()].level;
	}


	//curtainWallElement.curtainWall.height = (*storyInfo.data)[msg.upperindex()].level - (*storyInfo.data)[msg.bottomindex()].level;

	curtainWallElement.curtainWall.glassPanelData.category = APICWPaC_Primary;
	curtainWallElement.curtainWall.glassPanelData.useOwnMaterial = false;
	curtainWallElement.curtainWall.glassPanelData.outerSurfaceMaterial = searchOverrideMaterials(msg.secpanelmaterial());
	curtainWallElement.curtainWall.glassPanelData.innerSurfaceMaterial = searchOverrideMaterials(msg.secpanelmaterial());
	curtainWallElement.curtainWall.glassPanelData.cutSurfaceMaterial = searchOverrideMaterials(msg.secpanelmaterial());
	curtainWallElement.curtainWall.glassPanelData.thickness = msg.mainpanelthickness();

	curtainWallElement.curtainWall.glazedPanelData.category = APICWPaC_Secondary;
	curtainWallElement.curtainWall.glazedPanelData.useOwnMaterial = false;
	curtainWallElement.curtainWall.glazedPanelData.outerSurfaceMaterial = searchOverrideMaterials(msg.panelmaterial());
	curtainWallElement.curtainWall.glazedPanelData.innerSurfaceMaterial = searchOverrideMaterials(msg.panelmaterial());
	curtainWallElement.curtainWall.glazedPanelData.cutSurfaceMaterial = searchOverrideMaterials(msg.panelmaterial());
	curtainWallElement.curtainWall.glazedPanelData.thickness = msg.secondarypanelthickness();

	//Testing
	/*
	curtainWallElement.curtainWall.usePanelTypesAttributes = true;
	curtainWallElement.curtainWall.glassPanelData.objectType = APICWPaObjectType_GDL;

	GS::UniString us = searchLibGUIDS("CW Door Revolving 18");
	GS::Guid gs_guid(us);
	API_Guid ag = GSGuid2APIGuid(gs_guid);

	//API_Guid ag = GSGuid2APIGuid(guid);

	curtainWallElement.curtainWall.glassPanelData.tag = ag;
	curtainWallElement.curtainWall.glassPanelData.hasTag = true;
	
	char s[128];
	APIGuid2GSGuid(ag).ConvertToString(s);
	//msgArchiCAD(s);
	*/
	//----------------

	curtainWallElement.curtainWall.perimeterFrameData.material = searchOverrideMaterials(msg.framematerial());
	curtainWallElement.curtainWall.perimeterFrameData.a1 = msg.bframewidth() / 2;
	curtainWallElement.curtainWall.perimeterFrameData.a2 = curtainWallElement.curtainWall.perimeterFrameData.a1;
	curtainWallElement.curtainWall.perimeterFrameData.b1 = msg.bframedepth() - msg.bframeoffset();
	curtainWallElement.curtainWall.perimeterFrameData.b2 = msg.bframeoffset();
	
	curtainWallElement.curtainWall.primaryFrameData.material = searchOverrideMaterials(msg.verticalframematerial());
	curtainWallElement.curtainWall.primaryFrameData.a1 = msg.mframewidth() / 2;
	curtainWallElement.curtainWall.primaryFrameData.a2 = curtainWallElement.curtainWall.primaryFrameData.a1;
	curtainWallElement.curtainWall.primaryFrameData.b1 = msg.mframedepth() - msg.mframeoffset();
	curtainWallElement.curtainWall.primaryFrameData.b2 = msg.mframeoffset();
	
	curtainWallElement.curtainWall.secondaryFrameData.material = searchOverrideMaterials(msg.horizontalframematerial());
	curtainWallElement.curtainWall.secondaryFrameData.a1 = msg.tframewidth() / 2;
	curtainWallElement.curtainWall.secondaryFrameData.a2 = curtainWallElement.curtainWall.secondaryFrameData.a1;
	curtainWallElement.curtainWall.secondaryFrameData.b1 = msg.tframedepth() - msg.tframeoffset();
	curtainWallElement.curtainWall.secondaryFrameData.b2 = msg.tframeoffset();

	//TEST VALUES
	//curtainWallElement.curtainWall.angle = 45.0 * DEGRAD;
	//curtainWallElement.curtainWall.offset = 8000.0;
	//curtainWallElement.curtainWall.segmentData.begC.z = 10.0;
	//curtainWallElement.curtainWall.segmentData.endC.z = 10.0;
	//curtainWallElement.curtainWall.planeOffset = 8000.0;
	//curtainWallElement.curtainWall.planeOffset = 0.0;
	//double* tmx = curtainWallElement.curtainWall.planeMatrix.tmx;
	//tmx[0] = 1.0; tmx[1] = 0.0; tmx[2] = 0.0;  tmx[3] = 0.0;
	//tmx[4] = 0.0; tmx[5] = 1.0; tmx[6] = 0.0;  tmx[7] = 0.0;
	//tmx[8] = 0.0; tmx[9] = 0.0; tmx[10] = 1.0; tmx[11] = 0.0;

	curtainWallElement.curtainWall.polygon.nCoords = msg.pts().px_size();
	curtainWallElement.curtainWall.polygon.nSubPolys = 1;
	curtainWallElement.curtainWall.polygon.nArcs = msg.arcs().arcangle_size();

	//TODO --- VER MEMO.PENDS AQUI!!!!!!
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((curtainWallElement.curtainWall.polygon.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	(*memo.pends)[1] = curtainWallElement.curtainWall.polygon.nCoords;

	populateMemo(&memo, msg.pts(), msg.arcs());

	////////////////////////////////////////////////////////////////////

	//curtainWallElement.curtainWall.perimeterFrameData.material = searchOverrideMaterials("Glass - Blue");



	//angle of panels
	curtainWallElement.curtainWall.segmentData.gridAngle = msg.panelsangle();

	if (curtainWallElement.curtainWall.segmentData.gridAngle < 0){
		curtainWallElement.curtainWall.segmentData.negArc = true;
	}

	curtainWallElement.curtainWall.segmentData.primaryPatternNum = msg.primaries_size();
	curtainWallElement.curtainWall.segmentData.secondaryPatternNum = msg.secondaries_size();
	curtainWallElement.curtainWall.segmentData.panelPatternNum = msg.mainpanels_size();

	curtainWallElement.curtainWall.nSegments = curtainWallElement.curtainWall.segmentData.primaryPatternNum + curtainWallElement.curtainWall.segmentData.secondaryPatternNum;
	curtainWallElement.curtainWall.nPanels = curtainWallElement.curtainWall.segmentData.primaryPatternNum * curtainWallElement.curtainWall.segmentData.secondaryPatternNum;

	//junctions
	curtainWallElement.curtainWall.placementMethod = APICW_Placement_AllGridPoints;
	curtainWallElement.curtainWall.junctionData.clampFramesNum = curtainWallElement.curtainWall.nSegments;
	curtainWallElement.curtainWall.junctionData.clampFrameOrientsNum = curtainWallElement.curtainWall.junctionData.clampFramesNum;
	curtainWallElement.curtainWall.junctionData.clampPanelsNum = curtainWallElement.curtainWall.nPanels;
	curtainWallElement.curtainWall.junctionData.head.typeID = API_CurtainWallJunctionID;
	curtainWallElement.curtainWall.junctionData.category = APICWJunC_System;
	curtainWallElement.curtainWall.nJunctions = 16;

	memo.cWSegPrimaryPattern = (double*)BMReallocPtr((GSPtr)memo.cWSegPrimaryPattern, curtainWallElement.curtainWall.segmentData.primaryPatternNum*sizeof(double), REALLOC_MOVEABLE, 0);
	memo.cWSegSecondaryPattern = (double*)BMReallocPtr((GSPtr)memo.cWSegSecondaryPattern, curtainWallElement.curtainWall.segmentData.secondaryPatternNum*sizeof(double), REALLOC_MOVEABLE, 0);
	memo.cWSegPanelPattern = (GS::Bool8*) BMReallocPtr((GSPtr)memo.cWSegPanelPattern, curtainWallElement.curtainWall.segmentData.panelPatternNum*sizeof(GS::Bool8), REALLOC_MOVEABLE, 0);
	memo.cWallJunctions = reinterpret_cast<API_CWallJunctionType*> (BMAllocatePtr(curtainWallElement.curtainWall.nJunctions * sizeof(API_CWallJunctionType), ALLOCATE_CLEAR, 0));
	memo.cWallPanels = reinterpret_cast<API_CWallPanelType*> (BMAllocatePtr(curtainWallElement.curtainWall.nPanels * sizeof(API_CWallPanelType), ALLOCATE_CLEAR, 0));
	if (memo.cWSegPrimaryPattern == NULL 
		|| memo.cWSegSecondaryPattern == NULL 
		|| memo.cWSegPanelPattern == NULL
		|| memo.cWallJunctions == NULL
		|| memo.cWallPanels == NULL) {
		ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
		ACAPI_DisposeElemMemoHdls(&memo);
		quit();
		return;
	}

	for (int i = 0; i < curtainWallElement.curtainWall.nJunctions; i++){
		memo.cWallJunctions[i].clampFramesNum = 2;
		memo.cWallJunctions[i].clampFrameOrientsNum = 2;
		memo.cWallJunctions[i].clampPanelsNum = 4;
		memo.cWallJunctions[i].head.typeID = API_CurtainWallJunctionID;
		memo.cWallJunctions[i].category = APICWJunC_System;

		GS::UniString uniName = searchLibGUIDS("Junction 18");
		GS::Guid junctionGuid(uniName);
		memo.cWallJunctions[i].tag = GSGuid2APIGuid(junctionGuid);
		memo.cWallJunctions[i].hasTag = true;

	}

	for (int i = 0; i < msg.primaries_size(); i++){
		memo.cWSegPrimaryPattern[i] = msg.primaries(i);
	}

	for (int i = 0; i < msg.secondaries_size(); i++){
		memo.cWSegSecondaryPattern[i] = msg.secondaries(i);
	}

	for (int i = 0; i < msg.mainpanels_size(); i++){
		memo.cWSegPanelPattern[i] = !msg.mainpanels(i);
	}

	/*
	memo.cWSegSecondaryPattern = (double*)BMReallocPtr((GSPtr)memo.cWSegSecondaryPattern, curtainWallElement.curtainWall.segmentData.secondaryPatternNum*sizeof(double), REALLOC_MOVEABLE, 0);
	if (memo.cWSegSecondaryPattern != NULL) {
	memo.cWSegSecondaryPattern[0] = 0.35;
	memo.cWSegSecondaryPattern[curtainWallElement.curtainWall.segmentData.secondaryPatternNum - 1] = 1;
	}


	memo.cWSegPanelPattern = (GS::Bool8*) BMReallocPtr((GSPtr)memo.cWSegPanelPattern, curtainWallElement.curtainWall.segmentData.panelPatternNum*sizeof(GS::Bool8), REALLOC_MOVEABLE, 0);
	if (memo.cWSegPanelPattern != NULL) {
	for (UInt32 ii = 0; ii < curtainWallElement.curtainWall.segmentData.panelPatternNum; ++ii)
	memo.cWSegPanelPattern[ii] = GS::Bool8(ii % 3);
	}
	*/

	err = ACAPI_Element_Create(&curtainWallElement, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	sendElementID(getClientSocket(), curtainWallElement);
	
	/*
	API_Element elementTest;
	API_ElementMemo memoTest;
	BNZeroMemory(&elementTest, sizeof(API_Element));
	BNZeroMemory(&memoTest, sizeof(API_ElementMemo));
	elementTest.header.guid = curtainWallElement.header.guid;
	
	ACAPI_Element_Get(&elementTest);
	ACAPI_Element_GetMemo(curtainWallElement.header.guid, &memoTest);
	
	//elementTest.header.guid = APINULLGuid;
	elementTest.header.guid = emptyGuid;
	elementTest.header.typeID = API_CurtainWallID;
	//(memo.cWallSegments)->gridAngle = PI / 4;
	//memo.cWallSegments = memoTest.cWallSegments;
	
	//msgArchiCAD((int)elementTest.curtainWall.nPanels);

	err = ACAPI_Element_Create(&elementTest, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}
	
	//memoTest.cWSegGridMesh[0].meshPolygons[0].
	//int size = BMhGetSize(reinterpret_cast<GSHandle> (memoTest.cWallSegments)) / Sizeof32(API_CWallSegmentType);
	//for (int i = 0; i < curtainWallElement.curtainWall.nSegments; i++){
	//	sprintf(buffer, "begC.x = %f begC.y = %f begC.z = %f endC.x = %f endC.y = %f endC.z = %f", memoTest.cWallSegments[i].begC.x, memoTest.cWallSegments[i].begC.y, memoTest.cWallSegments[i].begC.z, memoTest.cWallSegments[i].endC.x, memoTest.cWallSegments[i].endC.y, memoTest.cWallSegments[i].endC.z);
	//	ACAPI_WriteReport(buffer, true);
	//}
	ACAPI_DisposeElemMemoHdls(&memoTest);
	*/

	ACAPI_DisposeElemMemoHdls(&memo);


}
//Not working - don't know why
void transformCW(){
	API_Element		element, mask;
	GSErrCode		err;
	transformmsg	transformMsg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &transformMsg);

	BNZeroMemory(&element, sizeof(API_Element));

	element.header.guid = APIGuidFromString(transformMsg.guid().c_str());

	if (ACAPI_Element_Get(&element) == NoError){

		TRANMAT t;
		for (int i = 0; i < 12; i++){
			t.tmx[i] = element.curtainWall.planeMatrix.tmx[i];
		}

		if (transformMsg.op() == "t"){
			Geometry::TMTranslation(&t, transformMsg.x(), transformMsg.y(), transformMsg.z());
		}
		if (transformMsg.op() == "x"){
			Geometry::TMRotation_X(&t, transformMsg.angle());
		}
		if (transformMsg.op() == "y"){
			Geometry::TMRotation_Y(&t, transformMsg.angle());
		}
		if (transformMsg.op() == "z"){
			Geometry::TMRotation_Z(&t, transformMsg.angle());
		}
		if (transformMsg.op() == "s"){
			Geometry::TMScale3D(&t, transformMsg.scale());
		}

		for (int i = 0; i < 12; i++){
			element.curtainWall.planeMatrix.tmx[i] = t.tmx[i];
		}

		ACAPI_ELEMENT_MASK_CLEAR(mask);
		ACAPI_ELEMENT_MASK_SET(mask, API_CurtainWallType, planeMatrix);

		elementChange(&element, NULL, &mask);
		
		/*
		err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
		if (err != NoError){
			ErrorBeep("ACAPI_Element_Create (slab)", err);
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}
		*/
		
		sendElementID(getClientSocket(), element);
	}
}

void addArcsToElement(){
	API_Element			element, mask, aux;
	API_ElementMemo		memo, memo2;
	GSErrCode			err;
	elementid			eleGuid;
	polyarcsmessage		polyarcsMsg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &eleGuid);

	readDelimitedFrom(getClientSocket(), &polyarcsMsg);

	BNZeroMemory(&element, sizeof(API_Element));

	element.header.guid = APIGuidFromString(eleGuid.guid().c_str());

	if (ACAPI_Element_Get(&element) == NoError
		&& ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_Polygon) == NoError) {

		/*
		for (int i = 1; i <= 3; i++){
		sprintf(buffer, "x: %f y: %f", (*memo.coords)[i].x, (*memo.coords)[i].y);
		ACAPI_WriteReport(buffer, true);
		}

		for (int i = 0; i < polyarcsMsg.arcangle_size(); i++){
		sprintf(buffer, "bindex: %d eindex: %d angle: %f", (*memo.parcs)[i].begIndex, (*memo.parcs)[i].endIndex, (*memo.parcs)[i].arcAngle);
		ACAPI_WriteReport(buffer, true);
		}
		*/

		BMKillHandle((GSHandle *)&memo.parcs);
		memo.parcs = (API_PolyArc**)BMhAllClear((polyarcsMsg.arcangle_size()) * sizeof(API_PolyArc));
		//memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((polyarcsMsg.arcangle_size() + 1) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

		if (memo.parcs != NULL){

			for (int i = 0; i < polyarcsMsg.arcangle_size(); i++){

				(*memo.parcs)[i].begIndex = polyarcsMsg.begindex(i);
				(*memo.parcs)[i].endIndex = polyarcsMsg.endindex(i);
				(*memo.parcs)[i].arcAngle = polyarcsMsg.arcangle(i);

				//sprintf(buffer, "bindex: %d eindex: %d", (*memo.parcs)[i].begIndex, (*memo.parcs)[i].endIndex);
				//ACAPI_WriteReport(buffer, true);

			}
			/*
			(*memo.parcs)[0].begIndex = 1;
			(*memo.parcs)[0].endIndex = 2;
			(*memo.parcs)[0].arcAngle = 90 * DEGRAD;

			(*memo.parcs)[1].begIndex = 2;
			(*memo.parcs)[1].endIndex = 3;
			(*memo.parcs)[1].arcAngle = 90 * DEGRAD;
			*/
			element.curtainWall.polygon.nArcs = 2;

			ACAPI_ELEMENT_MASK_CLEAR(mask);
			ACAPI_ELEMENT_MASK_SET(mask, API_CurtainWallType, polygon);
			err = ACAPI_Element_Change(&element, &mask, &memo, APIMemoMask_Polygon, true);

			if (err != NoError){
				ErrorBeep("Cannot add arcs", err);
				sprintf(buffer, "Error: %d", err);
				ACAPI_WriteReport(buffer, true);
			}

			ACAPI_Element_GetMemo(element.header.guid, &memo2, APIMemoMask_Polygon);
			for (int i = 0; i < polyarcsMsg.arcangle_size(); i++){
				sprintf(buffer, "bindex: %d eindex: %d angle: %f", (*memo2.parcs)[i].begIndex, (*memo2.parcs)[i].endIndex, (*memo2.parcs)[i].arcAngle);
				ACAPI_WriteReport(buffer, true);
			}
			ACAPI_DisposeElemMemoHdls(&memo2);

		}
		else {
			ErrorBeep("Cannot add arcs", APIERR_MEMFULL);
		}

	}

	sendElementID(getClientSocket(), element);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createColumn(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode			err;
	columnmsg		columnMsg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &columnMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.typeID = API_ColumnID;
	element.header.layer = 1;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	element.header.floorInd = getCurrentLevel();

	element.column.bottomOffset = columnMsg.bottom();
	element.column.height = columnMsg.height();
	element.column.circleBased = columnMsg.circlebased();
	element.column.origoPos.x = columnMsg.posx();
	element.column.origoPos.y = columnMsg.posy();
	//assume angle is given in degrees
	element.column.angle = columnMsg.angle() * DEGRAD;
	element.column.coreDepth = columnMsg.depth();
	element.column.coreWidth = columnMsg.width();

	err = ACAPI_Element_Create(&element, &memo);
	sendElementID(getClientSocket(), element);

	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);

}

void createNewColumn(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	columnmsg		columnMsg;
	API_StoryInfo	storyInfo;
	char buffer[256];
	bool crash = false;

	readDelimitedFrom(getClientSocket(), &columnMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.typeID = API_ColumnID;
	

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}
	element.header.layer = getLayerNumber(columnMsg.layer());
	element.header.floorInd = columnMsg.bottomindex();

	element.column.bottomOffset = 0;
	element.column.topOffset = 0;
	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	
	if (columnMsg.has_height()){
		//element.column.linkToSettings.newCreationMode = false;
		//element.column.bottomOffset = columnMsg.bottom();
		//element.column.topOffset = - (element.column.height - columnMsg.height());
		//element.column.topOffset = -columnMsg.height();
		element.column.height = columnMsg.height();
		element.column.relativeTopStory = 0;
	}
	else{
		element.column.height = (*storyInfo.data)[columnMsg.upperindex()].level - (*storyInfo.data)[columnMsg.bottomindex()].level;
		element.column.relativeTopStory = columnMsg.upperindex() - columnMsg.bottomindex();
	}

	element.column.circleBased = columnMsg.circlebased();
	element.column.origoPos.x = columnMsg.posx();
	element.column.origoPos.y = columnMsg.posy();

	element.column.angle = columnMsg.angle();
	element.column.coreDepth = columnMsg.depth();
	element.column.coreWidth = columnMsg.width();

	element.column.isSlanted = true;
	element.column.slantAngle = columnMsg.slantangle();
	element.column.slantDirectionAngle = columnMsg.slantdirection();

	element.column.bottomOffset = columnMsg.bottom();

	//Check if the wall uses a profile
	if (columnMsg.profilename() != ""){
		int profileIndex = searchStringIntProfileTypes(columnMsg.profilename());
		if (profileIndex == -1){
			crash = true;
		}
		else {
			element.column.modelElemStructureType = API_ProfileStructure;
			element.column.profileAttr = profileIndex;
			element.column.profileType = APISect_Poly;
		}
	}


	err = ACAPI_Element_Create(&element, &memo);
	sendElementID(getClientSocket(), element, crash);

	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);

}

void createBeam(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	beammsg		beamMsg;
	API_StoryInfo	storyInfo;
	char buffer[256];
	short			material = 0;
	bool			crash = false;

	readDelimitedFrom(getClientSocket(), &beamMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.typeID = API_BeamID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	element.header.layer = getLayerNumber(beamMsg.layer());

	element.header.floorInd = beamMsg.bottomlevel();

	element.beam.begC.x = beamMsg.x0();
	element.beam.begC.y = beamMsg.y0();

	element.beam.endC.x = beamMsg.x1();
	element.beam.endC.y = beamMsg.y1();

	element.beam.height = beamMsg.beamheight();

	element.beam.width = beamMsg.beamwidth();

	element.beam.level = beamMsg.levelheight();

	element.beam.isSlanted = true;

	element.beam.slantAngle = beamMsg.angle();

	element.beam.profileAngle = beamMsg.profileangle();

	material = searchBuildingMaterials(beamMsg.material());
	element.beam.buildingMaterial = material;
	element.beam.modelElemStructureType = API_BasicStructure;

	if (material == -1){
		crash = true;
	}


	//Check if the wall uses a profile
	if (beamMsg.profilename() != ""){
		int profileIndex = searchStringIntProfileTypes(beamMsg.profilename());
		if (profileIndex == -1){
			crash = true;
		}
		else {
			element.beam.modelElemStructureType = API_ProfileStructure;
			element.beam.profileAttr = profileIndex;
			element.beam.profileType = APISect_Poly;
		}
	}

	err = ACAPI_Element_Create(&element, &memo);
	sendElementID(getClientSocket(), element, crash);

	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createSlab(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	slabmessage		slabMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	short			material;
	bool crash = false;
	char buffer[256];

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	readDelimitedFrom(getClientSocket(), &slabMsg);
	readDelimitedFrom(getClientSocket(), &pointsMsg);
	readDelimitedFrom(getClientSocket(), &polyarcsMsg);

	element.header.typeID = API_SlabID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		sprintf(buffer, "Default Problem");
		ACAPI_WriteReport(buffer, true);
	}

	element.header.floorInd = getCurrentLevel();

	element.slab.level = slabMsg.level();

	if (slabMsg.type() == "BasicStructure"){
		material = searchBuildingMaterials(slabMsg.material());
		element.slab.modelElemStructureType = API_BasicStructure;
		element.slab.buildingMaterial = material;
		if (slabMsg.thickness() > 0){
			element.slab.thickness = slabMsg.thickness();
		}
	}
	else if (slabMsg.type() == "CompositeStructure"){
		material = searchCompositeMaterials(slabMsg.material());
		element.slab.composite = material;
		element.slab.modelElemStructureType = API_CompositeStructure;
	}

	if (material == -1){
		crash = true;
	}

	element.slab.poly.nCoords = pointsMsg.px_size();
	element.slab.poly.nSubPolys = 1;
	element.slab.poly.nArcs = polyarcsMsg.arcangle_size();

	memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((element.slab.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.slab.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((element.slab.poly.nArcs) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	if (memo.coords == NULL || memo.pends == NULL || memo.parcs == NULL) {
		ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
		ACAPI_DisposeElemMemoHdls(&memo);
	}

	memo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle((element.slab.poly.nCoords + 1) * sizeof(API_EdgeTrim), ALLOCATE_CLEAR, 0));
	memo.sideMaterials = reinterpret_cast<API_MaterialOverrideType*> (BMAllocatePtr((element.slab.poly.nCoords + 1) * sizeof(API_MaterialOverrideType), ALLOCATE_CLEAR, 0));

	for (int i = 1; i <= pointsMsg.px_size(); i++){
		(*memo.coords)[i].x = pointsMsg.px(i - 1);
		(*memo.coords)[i].y = pointsMsg.py(i - 1);
	}

	(*memo.pends)[1] = pointsMsg.px_size();

	for (int i = 0; i < polyarcsMsg.arcangle_size(); i++){
		(*memo.parcs)[i].begIndex = polyarcsMsg.begindex(i);
		(*memo.parcs)[i].endIndex = polyarcsMsg.endindex(i);
		(*memo.parcs)[i].arcAngle = polyarcsMsg.arcangle(i);
	}

	////////////////////////
	/*
	//HOLE

	memo.gables = reinterpret_cast<API_Gable**> (BMAllocateHandle(1 * sizeof(API_Gable), ALLOCATE_CLEAR, 0));
	int numHolePoints = 5;
	(*memo.gables)[0].coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((numHolePoints + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	(*memo.gables)[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((1 + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));

	(*memo.gables)[0].xb = 0.0;
	(*memo.gables)[0].xe = 1.0;
	(*memo.gables)[0].nx = 1.0;
	(*memo.gables)[0].ny = 0.0;

	//Plane
	(*memo.gables)[0].a = 0.0;
	(*memo.gables)[0].b = 0.0;
	(*memo.gables)[0].c = 1.0;
	(*memo.gables)[0].d = 0.0;

	(*(*memo.gables)[0].pends)[1] = numHolePoints;


	(*(*memo.gables)[0].coords)[1].x = 0.0;
	(*(*memo.gables)[0].coords)[1].y = 0.0;

	(*(*memo.gables)[0].coords)[2].x = 1.0;
	(*(*memo.gables)[0].coords)[2].y = 0.0;

	(*(*memo.gables)[0].coords)[3].x = 1.0;
	(*(*memo.gables)[0].coords)[3].y = 1.0;

	(*(*memo.gables)[0].coords)[4].x = 0.0;
	(*(*memo.gables)[0].coords)[4].y = 1.0;

	(*(*memo.gables)[0].coords)[5].x = 0.0;
	(*(*memo.gables)[0].coords)[5].y = 0.0;
	*/
	/*
	memo.shellContours = (API_ShellContourData *)BMAllocatePtr(1 * sizeof(API_ShellContourData), ALLOCATE_CLEAR, 0);
	if (memo.shellContours == NULL) {
	ErrorBeep("Not enough memory to create shell contour data", APIERR_MEMFULL);
	ACAPI_DisposeElemMemoHdls(&memo);
	return;
	}

	memo.shellContours[0].poly.nCoords = 3;
	memo.shellContours[0].poly.nSubPolys = 1;
	memo.shellContours[0].poly.nArcs = 2;
	memo.shellContours[0].coords = (API_Coord**)BMAllocateHandle((memo.shellContours[0].poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellContours[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((memo.shellContours[0].poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.shellContours[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(memo.shellContours[0].poly.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	if (memo.shellContours[0].coords == NULL || memo.shellContours[0].pends == NULL || memo.shellContours[0].parcs == NULL) {
	ErrorBeep("Not enough memory to create shell contour data", APIERR_MEMFULL);
	ACAPI_DisposeElemMemoHdls(&memo);
	return;
	}

	(*memo.shellContours[0].coords)[1].x = 0.0;
	(*memo.shellContours[0].coords)[1].y = 2.0;
	(*memo.shellContours[0].coords)[2].x = 0.0;
	(*memo.shellContours[0].coords)[2].y = 4.0;
	(*memo.shellContours[0].coords)[3].x = 0.0;
	(*memo.shellContours[0].coords)[3].y = 2.0;

	(*memo.shellContours[0].pends)[1] = memo.shellContours[0].poly.nCoords;

	(*memo.shellContours[0].parcs)[0].begIndex = 1;
	(*memo.shellContours[0].parcs)[0].endIndex = 2;
	(*memo.shellContours[0].parcs)[0].arcAngle = 180.0 * DEGRAD;
	(*memo.shellContours[0].parcs)[1].begIndex = 2;
	(*memo.shellContours[0].parcs)[1].endIndex = 3;
	(*memo.shellContours[0].parcs)[1].arcAngle = 180.0 * DEGRAD;
	*/
	////////////////////////

	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (slab)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		//sprintf(buffer, "No creation");
		//ACAPI_WriteReport(buffer, true);
	}

	sendElementID(getClientSocket(), element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);

}

void createNewSlab(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	slabmessage		msg;
	short			material;
	bool crash = false;
	char buffer[256];

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	readDelimitedFrom(getClientSocket(), &msg);

	element.header.typeID = API_SlabID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		sprintf(buffer, "Default Problem");
		ACAPI_WriteReport(buffer, true);
	}

	element.header.layer = getLayerNumber(msg.layer());

	element.header.floorInd = msg.bottomlevel();

	element.slab.level = msg.level();

	if (msg.reference() == "top" || msg.reference() == "Top"){
		element.slab.referencePlaneLocation = APISlabRefPlane_Top;
	}
	else if (msg.reference() == "bottom" || msg.reference() == "Bottom"){
		element.slab.referencePlaneLocation = APISlabRefPlane_Bottom;
	}
	else if (msg.type() == "Composite"){
		if (msg.reference() == "corebottom" || msg.reference() == "coreBottom" || msg.reference() == "CoreBottom"){
			element.slab.referencePlaneLocation = APISlabRefPlane_CoreBottom;
		}
		else if (msg.reference() == "coretop" || msg.reference() == "coreTop" || msg.reference() == "CoreTop"){
			element.slab.referencePlaneLocation = APISlabRefPlane_CoreTop;
		}
	}
	else{
		msgArchiCAD("Error: A slab with a simple material (i.e. not composite) does not support coreBottom or coreTop");
		msgArchiCAD(msg.reference());
		quit();
	}

	if (msg.type() == "Basic"){
		material = searchBuildingMaterials(msg.material());
		element.slab.modelElemStructureType = API_BasicStructure;
		element.slab.buildingMaterial = material;
		if (msg.thickness() > 0){
			element.slab.thickness = msg.thickness();
		}
	}
	else if (msg.type() == "Composite"){
		material = searchCompositeMaterials(msg.material());
		element.slab.composite = material;
		element.slab.modelElemStructureType = API_CompositeStructure;
		if (msg.thickness() != 0.3){
			msgArchiCAD("Composite Material do not support thickness!");
			quit();
		}
	}

	if (material == -1){
		crash = true;
	}

	element.slab.poly.nCoords = msg.pts().px_size();
	element.slab.poly.nSubPolys = msg.subpolygons_size();
	element.slab.poly.nArcs = msg.parcs().arcangle_size();

	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.slab.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));

	if (memo.pends == NULL) {
		ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
		ACAPI_DisposeElemMemoHdls(&memo);
		quit();
		return;
	}

	populateMemo(&memo, msg.pts(), msg.parcs());

	memo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle((element.slab.poly.nCoords + 1) * sizeof(API_EdgeTrim), ALLOCATE_CLEAR, 0));
	memo.sideMaterials = reinterpret_cast<API_MaterialOverrideType*> (BMAllocatePtr((element.slab.poly.nCoords + 1) * sizeof(API_MaterialOverrideType), ALLOCATE_CLEAR, 0));

	//for (int i = 1; i < element.slab.poly.nCoords; i++){
	//	(*memo.coords)[i].x = pointsMsg.px(i - 1);
	//	(*memo.coords)[i].y = pointsMsg.py(i - 1);
	//}

	//(*memo.coords)[element.slab.poly.nCoords].x = pointsMsg.px(0);
	//(*memo.coords)[element.slab.poly.nCoords].y = pointsMsg.py(0);

	//for (int i = 1; i <= element.slab.poly.nCoords; i++){
	//	sprintf(buffer, "i: %d X: %f Y: %f", i, (*memo.coords)[i].x, (*memo.coords)[i].y);
	//	ACAPI_WriteReport(buffer, true);
	//}

	for (int i = 1; i <= msg.subpolygons_size(); i++){
		(*memo.pends)[i] = msg.subpolygons(i - 1);
	}

	err = ACAPI_Element_Create(&element, &memo);
	if (hasError(err)){
		quit();
		return;
	}

	sendElementID(getClientSocket(), element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);

}

void createWallsFromSlab(){
	API_Element		slabElement, element, mask;
	API_ElementMemo slabMemo, memo;
	GSErrCode		err;
	wallsfromslab	msg;
	elementidlist	elementIDList;
	short			material;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &msg);

	BNZeroMemory(&slabElement, sizeof(API_Element));
	BNZeroMemory(&slabMemo, sizeof(API_ElementMemo));

	slabElement.header.guid = APIGuidFromString(msg.guid().c_str());


	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_WallID;
	element.header.layer = getLayerNumber(msg.layer());


	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	if (msg.type() == "BasicStructure"){
		material = searchBuildingMaterials(msg.material());
		element.wall.modelElemStructureType = API_BasicStructure;
		element.wall.buildingMaterial = material;
	}
	else if (msg.type() == "CompositeStructure"){
		material = searchCompositeMaterials(msg.material());
		element.wall.modelElemStructureType = API_CompositeStructure;
		element.wall.composite = material;
	}

	if (material == -1){
		crash = true;
	}


	element.wall.height = msg.height();
	if (msg.referenceline() == "Center"){
		element.wall.referenceLineLocation = APIWallRefLine_Center;
	}
	else if (msg.referenceline() == "Outside"){
		element.wall.referenceLineLocation = APIWallRefLine_Outside;
	}
	else if (msg.referenceline() == "Inside"){
		element.wall.referenceLineLocation = APIWallRefLine_Inside;
	}
	element.wall.referenceLineLocation = APIWallRefLine_Center;

	element.wall.thickness = msg.thickness();

	element.wall.refMat.overrideMaterial = false;
	element.wall.oppMat.overrideMaterial = false;
	element.wall.sidMat.overrideMaterial = false;

	if (ACAPI_Element_Get(&slabElement) == NoError
		&& ACAPI_Element_GetMemo(slabElement.header.guid, &slabMemo, APIMemoMask_Polygon) == NoError) {
		int pIndex = 1;
		element.header.floorInd = slabElement.header.floorInd;
		element.wall.useCompositePriority = false;
		for (int i = 0; i < slabElement.slab.poly.nCoords - 1; i++){


			//Beginning Point
			element.wall.begC.x = (*slabMemo.coords)[pIndex].x;
			element.wall.begC.y = (*slabMemo.coords)[pIndex].y;
			//Ending Point
			element.wall.endC.x = (*slabMemo.coords)[pIndex + 1].x;
			element.wall.endC.y = (*slabMemo.coords)[pIndex + 1].y;

			int numberOfAngles = BMhGetSize(reinterpret_cast<GSHandle> (slabMemo.parcs)) / Sizeof32(API_PolyArc);
			//sprintf(buffer, "size: %d", numberOfAngles);
			//ACAPI_WriteReport(buffer, true);
			element.wall.angle = 0;
			for (int j = 0; j < numberOfAngles; j++){
				if ((*slabMemo.parcs)[j].begIndex == pIndex && (*slabMemo.parcs)[j].endIndex == (pIndex + 1)){
					element.wall.angle = (*slabMemo.parcs)[j].arcAngle;
				}
			}

			//sprintf(buffer, "BegIndex: %d EndIndex: %d ArcAngle: %f", (*slabMemo.parcs)[i].begIndex, (*slabMemo.parcs)[i].endIndex, (*slabMemo.parcs)[i].arcAngle);
			//ACAPI_WriteReport(buffer, true);


			err = ACAPI_Element_Create(&element, &memo);
			if (err != NoError){
				ErrorBeep("ACAPI_Element_Create (slab)", err);
				sprintf(buffer, "No creation");
				ACAPI_WriteReport(buffer, true);
			}

			//Convert Element Id from GuId to char*
			GS::UniString guidString = APIGuidToString(element.header.guid);
			char s[64];
			APIGuid2GSGuid(element.header.guid).ConvertToString(s);
			elementIDList.add_guid(s);
			pIndex++;
		}
		elementIDList.set_crashmaterial(crash);
		writeDelimitedTo(getClientSocket(), elementIDList);
	}
	else{
		sprintf(buffer, "Fail");
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&slabMemo);
}

void createCWallsFromSlab(){
	API_Element		slabElement, element, mask;
	API_ElementMemo slabMemo, memo;
	GSErrCode		err;
	elementid		eleIdMsg;
	doublemessage	heightMsg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &eleIdMsg);
	readDelimitedFrom(getClientSocket(), &heightMsg);

	BNZeroMemory(&slabElement, sizeof(API_Element));
	BNZeroMemory(&slabMemo, sizeof(API_ElementMemo));

	slabElement.header.guid = APIGuidFromString(eleIdMsg.guid().c_str());

	if (ACAPI_Element_Get(&slabElement) == NoError
		&& ACAPI_Element_GetMemo(slabElement.header.guid, &slabMemo, APIMemoMask_Polygon) == NoError) {

		BNZeroMemory(&element, sizeof(API_Element));
		BNZeroMemory(&memo, sizeof(API_ElementMemo));

		element.header.typeID = API_CurtainWallID;
		element.header.layer = 1;

		err = ACAPI_Element_GetDefaults(&element, &memo);
		if (err != NoError) {
			ErrorBeep("ACAPI_Element_GetMemo", err);
			return;
		}
		element.header.floorInd = getCurrentLevel();
		element.curtainWall.polygon.nCoords = slabElement.slab.poly.nCoords;
		element.curtainWall.polygon.nSubPolys = slabElement.slab.poly.nSubPolys;
		element.curtainWall.polygon.nArcs = slabElement.slab.poly.nArcs;
		element.curtainWall.height = heightMsg.d();
		//sprintf(buffer, "offset: %f story: %f planeOffset: %f homeStory: %d", element.curtainWall.offset, element.curtainWall.storyRelLevel, element.curtainWall.planeOffset, element.curtainWall.linkToSettings.homeStoryDifference);
		//ACAPI_WriteReport(buffer, true);

		element.curtainWall.storyRelLevel = 10000;

		sprintf(buffer, "1 storyLevel: %f", element.curtainWall.storyRelLevel);
		ACAPI_WriteReport(buffer, true);

		memo.coords = (API_Coord**)BMAllocateHandle((element.curtainWall.polygon.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
		memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(element.curtainWall.polygon.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

		for (int i = 1; i <= element.curtainWall.polygon.nCoords; i++){
			(*memo.coords)[i].x = (*slabMemo.coords)[i].x;
			(*memo.coords)[i].y = (*slabMemo.coords)[i].y;
		}

		for (int i = 0; i < element.curtainWall.polygon.nArcs; i++){
			(*memo.parcs)[i].begIndex = (*slabMemo.parcs)[i].begIndex;
			(*memo.parcs)[i].endIndex = (*slabMemo.parcs)[i].endIndex;
			(*memo.parcs)[i].arcAngle = (*slabMemo.parcs)[i].arcAngle;
		}

		err = ACAPI_Element_Create(&element, &memo);
		if (err != NoError){
			ErrorBeep("ACAPI_Element_Create (slab)", err);
			sprintf(buffer, "No creation");
			ACAPI_WriteReport(buffer, true);
		}

		sprintf(buffer, "2 storyLevel: %f", element.curtainWall.storyRelLevel);
		ACAPI_WriteReport(buffer, true);

		ACAPI_DisposeElemMemoHdls(&memo);
		ACAPI_DisposeElemMemoHdls(&slabMemo);

		sendElementID(getClientSocket(), element);
	}
}

void createColumnsFromSlab(){
	API_Element		slabElement, element;
	API_ElementMemo slabMemo, memo;
	GSErrCode		err;
	columnsfromslab	msg;
	elementidlist	elementIDList;
	short			material;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &msg);

	BNZeroMemory(&slabElement, sizeof(API_Element));
	BNZeroMemory(&slabMemo, sizeof(API_ElementMemo));

	slabElement.header.guid = APIGuidFromString(msg.guid().c_str());

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_ColumnID;
	element.header.layer = getLayerNumber(msg.layer());

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	material = searchBuildingMaterials(msg.material());
	element.column.modelElemStructureType = API_BasicStructure;
	element.column.buildingMaterial = material;

	if (material == -1){
		crash = true;
	}

	element.column.topOffset = -msg.height();
	element.column.height = msg.height();
	element.column.coreDepth = msg.depth();
	element.column.coreWidth = msg.width();

	element.column.circleBased = msg.circlebased();

	if (ACAPI_Element_Get(&slabElement) == NoError
		&& ACAPI_Element_GetMemo(slabElement.header.guid, &slabMemo, APIMemoMask_Polygon) == NoError) {

		element.header.floorInd = slabElement.header.floorInd;
		element.column.bottomOffset = slabElement.slab.level;
		int pIndex = 1;
		for (int i = 0; i < slabElement.slab.poly.nCoords - 1; i++){
			element.column.origoPos.x = (*slabMemo.coords)[pIndex].x;
			element.column.origoPos.y = (*slabMemo.coords)[pIndex].y;

			err = ACAPI_Element_Create(&element, &memo);
			if (err != NoError){
				ErrorBeep("ACAPI_Element_Create (slab)", err);
				sprintf(buffer, "No creation");
				ACAPI_WriteReport(buffer, true);
			}

			//Convert Element Id from GuId to char*
			//GS::UniString guidString = APIGuidToString(element.header.guid);
			char s[64];
			APIGuid2GSGuid(element.header.guid).ConvertToString(s);
			elementIDList.add_guid(s);
			pIndex++;
		}
		elementIDList.set_crashmaterial(crash);
		writeDelimitedTo(getClientSocket(), elementIDList);
	}
	else{
		sprintf(buffer, "Fail");
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&slabMemo);
}

void createObject(){
	API_Element		element, mask;
	API_ElementMemo memo;
	GSErrCode		err;
	objectmsg		objectMsg;
	char buffer[256];


	API_DatabaseInfo dbInfo;
	API_DatabaseInfo floorInfo;
	floorInfo.typeID = APIWind_FloorPlanID;
	floorInfo.index = 0;
	ACAPI_Database(APIDb_GetCurrentDatabaseID, &dbInfo);
	ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &floorInfo);

	readDelimitedFrom(getClientSocket(), &objectMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_ObjectID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		//sprintf(buffer, "Default Problem");
		//ACAPI_WriteReport(buffer, true);
	}

	element.header.layer = getLayerNumber(objectMsg.layer());

	element.header.floorInd = objectMsg.level();

	//sprintf(buffer, "libInd: %d", element.object.libInd);
	//ACAPI_WriteReport(buffer, true);

	element.object.level = objectMsg.bottom();
	//Supposed to change the point of rotation. It changes in the library part menu, but the angle does not take that into consideration.
	//element.object.fixPoint = 2;
	element.object.angle = objectMsg.angle();

	if (objectMsg.has_name()){
		element.object.libInd = searchObjects(objectMsg.name());
	}
	else{
		element.object.libInd = objectMsg.index();
	}

	element.object.pos.x = objectMsg.posx();
	element.object.pos.y = objectMsg.posy();

	element.object.useObjSectAttrs = objectMsg.useobjsectattrs();
	element.object.useXYFixSize = objectMsg.usexyfixsize();
	element.object.xRatio = objectMsg.xratio();
	element.object.yRatio = objectMsg.yratio();


	API_ParamOwnerType   paramOwner;
	API_GetParamsType    getParams;

	BNZeroMemory(&paramOwner, sizeof(API_ParamOwnerType));
	paramOwner.libInd = element.object.libInd;

	BNZeroMemory(&getParams, sizeof(API_GetParamsType));

	err = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner, NULL);
	if (err == NoError) {
		err = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, NULL);
		if (err == NoError) {
			handleParams(objectMsg.params(), &paramOwner, &getParams);

			memo.params = getParams.params;
			err = ACAPI_Element_Create(&element, &memo);
			if (hasError(err)){
				msgArchiCAD("Aqui");
				quit();
				return;
			}

			ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &dbInfo);

			sendElementID(getClientSocket(), element);

			//required to force update on width/height
			elementRefresh(&element);

			//ACAPI_ELEMENT_MASK_CLEAR(mask);
			//ACAPI_ELEMENT_MASK_SET(mask, API_ObjectType, angle);
			//element.object.angle = objectMsg.angle();
			//err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
		}
		ACAPI_Goodies(APIAny_CloseParametersID, NULL, NULL);
	}

	ACAPI_DisposeAddParHdl(&getParams.params);

	if (hasError(err)){
		quit();
		return;
	}

	//ACAPI_DisposeElemMemoHdls(&memo);
}

void createLibraryPart(){
	GSErrCode err = NoError;
	API_LibPart libPart;
	bool crash = false;
	char buffer[256];
	libpartmsg msg;
	BNZeroMemory(&libPart, sizeof(API_LibPart));

	readDelimitedFrom(getClientSocket(), &msg);

	libPart.isTemplate = false;
	libPart.isPlaceable = true;

	int libPartID = searchStringIntTypes(msg.type());
	if (libPartID == -1){
		// not found - quit
		ACAPI_WriteReport("No Type was found", true);
		return;
	}
	else{
		libPart.typeID = (API_LibTypeID)libPartID;
	}

	/*
	stringIntMap::const_iterator iterInt = types.find(msg.type());
	if (iterInt == types.end()){
	// not found - quit
	ACAPI_WriteReport("No Type was found", true);
	return;
	}
	else{
	libPart.typeID = (API_LibTypeID) iterInt->second;
	}
	*/

	std::string parentString = searchParentsID(msg.parentid());
	if (parentString == ""){
		// not found - quit
		ACAPI_WriteReport("No Parent ID was found", true);
		return;
	}
	else{
		CHCopyC(parentString.c_str(), libPart.parentUnID);
	}

	/*
	stringStringMap::const_iterator iter = parentsID.find(msg.parentid());
	if (iter == parentsID.end()){
	// not found - quit
	ACAPI_WriteReport("No Parent ID was found", true);
	return;
	}
	else{
	CHCopyC(iter->second.c_str(), libPart.parentUnID);
	}
	*/

	GS::snuprintf(libPart.docu_UName, sizeof(libPart.docu_UName) / sizeof(GS::uchar_t), msg.name().c_str());

	err = GetLocation(libPart.location, true);
	if (err != NoError) {
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		return;
	}

	ACAPI_Environment(APIEnv_OverwriteLibPartID, (void *)(Int32)true, NULL);
	err = ACAPI_LibPart_Create(&libPart);
	ACAPI_Environment(APIEnv_OverwriteLibPartID, (void *)(Int32)false, NULL);

	if (err == NoError) {
		API_LibPartSection section;

		// Comment script section
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_SectComText;
		ACAPI_LibPart_NewSection(&section);
		sprintf(buffer, "Library Part written by addon");
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		ACAPI_LibPart_EndSection();

		// Master script section
		/*
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_Sect1DScript;
		ACAPI_LibPart_NewSection(&section);
		buffer[0] = '\0';
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		ACAPI_LibPart_EndSection();
		*/
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_Sect1DScript;
		ACAPI_LibPart_NewSection(&section);
		ACAPI_LibPart_WriteSection(msg.mastercode().size(), (GSPtr)msg.mastercode().c_str());
		ACAPI_LibPart_EndSection();

		// 3D script section
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_Sect3DScript;
		ACAPI_LibPart_NewSection(&section);
		ACAPI_LibPart_WriteSection(msg.threecode().size(), (GSPtr)msg.threecode().c_str());
		ACAPI_LibPart_EndSection();

		// 2D script section
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_Sect2DScript;
		ACAPI_LibPart_NewSection(&section);
		ACAPI_LibPart_WriteSection(msg.twocode().size(), (GSPtr)msg.twocode().c_str());
		ACAPI_LibPart_EndSection();

		// Parameter script section
		/*
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_SectVLScript;
		ACAPI_LibPart_NewSection(&section);
		sprintf(buffer, "VALUES \"zzyzx\" RANGE [6,]%s", GS::EOL);
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		ACAPI_LibPart_EndSection();
		*/
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_SectVLScript;
		ACAPI_LibPart_NewSection(&section);
		ACAPI_LibPart_WriteSection(msg.parametercode().size(), (GSPtr)msg.parametercode().c_str());
		ACAPI_LibPart_EndSection();

		// Parameters section
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_SectParamDef;

		API_AddParType** addPars = reinterpret_cast<API_AddParType**>(BMAllocateHandle(msg.names_size() * sizeof(API_AddParType), ALLOCATE_CLEAR, 0));
		if (addPars != NULL) {
			int doubleCounter = 0;
			int stringCounter = 0;
			int intCounter = 0;
			int boolCounter = 0;

			int doubleArrayCounter = 0;
			int stringArrayCounter = 0;
			int intArrayCounter = 0;
			int boolArrayCounter = 0;
			for (int i = 0; i < msg.names_size(); i++){
				std::string parameterName = msg.names(i);
				bool isArray = msg.isarray(i);

				CHTruncate(msg.names(i).c_str(), (*addPars)[i].name, sizeof((*addPars)[i].name));
				GS::UniString((*addPars)[i].uDescname) = msg.names(i).c_str();
				/*
				for (int k = 0; k < msg.names(i).size(); k++){
				(*addPars)[i].uDescname[k] = msg.names(i).at(k);
				}
				*/
				if (isArray){
					if (msg.paramtype(i) == "s"){
						std::string stringValue = msg.stringarrays(stringArrayCounter).lst(0);
						if (stringValue == "LineType"){
							(*addPars)[i].typeID = APIParT_LineTyp;
							if (msg.stringarrays(stringArrayCounter).lst_size() > 1){
								(*addPars)[i].value.real = searchLineTypes(msg.stringarrays(stringArrayCounter).lst(1));
							}
							else{
								(*addPars)[i].value.real = 1;
							}
						}
						else if (stringValue == "MaterialType"){
							(*addPars)[i].typeID = APIParT_Mater;
							if (msg.stringarrays(stringArrayCounter).lst_size()  > 1){
								(*addPars)[i].value.real = searchOverrideMaterials(msg.stringarrays(stringArrayCounter).lst(1));
							}
							else{
								(*addPars)[i].value.real = searchOverrideMaterials("Metal - Steel");
							}
						}
						else{
							(*addPars)[i].typeMod = API_ParArray;
							(*addPars)[i].typeID = APIParT_CString;

							int numberOfStrings = msg.stringarrays(stringArrayCounter).lst_size();
							int actualSize = numberOfStrings;
							for (int j = 0; j < numberOfStrings; j++){
								actualSize += msg.stringarrays(stringArrayCounter).lst(j).size();
							}

							(*addPars)[i].dim1 = numberOfStrings;
							(*addPars)[i].dim2 = 1;

							(*addPars)[i].value.array = BMAllocateHandle(actualSize * (*addPars)[i].dim2 * sizeof(GS::uchar_t), ALLOCATE_CLEAR, 0);
							GS::uchar_t** arrHdl = reinterpret_cast<GS::uchar_t**>((*addPars)[i].value.array);

							int previousStringSize = 0;
							for (Int32 j = 0; j < numberOfStrings; j++){
								for (Int32 k = 0; k < msg.stringarrays(stringArrayCounter).lst(j).size(); k++){
									(*arrHdl)[previousStringSize + j + k] = msg.stringarrays(stringArrayCounter).lst(j).at(k);
								}
								(*arrHdl)[previousStringSize + j + msg.stringarrays(stringArrayCounter).lst(j).size()] = '\0';
								previousStringSize += msg.stringarrays(stringArrayCounter).lst(j).size();
							}
						}
						stringArrayCounter++;
					}
					else if (msg.paramtype(i) == "d"){
						(*addPars)[i].typeMod = API_ParArray;
						(*addPars)[i].typeID = APIParT_RealNum;
						(*addPars)[i].dim1 = msg.doublearrays(doubleArrayCounter).lst_size();
						(*addPars)[i].dim2 = 1;

						(*addPars)[i].value.array = BMAllocateHandle((*addPars)[i].dim1 * (*addPars)[i].dim2 * sizeof(double), ALLOCATE_CLEAR, 0);
						double** arrHdl = reinterpret_cast<double**>((*addPars)[i].value.array);

						for (Int32 k = 0; k < (*addPars)[i].dim1; k++){
							(*arrHdl)[k] = msg.doublearrays(doubleArrayCounter).lst(k);
						}
						doubleArrayCounter++;
					}
					else if (msg.paramtype(i) == "i"){
						(*addPars)[i].typeMod = API_ParArray;
						(*addPars)[i].typeID = APIParT_Integer;
						(*addPars)[i].dim1 = msg.intarrays(intArrayCounter).lst_size();
						(*addPars)[i].dim2 = 1;

						//(*addPars)[i].value.array = BMAllocateHandle((*addPars)[i].dim1 * sizeof(char**), ALLOCATE_CLEAR, 0);

						(*addPars)[i].value.array = BMAllocateHandle((*addPars)[i].dim1 * (*addPars)[i].dim2 * sizeof(double), ALLOCATE_CLEAR, 0);
						double** arrHdl = reinterpret_cast<double**>((*addPars)[i].value.array);

						for (Int32 k = 0; k < (*addPars)[i].dim1; k++){
							(*arrHdl)[k] = msg.intarrays(intArrayCounter).lst(k);
						}

						/*
						//EXAMPLE WORKING MATRIX
						(*addPars)[i].typeID = APIParT_RealNum;
						(*addPars)[i].dim1 = 3;
						(*addPars)[i].dim2 = 4;

						(*addPars)[i].value.array = BMAllocateHandle((*addPars)[i].dim1 * (*addPars)[i].dim2 * sizeof(double), ALLOCATE_CLEAR, 0);
						double** arrHdl = reinterpret_cast<double**>((*addPars)[i].value.array);

						for (Int32 k = 0; k < (*addPars)[i].dim1; k++){
						for (Int32 j = 0; j < (*addPars)[i].dim2; j++){
						//(*arrHdl)[k * (*addPars)[i].dim2 + j] = msg.intarrays(intArrayCounter).lst(k);
						(*arrHdl)[k * (*addPars)[i].dim2 + j] = (k == j ? 1.1 : 0.0);
						}
						}
						*/
						//(*addPars)[i].value.array = BMAllocateHandle(sizeof(GSHandle), ALLOCATE_CLEAR, 0);
						//*(*addPars)[i].value.array = BMAllocatePtr((*addPars)[i].dim1 * sizeof(GSPtr), ALLOCATE_CLEAR, 0);

						intArrayCounter++;
					}
					else if (msg.paramtype(i) == "b"){
						(*addPars)[i].typeMod = API_ParArray;
						(*addPars)[i].typeID = APIParT_Boolean;
						(*addPars)[i].dim1 = msg.boolarrays(boolArrayCounter).lst_size();
						(*addPars)[i].dim2 = 1;
						(*addPars)[i].value.array = BMAllocateHandle((*addPars)[i].dim1 * (*addPars)[i].dim2 * sizeof(double), ALLOCATE_CLEAR, 0);
						double** arrHdl = reinterpret_cast<double**>((*addPars)[i].value.array);

						for (Int32 k = 0; k < (*addPars)[i].dim1; k++){
							(*arrHdl)[k] = msg.boolarrays(boolArrayCounter).lst(k);
						}

						boolArrayCounter++;
					}
				}
				else{
					(*addPars)[i].typeMod = 0;
					if (msg.paramtype(i) == "s"){
						std::string stringValue = msg.strings(stringCounter);	
						if (stringValue == "LineType"){
							(*addPars)[i].typeID = APIParT_LineTyp;
							(*addPars)[i].value.real = 1;
						}
						else if (stringValue == "MaterialType"){
							(*addPars)[i].typeID = APIParT_Mater;
							(*addPars)[i].value.real = searchOverrideMaterials("Metal - Steel");
						} else{
							(*addPars)[i].typeID = APIParT_CString;
							GS::UniString((*addPars)[i].value.uStr) = stringValue.c_str();
						}
						/*
						for (int k = 0; k < stringValue.size(); k++){
						(*addPars)[i].value.uStr[k] = stringValue.at(k);
						}
						*/
						stringCounter++;
					}
					else if (msg.paramtype(i) == "d"){
						(*addPars)[i].typeID = APIParT_RealNum;
						(*addPars)[i].value.real = msg.doubles(doubleCounter);
						doubleCounter++;
					}
					else if (msg.paramtype(i) == "i"){
						(*addPars)[i].typeID = APIParT_Integer;
						(*addPars)[i].value.real = msg.integers(intCounter);
						intCounter++;
					}
					else if (msg.paramtype(i) == "b"){
						(*addPars)[i].typeID = APIParT_Boolean;
						if (msg.booleans(boolCounter)){
							(*addPars)[i].value.real = 1;
						}
						else{
							(*addPars)[i].value.real = 0;
						}
						boolCounter++;
					}
				}
			}
			double aa = 1.0;
			double bb = 1.0;
			GSHandle sectionHdl = NULL;
			ACAPI_LibPart_GetSect_ParamDef(&libPart, addPars, &aa, &bb, NULL, &sectionHdl);

			API_LibPartDetails details;
			BNZeroMemory(&details, sizeof(API_LibPartDetails));
			details.object.autoHotspot = false;
			ACAPI_LibPart_SetDetails_ParamDef(&libPart, sectionHdl, &details);

			ACAPI_LibPart_AddSection(&section, sectionHdl, NULL);
			BMKillHandle(reinterpret_cast<GSHandle*>(&addPars));
			BMKillHandle(&sectionHdl);
		}
		else{
			err = APIERR_MEMFULL;
		}

		/*

		BNZeroMemory(&paramOwner, sizeof(API_ParamOwnerType));
		paramOwner.libInd = searchObjects("Elevator 18");

		BNZeroMemory(&getParams, sizeof(API_GetParamsType));

		err = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner, NULL);
		if (err == NoError) {
		err = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, NULL);
		double aa = 1.0;
		double bb = 1.0;
		GSHandle sectionHdl = NULL;
		ACAPI_LibPart_GetSect_ParamDef(&libPart, getParams.params, &aa, &bb, NULL, &sectionHdl);
		API_LibPartDetails details;
		BNZeroMemory(&details, sizeof(API_LibPartDetails));
		details.object.autoHotspot = false;
		ACAPI_LibPart_SetDetails_ParamDef(&libPart, sectionHdl, &details);

		ACAPI_LibPart_AddSection(&section, sectionHdl, NULL);

		BMKillHandle(&sectionHdl);
		}
		*/
		/*
		API_AddParType** addPars = reinterpret_cast<API_AddParType**>(BMAllocateHandle(sizeof(API_AddParType), ALLOCATE_CLEAR, 0));
		if (addPars != NULL) {


		API_AddParType* pAddPar = (*addPars);
		pAddPar->typeID = APIParT_Mater;
		pAddPar->typeMod = 0;
		CHTruncate("mat", pAddPar->name, sizeof(pAddPar->name));
		GS::ucscpy(pAddPar->uDescname, L("Material"));
		pAddPar->value.real = 1;

		double aa = 1.0;
		double bb = 1.0;
		GSHandle sectionHdl = NULL;
		ACAPI_LibPart_GetSect_ParamDef(&libPart, addPars, &aa, &bb, NULL, &sectionHdl);

		API_LibPartDetails details;
		BNZeroMemory(&details, sizeof(API_LibPartDetails));
		details.object.autoHotspot = false;
		ACAPI_LibPart_SetDetails_ParamDef(&libPart, sectionHdl, &details);

		ACAPI_LibPart_AddSection(&section, sectionHdl, NULL);
		BMKillHandle(reinterpret_cast<GSHandle*>(&addPars));
		BMKillHandle(&sectionHdl);


		}
		else {
		err = APIERR_MEMFULL;
		}
		*/

		// Save the constructed library part
		if (err == NoError){
			err = ACAPI_LibPart_Save(&libPart);
			addLibraryPart(libPart);
		}

		

		if (libPart.location != NULL) {
			delete libPart.location;
			libPart.location = NULL;
		}
	}

	if (err != NoError){
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	if (err != NoError){
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	return;
}

void createRoof(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	roofmsg			roofMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	short			material;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &roofMsg);
	readDelimitedFrom(getClientSocket(), &pointsMsg);
	readDelimitedFrom(getClientSocket(), &polyarcsMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_RoofID;
	//element.roof.roofClass = API_PolyRoofID;
	element.roof.roofClass = API_PlaneRoofID;
	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		sprintf(buffer, "Default Problem");
		ACAPI_WriteReport(buffer, true);
	}

	if (roofMsg.type() == "BasicStructure"){
		material = searchBuildingMaterials(roofMsg.material());
		element.roof.shellBase.modelElemStructureType = API_BasicStructure;
		element.roof.shellBase.buildingMaterial = material;
		if (roofMsg.thickness() > 0){
			element.slab.thickness = roofMsg.thickness();
		}
	}
	else if (roofMsg.type() == "CompositeStructure"){
		material = searchCompositeMaterials(roofMsg.material());
		element.roof.shellBase.composite = material;
		element.roof.shellBase.modelElemStructureType = API_CompositeStructure;
		if (roofMsg.thickness() != 0.3){
			msgArchiCAD("Composite Material do not support thickness!");
			quit();
		}
	}

	if (material == -1){
		crash = true;
	}

	element.roof.shellBase.level = roofMsg.height();

	element.roof.u.planeRoof.baseLine.c1.x = 0;
	element.roof.u.planeRoof.baseLine.c1.y = 0;
	element.roof.u.planeRoof.baseLine.c2.x = 0;
	element.roof.u.planeRoof.baseLine.c2.y = 0;

	element.roof.u.planeRoof.angle = 0;

	element.header.floorInd = getCurrentLevel();

	//sprintf(buffer, "libInd: %d", element.object.libInd);
	//ACAPI_WriteReport(buffer, true);

	element.roof.u.planeRoof.poly.nCoords = pointsMsg.px_size();
	element.roof.u.planeRoof.poly.nSubPolys = 1;
	element.roof.u.planeRoof.poly.nArcs = polyarcsMsg.arcangle_size();

	memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((element.roof.u.planeRoof.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.roof.u.planeRoof.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((element.roof.u.planeRoof.poly.nArcs) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	if (memo.coords == NULL || memo.pends == NULL || memo.parcs == NULL) {
		ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
		ACAPI_DisposeElemMemoHdls(&memo);
	}

	memo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle((element.roof.u.planeRoof.poly.nCoords + 1) * sizeof(API_EdgeTrim), ALLOCATE_CLEAR, 0));
	memo.sideMaterials = reinterpret_cast<API_MaterialOverrideType*> (BMAllocatePtr((element.roof.u.planeRoof.poly.nCoords + 1) * sizeof(API_MaterialOverrideType), ALLOCATE_CLEAR, 0));

	for (int i = 1; i <= pointsMsg.px_size(); i++){
		(*memo.coords)[i].x = pointsMsg.px(i - 1);
		(*memo.coords)[i].y = pointsMsg.py(i - 1);
	}

	(*memo.pends)[1] = pointsMsg.px_size();

	for (int i = 0; i < polyarcsMsg.arcangle_size(); i++){
		(*memo.parcs)[i].begIndex = polyarcsMsg.begindex(i);
		(*memo.parcs)[i].endIndex = polyarcsMsg.endindex(i);
		(*memo.parcs)[i].arcAngle = polyarcsMsg.arcangle(i);
	}

	/*
	// constructing pivot polygon data
	element.roof.u.polyRoof.pivotPolygon.nCoords = 8;
	//element.roof.u.polyRoof.pivotPolygon.nSubPolys = 2;
	element.roof.u.polyRoof.pivotPolygon.nSubPolys = 0;
	element.roof.u.polyRoof.pivotPolygon.nArcs = 2;

	memo.additionalPolyCoords = reinterpret_cast<API_Coord**> (BMAllocateHandle((element.roof.u.polyRoof.pivotPolygon.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	memo.additionalPolyPends = reinterpret_cast<Int32**> (BMAllocateHandle((element.roof.u.polyRoof.pivotPolygon.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.additionalPolyParcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(element.roof.u.polyRoof.pivotPolygon.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	if (memo.additionalPolyCoords == NULL || memo.additionalPolyPends == NULL || memo.additionalPolyParcs == NULL) {
	ErrorBeep("Not enough memory to create roof pivot polygon data", APIERR_MEMFULL);
	ACAPI_DisposeElemMemoHdls(&memo);
	return;
	}

	(*memo.additionalPolyCoords)[1].x = centerPoint.x + 10.0;
	(*memo.additionalPolyCoords)[1].y = centerPoint.x + 10.0;
	(*memo.additionalPolyCoords)[2].x = centerPoint.x + 10.0;
	(*memo.additionalPolyCoords)[2].y = centerPoint.x - 10.0;
	(*memo.additionalPolyCoords)[3].x = centerPoint.x - 10.0;
	(*memo.additionalPolyCoords)[3].y = centerPoint.x - 10.0;
	(*memo.additionalPolyCoords)[4].x = centerPoint.x - 10.0;
	(*memo.additionalPolyCoords)[4].y = centerPoint.x + 10.0;
	(*memo.additionalPolyCoords)[5] = (*memo.additionalPolyCoords)[1];
	(*memo.additionalPolyPends)[1] = 5;
	(*memo.additionalPolyCoords)[6].x = centerPoint.x + 5.0;
	(*memo.additionalPolyCoords)[6].y = centerPoint.x + 0.0;
	(*memo.additionalPolyCoords)[7].x = centerPoint.x - 5.0;
	(*memo.additionalPolyCoords)[7].y = centerPoint.x + 0.0;
	(*memo.additionalPolyCoords)[8] = (*memo.additionalPolyCoords)[6];
	(*memo.additionalPolyPends)[2] = 8;
	(*memo.additionalPolyParcs)[0].begIndex = 6;							// makes a circle-shaped hole in the pivot polygon
	(*memo.additionalPolyParcs)[0].endIndex = 7;
	(*memo.additionalPolyParcs)[0].arcAngle = PI;
	(*memo.additionalPolyParcs)[1].begIndex = 7;
	(*memo.additionalPolyParcs)[1].endIndex = 8;
	(*memo.additionalPolyParcs)[1].arcAngle = PI;

	// setting eaves overhang and plane levels
	element.roof.u.polyRoof.overHangType = API_OffsetOverhang;		// contourPolygon will be calculated automatically by offsetting the pivot polygon
	element.roof.u.polyRoof.eavesOverHang = 1.0;
	element.roof.u.polyRoof.levelNum = 16;
	for (Int32 i = 0; i < element.roof.u.polyRoof.levelNum; i++) {
	element.roof.u.polyRoof.levelData[i].levelAngle = 5.0 * DEGRAD * (i + 1);
	element.roof.u.polyRoof.levelData[i].levelHeight = 0.05 * (i + 1);
	}
	*/

	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (slab)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		//sprintf(buffer, "No creation");
		//ACAPI_WriteReport(buffer, true);
	}

	sendElementID(getClientSocket(), element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createNewRoof(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	roofmsg			roofMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	short			material;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &roofMsg);
	readDelimitedFrom(getClientSocket(), &pointsMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_RoofID;
	//element.roof.roofClass = API_PolyRoofID;
	element.roof.roofClass = API_PlaneRoofID;
	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		sprintf(buffer, "Default Problem");
		ACAPI_WriteReport(buffer, true);
	}

	element.header.layer = getLayerNumber(roofMsg.layer());

	if (roofMsg.type() == "Basic"){
		material = searchBuildingMaterials(roofMsg.material());
		element.roof.shellBase.modelElemStructureType = API_BasicStructure;
		element.roof.shellBase.buildingMaterial = material;
		if (roofMsg.thickness() > 0){
			element.roof.shellBase.thickness = roofMsg.thickness();
		}
	}
	else if (roofMsg.type() == "Composite"){
		material = searchCompositeMaterials(roofMsg.material());
		element.roof.shellBase.composite = material;
		element.roof.shellBase.modelElemStructureType = API_CompositeStructure;
		if (roofMsg.thickness() != 0.3){
			msgArchiCAD("Composite Material do not support thickness!");
			quit();
		}
	}

	if (material == -1){
		crash = true;
	}

	element.roof.shellBase.level = roofMsg.height();

	element.roof.u.planeRoof.baseLine.c1.x = 0;
	element.roof.u.planeRoof.baseLine.c1.y = 0;
	element.roof.u.planeRoof.baseLine.c2.x = 0;
	element.roof.u.planeRoof.baseLine.c2.y = 0;

	element.roof.u.planeRoof.angle = 0;

	element.header.floorInd = roofMsg.bottomlevel();

	//sprintf(buffer, "libInd: %d", element.object.libInd);
	//ACAPI_WriteReport(buffer, true);

	element.roof.u.planeRoof.poly.nCoords = pointsMsg.px_size();
	element.roof.u.planeRoof.poly.nSubPolys = 1;
	element.roof.u.planeRoof.poly.nArcs = polyarcsMsg.arcangle_size();

	memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((element.roof.u.planeRoof.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.roof.u.planeRoof.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((element.roof.u.planeRoof.poly.nArcs) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	if (memo.coords == NULL || memo.pends == NULL || memo.parcs == NULL) {
		ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
		ACAPI_DisposeElemMemoHdls(&memo);
	}

	memo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle((element.roof.u.planeRoof.poly.nCoords + 1) * sizeof(API_EdgeTrim), ALLOCATE_CLEAR, 0));
	memo.sideMaterials = reinterpret_cast<API_MaterialOverrideType*> (BMAllocatePtr((element.roof.u.planeRoof.poly.nCoords + 1) * sizeof(API_MaterialOverrideType), ALLOCATE_CLEAR, 0));

	for (int i = 1; i <= pointsMsg.px_size(); i++){
		(*memo.coords)[i].x = pointsMsg.px(i - 1);
		(*memo.coords)[i].y = pointsMsg.py(i - 1);
	}

	(*memo.pends)[1] = pointsMsg.px_size();

	for (int i = 0; i < polyarcsMsg.arcangle_size(); i++){
		(*memo.parcs)[i].begIndex = polyarcsMsg.begindex(i);
		(*memo.parcs)[i].endIndex = polyarcsMsg.endindex(i);
		(*memo.parcs)[i].arcAngle = polyarcsMsg.arcangle(i);
	}

	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (slab)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		//sprintf(buffer, "No creation");
		//ACAPI_WriteReport(buffer, true);
	}

	sendElementID(getClientSocket(), element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createPolyRoof(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	roofmsg			roofMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	intlistmsg		subPolyMsg;
	rooflevelsmsg	roofLevelsMsg;
	short			material;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &roofMsg);
	readDelimitedFrom(getClientSocket(), &pointsMsg);
	readDelimitedFrom(getClientSocket(), &polyarcsMsg);
	readDelimitedFrom(getClientSocket(), &subPolyMsg);
	readDelimitedFrom(getClientSocket(), &roofLevelsMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_RoofID;
	element.roof.roofClass = API_PolyRoofID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		sprintf(buffer, "Default Problem");
		ACAPI_WriteReport(buffer, true);
	}

	element.header.layer = getLayerNumber(roofMsg.layer());

	if (roofMsg.type() == "BasicStructure"){
		material = searchBuildingMaterials(roofMsg.material());
		element.roof.shellBase.modelElemStructureType = API_BasicStructure;
		element.roof.shellBase.buildingMaterial = material;
		if (roofMsg.thickness() > 0){
			element.roof.shellBase.thickness = roofMsg.thickness();
		}
	}
	else if (roofMsg.type() == "CompositeStructure"){
		material = searchCompositeMaterials(roofMsg.material());
		element.roof.shellBase.composite = material;
		element.roof.shellBase.modelElemStructureType = API_CompositeStructure;
	}

	if (material == -1){
		crash = true;
	}

	element.roof.shellBase.level = roofMsg.height();

	element.header.floorInd = getCurrentLevel();

	//sprintf(buffer, "libInd: %d", element.object.libInd);
	//ACAPI_WriteReport(buffer, true);

	// constructing pivot polygon data
	element.roof.u.polyRoof.pivotPolygon.nCoords = pointsMsg.px_size();
	element.roof.u.polyRoof.pivotPolygon.nSubPolys = subPolyMsg.ilist_size();
	element.roof.u.polyRoof.pivotPolygon.nArcs = polyarcsMsg.arcangle_size();

	memo.additionalPolyCoords = reinterpret_cast<API_Coord**> (BMAllocateHandle((element.roof.u.polyRoof.pivotPolygon.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	memo.additionalPolyPends = reinterpret_cast<Int32**> (BMAllocateHandle((element.roof.u.polyRoof.pivotPolygon.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.additionalPolyParcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(element.roof.u.polyRoof.pivotPolygon.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	if (memo.additionalPolyCoords == NULL || memo.additionalPolyPends == NULL || memo.additionalPolyParcs == NULL) {
		ErrorBeep("Not enough memory to create roof pivot polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}

	for (int i = 1; i <= pointsMsg.px_size(); i++){
		(*memo.additionalPolyCoords)[i].x = pointsMsg.px(i - 1);
		(*memo.additionalPolyCoords)[i].y = pointsMsg.py(i - 1);
		//sprintf(buffer, "x %f y %f", (*memo.additionalPolyCoords)[i].x, (*memo.additionalPolyCoords)[i].y);
		//ACAPI_WriteReport(buffer, true);
	}

	for (int i = 1; i <= subPolyMsg.ilist_size(); i++){
		if (i > 1){
			(*memo.additionalPolyPends)[i] = (*memo.additionalPolyPends)[i - 1] + subPolyMsg.ilist(i - 1);
		}
		else{
			(*memo.additionalPolyPends)[i] = subPolyMsg.ilist(i - 1);
		}

		//sprintf(buffer, "polypends %d", (*memo.additionalPolyPends)[i]);
		//ACAPI_WriteReport(buffer, true);
	}

	for (int i = 0; i < polyarcsMsg.arcangle_size(); i++){
		(*memo.additionalPolyParcs)[i].begIndex = polyarcsMsg.begindex(i);
		(*memo.additionalPolyParcs)[i].endIndex = polyarcsMsg.endindex(i);
		(*memo.additionalPolyParcs)[i].arcAngle = polyarcsMsg.arcangle(i);
	}
	/*
	(*memo.additionalPolyCoords)[1].x = centerPoint.x + 10.0;
	(*memo.additionalPolyCoords)[1].y = centerPoint.x + 10.0;
	(*memo.additionalPolyCoords)[2].x = centerPoint.x + 10.0;
	(*memo.additionalPolyCoords)[2].y = centerPoint.x - 10.0;
	(*memo.additionalPolyCoords)[3].x = centerPoint.x - 10.0;
	(*memo.additionalPolyCoords)[3].y = centerPoint.x - 10.0;
	(*memo.additionalPolyCoords)[4].x = centerPoint.x - 10.0;
	(*memo.additionalPolyCoords)[4].y = centerPoint.x + 10.0;
	(*memo.additionalPolyCoords)[5] = (*memo.additionalPolyCoords)[1];
	(*memo.additionalPolyPends)[1] = 5;
	(*memo.additionalPolyCoords)[6].x = centerPoint.x + 5.0;
	(*memo.additionalPolyCoords)[6].y = centerPoint.x + 0.0;
	(*memo.additionalPolyCoords)[7].x = centerPoint.x - 5.0;
	(*memo.additionalPolyCoords)[7].y = centerPoint.x + 0.0;
	(*memo.additionalPolyCoords)[8] = (*memo.additionalPolyCoords)[6];
	(*memo.additionalPolyPends)[2] = 8;
	(*memo.additionalPolyParcs)[0].begIndex = 6;							// makes a circle-shaped hole in the pivot polygon
	(*memo.additionalPolyParcs)[0].endIndex = 7;
	(*memo.additionalPolyParcs)[0].arcAngle = PI;
	(*memo.additionalPolyParcs)[1].begIndex = 7;
	(*memo.additionalPolyParcs)[1].endIndex = 8;
	(*memo.additionalPolyParcs)[1].arcAngle = PI;
	*/
	// setting eaves overhang and plane levels

	//TODO - Make this available
	element.roof.u.polyRoof.overHangType = API_OffsetOverhang;		// contourPolygon will be calculated automatically by offsetting the pivot polygon
	element.roof.u.polyRoof.eavesOverHang = 1.0;
	//element.roof.u.polyRoof.levelNum = 16;
	element.roof.u.polyRoof.levelNum = roofLevelsMsg.angle_size();
	for (Int32 i = 0; i < element.roof.u.polyRoof.levelNum; i++) {
		element.roof.u.polyRoof.levelData[i].levelAngle = roofLevelsMsg.angle(i);
		element.roof.u.polyRoof.levelData[i].levelHeight = roofLevelsMsg.height(i);
	}

	/*
	for (Int32 i = 0; i < element.roof.u.polyRoof.levelNum; i++) {
	element.roof.u.polyRoof.levelData[i].levelAngle = 5.0 * DEGRAD * (i + 1);
	element.roof.u.polyRoof.levelData[i].levelHeight = 0.05 * (i + 1);
	}
	*/

	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (slab)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		//sprintf(buffer, "No creation");
		//ACAPI_WriteReport(buffer, true);
	}

	sendElementID(getClientSocket(), element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createHole(){
	API_Element		element, mask;
	API_ElementMemo memo, oldMemo;
	GSErrCode		err;
	holemsg			holeMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &holeMsg);

	pointsMsg = holeMsg.pts();
	polyarcsMsg = holeMsg.arcs();

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&oldMemo, sizeof(API_ElementMemo));

	element.header.guid = APIGuidFromString(holeMsg.guid().c_str());
	/*
	if (ACAPI_Element_Get(&element) == NoError
	&& ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_Polygon) == NoError) {

	ACAPI_ELEMENT_MASK_CLEAR(mask);
	ACAPI_ELEMENT_MASK_SET(mask, API_SlabType, poly);

	int oldSize = element.slab.poly.nCoords;
	int oldPends = element.slab.poly.nSubPolys;
	int oldArcSize = element.slab.poly.nArcs;

	element.slab.poly.nCoords += pointsMsg.px_size();
	element.slab.poly.nSubPolys++;
	element.slab.poly.nArcs += polyarcsMsg.arcangle_size();


	oldMemo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((element.slab.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	oldMemo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.slab.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	oldMemo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((element.slab.poly.nArcs) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	//Keep the old information
	for (int i = 1; i <= oldSize; i++){
	(*oldMemo.coords)[i].x = (*memo.coords)[i].x;
	(*oldMemo.coords)[i].y = (*memo.coords)[i].y;
	}

	(*oldMemo.pends)[0] = 0;
	for (int i = 1; i <= oldPends; i++){
	(*oldMemo.pends)[i] = (*memo.pends)[i];
	}

	for (int i = 0; i < oldArcSize; i++){
	(*oldMemo.parcs)[i].begIndex = (*memo.parcs)[i].begIndex;
	(*oldMemo.parcs)[i].endIndex = (*memo.parcs)[i].endIndex;
	(*oldMemo.parcs)[i].arcAngle = (*memo.parcs)[i].arcAngle;
	}

	memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((element.slab.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.slab.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((element.slab.poly.nArcs) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	if (memo.coords == NULL || memo.pends == NULL || memo.parcs == NULL) {
	ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
	sprintf(buffer, "Memory Problem");
	ACAPI_WriteReport(buffer, true);
	ACAPI_DisposeElemMemoHdls(&memo);
	}

	memo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle((element.slab.poly.nCoords + 1) * sizeof(API_EdgeTrim), ALLOCATE_CLEAR, 0));
	memo.sideMaterials = reinterpret_cast<API_MaterialOverrideType*> (BMAllocatePtr((element.slab.poly.nCoords + 1) * sizeof(API_MaterialOverrideType), ALLOCATE_CLEAR, 0));
	//Populate memo with old information
	//Keep the old information
	for (int i = 1; i <= oldSize; i++){
	(*memo.coords)[i].x = (*oldMemo.coords)[i].x;
	(*memo.coords)[i].y = (*oldMemo.coords)[i].y;
	}

	(*memo.pends)[0] = 0;
	for (int i = 1; i <= oldPends; i++){
	(*memo.pends)[i] = (*oldMemo.pends)[i];
	}

	for (int i = 0; i < oldArcSize; i++){
	(*memo.parcs)[i].begIndex = (*oldMemo.parcs)[i].begIndex;
	(*memo.parcs)[i].endIndex = (*oldMemo.parcs)[i].endIndex;
	(*memo.parcs)[i].arcAngle = (*oldMemo.parcs)[i].arcAngle;
	}
	//////////////////////////////////////////////////

	//Populate memo with new information, hole
	int auxIndex = 0;
	for (int i = oldSize + 1; i <= element.slab.poly.nCoords; i++){

	(*memo.coords)[i].x = pointsMsg.px(auxIndex);
	(*memo.coords)[i].y = pointsMsg.py(auxIndex);
	auxIndex++;
	}

	(*memo.pends)[element.slab.poly.nSubPolys] = element.slab.poly.nCoords;

	for (int i = 0; i <= element.slab.poly.nSubPolys; i++){
	sprintf(buffer, "pends %d", (*memo.pends)[i]);
	ACAPI_WriteReport(buffer, true);
	}

	auxIndex = 0;
	for (int i = oldArcSize; i < polyarcsMsg.arcangle_size(); i++){
	(*memo.parcs)[i].begIndex = polyarcsMsg.begindex(auxIndex);
	(*memo.parcs)[i].endIndex = polyarcsMsg.endindex(auxIndex);
	(*memo.parcs)[i].arcAngle = polyarcsMsg.arcangle(auxIndex);
	auxIndex++;
	}
	//////////////////////////////////////////////////

	err = ACAPI_Element_Change(&element, &mask, &memo, APIMemoMask_All, true);
	//err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
	ErrorBeep("ACAPI_Element_Create (slab)", err);
	sprintf(buffer, ErrID_To_Name(err));
	ACAPI_WriteReport(buffer, true);
	}

	}
	else{
	sprintf(buffer, "Error on retreiving slab");
	ACAPI_WriteReport(buffer, true);
	}
	*/

	

	if (ACAPI_Element_Get(&element) == NoError
		&& ACAPI_Element_GetMemo(element.header.guid, &oldMemo, APIMemoMask_Polygon) == NoError) {

		int oldSize;
		int oldPends;
		int oldArcSize;
		int* newSize;
		int* newPends;
		int* newArcSize;

		ACAPI_ELEMENT_MASK_CLEAR(mask);

		switch (holeMsg.type()){
		case 0:
			ACAPI_ELEMENT_MASK_SET(mask, API_SlabType, poly);
			oldSize = element.slab.poly.nCoords;
			oldPends = element.slab.poly.nSubPolys;
			oldArcSize = element.slab.poly.nArcs;
			newSize = &(element.slab.poly.nCoords);
			newPends = &(element.slab.poly.nSubPolys);
			newArcSize = &(element.slab.poly.nArcs);
			break;
		case 1:
			ACAPI_ELEMENT_MASK_SET(mask, API_RoofType, u);
			oldSize = element.roof.u.planeRoof.poly.nCoords;
			oldPends = element.roof.u.planeRoof.poly.nSubPolys;
			oldArcSize = element.roof.u.planeRoof.poly.nArcs;
			newSize = &(element.roof.u.planeRoof.poly.nCoords);
			newPends = &(element.roof.u.planeRoof.poly.nSubPolys);
			newArcSize = &(element.roof.u.planeRoof.poly.nArcs);
		}
		
		holeMemo(oldSize, oldPends, oldArcSize, pointsMsg, polyarcsMsg, oldMemo, &memo, newSize, newPends, newArcSize);
		


		/*
		element.slab.poly.nCoords += pointsMsg.px_size();
		element.slab.poly.nSubPolys++;
		element.slab.poly.nArcs += polyarcsMsg.arcangle_size();

		memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((element.slab.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
		memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.slab.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
		memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((element.slab.poly.nArcs) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

		if (memo.coords == NULL || memo.pends == NULL || memo.parcs == NULL) {
			ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
			sprintf(buffer, "Memory Problem");
			ACAPI_WriteReport(buffer, true);
			ACAPI_DisposeElemMemoHdls(&memo);
			quit();
			return;
		}

		memo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle((element.slab.poly.nCoords + 1) * sizeof(API_EdgeTrim), ALLOCATE_CLEAR, 0));
		memo.sideMaterials = reinterpret_cast<API_MaterialOverrideType*> (BMAllocatePtr((element.slab.poly.nCoords + 1) * sizeof(API_MaterialOverrideType), ALLOCATE_CLEAR, 0));

		//Populate memo with old information
		for (int i = 1; i <= oldSize; i++){
			(*memo.coords)[i].x = (*oldMemo.coords)[i].x;
			(*memo.coords)[i].y = (*oldMemo.coords)[i].y;
		}

		(*memo.pends)[0] = 0;
		for (int i = 1; i <= oldPends; i++){
			(*memo.pends)[i] = (*oldMemo.pends)[i];
		}

		for (int i = 0; i < oldArcSize; i++){
			(*memo.parcs)[i].begIndex = (*oldMemo.parcs)[i].begIndex;
			(*memo.parcs)[i].endIndex = (*oldMemo.parcs)[i].endIndex;
			(*memo.parcs)[i].arcAngle = (*oldMemo.parcs)[i].arcAngle;
		}
		//////////////////////////////////////////////////

		//Populate memo with new information, hole
		int auxIndex = 0;
		for (int i = oldSize + 1; i <= element.slab.poly.nCoords; i++){
			(*memo.coords)[i].x = pointsMsg.px(auxIndex);
			(*memo.coords)[i].y = pointsMsg.py(auxIndex);
			auxIndex++;
		}

		(*memo.pends)[element.slab.poly.nSubPolys] = element.slab.poly.nCoords;
		auxIndex = 0;
		for (int i = oldArcSize; i < element.slab.poly.nArcs; i++){
			(*memo.parcs)[i].begIndex = oldSize + polyarcsMsg.begindex(auxIndex);
			(*memo.parcs)[i].endIndex = oldSize + polyarcsMsg.endindex(auxIndex);
			(*memo.parcs)[i].arcAngle = polyarcsMsg.arcangle(auxIndex);
			auxIndex++;
		}
		//////////////////////////////////////////////////
		*/
		/*
		//err = ACAPI_Element_Change(&element, &mask, &memo, APIMemoMask_Polygon, true);
		err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
		if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (slab)", err);
		sprintf(buffer, "element");
		ACAPI_WriteReport(buffer, true);

		}
		err = ACAPI_Element_ChangeMemo(element.header.guid, APIMemoMask_All, &memo);
		*/

		err = ACAPI_Element_Create(&element, &memo);

		if (hasError(err)){
			quit();
			return;
		}

		sendElementID(getClientSocket(), element);

		element.header.guid = APIGuidFromString(holeMsg.guid().c_str());
		ACAPI_Element_Get(&element);
		API_Elem_Head* toDelete;
		toDelete = &element.header;
		ACAPI_Element_Delete(&toDelete, 1);
	}
	else{
		sprintf(buffer, "Error on retreiving slab");
		ACAPI_WriteReport(buffer, true);
		sendElementID(getClientSocket(), element);

	}

	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&oldMemo);

}

void createHoleTest(){
	API_Element			element;
	API_ElementMemo		memo, subMemo;
	GSErrCode			err;
	holemsg			holeMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &holeMsg);
	readDelimitedFrom(getClientSocket(), &pointsMsg);
	readDelimitedFrom(getClientSocket(), &polyarcsMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&subMemo, sizeof(API_ElementMemo));

	element.header.guid = APIGuidFromString(holeMsg.guid().c_str());

	if (ACAPI_Element_Get(&element) == NoError
		&& ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_Polygon) == NoError) {

		subMemo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((pointsMsg.px_size() + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
		subMemo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((1 + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
		subMemo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((polyarcsMsg.arcangle_size()) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

		if (subMemo.coords == NULL || subMemo.pends == NULL || subMemo.parcs == NULL) {
			ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
			sprintf(buffer, "Memory Problem");
			ACAPI_WriteReport(buffer, true);
			ACAPI_DisposeElemMemoHdls(&memo);
		}

		for (int i = 1; i <= pointsMsg.px_size(); i++){
			(*subMemo.coords)[i].x = pointsMsg.px(i - 1);
			(*subMemo.coords)[i].y = pointsMsg.py(i - 1);
		}

		(*subMemo.pends)[1] = pointsMsg.px_size();

		for (int i = 0; i < polyarcsMsg.arcangle_size(); i++){
			(*subMemo.parcs)[i].begIndex = polyarcsMsg.begindex(i);
			(*subMemo.parcs)[i].endIndex = polyarcsMsg.endindex(i);
			(*subMemo.parcs)[i].arcAngle = polyarcsMsg.arcangle(i);
		}

		sprintf(buffer, "x: %f y: %f", (*memo.coords)[1].x, (*memo.coords)[1].y);
		ACAPI_WriteReport(buffer, true);

		err = ACAPI_Goodies(APIAny_InsertSubPolyID, &memo, &subMemo);


		//err = ACAPI_Element_ChangeMemo(element.header.guid, APIMemoMask_Polygon, &memo);
		err = ACAPI_Element_Create(&element, &memo);
		if (err != NoError){
			ErrorBeep("ACAPI_Element_Create (slab)", err);
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}

		sendElementID(getClientSocket(), element);
	}
}

void createStairs(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	stairsmsg		stairsMsg;
	char buffer[256];
	bool crash = false;

	readDelimitedFrom(getClientSocket(), &stairsMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.variationID = APIVarId_SymbStair;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		sprintf(buffer, "Default Problem");
		ACAPI_WriteReport(buffer, true);
	}

	element.header.layer = getLayerNumber(stairsMsg.layer());

	element.header.floorInd = stairsMsg.bottomindex();

	element.object.pos.x = stairsMsg.posx();
	element.object.pos.y = stairsMsg.posy();
	element.object.useXYFixSize = stairsMsg.usexyfixsize();
	element.object.useObjSectAttrs = true;
	element.object.xRatio = stairsMsg.xratio();
	element.object.yRatio = stairsMsg.yratio();
	element.object.level = stairsMsg.bottom();
	element.object.angle = stairsMsg.angle();
	element.object.libInd = searchObjects(stairsMsg.name());

	if (element.object.libInd == -1){
		crash = true;
	}
	else{
		API_ParamOwnerType   paramOwner;
		API_GetParamsType    getParams;
		BNZeroMemory(&paramOwner, sizeof(API_ParamOwnerType));
		paramOwner.libInd = element.object.libInd;

		BNZeroMemory(&getParams, sizeof(API_GetParamsType));

		err = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner, NULL);
		if (err == NoError) {
			err = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, NULL);
			if (err == NoError) {
				handleParams(stairsMsg.params(), &paramOwner, &getParams);
				memo.params = getParams.params;
			}
		}

		int numParams = BMhGetSize(reinterpret_cast<GSHandle> (getParams.params)) / Sizeof32(API_AddParType) - 1;
		for (int j = 0; j < numParams; j++){
			if ((std::string)(*getParams.params)[j].name == "zzyzx"){
				if (stairsMsg.height() != 0){
					(*getParams.params)[j].value.real = stairsMsg.height();
				} else{
					API_StoryInfo	storyInfo;
					err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
					//(*getParams.params)[j].value.real = (*storyInfo.data)[stairsMsg.upperindex()].level - (*storyInfo.data)[stairsMsg.bottomindex()].level - stairsMsg.bottom();
					(*getParams.params)[j].value.real = (*storyInfo.data)[stairsMsg.upperindex()].level - (*storyInfo.data)[stairsMsg.bottomindex()].level;
				}
			}
		}
		
		err = ACAPI_Element_Create(&element, &memo);
		if (err != NoError){
			ErrorBeep("Object Create", err);
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}
		ACAPI_Goodies(APIAny_CloseParametersID, NULL, NULL);
		ACAPI_DisposeAddParHdl(&getParams.params);
	}

	sendElementID(getClientSocket(), element, crash);
	//required to force update on width/height
	elementRefresh(&element);

	/*
	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
	ErrorBeep("ACAPI_Element_Create (slab)", err);
	sprintf(buffer, ErrID_To_Name(err));
	ACAPI_WriteReport(buffer, true);
	}

	sendElementID(element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);
	*/
}

void createMesh(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	meshmessage		meshMsg;
	pointsmessage	pointsMsg;
	pointsmessage	linesMsg;
	short			material;
	bool crash = false;
	char buffer[256];

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	readDelimitedFrom(getClientSocket(), &meshMsg);
	readDelimitedFrom(getClientSocket(), &pointsMsg);
	readDelimitedFrom(getClientSocket(), &linesMsg);

	element.header.typeID = API_MeshID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		sprintf(buffer, "Default Problem");
		ACAPI_WriteReport(buffer, true);
	}

	element.header.layer = getLayerNumber(meshMsg.layer());

	element.header.floorInd = meshMsg.bottomlevel();

	element.mesh.level = meshMsg.level();

	material = searchBuildingMaterials(meshMsg.material());

	if (material == -1){
		crash = true;
	}
	else{
		element.mesh.buildingMaterial = material;
	}

	if (meshMsg.has_overridematerial()){
		short overrideMaterial = searchOverrideMaterials(meshMsg.overridematerial());
		if (material == -1){
			crash = true;
		}
		else{
			element.mesh.topMat.material = overrideMaterial;
			element.mesh.topMat.overrideMaterial = true;
		}
	}
	else{
		element.mesh.topMat.overrideMaterial = false;
	}

	element.mesh.poly.nCoords = pointsMsg.px_size();
	element.mesh.poly.nSubPolys = 1;
	element.mesh.poly.nArcs = 0;

	memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((element.mesh.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.mesh.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((element.mesh.poly.nArcs) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	if (memo.coords == NULL || memo.pends == NULL || memo.parcs == NULL) {
		ErrorBeep("Not enough memory to create mesh polygon data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
		ACAPI_DisposeElemMemoHdls(&memo);
	}

	memo.meshPolyZ = reinterpret_cast<double**> (BMAllocateHandle((element.mesh.poly.nCoords + 1) * sizeof(double), ALLOCATE_CLEAR, 0));
	if (memo.meshPolyZ == NULL) {
		ErrorBeep("Not enough memory to create mesh vertex data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}

	memo.edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle((element.mesh.poly.nCoords + 1) * sizeof(API_EdgeTrim), ALLOCATE_CLEAR, 0));
	memo.sideMaterials = reinterpret_cast<API_MaterialOverrideType*> (BMAllocatePtr((element.mesh.poly.nCoords + 1) * sizeof(API_MaterialOverrideType), ALLOCATE_CLEAR, 0));

	for (int i = 1; i <= pointsMsg.px_size(); i++){
		(*memo.coords)[i].x = pointsMsg.px(i - 1);
		(*memo.coords)[i].y = pointsMsg.py(i - 1);
		(*memo.meshPolyZ)[i] = pointsMsg.pz(i - 1);
	}

	(*memo.pends)[1] = pointsMsg.px_size();


	//element.mesh.levelLines.nCoords = 1;
	element.mesh.levelLines.nCoords = linesMsg.px_size();
	element.mesh.levelLines.nSubLines = 1;

	memo.meshLevelCoords = reinterpret_cast<API_MeshLevelCoord**> (BMAllocateHandle((element.mesh.levelLines.nCoords) * sizeof(API_MeshLevelCoord), ALLOCATE_CLEAR, 0));
	memo.meshLevelEnds = reinterpret_cast<Int32**> (BMAllocateHandle((element.mesh.levelLines.nSubLines) * sizeof(Int32), ALLOCATE_CLEAR, 0));

	if (memo.meshLevelCoords == NULL || memo.meshLevelEnds == NULL) {
		ErrorBeep("Not enough memory to create mesh leveLines data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
		ACAPI_DisposeElemMemoHdls(&memo);
	}

	for (int i = 0; i < linesMsg.px_size(); i++){
		(*memo.meshLevelCoords)[i].vertexID = i + 1;
		(*memo.meshLevelCoords)[i].c.x = linesMsg.px(i);
		(*memo.meshLevelCoords)[i].c.y = linesMsg.py(i);
		(*memo.meshLevelCoords)[i].c.z = linesMsg.pz(i);
	}
	(*memo.meshLevelEnds)[0] = linesMsg.px_size();

	/*
	(*memo.meshLevelCoords)[0].vertexID = 1;
	(*memo.meshLevelCoords)[0].c.x = 5.0;
	(*memo.meshLevelCoords)[0].c.y = 5.0;
	(*memo.meshLevelCoords)[0].c.z = 20.0;
	(*memo.meshLevelEnds)[0] = 1;
	*/

	/*
	API_Coord** coords;
	err = ACAPI_Goodies(APIAny_TriangulatePolyID, &memo, &coords);
	if (err != NoError){
	ErrorBeep("ACAPI_Element_Create (mesh)", err);
	sprintf(buffer, ErrID_To_Name(err));
	ACAPI_WriteReport(buffer, true);
	sprintf(buffer, "Triangulate");
	ACAPI_WriteReport(buffer, true);
	}
	else{
	Int32 nTriangles = BMGetHandleSize((GSHandle)coords) / (3 * sizeof(API_Coord));
	Int32 nPoints = BMGetHandleSize((GSHandle)coords) / (sizeof(API_Coord));
	element.mesh.poly.nCoords = nPoints + 1;
	sprintf(buffer, "nPoints: %d", nPoints);
	ACAPI_WriteReport(buffer, true);
	BMKillHandle((GSHandle *)&memo.coords);
	memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((element.mesh.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	for (int i = 1; i <= nPoints; i++){
	(*memo.coords)[i].x = (*coords)[i-1].x;
	(*memo.coords)[i].y = (*coords)[i-1].y;
	sprintf(buffer, "x: %f y: %f", (*coords)[i-1].x, (*coords)[i-1].y);
	ACAPI_WriteReport(buffer, true);
	}
	(*memo.coords)[nPoints + 1].x = (*memo.coords)[1].x;
	(*memo.coords)[nPoints + 1].y = (*memo.coords)[1].y;

	(*memo.pends)[1] = nPoints + 1;

	//BMKillHandle((GSHandle *)&coords);
	}
	*/
	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (mesh)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		//sprintf(buffer, "No creation");
		//ACAPI_WriteReport(buffer, true);
	}
	sendElementID(getClientSocket(), element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void translateElement(){
	API_Element		element;
	API_EditPars	editPars;
	GSErrCode		err = NoError;
	translatemsg	translateMsg;
	API_Neig		**items;
	API_Neig		*auxItems;
	API_ElemTypeID  elemTypeID;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &translateMsg);

	items = (API_Neig **)BMAllocateHandle(0, ALLOCATE_CLEAR, 0);

	items = (API_Neig **)BMReallocHandle((GSHandle)items, 2 * sizeof(API_Neig), 0, 0);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&editPars, sizeof(API_EditPars));

	element.header.guid = APIGuidFromString(translateMsg.guid().c_str());
	element.header.typeID = API_CurtainWallID;


	editPars.typeID = APIEdit_Drag;
	editPars.withDelete = false;
	editPars.begC.x = 0.0;
	editPars.begC.y = 0.0;
	editPars.begC.z = 0.0;

	editPars.endC.x = 10.0;
	editPars.endC.y = 10.0;
	editPars.endC.z = 10.0;


	//ElemHead_To_Neig(*items, &element.header);

	BNZeroMemory((*items), sizeof(API_Neig));

	(*items)[0].neigID = APINeig_CurtainWall;			
	(*items)[0].inIndex = 1;

	//ACAPI_Goodies(APIAny_ElemTypeToNeigID, &element.header.typeID, (*items));

	(*items)[0].guid = element.header.guid;

	//err = ACAPI_Element_Edit(items, 1, &editPars);
	err = ACAPI_Element_Edit(items, 1, &editPars);

	if (err != NoError){
		ErrorBeep("ACAPI_Element_Edit (drag)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	BMKillHandle((GSHandle *)&items);

}

void rotateElementZ(){
	API_Element		element, mask;
	API_ElementMemo memo;
	GSErrCode		err;
	rotatemsg		rotMsg;
	elementidlist	elementIDList;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &rotMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	API_DatabaseInfo dbInfo;
	API_DatabaseInfo floorInfo;
	floorInfo.typeID = APIWind_FloorPlanID;
	floorInfo.index = 0;
	ACAPI_Database(APIDb_GetCurrentDatabaseID, &dbInfo);
	ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &floorInfo);

	for (int i = 0; i < rotMsg.guid_size(); i++){

		element.header.guid = APIGuidFromString(rotMsg.guid(i).c_str());

		if (ACAPI_Element_Get(&element) == NoError
			&& ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_Polygon) == NoError) {

			double theta = rotMsg.angle();
			double xx, yy;
			if (element.header.typeID == API_WallID){
				xx = element.wall.begC.x;
				yy = element.wall.begC.y;

				element.wall.begC.x = xx * cos(theta) - yy * sin(theta);
				element.wall.begC.y = xx * sin(theta) + yy * cos(theta);

				xx = element.wall.endC.x;
				yy = element.wall.endC.y;

				element.wall.endC.x = xx * cos(theta) - yy * sin(theta);
				element.wall.endC.y = xx * sin(theta) + yy * cos(theta);

				ACAPI_ELEMENT_MASK_CLEAR(mask);
				ACAPI_ELEMENT_MASK_SET(mask, API_WallType, begC);
				ACAPI_ELEMENT_MASK_SET(mask, API_WallType, endC);

				if (rotMsg.copy()){
					err = ACAPI_Element_Create(&element, &memo);
				}
				else{
					err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
				}

				if (err != NoError){
					ErrorBeep("ACAPI_Element_Create (slab)", err);
					sprintf(buffer, ErrID_To_Name(err));
					ACAPI_WriteReport(buffer, true);
				}
			}
			else if (element.header.typeID == API_ColumnID){
				xx = element.column.origoPos.x;
				yy = element.column.origoPos.y;

				element.column.origoPos.x = xx * cos(theta) - yy * sin(theta);
				element.column.origoPos.y = xx * sin(theta) + yy * cos(theta);

				ACAPI_ELEMENT_MASK_CLEAR(mask);
				ACAPI_ELEMENT_MASK_SET(mask, API_ColumnType, origoPos);

				if (rotMsg.copy()){
					err = ACAPI_Element_Create(&element, &memo);
				}
				else{
					err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
				}

				if (err != NoError){
					ErrorBeep("ACAPI_Element_Create (slab)", err);
					sprintf(buffer, ErrID_To_Name(err));
					ACAPI_WriteReport(buffer, true);
				}
			}
			else{

				//x' = x * cos(theta) - y * sin(theta)
				//y' = x * sin(theta) + y * cos(theta)

				int pIndex = 1;
				int numPoints = BMhGetSize(reinterpret_cast<GSHandle> (memo.coords)) / Sizeof32(API_Coord) - 1;

				//Debug
				//sprintf(buffer, "numPoints: %d ", numPoints);
				//ACAPI_WriteReport(buffer, true);



				for (int i = 0; i < numPoints; i++){

					xx = (*memo.coords)[pIndex].x;
					yy = (*memo.coords)[pIndex].y;

					(*memo.coords)[pIndex].x = xx * cos(theta) - yy * sin(theta);
					(*memo.coords)[pIndex].y = xx * sin(theta) + yy * cos(theta);

					pIndex++;
				}

				/*
				//Debug
				sprintf(buffer, "pIndex: %d", pIndex);
				ACAPI_WriteReport(buffer, true);

				sprintf(buffer, "x: %f y: %f ", (*memo.coords)[1].x, (*memo.coords)[1].y);
				ACAPI_WriteReport(buffer, true);

				sprintf(buffer, "x: %f y: %f ", (*memo.coords)[pIndex].x, (*memo.coords)[pIndex].y);
				ACAPI_WriteReport(buffer, true);
				*/

				if (rotMsg.copy()){
					err = ACAPI_Element_Create(&element, &memo);
				}
				else{
					err = ACAPI_Element_ChangeMemo(element.header.guid, APIMemoMask_All, &memo);
				}

				if (err != NoError){
					ErrorBeep("ACAPI_Element_Create (slab)", err);
					sprintf(buffer, ErrID_To_Name(err));
					ACAPI_WriteReport(buffer, true);
				}

			}
			char s[64];
			APIGuid2GSGuid(element.header.guid).ConvertToString(s);
			elementIDList.add_guid(s);
		}
	}

	ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &dbInfo);
	ACAPI_DisposeElemMemoHdls(&memo);
	elementIDList.set_crashmaterial(false);
	writeDelimitedTo(getClientSocket(), elementIDList);
}

//ONLY WORKS WITH SHELLS AND ROOFS - NO SLABS 
void trimElement(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	trimmsg			trimMsg;
	GS::Array<API_Guid> guidsToTrim;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &trimMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	for (int i = 0; i < trimMsg.guids_size(); i++){
		guidsToTrim.Push(APIGuidFromString(trimMsg.guids(i).c_str()));
	}
	for (int i = 0; i < trimMsg.guids2_size(); i++){
		guidsToTrim.Push(APIGuidFromString(trimMsg.guids2(i).c_str()));
	}

	err = ACAPI_Element_Trim_Elements(guidsToTrim);
	//err = ACAPI_Element_Merge_Elements(guidsToTrim);
	
	ACAPI_DisposeElemMemoHdls(&memo);
	if (hasError(err)){
		quit(); 
		return;
	}
}

/*
** In:
*	polygon: list of API_Coord that defines polygon
*	nPoints: size of that list
*	beg: beginning coord for intersection
*	end: ending coord for intersection
** Out:
*	intersections: list of intersections
*/
bool debug = false;

void intersections(API_Coord* polygon, int nPoints, API_Coord beg, API_Coord end, std::list<API_Coord>* intersections){
	int numPoints = nPoints;
	double x0, x1, x2, x3;
	double y0, y1, y2, y3;
	double u, v;
	double denominator;
	int pIndex = 1;
	bool repeatedIntersection = false;

	char buffer[256];

	API_Coord intrsct;

	x0 = beg.x;
	y0 = beg.y;
	x1 = end.x;
	y1 = end.y;

	for (int i = 0; i < numPoints - 1; i++){
		x2 = polygon[pIndex].x;
		y2 = polygon[pIndex].y;
		x3 = polygon[pIndex + 1].x;
		y3 = polygon[pIndex + 1].y;

		denominator = ((y3 - y2) * (x1 - x0)) - ((x3 - x2) * (y1 - y0));

		//Check if lines aren't parallel
		if (denominator != 0){
			u = (((x3 - x2) * (y0 - y2)) - ((y3 - y2) * (x0 - x2))) / denominator;
			v = (((x1 - x0) * (y0 - y2)) - ((y1 - y0) * (x0 - x2))) / denominator;
			if (u >= 0 && u <= 1
				&& v >= 0 && v <= 1){

				intrsct.x = x0 + u*(x1 - x0);
				intrsct.y = y0 + u*(y1 - y0);

				//intrsct.x = x2 + v*(x3 - x2);
				//intrsct.y = y2 + v*(y3 - y2);
				if (debug){
					sprintf(buffer, "Start");
					ACAPI_WriteReport(buffer, true);
				}
				double epslon = 0.00001;
				for (std::list<API_Coord>::iterator point = intersections->begin(); point != intersections->end(); point++){
					double px = point->x;
					double py = point->y;
					double diffBetweenX = abs(intrsct.x - point->x);
					double diffBetweenY = abs(intrsct.y - point->y);

					if (debug){
						sprintf(buffer, "ix %f iy %f px %f py %f", intrsct.x, intrsct.y, px, py);
						ACAPI_WriteReport(buffer, true);
					}
					//if ( !(diffBetweenX <= epslon && diffBetweenY <= epslon) ){
					if ((px == intrsct.x && py == intrsct.y) || (diffBetweenX <= epslon && diffBetweenY <= epslon)){
						if (debug){
							sprintf(buffer, "Checked");
							ACAPI_WriteReport(buffer, true);
						}
						repeatedIntersection = true;
						break;
					}
				}
				if (repeatedIntersection){
					repeatedIntersection = false;
				}
				else{
					intersections->push_back(intrsct);
				}



			}

		}

		pIndex++;
	}
}

//TODO CHANGE AND NOT DELETE - DIFFERENT GUIDS
void intersectWall(){
	API_Element		element, mask, elementToDelete;
	API_Element		elementToIntersect;
	//API_ElementMemo memo;
	API_ElementMemo memo, memoToIntersect;
	GSErrCode		err;
	intersectmsg	intersectMsg;
	API_Elem_Head*  toDelete;
	char buffer[256];

	API_Guid emptyGuid;

	readDelimitedFrom(getClientSocket(), &intersectMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&elementToIntersect, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&memoToIntersect, sizeof(API_ElementMemo));

	emptyGuid = element.header.guid;

	element.header.guid = APIGuidFromString(intersectMsg.guid1().c_str());
	elementToIntersect.header.guid = APIGuidFromString(intersectMsg.guid2().c_str());

	if (ACAPI_Element_Get(&element) == NoError
		&& ACAPI_Element_Get(&elementToIntersect) == NoError
		&& ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_Polygon) == NoError
		&& ACAPI_Element_GetMemo(elementToIntersect.header.guid, &memoToIntersect, APIMemoMask_Polygon) == NoError){

		layermsg layerMsg;
		layerMsg.set_name(searchLayers(element.header.layer));
		bool hidden = hiddenLayer(layerMsg);
		if (hidden){
			controlLayer(layerMsg, true);
		}

		int numPoints = BMhGetSize(reinterpret_cast<GSHandle> (memoToIntersect.coords)) / Sizeof32(API_Coord) - 1;
		int pIndex = 1;

		double xmax = -100000;

		bool begInside, endInside;

		for (int i = 0; i < numPoints; i++){
			if (xmax < (*memoToIntersect.coords)[pIndex].x){
				xmax = (*memoToIntersect.coords)[pIndex].x;
			}
			pIndex++;
		}
		pIndex = 1;

		std::list<API_Coord> iPoints;
		API_Coord q;
		q.x = xmax + 10;
		q.y = element.wall.begC.y;
		intersections((*memoToIntersect.coords), numPoints, element.wall.begC, q, &iPoints);

		//check if odd number of intersections
		if (iPoints.size() % 2 == 1){
			begInside = true;
		}
		else {
			begInside = false;
		}
		iPoints.clear();

		q.y = element.wall.endC.y;
		intersections((*memoToIntersect.coords), numPoints, element.wall.endC, q, &iPoints);
		//check if odd number of intersections
		if (iPoints.size() % 2 == 1){
			endInside = true;
		}
		else {
			endInside = false;
		}
		//for (std::list<API_Coord>::iterator point = iPoints.begin(); point != iPoints.end(); point++){
		//sprintf(buffer, "ix %f iy %f", point->x, point->y);
		//ACAPI_WriteReport(buffer, true);
		//}
		iPoints.clear();

		intersections((*memoToIntersect.coords), numPoints, element.wall.begC, element.wall.endC, &iPoints);

		if (iPoints.size() == 0 && !begInside && !endInside){
			API_Elem_Head* test;

			test = &element.header;
			ACAPI_Element_Delete(&test, 1);

			sendElementID(getClientSocket(), elementToIntersect);
		}
		else{
			double distBeg, distEnd;
			for (std::list<API_Coord>::iterator point = iPoints.begin(); point != iPoints.end(); point++){
				//sprintf(buffer, "ix %f iy %f", point->x, point->y);
				//ACAPI_WriteReport(buffer, true);

				distBeg = pow(pow(point->x - element.wall.begC.x, 2) + pow(point->y - element.wall.begC.y, 2), 0.5);
				distEnd = pow(pow(point->x - element.wall.endC.x, 2) + pow(point->y - element.wall.endC.y, 2), 0.5);

				if (!begInside && (distBeg <= distEnd || endInside)){
					element.wall.begC.x = point->x;
					element.wall.begC.y = point->y;
					begInside = true;
				}
				else{
					if (!endInside && (distEnd <= distBeg || begInside)){
						element.wall.endC.x = point->x;
						element.wall.endC.y = point->y;
						endInside = true;
					}
				}
			}

			ACAPI_ELEMENT_MASK_CLEAR(mask);
			ACAPI_ELEMENT_MASK_SET(mask, API_WallType, begC);
			ACAPI_ELEMENT_MASK_SET(mask, API_WallType, endC);

			
			toDelete = &element.header;
			ACAPI_Element_Delete(&toDelete, 1);
			
			element.header.guid = emptyGuid;
			//err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
			err = ACAPI_Element_Create(&element, &memo);
			if (err != NoError){
				ErrorBeep("ACAPI_Element_Create (slab)", err);
				sprintf(buffer, ErrID_To_Name(err));
				ACAPI_WriteReport(buffer, true);
			}
			sendElementID(getClientSocket(), element);
		}
		if (hidden){
			controlLayer(layerMsg, false);
		}

		ACAPI_DisposeElemMemoHdls(&memoToIntersect);
	}
}

void destructiveIntersectWall(){
	API_Element		element, mask;
	API_Element		elementToIntersect;
	//API_ElementMemo memo;
	API_ElementMemo memoToIntersect;
	GSErrCode		err;
	intersectmsg	intersectMsg;
	API_Elem_Head*  test;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &intersectMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&elementToIntersect, sizeof(API_Element));
	//BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&memoToIntersect, sizeof(API_ElementMemo));

	element.header.guid = APIGuidFromString(intersectMsg.guid1().c_str());
	elementToIntersect.header.guid = APIGuidFromString(intersectMsg.guid2().c_str());

	if (ACAPI_Element_Get(&element) == NoError
		&& ACAPI_Element_Get(&elementToIntersect) == NoError
		//&& ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_Polygon) == NoError
		&& ACAPI_Element_GetMemo(elementToIntersect.header.guid, &memoToIntersect, APIMemoMask_Polygon) == NoError){

		int numPoints = BMhGetSize(reinterpret_cast<GSHandle> (memoToIntersect.coords)) / Sizeof32(API_Coord) - 1;
		int pIndex = 1;

		double xmax = -100000;

		bool begInside, endInside;

		for (int i = 0; i < numPoints; i++){
			if (xmax < (*memoToIntersect.coords)[pIndex].x){
				xmax = (*memoToIntersect.coords)[pIndex].x;
			}
			pIndex++;
		}
		pIndex = 1;

		std::list<API_Coord> iPoints;
		API_Coord q;
		q.x = xmax + 10;
		q.y = element.wall.begC.y;
		intersections((*memoToIntersect.coords), numPoints, element.wall.begC, q, &iPoints);

		//check if odd number of intersections
		if (iPoints.size() % 2){
			begInside = true;
		}
		else {
			begInside = false;
		}
		iPoints.clear();

		q.y = element.wall.endC.y;
		intersections((*memoToIntersect.coords), numPoints, element.wall.endC, q, &iPoints);
		//check if odd number of intersections
		if (iPoints.size() % 2){
			endInside = true;
		}
		else {
			endInside = false;
		}
		//for (std::list<API_Coord>::iterator point = iPoints.begin(); point != iPoints.end(); point++){
		//sprintf(buffer, "ix %f iy %f", point->x, point->y);
		//ACAPI_WriteReport(buffer, true);
		//}
		iPoints.clear();

		intersections((*memoToIntersect.coords), numPoints, element.wall.begC, element.wall.endC, &iPoints);

		if (iPoints.size() == 0 && !begInside && !endInside){
			test = &element.header;
			ACAPI_Element_Delete(&test, 1);

			sendElementID(getClientSocket(), elementToIntersect);
		}
		else{
			double distBeg, distEnd;
			for (std::list<API_Coord>::iterator point = iPoints.begin(); point != iPoints.end(); point++){
				//sprintf(buffer, "ix %f iy %f", point->x, point->y);
				//ACAPI_WriteReport(buffer, true);

				distBeg = pow(pow(point->x - element.wall.begC.x, 2) + pow(point->y - element.wall.begC.y, 2), 0.5);
				distEnd = pow(pow(point->x - element.wall.endC.x, 2) + pow(point->y - element.wall.endC.y, 2), 0.5);

				if (!begInside && (distBeg <= distEnd || endInside)){
					element.wall.begC.x = point->x;
					element.wall.begC.y = point->y;
					begInside = true;
				}
				else{
					if (!endInside && (distEnd <= distBeg || begInside)){
						element.wall.endC.x = point->x;
						element.wall.endC.y = point->y;
						endInside = true;
					}
				}
			}

			ACAPI_ELEMENT_MASK_CLEAR(mask);
			ACAPI_ELEMENT_MASK_SET(mask, API_WallType, begC);
			ACAPI_ELEMENT_MASK_SET(mask, API_WallType, endC);

			err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
			//err = ACAPI_Element_Create(&element, &memo);
			if (err != NoError){
				ErrorBeep("ACAPI_Element_Create (slab)", err);
				sprintf(buffer, ErrID_To_Name(err));
				ACAPI_WriteReport(buffer, true);
			}
			sendElementID(getClientSocket(), element);
		}

		test = &elementToIntersect.header;
		ACAPI_Element_Delete(&test, 1);

		ACAPI_DisposeElemMemoHdls(&memoToIntersect);

	}
}

void mirrorElement(){
	API_Element		element, mask;
	API_ElementMemo memo;
	GSErrCode		err;
	mirrormsg		mirrorMsg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &mirrorMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.guid = APIGuidFromString(mirrorMsg.guid().c_str());

	if (ACAPI_Element_Get(&element) == NoError
		&& ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_Polygon) == NoError) {

		double xx, yy, xx2, yy2;
		if (element.header.typeID == API_WallID){
			if (mirrorMsg.axis() == "x"){
				xx = element.wall.endC.x;
				yy = element.wall.endC.y;

				xx2 = element.wall.begC.x;
				yy2 = element.wall.begC.y;

				element.wall.begC.x = xx;
				element.wall.begC.y = -yy;

				element.wall.endC.x = xx2;
				element.wall.endC.y = -yy2;
			}
			else if (mirrorMsg.axis() == "y"){
				xx = element.wall.endC.x;
				yy = element.wall.endC.y;

				xx2 = element.wall.begC.x;
				yy2 = element.wall.begC.y;

				element.wall.begC.x = -xx;
				element.wall.begC.y = yy;

				element.wall.endC.x = -xx2;
				element.wall.endC.y = yy2;
			}

			ACAPI_ELEMENT_MASK_CLEAR(mask);
			ACAPI_ELEMENT_MASK_SET(mask, API_WallType, begC);
			ACAPI_ELEMENT_MASK_SET(mask, API_WallType, endC);

			if (mirrorMsg.copy()){
				err = ACAPI_Element_Create(&element, &memo);
			}
			else{
				err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
			}

			if (err != NoError){
				ErrorBeep("ACAPI_Element_Create (slab)", err);
				sprintf(buffer, ErrID_To_Name(err));
				ACAPI_WriteReport(buffer, true);
			}
		}
		else if (element.header.typeID == API_ColumnID){
			xx = element.column.origoPos.x;
			yy = element.column.origoPos.y;

			if (mirrorMsg.axis() == "x"){
				element.column.origoPos.x = xx;
				element.column.origoPos.y = -yy;
			}
			else if (mirrorMsg.axis() == "y"){
				element.column.origoPos.x = -xx;
				element.column.origoPos.y = yy;
			}

			ACAPI_ELEMENT_MASK_CLEAR(mask);
			ACAPI_ELEMENT_MASK_SET(mask, API_ColumnType, origoPos);

			if (mirrorMsg.copy()){
				err = ACAPI_Element_Create(&element, &memo);
			}
			else{
				err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
			}

			if (err != NoError){
				ErrorBeep("ACAPI_Element_Create (slab)", err);
				sprintf(buffer, ErrID_To_Name(err));
				ACAPI_WriteReport(buffer, true);
			}
		}
		else{
			//TODO
			/*
			for (int i = 0; i < numPoints; i++){

			xx = (*memo.coords)[pIndex].x;
			yy = (*memo.coords)[pIndex].y;

			(*memo.coords)[pIndex].x = xx * cos(theta) - yy * sin(theta);
			(*memo.coords)[pIndex].y = xx * sin(theta) + yy * cos(theta);

			pIndex++;
			}


			if (mirrorMsg.copy()){
			err = ACAPI_Element_Create(&element, &memo);
			}
			else{
			err = ACAPI_Element_ChangeMemo(element.header.guid, APIMemoMask_All, &memo);
			}

			if (err != NoError){
			ErrorBeep("ACAPI_Element_Create (slab)", err);
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
			}
			*/

		}
	}
	ACAPI_DisposeElemMemoHdls(&memo);
	sendElementID(getClientSocket(), element);
}

//----------- Profiles

void buildProfileDescription(profilemsg msg, VectorImage* image)
{
	short materialIndex = 1;
	materialIndex = searchBuildingMaterials(msg.material());

	if (materialIndex == -1){
		//TODO crash?
		materialIndex = 1;
	}

	const short		nCoords = msg.pts().px_size();
	//Coord			coords[nCoords + 1] = { Coord(0.0, 0.0), Coord(0.0, 0.0), Coord(1.0, 0.0), Coord(1.0, 1.0), Coord(0.0, 1.0), Coord(0.0, 0.0) };
	Coord** coords = reinterpret_cast<Coord**> (BMAllocateHandle((nCoords + 1) * sizeof(Coord), ALLOCATE_CLEAR, 0));
	(*coords)[0].x = 0;
	(*coords)[0].y = 0;
	for (int i = 0; i < msg.pts().px_size(); i++){
		(*coords)[i + 1].x = msg.pts().px(i);
		(*coords)[i + 1].y = msg.pts().py(i);
	}

	double* arcs;
	if (msg.arcs().arcangle_size() > 0){
		arcs = new double[msg.arcs().arcangle_size() + 1];
		arcs[0] = 0;
		for (int i = 0; i < msg.arcs().arcangle_size(); i++){
			arcs[i + 1] = msg.arcs().arcangle(i);
		}
	}
	else {
		arcs = NULL;
	}


	Int32 size = sizeof(ProfileItem) + (nCoords + 1) * sizeof(ProfileEdgeData);
	GSHandle addInfo = BMAllocateHandle(size, ALLOCATE_CLEAR, 0);
	Int32 boends[2] = { 0, nCoords };

	if (!DBERROR(addInfo == NULL)) {
		BNZeroMemory(*addInfo, size);

		ProfileItem*	profileItem = reinterpret_cast<ProfileItem*>(*addInfo);
		profileItem->obsoletePriorityValue = 0;		// not used
		profileItem->profileItemVersion = ProfileItemVersion;
		profileItem->SetCutEndLinePen(nCoords);
		profileItem->SetCutEndLineType(nCoords);
		profileItem->SetVisibleCutEndLines(true);
		profileItem->SetCore(true);

		ProfileEdgeData*	profileEdgeData = reinterpret_cast<ProfileEdgeData*>(reinterpret_cast<char*>(*addInfo) + sizeof(ProfileItem));

		profileEdgeData[0].SetPen(0);
		profileEdgeData[0].SetLineType(0);
		profileEdgeData[0].SetMaterial(0);
		profileEdgeData[0].SetFlags(0);

		for (short i = 1; i <= nCoords; i++) {
			profileEdgeData[i].SetCurved(true);
			profileEdgeData[i].SetPen(1);
			profileEdgeData[i].SetLineType(0);
			profileEdgeData[i].SetMaterial(0);		// set different material attribute for each edges
			profileEdgeData[i].SetFlags(ProfileEdgeData::SurfaceFromBuildMatFlag);
		}
	}

	PlaneEq pe;

	VBUtil::HatchTran	hatchTrafo;
	hatchTrafo.SetGlobal();

	try {
		image->AddHatchWhole(true,								// hatch contour visible
			0, //5,								// drawing pen index of the hatch contour
			0, //5,								// line type attribute index of the hatch contour
			materialIndex,								// building material
			0,//7,								// fill attribute index of the hatch
			VBUtil::OverriddenPen(VBUtil::DoNotUseThisAttribute, 0),	// override pen index of the fill (false means no override, pen coming from building material)
			VBUtil::OverriddenPen(VBUtil::DoNotUseThisAttribute, 0),	// override pen index of the fill background (zero means transparent; false means no override, pen coming from building material)
			1,								// layer index of the hatch [1..5]
			DrwIndexForHatches,				// drawing index of the hatch, determining the drawing order of the item
			GS::NULLGuid,						// reserved for internal use, should be NULLGuid
			hatchTrafo,
			1,								// number of the polygon contours
			boends,							// ending vertex indices of contours
			nCoords,							// number of vertices of the polygon
			(*coords),							// vertex coordinates (array of pairs of double values)
			arcs,								// view angles of the edges, applicable if the polygon contains curved edges
			pe,
			DefaultDisplayOrder,
			0,								// fill category: 0 - DraftingFills, 1 - CutFills, 2 - CoverFills
			addInfo);							// profile related additional parameters
	}
	catch (const GS::Exception&) {}

	if (addInfo != NULL){
		BMKillHandle(&addInfo);
	}
	if (coords != NULL){
		BMKillHandle(reinterpret_cast<GSHandle *>(&coords));
	}
	if (arcs != NULL){
		delete(arcs);
	}


}		// BuildProfileDescription

void modifyProfileDescription(profilemsg msg, VectorImage& previousImage, VectorImage* image){
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	char buffer[256];
	short materialIndex = 1;
	materialIndex = searchBuildingMaterials(msg.material());

	if (materialIndex == -1){
		//TODO crash?
		materialIndex = 1;
	}

	ConstVectorImageIterator profileDescriptionIt(previousImage);

	while (!profileDescriptionIt.IsEOI()){
		switch (profileDescriptionIt->item_Typ){
		case SyHatch:{
			const Sy_HatchType* pSyHatch = static_cast <const Sy_HatchType*> (profileDescriptionIt);
			GSConstPtr			hatchCharPtr = reinterpret_cast<GSConstPtr> (pSyHatch);

			//Coords
			const API_Coord* previousCoords = reinterpret_cast<const API_Coord*> (hatchCharPtr + pSyHatch->coorOff);
			UInt32 previousNCoords = (pSyHatch->coorLen / sizeof(API_Coord)) - 1;
			const short nCoords = previousNCoords + msg.pts().px_size();
			Coord** coords = reinterpret_cast<Coord**> (BMAllocateHandle((nCoords + 1) * sizeof(Coord), ALLOCATE_CLEAR, 0));
			previousCoords++;
			(*coords)[0].x = 0;
			(*coords)[0].y = 0;
			for (int i = 0; i < previousNCoords; i++, previousCoords++){
				(*coords)[i + 1].x = previousCoords->x;
				(*coords)[i + 1].y = previousCoords->y;
				//sprintf(buffer, "X %f Y %f", (*coords)[i + 1].x, (*coords)[i + 1].y);
				//ACAPI_WriteReport(buffer, true);
			}
			for (int i = previousNCoords; i < nCoords; i++){
				(*coords)[i + 1].x = msg.pts().px(i - previousNCoords);
				(*coords)[i + 1].y = msg.pts().py(i - previousNCoords);
				//sprintf(buffer, "X %f Y %f", (*coords)[i + 1].x, (*coords)[i + 1].y);
				//ACAPI_WriteReport(buffer, true);
			}

			//Arcs
			const double* previousArcs = reinterpret_cast<const double*> (hatchCharPtr + pSyHatch->arcsOff);
			UInt32 previousNArcs = (pSyHatch->arcsLen / sizeof(double)) - 1;
			const short nArcs = previousNArcs + msg.arcs().arcangle_size();
			double* arcs;
			if (nArcs > 0){
				arcs = new double[nArcs + 1];
				arcs[0] = 0;
				previousArcs++;
				for (int i = 0; i < previousNArcs; i++, previousArcs++){
					arcs[i + 1] = (*previousArcs);
				}
				for (int i = previousNArcs; i < nArcs; i++){
					arcs[i + 1] = msg.arcs().arcangle(i - previousNArcs);
				}
			}
			else {
				arcs = NULL;
			}


			//Ends
			const Int32* previousEnds = reinterpret_cast<const Int32*>(hatchCharPtr + pSyHatch->endsOff + sizeof(Int32));
			UInt32 previousNEnds = (pSyHatch->endsLen / sizeof(Int32)) - 1;
			const short nEnds = previousNEnds + 2;
			Int32* boends = new Int32[nEnds];
			boends[0] = 0;
			for (int i = 0; i < previousNEnds; i++, previousEnds++){
				boends[i + 1] = (*previousEnds);
				//sprintf(buffer, "boends %d", boends[i + 1]);
				//ACAPI_WriteReport(buffer, true);
			}
			boends[previousNEnds + 1] = boends[previousNEnds] + msg.pts().px_size();

			//Profile Data
			Int32 size = sizeof(ProfileItem) + (nCoords + 1) * sizeof(ProfileEdgeData);
			GSHandle addInfo = BMAllocateHandle(size, ALLOCATE_CLEAR, 0);

			if (!DBERROR(addInfo == NULL)) {
				BNZeroMemory(*addInfo, size);

				ProfileItem*	profileItem = reinterpret_cast<ProfileItem*>(*addInfo);
				profileItem->obsoletePriorityValue = 0;		// not used
				profileItem->profileItemVersion = ProfileItemVersion;
				profileItem->SetCutEndLinePen(nCoords);
				profileItem->SetCutEndLineType(nCoords);
				profileItem->SetVisibleCutEndLines(true);
				profileItem->SetCore(true);

				ProfileEdgeData*	profileEdgeData = reinterpret_cast<ProfileEdgeData*>(reinterpret_cast<char*>(*addInfo) + sizeof(ProfileItem));

				profileEdgeData[0].SetPen(0);
				profileEdgeData[0].SetLineType(0);
				profileEdgeData[0].SetMaterial(0);
				profileEdgeData[0].SetFlags(0);

				for (short i = 1; i <= nCoords; i++) {
					profileEdgeData[i].SetCurved(true);
					profileEdgeData[i].SetPen(1);
					profileEdgeData[i].SetLineType(0);
					profileEdgeData[i].SetMaterial(0);		// set different material attribute for each edges
					profileEdgeData[i].SetFlags(ProfileEdgeData::SurfaceFromBuildMatFlag);
				}
			}
			PlaneEq pe;

			VBUtil::HatchTran	hatchTrafo;
			hatchTrafo.SetGlobal();

			try {
				image->AddHatchWhole(true,								// hatch contour visible
					0, //5,								// drawing pen index of the hatch contour
					0, //5,								// line type attribute index of the hatch contour
					materialIndex,								// building material
					0,//7,								// fill attribute index of the hatch
					VBUtil::OverriddenPen(VBUtil::DoNotUseThisAttribute, 0),	// override pen index of the fill (false means no override, pen coming from building material)
					VBUtil::OverriddenPen(VBUtil::DoNotUseThisAttribute, 0),	// override pen index of the fill background (zero means transparent; false means no override, pen coming from building material)
					1,								// layer index of the hatch [1..5]
					DrwIndexForHatches,				// drawing index of the hatch, determining the drawing order of the item
					GS::NULLGuid,						// reserved for internal use, should be NULLGuid
					hatchTrafo,
					previousNEnds + 1,								// number of the polygon contours
					boends,							// ending vertex indices of contours
					nCoords,							// number of vertices of the polygon
					(*coords),							// vertex coordinates (array of pairs of double values)
					arcs,								// view angles of the edges, applicable if the polygon contains curved edges
					pe,
					DefaultDisplayOrder,
					0,								// fill category: 0 - DraftingFills, 1 - CutFills, 2 - CoverFills
					addInfo);							// profile related additional parameters
			}
			catch (const GS::Exception&) {}

			if (addInfo != NULL){
				BMKillHandle(&addInfo);
			}
			if (coords != NULL){
				BMKillHandle(reinterpret_cast<GSHandle *>(&coords));
			}
			if (arcs != NULL){
				delete(arcs);
			}
			if (boends != NULL){
				delete(boends);
			}

		}
					 break;
		}
		++profileDescriptionIt;
	}
}

void createProfile(){
	API_Attribute		profile_attr;
	API_AttributeDefExt	profile_attrdef;
	GSErrCode			err = NoError;

	BNZeroMemory((GSPtr)&profile_attr, sizeof(API_Attribute));
	BNZeroMemory((GSPtr)&profile_attrdef, sizeof(API_AttributeDefExt));

	profilemsg msg;
	readDelimitedFrom(getClientSocket(), &msg);

	profile_attr.header.typeID = API_ProfileID;
	profile_attr.header.flags = 0;
	strcpy(profile_attr.header.name, msg.name().c_str());
	profile_attr.profile.wallType = true;
	profile_attr.profile.beamType = true;
	profile_attr.profile.coluType = true;

	VectorImage profileDescription;

	buildProfileDescription(msg, &profileDescription);
	profile_attrdef.profile_vectorImageItems = BMhAll(0);
	try { profileDescription.Export(profile_attrdef.profile_vectorImageItems); }
	catch (const GS::Exception&) {}

	profile_attrdef.profile_vectorImageItems;

	err = ACAPI_Attribute_CreateExt(&profile_attr, &profile_attrdef);
	if (err == APIERR_ATTREXIST) {
		err = ACAPI_Attribute_ModifyExt(&profile_attr, &profile_attrdef);
	}
	ACAPI_DisposeAttrDefsHdlsExt(&profile_attrdef);
	if (err != NoError) {
		ACAPI_WriteReport("Profile Error", true);
		return;
	}

	profileTypesInsert(msg.name(), profile_attr.header.index);

	return;
}

void addHatchWholeProfile(){
	API_Attribute		profile_attr;
	API_Attribute		previous_profile_attr;
	API_AttributeDefExt	profile_attrdef;
	API_AttributeDefExt	previous_profile_attrdef;
	GSErrCode			err = NoError;

	BNZeroMemory((GSPtr)&profile_attr, sizeof(API_Attribute));
	BNZeroMemory((GSPtr)&profile_attrdef, sizeof(API_AttributeDefExt));

	profilemsg msg;
	readDelimitedFrom(getClientSocket(), &msg);

	profile_attr.header.typeID = API_ProfileID;
	profile_attr.header.flags = 0;
	strcpy(profile_attr.header.name, msg.name().c_str());
	profile_attr.profile.wallType = true;
	profile_attr.profile.beamType = true;
	profile_attr.profile.coluType = true;

	previous_profile_attr.header.typeID = API_ProfileID;
	strcpy(previous_profile_attr.header.name, msg.name().c_str());

	VectorImage previousProfileDescription;
	VectorImage profileDescription;
	if (ACAPI_Attribute_Get(&previous_profile_attr) == NoError &&
		ACAPI_Attribute_GetDefExt(API_ProfileID, previous_profile_attr.header.index, &previous_profile_attrdef) == NoError){
		previousProfileDescription.Import(previous_profile_attrdef.profile_vectorImageItems);

		modifyProfileDescription(msg, previousProfileDescription, &profileDescription);
		profile_attrdef.profile_vectorImageItems = BMhAll(0);
		try { profileDescription.Export(profile_attrdef.profile_vectorImageItems); }
		catch (const GS::Exception&) {
			ACAPI_WriteReport("Profile Error", true);
		}

		profile_attrdef.profile_vectorImageItems;

		err = ACAPI_Attribute_CreateExt(&profile_attr, &profile_attrdef);
		if (err == APIERR_ATTREXIST) {
			err = ACAPI_Attribute_ModifyExt(&profile_attr, &profile_attrdef);
		}

		ACAPI_DisposeAttrDefsHdlsExt(&profile_attrdef);
		if (err != NoError) {
			ACAPI_WriteReport("Profile Error", true);
			return;
		}

	}
	else{
		ACAPI_WriteReport("Profile Error", true);
	}
	return;
}

//----------- Layers

void internalCreateLayer(layermsg msg){
	API_Attribute		attr;
	API_AttributeDef	defs;
	GSErrCode			err = NoError;

	BNZeroMemory((GSPtr)&attr, sizeof(API_Attribute));
	BNZeroMemory((GSPtr)&defs, sizeof(API_AttributeDef));

	attr.header.typeID = API_LayerID;
	strcpy(attr.header.name, msg.name().c_str());

	attr.layer.conClassId = msg.connection();

	err = ACAPI_Attribute_Create(&attr, &defs);
	if (err != APIERR_ATTREXIST){
		if (hasError(err)){
			quit();
			return;
		}
	}

	layersInsert(attr.layer.head.name, attr.layer.head.index);
}

void createLayer(){
	layermsg msg;
	readDelimitedFrom(getClientSocket(), &msg);
	internalCreateLayer(msg);
}

void attributeLayerToElement(){
	API_Element element, mask;
	API_Attribute attr;
	GSErrCode err = NoError;
	char buffer[256];
	layerelementmsg msg;
	readDelimitedFrom(getClientSocket(), &msg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&attr, sizeof(API_Attribute));

	element.header.guid = APIGuidFromString(msg.guid().c_str());

	API_DatabaseInfo dbInfo;
	API_DatabaseInfo floorInfo;
	floorInfo.typeID = APIWind_FloorPlanID;
	floorInfo.index = 0;
	ACAPI_Database(APIDb_GetCurrentDatabaseID, &dbInfo);
	ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &floorInfo);


	err = ACAPI_Element_Get(&element);
	if (hasError(err)){
		quit();
		return;
	}
	
	if (msg.layer() == "ArchiCAD Layer"){
		element.header.layer = 1;
	}
	else{
		attr.header.typeID = API_LayerID;
		strcpy(attr.header.name, msg.layer().c_str());
		err = ACAPI_Attribute_Get(&attr);
		if (hasError(err)){
			quit();
			return;
		}
		element.header.layer = attr.header.index;

		ACAPI_ELEMENT_MASK_CLEAR(mask);
		ACAPI_ELEMENT_MASK_SET(mask, API_Elem_Head, layer);

		err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
		if (hasError(err)){
			quit();
			return;
		}

		ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &dbInfo);
	}
}

void controlLayer(layermsg msg, bool show){
	API_Attribute          attr;
	API_AttributeDef       defs;
	short                   ltypeIndex;
	GSErrCode               err;

	BNZeroMemory(&attr, sizeof(API_Attribute));
	BNZeroMemory(&defs, sizeof(API_AttributeDef));

	attr.header.typeID = API_LayerID;
	strcpy(attr.header.name, msg.name().c_str());

	err = ACAPI_Attribute_Get(&attr);
	if (hasError(err)){
		quit();
		return;
	}

	if (show){
		attr.header.flags = 0;
	}
	else{
		attr.header.flags |= APILay_Hidden;
	}
	err = ACAPI_Attribute_Modify(&attr, &defs);
	ACAPI_DisposeAttrDefsHdls(&defs);
	if (hasError(err)){
		quit();
		return;
	}
}

void hideLayer(){
	layermsg msg;
	readDelimitedFrom(getClientSocket(), &msg);
	controlLayer(msg, false);
}

void showLayer(){
	layermsg msg;
	readDelimitedFrom(getClientSocket(), &msg);
	controlLayer(msg, true);
}

bool hiddenLayer(layermsg msg){
	API_Attribute          attr;
	API_AttributeDef       defs;
	short                   ltypeIndex;
	GSErrCode               err;

	BNZeroMemory(&attr, sizeof(API_Attribute));
	BNZeroMemory(&defs, sizeof(API_AttributeDef));

	attr.header.typeID = API_LayerID;
	strcpy(attr.header.name, msg.name().c_str());

	err = ACAPI_Attribute_Get(&attr);
	if (hasError(err)){
		quit();
		return false;
	}

	return attr.header.flags == APILay_Hidden;
}

void getLayerFromElement(){
	API_Element element;
	char buffer[256];
	elementid msg;
	namemessage namemsg;
	GSErrCode err = NoError;
	
	readDelimitedFrom(getClientSocket(), &msg);

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.guid = APIGuidFromString(msg.guid().c_str());;
	err = ACAPI_Element_Get(&element);

	if (hasError(err)){
		quit();
		return;
	}

	API_Attribute attr;
	API_AttributeDef defs;
	short ltypeIndex;

	BNZeroMemory(&attr, sizeof(API_Attribute));
	BNZeroMemory(&defs, sizeof(API_AttributeDef));

	attr.header.typeID = API_LayerID;
	attr.header.index = element.header.layer;

	err = ACAPI_Attribute_Get(&attr);
	if (hasError(err)){
		quit();
		return;
	}

	namemsg.set_name(attr.header.name);

	writeDelimitedTo(getClientSocket(), namemsg);

	ACAPI_DisposeAttrDefsHdls(&defs);
}

//----------- Camera Functions
/*
 This function creates a camera set and a camera within said set.
 When the user creates another camera, the set is deleted and we create a new one with the new camera.
 The problem is that I do not know how to change the view to the created camera.
 */
void OLDcreateCamera(){
	API_Element		element, auxEl, mask;
	API_ElementMemo memo, auxMemo;
	GSErrCode		err;
	cameramsg		msg;
	API_StoryInfo	storyInfo;
	char buffer[256];
	bool crash = false;
	std::string name = "Rosetta CamSet";

	readDelimitedFrom(getClientSocket(), &msg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	element.header.typeID = API_CameraID;
	auxEl.header.typeID = API_CamSetID;

	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_CamSetID, &elemList);
	bool failed = false;
	bool alreadyExists = false;
	for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate(); it != NULL; ++it) {
		BNZeroMemory(&auxEl, sizeof(API_Element));
		auxEl.header.guid = *it;
		err = ACAPI_Element_Get(&auxEl);
		if (err == NoError) {
			for (int i = 0; i < name.length(); i++){
				if (auxEl.camset.name[i] != name.at(i)){
					failed = true;
					break;
				}
			}
			if (!failed){
				API_Elem_Head* toDelete;
				toDelete = &auxEl.header;
				ACAPI_Element_Delete(&toDelete, 1);
				break;
			}
			else{
				failed = false;
			}
		}
		else{
			ErrorBeep("ACAPI_Element_Create", err);
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
			quit();
			return;
		}
	}
	
	BNZeroMemory(&auxEl, sizeof(API_Element));
	BNZeroMemory(&auxMemo, sizeof(API_ElementMemo));

	auxEl.header.typeID = API_CamSetID;

	auxEl.camset.active = true;

	GS::snuprintf(auxEl.camset.name, sizeof(auxEl.camset.name) / sizeof(GS::uchar_t), name.c_str());

	err = ACAPI_Element_Create(&auxEl, &auxMemo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		msgArchiCAD("create camset");
		quit();
		return;
	}

	element.camera.camSetGuid = auxEl.camset.head.guid;
	//element.camera.perspCam.active = true;
	//element.camera.perspCam.prevCam = APINULLGuid;
	//element.camera.perspCam.nextCam = APINULLGuid;
	element.camera.perspCam.persp.viewCone = msg.lens();

	//Sun
	element.camera.perspCam.persp.sunAzimuth = msg.sunazimuth();
	element.camera.perspCam.persp.sunAltitude = msg.sunaltitude();

	//element.camera.perspCam.persp.azimuth = msg.azimuth();
	//element.camera.perspCam.persp.rollAngle = msg.rollangle();
	//element.camera.perspCam.persp.distance = msg.distance();

	element.camera.perspCam.persp.pos.x = msg.cx();
	element.camera.perspCam.persp.pos.y = msg.cy();
	element.camera.perspCam.persp.cameraZ = msg.cz();
	element.camera.perspCam.persp.target.x = msg.tx();
	element.camera.perspCam.persp.target.y = msg.ty();
	element.camera.perspCam.persp.targetZ = msg.tz();


	err = ACAPI_Element_Create(&element, &memo);
	sendElementID(getClientSocket(), element, crash);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		msgArchiCAD("create cam");
		quit();
	}

	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&auxMemo);
}

/*
 This functions changes the prespective properties of the Generic Prespective of ArchiCAD.
 */
void createCamera(){
	GSErrCode		err;
	cameramsg		msg;
	API_StoryInfo	storyInfo;
	char buffer[256];
	bool crash = false;
	std::string name = "Rosetta CamSet";

	readDelimitedFrom(getClientSocket(), &msg);
	
	API_3DProjectionInfo info;
	BNZeroMemory(&info, sizeof(API_3DProjectionInfo));
	
	err = ACAPI_Environment(APIEnv_Get3DProjectionSetsID, &info, NULL, NULL);
	if (err == NoError) {
			
			info.u.persp.viewCone = msg.lens();

			//Sun
			info.u.persp.sunAzimuth = msg.sunazimuth();
			info.u.persp.sunAltitude = msg.sunaltitude();

			info.u.persp.pos.x = msg.cx();
			info.u.persp.pos.y = msg.cy();
			info.u.persp.cameraZ = msg.cz();
			info.u.persp.target.x = msg.tx();
			info.u.persp.target.y = msg.ty();
			info.u.persp.targetZ = msg.tz();
			err = ACAPI_Environment(APIEnv_Change3DProjectionSetsID, &info, NULL, NULL);
			if (hasError(err)){
				quit();
				return;
			}
	}
	else{
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		quit();
		return;
	}

	view3D();
}

//----------- Test Functions

void TestBuildProfileDescription(VectorImage* image)
{

	const short		nCoords = 5;
	Coord			coords[nCoords + 1] = { Coord(0.0, 0.0), Coord(0.0, 0.0), Coord(1.0, 0.0), Coord(1.0, 1.0), Coord(0.0, 1.0), Coord(0.0, 0.0) };
	Int32			size = sizeof(ProfileItem) + (nCoords + 1) * sizeof(ProfileEdgeData);
	GSHandle		addInfo = BMAllocateHandle(size, ALLOCATE_CLEAR, 0);
	Int32			boends[2] = { 0, nCoords };

	if (!DBERROR(addInfo == NULL)) {
		BNZeroMemory(*addInfo, size);

		ProfileItem*	profileItem = reinterpret_cast<ProfileItem*>(*addInfo);
		profileItem->obsoletePriorityValue = 0;		// not used
		profileItem->profileItemVersion = ProfileItemVersion;
		profileItem->SetCutEndLinePen(5);
		profileItem->SetCutEndLineType(5);
		profileItem->SetVisibleCutEndLines(true);
		profileItem->SetCore(true);

		ProfileEdgeData*	profileEdgeData = reinterpret_cast<ProfileEdgeData*>(reinterpret_cast<char*>(*addInfo) + sizeof(ProfileItem));

		profileEdgeData[0].SetPen(0);
		profileEdgeData[0].SetLineType(0);
		profileEdgeData[0].SetMaterial(0);
		profileEdgeData[0].SetFlags(0);

		for (short i = 1; i <= nCoords; i++) {
			profileEdgeData[i].SetPen(1);
			profileEdgeData[i].SetLineType(1);
			profileEdgeData[i].SetMaterial(i);		// set different material attribute for each edges
			profileEdgeData[i].SetFlags(ProfileEdgeData::IsVisibleLineFlag);
		}
	}

	PlaneEq pe;

	VBUtil::HatchTran	hatchTrafo;
	hatchTrafo.SetGlobal();

	try {
		image->AddHatchWhole(true,								// hatch contour visible
			5,								// drawing pen index of the hatch contour
			5,								// line type attribute index of the hatch contour
			7,								// building material
			7,								// fill attribute index of the hatch
			VBUtil::OverriddenPen(VBUtil::UseThisAttribute, 4),	// override pen index of the fill (false means no override, pen coming from building material)
			VBUtil::OverriddenPen(VBUtil::UseThisAttribute, 12),	// override pen index of the fill background (zero means transparent; false means no override, pen coming from building material)
			1,								// layer index of the hatch [1..5]
			DrwIndexForHatches,				// drawing index of the hatch, determining the drawing order of the item
			GS::NULLGuid,						// reserved for internal use, should be NULLGuid
			hatchTrafo,
			1,								// number of the polygon contours
			boends,							// ending vertex indices of contours
			nCoords,							// number of vertices of the polygon
			coords,							// vertex coordinates (array of pairs of double values)
			NULL,								// view angles of the edges, applicable if the polygon contains curved edges
			pe,
			DefaultDisplayOrder,
			0,								// fill category: 0 - DraftingFills, 1 - CutFills, 2 - CoverFills
			addInfo);							// profile related additional parameters
	}
	catch (const GS::Exception&) {}

	if (addInfo != NULL)
		BMKillHandle(&addInfo);


}		// BuildProfileDescription

void profileTest(){
	API_Attribute		profile_attr;
	API_AttributeDefExt	profile_attrdef;
	GSErrCode			err = NoError;

	BNZeroMemory((GSPtr)&profile_attr, sizeof(API_Attribute));
	BNZeroMemory((GSPtr)&profile_attrdef, sizeof(API_AttributeDefExt));

	profile_attr.header.typeID = API_ProfileID;
	profile_attr.header.flags = 0;
	strcpy(profile_attr.header.name, "Profile from API");
	profile_attr.profile.wallType = true;
	profile_attr.profile.beamType = false;
	profile_attr.profile.coluType = true;

	VectorImage profileDescription;

	TestBuildProfileDescription(&profileDescription);
	profile_attrdef.profile_vectorImageItems = BMhAll(0);
	try { profileDescription.Export(profile_attrdef.profile_vectorImageItems); }
	catch (const GS::Exception&) {}

	profile_attrdef.profile_vectorImageItems;

	err = ACAPI_Attribute_CreateExt(&profile_attr, &profile_attrdef);
	if (err == APIERR_ATTREXIST) {
		err = ACAPI_Attribute_ModifyExt(&profile_attr, &profile_attrdef);
	}
	ACAPI_DisposeAttrDefsHdlsExt(&profile_attrdef);
	if (err != NoError) {
		ACAPI_WriteReport("Profile Error", true);
		return;
	}

	return;
}

void createTestLibraryPart(){
	//Creation of a Library Part using GDL code as strings...
	GSErrCode err = NoError;

	API_LibPart libPart;
	BNZeroMemory(&libPart, sizeof(API_LibPart));
	libPart.typeID = APILib_ObjectID;
	libPart.isTemplate = false;
	libPart.isPlaceable = true;
	CHCopyC("{103E8D2C-8230-42E1-9597-46F84CCE28C0}-{00000000-0000-0000-0000-000000000000}", libPart.parentUnID);	// Model Element subtype

	GSTimeRecord timeRecord;
	TIGetTimeRecord(0, &timeRecord, TI_CURRENT_TIME);
	UInt32 fraction = TIGetTicks() % TIGetTicksPerSec();
	GS::snuprintf(libPart.docu_UName, sizeof(libPart.docu_UName) / sizeof(GS::uchar_t), L("LPTest_%d-%02d-%02d_%02d%02d%02d_%d"),
		timeRecord.year, timeRecord.month, timeRecord.day, timeRecord.hour, timeRecord.minute, timeRecord.second, fraction);

	err = GetLocation(libPart.location, true);
	if (err != NoError) {
		WriteReport("Library Part creation failed");
		return;
	}

	ACAPI_Environment(APIEnv_OverwriteLibPartID, (void *)(Int32)true, NULL);
	err = ACAPI_LibPart_Create(&libPart);
	ACAPI_Environment(APIEnv_OverwriteLibPartID, (void *)(Int32)false, NULL);

	if (err == NoError) {
		char buffer[1000];

		API_LibPartSection section;

		// Comment script section
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_SectComText;
		ACAPI_LibPart_NewSection(&section);
		sprintf(buffer, "Library Part written by LibPart_Test add-on");
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		ACAPI_LibPart_EndSection();

		// Master script section
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_Sect1DScript;
		ACAPI_LibPart_NewSection(&section);
		buffer[0] = '\0';
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		ACAPI_LibPart_EndSection();

		/*
		// 3D script section
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_Sect3DScript;
		ACAPI_LibPart_NewSection(&section);
		sprintf(buffer, "MATERIAL mat%s%s", GS::EOL, GS::EOL);
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		sprintf(buffer, "BLOCK a, b, 1%s", GS::EOL);
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		sprintf(buffer, "ADD a * 0.5, b* 0.5, 1%s", GS::EOL);
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		sprintf(buffer, "CYLIND zzyzx - 3, MIN (a, b) * 0.5%s", GS::EOL);
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		sprintf(buffer, "ADDZ zzyzx - 3%s", GS::EOL);
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		sprintf(buffer, "CONE 2, MIN (a, b) * 0.5, 0.0, 90, 90%s", GS::EOL);
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		ACAPI_LibPart_EndSection();
		*/
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_Sect3DScript;
		ACAPI_LibPart_NewSection(&section);
		std::string s = "MATERIAL mat \r\n BLOCK a, b, 1 \r\n ADD a * 0.5, b* 0.5, 1 \r\n CYLIND zzyzx - 3, MIN (a, b) * 0.5 \r\n ADDZ zzyzx - 3 \r\n CONE 2, MIN (a, b) * 0.5, 0.0, 90, 90 \r\n";
		sprintf(buffer, "%s", s.c_str());
		ACAPI_LibPart_WriteSection(s.size(), (GSPtr)s.c_str());
		//ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		ACAPI_LibPart_EndSection();

		// 2D script section
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_Sect2DScript;
		ACAPI_LibPart_NewSection(&section);
		sprintf(buffer, "PROJECT2 3, 270, 2%s", GS::EOL);
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		ACAPI_LibPart_EndSection();

		// Parameter script section
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_SectVLScript;
		ACAPI_LibPart_NewSection(&section);
		sprintf(buffer, "VALUES \"zzyzx\" RANGE [6,]%s", GS::EOL);
		ACAPI_LibPart_WriteSection(Strlen32(buffer), buffer);
		ACAPI_LibPart_EndSection();

		// Parameters section
		BNZeroMemory(&section, sizeof(API_LibPartSection));
		section.sectType = API_SectParamDef;

		API_AddParType** addPars = reinterpret_cast<API_AddParType**>(BMAllocateHandle(sizeof(API_AddParType), ALLOCATE_CLEAR, 0));
		if (addPars != NULL) {
			API_AddParType* pAddPar = (*addPars);
			pAddPar->typeID = APIParT_Mater;
			pAddPar->typeMod = 0;
			CHTruncate("mat", pAddPar->name, sizeof(pAddPar->name));
			GS::ucscpy(pAddPar->uDescname, L("Material"));
			pAddPar->value.real = 1;

			double aa = 1.0;
			double bb = 1.0;
			GSHandle sectionHdl = NULL;
			ACAPI_LibPart_GetSect_ParamDef(&libPart, addPars, &aa, &bb, NULL, &sectionHdl);

			API_LibPartDetails details;
			BNZeroMemory(&details, sizeof(API_LibPartDetails));
			details.object.autoHotspot = false;
			ACAPI_LibPart_SetDetails_ParamDef(&libPart, sectionHdl, &details);

			ACAPI_LibPart_AddSection(&section, sectionHdl, NULL);

			BMKillHandle(reinterpret_cast<GSHandle*>(&addPars));
			BMKillHandle(&sectionHdl);
		}
		else {
			err = APIERR_MEMFULL;
		}

		// Save the constructed library part
		if (err == NoError)
			err = ACAPI_LibPart_Save(&libPart);

		if (libPart.location != NULL) {
			delete libPart.location;
			libPart.location = NULL;
		}
	}

	if (err == NoError)
		WriteReport("Library Part \"%s\" created", static_cast<const char*> (GS::UniString(libPart.docu_UName).ToCStr()));
	else
		WriteReport("Library Part creation failed");

	return;
}

void getLibraryPartParam(){
	API_ParamOwnerType   paramOwner;
	API_ChangeParamType  chgParam;
	API_GetParamsType    getParams;
	API_Element          element, mask;
	API_ElementMemo      memo;
	GSErrCode            err;
	char buffer[256];

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_ObjectID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		//sprintf(buffer, "Default Problem");
		//ACAPI_WriteReport(buffer, true);
	}

	element.header.floorInd = getCurrentLevel();

	//sprintf(buffer, "libInd: %d", element.object.libInd);
	//ACAPI_WriteReport(buffer, true);

	element.object.level = 0;

	element.object.libInd = 1109;
	element.object.pos.x = 0;
	element.object.pos.y = 0;

	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("Object Create", err);
		//sprintf(buffer, ErrID_To_Name(err));
		//ACAPI_WriteReport(buffer, true);
	}

	BNZeroMemory(&paramOwner, sizeof(API_ParamOwnerType));
	paramOwner.libInd = 0;
	paramOwner.typeID = API_ObjectID;
	paramOwner.guid = element.header.guid;

	BNZeroMemory(&getParams, sizeof(API_GetParamsType));

	err = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner, NULL);
	if (err == NoError) {
		BNZeroMemory(&chgParam, sizeof(API_ChangeParamType));
		err = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, NULL);
		if (err == NoError) {

			int numParams = BMhGetSize(reinterpret_cast<GSHandle> (getParams.params)) / Sizeof32(API_AddParType) - 1;
			/*
			for (int i = 0; i < numParams && i < 10; i++){
			sprintf(buffer, (*getParams.params)[i].name);
			ACAPI_WriteReport(buffer, true);
			}
			*/
			for (int i = 0; i < numParams; i++){
				if ((std::string)(*getParams.params)[i].name == "story_elev_above"){
					//sprintf(buffer, "typeMod: %d dim1: %d dim2: %d", (*getParams.params)[i].typeMod, (*getParams.params)[i].dim1, (*getParams.params)[i].dim2);
					//ACAPI_WriteReport(buffer, true);
					int auxSize = BMhGetSize(reinterpret_cast<GSHandle> ((*getParams.params)[i].value.array)) / Sizeof32(GSHandle) - 1;
					double** arrHdl = reinterpret_cast<double**>((*getParams.params)[i].value.array);

					//sprintf(buffer, "array[0] = %c", arrHdl[0]);
					//ACAPI_WriteReport(buffer, true);

					for (int j = 0; j < auxSize; j++){
						//sprintf(buffer, "array %d: %d", j, ((int*)(*getParams.params)[i].value.array)[j]);
						//sprintf(buffer, "array %d: %f", j, (const char *)((string)((string *)*(*getParams.params)[i].value.array)[j]).c_str());
						sprintf(buffer, "array[0][%d]: %f", j, arrHdl[0][j]);
						//buffer[0] = arrHdl[j][0];
						ACAPI_WriteReport(buffer, true);
					}
					/*
					sprintf(buffer, "Initial Value: %f", (*getParams.params)[i].value.real);
					ACAPI_WriteReport(buffer, true);
					//chgParam.index = i;

					CHCopyC("car_height", chgParam.name);
					chgParam.realValue = 6.0;
					err = ACAPI_Goodies(APIAny_ChangeAParameterID, &chgParam, NULL);
					if (err == NoError){
					err = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, NULL);
					sprintf(buffer, "Changed to: %f", (*getParams.params)[i].value.real);
					ACAPI_WriteReport(buffer, true);
					}
					*/
					break;
				}
			}

		}
		ACAPI_Goodies(APIAny_CloseParametersID, NULL, NULL);
	}

	if (err == NoError) {
		BNZeroMemory(&element, sizeof(API_Element));
		BNZeroMemory(&memo, sizeof(API_ElementMemo));

		err = ACAPI_Element_GetDefaults(&element, &memo);
		if (err != NoError) {
			ErrorBeep("ACAPI_Element_GetMemo", err);
			//sprintf(buffer, "Default Problem");
			//ACAPI_WriteReport(buffer, true);
		}

		element.header.typeID = API_ObjectID;
		element.object.xRatio = getParams.a;
		element.object.yRatio = getParams.b;

		element.header.floorInd = getCurrentLevel();
		element.object.level = 0;
		element.object.libInd = 1109;
		element.object.pos.x = 5;
		element.object.pos.y = 5;

		//ACAPI_ELEMENT_MASK_CLEAR(mask);
		//ACAPI_ELEMENT_MASK_SET(mask, API_ObjectType, xRatio);
		//ACAPI_ELEMENT_MASK_SET(mask, API_ObjectType, yRatio);
		//memo.params = getParams.params;

		err = ACAPI_Element_Create(&element, &memo);
		if (err != NoError){
			ErrorBeep("Object Create", err);
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}

		//err = ACAPI_Element_ChangeDefaults(&element, &memo, &mask);
	}

	ACAPI_DisposeAddParHdl(&getParams.params);
}

void changeParameterOfLibPart(){
	API_ParamOwnerType   paramOwner;
	API_ChangeParamType  chgParam;
	API_GetParamsType    getParams;
	API_Element          element, mask;
	API_ElementMemo      memo;
	GSErrCode            err;
	char buffer[256];

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_ObjectID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		//sprintf(buffer, "Default Problem");
		//ACAPI_WriteReport(buffer, true);
	}

	element.header.floorInd = getCurrentLevel();
	element.object.level = 0;
	element.object.libInd = 1109;
	element.object.pos.x = 0;
	element.object.pos.y = 0;

	BNZeroMemory(&paramOwner, sizeof(API_ParamOwnerType));
	paramOwner.libInd = 1109;
	//paramOwner.typeID = API_ObjectID;
	//paramOwner.guid = element.header.guid;

	BNZeroMemory(&getParams, sizeof(API_GetParamsType));

	err = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner, NULL);
	if (err == NoError) {
		BNZeroMemory(&chgParam, sizeof(API_ChangeParamType));
		err = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, NULL);
		if (err == NoError) {
			int numParams = BMhGetSize(reinterpret_cast<GSHandle> (getParams.params)) / Sizeof32(API_AddParType) - 1;
			std::string parameterName = "car_height";
			for (int i = 0; i < numParams; i++){
				if ((std::string)(*getParams.params)[i].name == parameterName){
					(*getParams.params)[i].value.real = 6.0;
					memo.params = getParams.params;
					err = ACAPI_Element_Create(&element, &memo);
					if (err != NoError){
						ErrorBeep("Object Create", err);
						sprintf(buffer, ErrID_To_Name(err));
						ACAPI_WriteReport(buffer, true);
					}
					break;
				}
			}
		}
		ACAPI_Goodies(APIAny_CloseParametersID, NULL, NULL);
	}

	ACAPI_DisposeAddParHdl(&getParams.params);
}

void testFunction(){
	/*
	GS::Array<API_LibraryInfo>    libInfo;
	char buffer[256];

	if (ACAPI_Environment(APIEnv_GetLibrariesID, &libInfo) == NoError) {
	DBPrintf("Environment Control :: The number of loaded libraries is %u\n", libInfo.GetSize());
	for (UInt32 ii = 0; ii < libInfo.GetSize(); ii++) {
	IO::Path    libPath;

	libInfo[ii].location.ToPath(&libPath);

	sprintf(buffer, (const char *)libPath);
	sprintf(buffer, libInfo[ii].name.ToCStr());
	ACAPI_WriteReport(buffer, true);
	}
	}
	*/
	/*
	API_Element element;
	BNZeroMemory(&element, sizeof(API_Element));
	element.header.typeID = API_MorphID;
	GSErrCode err = ACAPI_Element_GetDefaults(&element, NULL);
	if (err != NoError) {
	ErrorBeep("ACAPI_Element_GetDefaults (morph)", err);
	return;
	}

	double* tmx = element.morph.tranmat.tmx;
	tmx[0] = 1.0;				tmx[4] = 0.0;				tmx[8] = 0.0;
	tmx[1] = 0.0;				tmx[5] = 1.0;				tmx[9] = 0.0;
	tmx[2] = 0.0;				tmx[6] = 0.0;				tmx[10] = 1.0;
	tmx[3] = 0.0;	tmx[7] = 0.0;	tmx[11] = 1.0;

	// build the body structure
	void* bodyData = NULL;
	ACAPI_Body_Create(NULL, NULL, &bodyData);
	if (bodyData == NULL) {
	ErrorBeep("bodyData == NULL", APIERR_MEMFULL);
	return;
	}

	// define the vertices
	double dx = 3.0, dy = 2.0, dz = 1.0;	// the dimensions of the morph element to be created
	API_Coord3D coords[] = { { 0.0, 0.0, 0.0 },	// #1
	{ dx, 0.0, 0.0 },	// #2
	{ 0.0, dy, 0.0 },	// #3
	{ 0.0, 0.0, dz },	// #4
	{ dx / 4.0, 0.0, dz / 4.0 },	// #5
	{ dx / 2.0, 0.0, dz / 4.0 },	// #6
	{ dx / 4.0, 0.0, dz / 2.0 },	// #7
	{ 0.0, dy / 2.0, dz / 4.0 },	// #8
	{ 0.0, dy / 4.0, dz / 4.0 },	// #9
	{ 0.0, dy / 4.0, dz / 2.0 } };	// #10
	UInt32 vertices[10];
	for (UIndex i = 0; i < 10; i++)
	ACAPI_Body_AddVertex(bodyData, coords[i], vertices[i]);

	// connect the vertices to determine edges
	Int32 edges[15];
	ACAPI_Body_AddEdge(bodyData, vertices[0], vertices[1], edges[0]);
	ACAPI_Body_AddEdge(bodyData, vertices[0], vertices[2], edges[1]);
	ACAPI_Body_AddEdge(bodyData, vertices[0], vertices[3], edges[2]);
	ACAPI_Body_AddEdge(bodyData, vertices[1], vertices[2], edges[3]);
	ACAPI_Body_AddEdge(bodyData, vertices[1], vertices[3], edges[4]);
	ACAPI_Body_AddEdge(bodyData, vertices[2], vertices[3], edges[5]);

	ACAPI_Body_AddEdge(bodyData, vertices[4], vertices[5], edges[6]);
	ACAPI_Body_AddEdge(bodyData, vertices[5], vertices[6], edges[7]);
	ACAPI_Body_AddEdge(bodyData, vertices[6], vertices[4], edges[8]);

	ACAPI_Body_AddEdge(bodyData, vertices[7], vertices[8], edges[9]);
	ACAPI_Body_AddEdge(bodyData, vertices[8], vertices[9], edges[10]);
	ACAPI_Body_AddEdge(bodyData, vertices[9], vertices[7], edges[11]);

	ACAPI_Body_AddEdge(bodyData, vertices[4], vertices[8], edges[12]);
	ACAPI_Body_AddEdge(bodyData, vertices[6], vertices[9], edges[13]);
	ACAPI_Body_AddEdge(bodyData, vertices[5], vertices[7], edges[14]);

	// add polygon normal vector
	Int32	polyNormals[1];
	API_Vector3D normal;
	normal.x = normal.z = 0.0;
	normal.y = 1.0;
	ACAPI_Body_AddPolyNormal(bodyData, normal, polyNormals[0]);

	// determine polygons from edges
	API_MaterialOverrideType material;
	material.overrideMaterial = true;

	UInt32 polygons[7];
	Int32 polyEdges[7];
	polyEdges[0] = edges[0];
	polyEdges[1] = edges[4];
	polyEdges[2] = -edges[2];
	polyEdges[3] = 0;
	polyEdges[4] = -edges[8];
	polyEdges[5] = -edges[7];
	polyEdges[6] = -edges[6];
	material.material = 1;
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 7, -polyNormals[0], material, polygons[0]);

	polyEdges[0] = edges[1];
	polyEdges[1] = -edges[3];
	polyEdges[2] = -edges[0];
	material.material = 2;
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 3, 0, material, polygons[1]);

	polyEdges[0] = -edges[4];
	polyEdges[1] = edges[3];
	polyEdges[2] = edges[5];
	material.material = 3;
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 3, 0, material, polygons[2]);

	polyEdges[0] = edges[2];
	polyEdges[1] = -edges[5];
	polyEdges[2] = -edges[1];
	polyEdges[3] = 0;
	polyEdges[4] = -edges[11];
	polyEdges[5] = -edges[10];
	polyEdges[6] = -edges[9];
	material.material = 4;
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 7, 0, material, polygons[3]);

	material.overrideMaterial = false;

	polyEdges[0] = edges[6];
	polyEdges[1] = edges[14];
	polyEdges[2] = edges[9];
	polyEdges[3] = -edges[12];
	material.material = 5;
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[4]);

	polyEdges[0] = edges[7];
	polyEdges[1] = edges[13];
	polyEdges[2] = edges[11];
	polyEdges[3] = -edges[14];
	material.material = 6;
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[5]);

	polyEdges[0] = -edges[13];
	polyEdges[1] = edges[8];
	polyEdges[2] = edges[12];
	polyEdges[3] = edges[10];
	material.material = 7;
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[6]);

	// close the body and copy it to the memo
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	ACAPI_Body_Finish(bodyData, &memo.morphBody, &memo.morphMaterialMapTable);
	ACAPI_Body_Dispose(&bodyData);

	// create the morph element
	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError)
	ErrorBeep("ACAPI_Element_Create (morph)", err);

	ACAPI_DisposeElemMemoHdls(&memo);

	return;
	*/
	/*
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	char buffer[256];

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_ObjectID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
	ErrorBeep("ACAPI_Element_GetMemo", err);
	//sprintf(buffer, "Default Problem");
	//ACAPI_WriteReport(buffer, true);
	}
	element.object.useXYFixSize = true;
	element.object.xRatio = 2;
	element.object.yRatio = 2;

	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
	ErrorBeep("ACAPI_Element_Create (slab)", err);
	//sprintf(buffer, ErrID_To_Name(err));
	//ACAPI_WriteReport(buffer, true);
	}

	sprintf(buffer, "Index: %d", element.object.libInd);
	ACAPI_WriteReport(buffer, true);

	ACAPI_DisposeElemMemoHdls(&memo);
	*/
	/*
	API_Element element;
	API_ElementMemo memo;
	GSErrCode err;
	char buffer[256];
	API_StoryInfo storyInfo;

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
	ErrorBeep("APIEnv_GetStorySettingsID", err);
	return;
	}

	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_MeshID, &elemList);
	for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate(); it != NULL; ++it) {

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.guid = *it;
	err = ACAPI_Element_Get(&element);
	if (err == NoError) {
	err = ACAPI_Element_GetMemo(element.header.guid, &memo);
	sprintf(buffer, "nCoords: %d", element.mesh.poly.nCoords);
	ACAPI_WriteReport(buffer, true);
	sprintf(buffer, "LevelLines: %d", element.mesh.levelLines.nCoords);
	ACAPI_WriteReport(buffer, true);
	sprintf(buffer, "LevelLinesSub: %d", element.mesh.levelLines.nSubLines);
	ACAPI_WriteReport(buffer, true);
	if (err == NoError){
	for (int i = 1; i <= element.mesh.poly.nCoords; i++){
	//sprintf(buffer, "x: %f y: %f z: %f", (*memo.coords)[i].x, (*memo.coords)[i].y, (*memo.meshPolyZ)[i]);
	//ACAPI_WriteReport(buffer, true);
	}
	//nTriangles = BMGetHandleSize ((GSHandle) coords) / (3 * sizeof (API_Coord));
	for (int i = 0; i < element.mesh.levelLines.nCoords; i++){
	//sprintf(buffer, "x: %f y: %f z: %f", (*memo.meshLevelCoords)[i].c.x, (*memo.meshLevelCoords)[i].c.y, (*memo.meshLevelCoords)[i].c.z);
	//ACAPI_WriteReport(buffer, true);
	}
	for (int i = 0; i < element.mesh.levelLines.nSubLines; i++){
	sprintf(buffer, "levelEnds: %d", (*memo.meshLevelEnds)[i]);
	ACAPI_WriteReport(buffer, true);
	}
	}
	}
	else{
	sprintf(buffer, ErrID_To_Name(err));
	ACAPI_WriteReport(buffer, true);
	}
	}
	*/
	/*
	GSErrCode err;
	API_NeigID neigID;
	API_Element element;
	elementid eleMsg;
	API_StoryCmdType	storyCmd;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &eleMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.guid = APIGuidFromString(eleMsg.guid().c_str());
	err = ACAPI_Element_Get(&element);
	if (err != NoError) {
	sprintf(buffer, ErrID_To_Name(err));
	ACAPI_WriteReport(buffer, true);
	return;
	}

	err = ACAPI_Goodies(APIAny_ElemTypeToNeigID, (void*)element.header.typeID, &neigID);

	API_Neig** neigHdl = (API_Neig**)BMhAll(sizeof(API_Neig));
	API_Neig neig;

	neig.neigID = neigID;
	neig.guid = element.header.guid;
	neig.flags = API_NeigFlg_Normal;
	neig.elemPartType = APINeigElemPart_None;

	**neigHdl = neig;

	Int32 nItem = 1;
	bool add = true;

	//Clear all selected elements
	err = ACAPI_Element_Select(NULL, 0, add);

	//Add the current element to the selection
	err = ACAPI_Element_Select(neigHdl, nItem, add);
	if (err != NoError) {
	sprintf(buffer, ErrID_To_Name(err));
	ACAPI_WriteReport(buffer, true);
	return;
	}

	BMhKill((GSHandle*)&neigHdl);
	*/
	/*
	Geometry::Init();

	Box3DType box;
	box.xMax = 1;
	box.yMax = 1;
	box.zMax = 1;

	box.xMin = 0;
	box.yMin = 0;
	box.zMin = 0;

	Geometry::InitBox3D(&box);

	Geometry::Free();
	*/
	/*
	API_Attribute		profile_attr;
	API_AttributeDefExt	profile_attrdef;
	GSErrCode			err = NoError;

	WriteReport("# Create a profile attribute:");
	WriteReport("# ArchiCAD menus must be updated");
	WriteReport("# Check the \"Profiles...\" dialog after execution");

	BNZeroMemory((GSPtr)&profile_attr, sizeof(API_Attribute));
	BNZeroMemory((GSPtr)&profile_attrdef, sizeof(API_AttributeDefExt));

	profile_attr.header.typeID = API_ProfileID;
	profile_attr.header.flags = 0;
	strcpy(profile_attr.header.name, "Profile from API");
	profile_attr.profile.wallType = true;
	profile_attr.profile.beamType = false;
	profile_attr.profile.coluType = true;

	VectorImage profileDescription;
	BuildProfileDescription(&profileDescription);
	profile_attrdef.profile_vectorImageItems = BMhAll(0);
	try { profileDescription.Export(profile_attrdef.profile_vectorImageItems); }
	catch (const GS::Exception&) {}

	profile_attrdef.profile_vectorImageItems;

	err = ACAPI_Attribute_CreateExt(&profile_attr, &profile_attrdef);
	if (err == APIERR_ATTREXIST) {
	err = ACAPI_Attribute_ModifyExt(&profile_attr, &profile_attrdef);
	}
	ACAPI_DisposeAttrDefsHdlsExt(&profile_attrdef);
	if (err != NoError) {
	WriteReport_Err("Error while creating the profile", err);
	}
	else {
	WriteReport("Profile \"Profile from API\" was created successfully.");
	}

	WriteReport_End(err);

	return;
	*/

	//createTestLibraryPart();

	//getLibraryPartParam();

	//changeParameterOfLibPart();

	profileTest();

	return;

}