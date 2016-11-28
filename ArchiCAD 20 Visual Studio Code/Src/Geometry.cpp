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
#include "basicgeometry.h"
#include "Messages.pb.h"
#include "RosettaArchiCAD.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message_lite.h>
*/

//TODO ELIMINATE THIS PLAGUE!!! element.header.floorInd

void createCircle(){
	API_ElementMemo memo;
	API_Element		circleElement;
	GSErrCode		err;
	circlemessage	circleMsg;

	readDelimitedFrom(getClientSocket(), &circleMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&circleElement, sizeof(API_Element));

	circleElement.circle.origC.x = circleMsg.p0x();
	circleElement.circle.origC.y = circleMsg.p0y();
	circleElement.circle.r = circleMsg.radius();

	circleElement.circle.ratio = 1.0;
	circleElement.header.typeID = API_CircleID;
	circleElement.header.layer = 1;
	//TODO
	circleElement.header.floorInd = 0;

	err = ACAPI_Element_Create(&circleElement, &memo);
	if (err != NoError){
		//createCircleNoCom();
		ErrorBeep("ACAPI_Element_Create (circle)", err);
	}
	sendElementID(getClientSocket(), circleElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createLine(){
	API_Element element;
	GSErrCode		err;
	linemsg msg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &msg);

	BNZeroMemory(&element, sizeof(API_Element));

	element.header.typeID = API_LineID;
	element.header.layer = 1;

	err = ACAPI_Element_GetDefaults(&element, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	element.line.begC.x = msg.pts().px(0);
	element.line.begC.y = msg.pts().py(0);

	element.line.endC.x = msg.pts().px(1);
	element.line.endC.y = msg.pts().py(1);
	
	err = ACAPI_Element_Create(&element, NULL);
	if (hasError(err)){
		quit();
		return;
	}

	sendElementID(getClientSocket(), element);
}

void createPolyLine(){
	API_Element element;
	API_ElementMemo memo;
	GSErrCode		err;
	polylinemsg msg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &msg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_PolyLineID;
	element.header.layer = 1;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	element.polyLine.poly.nCoords = msg.pts().px_size();
	element.polyLine.poly.nSubPolys = 1;
	element.polyLine.poly.nArcs = msg.arcs().arcangle_size();

	populateMemo(&memo, msg.pts(), msg.arcs());

	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.polyLine.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	if (memo.pends == NULL) {
		ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
		ACAPI_DisposeElemMemoHdls(&memo);
		quit();
		return;
	}
	
	(*memo.pends)[1] = element.polyLine.poly.nCoords;

	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (spline)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		//sprintf(buffer, "No creation");
		//ACAPI_WriteReport(buffer, true);
	}

	sendElementID(getClientSocket(), element);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createSpline(){
	API_Element element;
	API_ElementMemo memo;
	GSErrCode		err;
	splinemsg msg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &msg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_SplineID;
	element.header.layer = 1;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	element.spline.autoSmooth = true;
	element.spline.closed = msg.closed();

	memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((msg.points().px_size() + 0) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	if (memo.coords == NULL) {
		ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
		ACAPI_DisposeElemMemoHdls(&memo);
	}

	for (int i = 0; i < msg.points().px_size(); i++){
		(*memo.coords)[i].x = msg.points().px(i);
		(*memo.coords)[i].y = msg.points().py(i);
	}

	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (spline)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		//sprintf(buffer, "No creation");
		//ACAPI_WriteReport(buffer, true);
	}

	sendElementID(getClientSocket(), element);

	ACAPI_DisposeElemMemoHdls(&memo);
	/*
	API_Element slab;
	API_ElementMemo slabMemo;

	BNZeroMemory(&slab, sizeof(API_Element));
	BNZeroMemory(&slabMemo, sizeof(API_ElementMemo));

	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	ACAPI_Element_GetMemo(element.header.guid, &memo);

	slab.header.typeID = API_SlabID;

	err = ACAPI_Element_GetDefaults(&slab, &slabMemo);
	if (err != NoError) {
	ErrorBeep("ACAPI_Element_GetMemo", err);
	return;
	}

	slab.slab.poly.nCoords = msg.points().px_size() + 1;
	slab.slab.poly.nSubPolys = 1;
	//slab.slab.poly.nArcs = 0;
	slab.slab.poly.nArcs = BMGetHandleSize((GSHandle)memo.bezierDirs) / sizeof(API_SplineDir);

	slabMemo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((slab.slab.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	slabMemo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((slab.slab.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	slabMemo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((slab.slab.poly.nArcs) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	(*slabMemo.coords)[0].x = 0;
	(*slabMemo.coords)[0].y = 0;

	for (int i = 0; i < msg.points().px_size(); i++){
	(*slabMemo.coords)[i + 1].x = msg.points().px(i);
	(*slabMemo.coords)[i + 1].y = msg.points().py(i);
	}
	(*slabMemo.coords)[msg.points().px_size() + 1].x = msg.points().px(0);
	(*slabMemo.coords)[msg.points().px_size() + 1].y = msg.points().py(0);


	for (int i = 0; i < slab.slab.poly.nArcs; i++){
	(*slabMemo.parcs)[i].arcAngle = (*memo.bezierDirs)[i].dirAng;
	(*slabMemo.parcs)[i].begIndex = i + 1;
	(*slabMemo.parcs)[i].endIndex = i + 2;
	sprintf(buffer, "bezierDirs.dirAng: %f", (*memo.bezierDirs)[i].dirAng);
	ACAPI_WriteReport(buffer, true);
	}


	//(*slabMemo.pends)[0] = 0;
	(*slabMemo.pends)[1] = slab.slab.poly.nCoords;

	err = ACAPI_Element_Create(&slab, &slabMemo);
	if (err != NoError){
	ErrorBeep("ACAPI_Element_Create (spline)", err);
	sprintf(buffer, ErrID_To_Name(err));
	ACAPI_WriteReport(buffer, true);
	//sprintf(buffer, "No creation");
	//ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&slabMemo);
	*/
}

void createArc(){
	API_ElementMemo memo;
	API_Element		arcElement;
	GSErrCode		err;
	arcmessage		arcMsg;

	readDelimitedFrom(getClientSocket(), &arcMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&arcElement, sizeof(API_Element));

	arcElement.arc.origC.x = arcMsg.p0x();
	arcElement.arc.origC.y = arcMsg.p0y();
	arcElement.arc.r = arcMsg.radius();
	arcElement.arc.angle = arcMsg.angle();
	arcElement.arc.begAng = arcMsg.begang();
	arcElement.arc.endAng = arcMsg.endang();

	arcElement.arc.ratio = 1.0;
	arcElement.header.typeID = API_ArcID;
	arcElement.header.layer = 1;
	//TODO
	arcElement.header.floorInd = 0;

	err = ACAPI_Element_Create(&arcElement, &memo);

	sendElementID(getClientSocket(), arcElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createSphere(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	spheremessage	sphereMsg;
	API_StoryInfo		storyInfo;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &sphereMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&shellElement, sizeof(API_Element));

	shellElement.header.typeID = API_ShellID;
	//shellElement.header.layer = 1;

	shellElement.shell.shellClass = API_RevolvedShellID;

	err = ACAPI_Element_GetDefaults(&shellElement, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (shell)", err);
		return;
	}

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	double levelHeight = (*storyInfo.data)[getCurrentLevel() - storyInfo.firstStory].level;
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));

	shellElement.header.floorInd = sphereMsg.level();

	double* tmx = shellElement.shell.basePlane.tmx;

	tmx[0] = 1.0;				tmx[4] = 0.0;				tmx[8] = 0.0;
	tmx[1] = 0.0;				tmx[5] = 1.0;				tmx[9] = 0.0;
	tmx[2] = 0.0;				tmx[6] = 0.0;				tmx[10] = 1.0;
	//center Coords (x, y, z)
	tmx[3] = sphereMsg.c0x();		tmx[7] = sphereMsg.c0y();		tmx[11] = levelHeight + sphereMsg.c0z();


	shellElement.shell.shellBase.modelElemStructureType = API_BasicStructure;

	shellElement.shell.shellBase.thickness = sphereMsg.radius();

	shellElement.shell.isFlipped = false;

	shellElement.shell.u.revolvedShell.slantAngle = 0;
	shellElement.shell.u.revolvedShell.revolutionAngle = 360 * DEGRAD;
	shellElement.shell.u.revolvedShell.distortionAngle = 90 * DEGRAD;
	shellElement.shell.u.revolvedShell.segmentedSurfaces = false;
	shellElement.shell.u.revolvedShell.segmentType = APIShellBase_SegmentsByCircle;
	shellElement.shell.u.revolvedShell.segmentsByCircle = 36;
	BNZeroMemory(&shellElement.shell.u.revolvedShell.axisBase, sizeof(API_Tranmat));

	shellElement.shell.u.revolvedShell.distortionVector.x = 0.0;
	shellElement.shell.u.revolvedShell.distortionVector.y = 0.0;
	shellElement.shell.u.revolvedShell.begAngle = 0.0;

	// constructing the revolving profile polyline
	shellElement.shell.u.revolvedShell.shellShape.nCoords = 3;
	shellElement.shell.u.revolvedShell.shellShape.nSubPolys = 1;
	shellElement.shell.u.revolvedShell.shellShape.nArcs = 1;


	memo.shellShapes[0].coords = (API_Coord**)BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(shellElement.shell.u.revolvedShell.shellShape.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof(GS::Bool8), ALLOCATE_CLEAR, 0);
	if (memo.shellShapes[0].coords == NULL || memo.shellShapes[0].pends == NULL || memo.shellShapes[0].parcs == NULL || memo.shellShapes[0].bodyFlags == NULL) {
		ErrorBeep("Not enough memory to create shell polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}

	(*memo.shellShapes[0].coords)[1].x = sphereMsg.radius();
	(*memo.shellShapes[0].coords)[1].y = 0.0;

	(*memo.shellShapes[0].coords)[2].x = -sphereMsg.radius();
	(*memo.shellShapes[0].coords)[2].y = 0.0;

	(*memo.shellShapes[0].coords)[3] = (*memo.shellShapes[0].coords)[1];

	(*memo.shellShapes[0].pends)[1] = shellElement.shell.u.revolvedShell.shellShape.nCoords;

	(*memo.shellShapes[0].parcs)[0].begIndex = 1;
	(*memo.shellShapes[0].parcs)[0].endIndex = 2;
	//this will make a sphere
	(*memo.shellShapes[0].parcs)[0].arcAngle = PI;

	(*memo.shellShapes[0].bodyFlags)[1] = true;
	(*memo.shellShapes[0].bodyFlags)[2] = true;
	(*memo.shellShapes[0].bodyFlags)[3] = (*memo.shellShapes[0].bodyFlags)[1];

	// constructing the shell contour data
	shellElement.shell.hasContour = false;		// this shell will not be clipped
	shellElement.shell.numHoles = 0;

	// create the shell element
	err = ACAPI_Element_Create(&shellElement, &memo);
	if (err != NoError){
		//createCircle();
		ErrorBeep("ACAPI_Element_Create (revolved)", err);
	}

	sendElementID(getClientSocket(), shellElement);
	//createdElement = true;
	//elementCreated.header.guid = shellElement.header.guid;

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createCylinder(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	cylindermsg	cylinderMsg;
	API_StoryInfo		storyInfo;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &cylinderMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&shellElement, sizeof(API_Element));

	shellElement.header.typeID = API_ShellID;
	//shellElement.header.layer = 1;

	shellElement.shell.shellClass = API_ExtrudedShellID;

	err = ACAPI_Element_GetDefaults(&shellElement, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (shell)", err);
		return;
	}

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}
	//TODO
	double levelHeight = (*storyInfo.data)[getCurrentLevel() - storyInfo.firstStory].level;
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));

	shellElement.header.floorInd = cylinderMsg.level();

	double* tmx = shellElement.shell.basePlane.tmx;

	tmx[0] = 1.0;				tmx[4] = 0.0;				tmx[8] = 0.0;
	tmx[1] = 0.0;				tmx[5] = 1.0;				tmx[9] = 0.0;
	tmx[2] = 0.0;				tmx[6] = 0.0;				tmx[10] = 1.0;
	//center Coords (x, y, z)
	//tmx[3] = (cylinderMsg.p0x() + cylinderMsg.p1x()) / 2.0;		tmx[7] = (cylinderMsg.p0y() + cylinderMsg.p1y()) / 2.0;		tmx[11] = (cylinderMsg.p0z() + cylinderMsg.p1z()) / 2.0;
	//tmx[3] = cylinderMsg.p0x();		tmx[7] = cylinderMsg.p0y();		tmx[11] = cylinderMsg.p0z();
	tmx[3] = 0.0;		tmx[7] = 0.0;		tmx[11] = 0.0;

	shellElement.shell.isFlipped = false;

	shellElement.shell.shellBase.buildingMaterial = searchBuildingMaterials("GENERIC - EXTERNAL CLADDING");
	shellElement.shell.shellBase.modelElemStructureType = API_BasicStructure;
	shellElement.shell.shellBase.thickness = cylinderMsg.radius();

	shellElement.shell.u.extrudedShell.begC.x = cylinderMsg.p0x();
	shellElement.shell.u.extrudedShell.begC.y = cylinderMsg.p0y();
	shellElement.shell.u.extrudedShell.begC.z = cylinderMsg.p0z();
	//shellElement.shell.u.extrudedShell.slantAngle = 0;
	//shellElement.shell.u.extrudedShell.shapePlaneTilt = 0;
	//shellElement.shell.u.extrudedShell.begPlaneTilt = 0;
	//shellElement.shell.u.extrudedShell.endPlaneTilt = 0;
	double vx = cylinderMsg.p1x() - cylinderMsg.p0x();
	double vy = cylinderMsg.p1y() - cylinderMsg.p0y();
	double vz = cylinderMsg.p1z() - cylinderMsg.p0z();

	double vLength = sqrtf(pow(vx, 2) + pow(vy, 2) + pow(vz, 2));
	shellElement.shell.u.extrudedShell.extrusionVector.x = vx;
	shellElement.shell.u.extrudedShell.extrusionVector.y = vy;
	shellElement.shell.u.extrudedShell.extrusionVector.z = vz;

	// constructing the revolving profile polyline
	shellElement.shell.u.extrudedShell.shellShape.nCoords = 3;
	shellElement.shell.u.extrudedShell.shellShape.nSubPolys = 1;
	shellElement.shell.u.extrudedShell.shellShape.nArcs = 2;

	memo.shellShapes[0].coords = (API_Coord**)BMAllocateHandle((shellElement.shell.u.extrudedShell.shellShape.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((shellElement.shell.u.extrudedShell.shellShape.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(shellElement.shell.u.extrudedShell.shellShape.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle((shellElement.shell.u.extrudedShell.shellShape.nCoords + 1) * sizeof(GS::Bool8), ALLOCATE_CLEAR, 0);
	if (memo.shellShapes[0].coords == NULL || memo.shellShapes[0].pends == NULL || memo.shellShapes[0].parcs == NULL || memo.shellShapes[0].bodyFlags == NULL) {
		ErrorBeep("Not enough memory to create shell polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}

	(*memo.shellShapes[0].coords)[1].x = cylinderMsg.radius();
	(*memo.shellShapes[0].coords)[1].y = 0.0;
	(*memo.shellShapes[0].coords)[2].x = -cylinderMsg.radius();
	(*memo.shellShapes[0].coords)[2].y = 0.0;
	(*memo.shellShapes[0].coords)[3] = (*memo.shellShapes[0].coords)[1];

	(*memo.shellShapes[0].pends)[1] = shellElement.shell.u.extrudedShell.shellShape.nCoords;
	(*memo.shellShapes[0].parcs)[0].begIndex = 1;
	(*memo.shellShapes[0].parcs)[0].endIndex = 2;
	(*memo.shellShapes[0].parcs)[0].arcAngle = PI;
	(*memo.shellShapes[0].parcs)[1].begIndex = 2;
	(*memo.shellShapes[0].parcs)[1].endIndex = 3;
	(*memo.shellShapes[0].parcs)[1].arcAngle = PI;

	(*memo.shellShapes[0].bodyFlags)[1] = true;
	(*memo.shellShapes[0].bodyFlags)[2] = true;
	(*memo.shellShapes[0].bodyFlags)[3] = (*memo.shellShapes[0].bodyFlags)[1];

	// constructing the shell contour data
	shellElement.shell.hasContour = false;		// this shell will not be clipped
	shellElement.shell.numHoles = 0;

	// create the shell element
	err = ACAPI_Element_Create(&shellElement, &memo);
	if (err != NoError){
		//createCircle();
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		ErrorBeep("ACAPI_Element_Create (revolved)", err);
	}

	sendElementID(getClientSocket(), shellElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

//Create Morph - change name to createMorph to make it available
void createMorph(){
	API_Element		element;
	GSErrCode		err;
	morphmsg		morphMsg;
	pointsmessage	pointsMsg;
	pointsmessage	edgesMsg;
	pointsmessage	polygonsMsg;
	intlistmsg		polygonsSizes;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &morphMsg);
	/*
	readDelimitedFrom(getClientSocket(), &pointsMsg);
	readDelimitedFrom(getClientSocket(), &edgesMsg);
	readDelimitedFrom(getClientSocket(), &polygonsMsg);
	readDelimitedFrom(getClientSocket(), &polygonsSizes);
	*/
	pointsMsg = morphMsg.pts();
	edgesMsg = morphMsg.edges();
	polygonsMsg = morphMsg.polygons();
	polygonsSizes = morphMsg.sizespolygons();

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.typeID = API_MorphID;
	err = ACAPI_Element_GetDefaults(&element, NULL);
	if (hasError(err)){
		quit(); return;
	}

	element.header.floorInd = morphMsg.level();
	element.morph.level = 0;
	element.morph.bodyType = APIMorphBodyType_SolidBody;

	element.morph.buildingMaterial = searchBuildingMaterials(morphMsg.material());

	double* tmx = element.morph.tranmat.tmx;
	tmx[0] = 1.0; tmx[4] = 0.0; tmx[8] = 0.0;
	tmx[1] = 0.0; tmx[5] = 1.0; tmx[9] = 0.0;
	tmx[2] = 0.0; tmx[6] = 0.0;	tmx[10] = 1.0;
	//tmx[3] = 0.0; tmx[7] = 0.0; tmx[11] = 0.0;
	tmx[3] = morphMsg.refx(); tmx[7] = morphMsg.refy(); tmx[11] = morphMsg.refz();

	// build the body structure
	void* bodyData = NULL;
	ACAPI_Body_Create(NULL, NULL, &bodyData);
	if (bodyData == NULL) {
		ErrorBeep("bodyData == NULL", APIERR_MEMFULL);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		quit();
		return;
	}

	// define the vertices
	// the dimensions of the morph element to be created
	const int numVertices = pointsMsg.px_size();
	//UInt32* vertices = reinterpret_cast<UInt32*>(BMAllocateHandle(numVertices * sizeof(UInt32), ALLOCATE_CLEAR, 0));
	//API_Coord3D* coords = reinterpret_cast<API_Coord3D*>(BMAllocateHandle(numVertices * sizeof(API_Coord3D), ALLOCATE_CLEAR, 0));
	UInt32* vertices = new UInt32[numVertices];
	API_Coord3D* coords = new API_Coord3D[numVertices];

	for (int i = 0; i < numVertices; i++){
		//API_Coord3D coord = {pointsMsg.px(i), pointsMsg.py(i), pointsMsg.pz(i)};
		coords[i].x = pointsMsg.px(i);
		coords[i].y = pointsMsg.py(i);
		coords[i].z = pointsMsg.pz(i);
		ACAPI_Body_AddVertex(bodyData, coords[i], vertices[i]);
	}

	// connect the vertices to determine edges
	const int numEdges = edgesMsg.px_size();
	//Int32* edges = (Int32*)BMAllocateHandle(numEdges * sizeof(Int32), ALLOCATE_CLEAR, 0);
	Int32* edges = new Int32[numEdges];
	for (int i = 0; i < numEdges; i++){
		ACAPI_Body_AddEdge(bodyData, vertices[(int)edgesMsg.px(i)], vertices[(int)edgesMsg.py(i)], edges[i]);
	}

	// determine polygons from edges
	API_OverriddenAttribute material;
	material.overridden = false;
	material.attributeIndex = 1;
	int max = 0;
	for (int i = 0; i < polygonsSizes.ilist_size(); i++){
		if (max < polygonsSizes.ilist(i)){
			max = polygonsSizes.ilist(i);
		}
	}

	//UInt32* polygons = (UInt32*)BMAllocateHandle(polygonsSizes.ilist_size() * sizeof(UInt32), ALLOCATE_CLEAR, 0);
	//Int32* polyEdges = (Int32*)BMAllocateHandle(max * sizeof(Int32), ALLOCATE_CLEAR, 0);

	UInt32* polygons = new UInt32[polygonsSizes.ilist_size()];
	Int32* polyEdges = new Int32[max];

	//sprintf(buffer, "size: %d", polygonsSizes.ilist_size());
	//ACAPI_WriteReport(buffer, true);

	int counter = 0;
	for (int i = 0; i < polygonsSizes.ilist_size(); i++){
		for (int j = 0; j < polygonsSizes.ilist(i); j++){
			if (polygonsMsg.py(j + counter) == 0){
				polyEdges[j] = edges[(int)polygonsMsg.px(j + counter)];
			}
			else{
				polyEdges[j] = -edges[(int)polygonsMsg.px(j + counter)];
			}
		}
		ACAPI_Body_AddPolygon(bodyData, polyEdges, polygonsSizes.ilist(i), 0, material, polygons[i]);
		counter += polygonsSizes.ilist(i);
	}

	// close the body and copy it to the memo
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	ACAPI_Body_Finish(bodyData, &memo.morphBody, &memo.morphMaterialMapTable);
	ACAPI_Body_Dispose(&bodyData);

	// create the morph element
	err = ACAPI_Element_Create(&element, &memo);

	if (hasError(err)){
		quit(); return;
	}

	sendElementID(getClientSocket(), element);

	ACAPI_DisposeElemMemoHdls(&memo);
	delete(vertices);
	delete(edges);
	delete(polygons);
	delete(polyEdges);
	delete(coords);
	return;

}

//Used for testing - name change
void testcreateMorph(){
	API_Element		element;
	GSErrCode		err;
	morphmsg		morphMsg;
	pointsmessage	pointsMsg;
	pointsmessage	edgesMsg;
	pointsmessage	polygonsMsg;
	intlistmsg		polygonsSizes;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &morphMsg);
	readDelimitedFrom(getClientSocket(), &pointsMsg);
	readDelimitedFrom(getClientSocket(), &edgesMsg);
	readDelimitedFrom(getClientSocket(), &polygonsMsg);
	readDelimitedFrom(getClientSocket(), &polygonsSizes);

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.typeID = API_MorphID;
	err = ACAPI_Element_GetDefaults(&element, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (morph)", err);
		return;
	}

	element.header.floorInd = 0;
	element.morph.level = 0;
	element.morph.bodyType = APIMorphBodyType_SolidBody;

	double* tmx = element.morph.tranmat.tmx;
	tmx[0] = 1.0; tmx[4] = 0.0; tmx[8] = 0.0;
	tmx[1] = 0.0; tmx[5] = 1.0; tmx[9] = 0.0;
	tmx[2] = 0.0; tmx[6] = 0.0;	tmx[10] = 1.0;
	//tmx[3] = 0.0; tmx[7] = 0.0; tmx[11] = 0.0;
	tmx[3] = morphMsg.refx(); tmx[7] = morphMsg.refy(); tmx[11] = morphMsg.refz();

	// build the body structure
	void* bodyData = NULL;
	ACAPI_Body_Create(NULL, NULL, &bodyData);
	if (bodyData == NULL) {
		ErrorBeep("bodyData == NULL", APIERR_MEMFULL);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		return;
	}

	// define the vertices
	// the dimensions of the morph element to be created
	const int numVertices = 12;
	//const int numVertices = 8;
	//UInt32* vertices = reinterpret_cast<UInt32*>(BMAllocateHandle(numVertices * sizeof(UInt32), ALLOCATE_CLEAR, 0));
	//API_Coord3D* coords = reinterpret_cast<API_Coord3D*>(BMAllocateHandle(numVertices * sizeof(API_Coord3D), ALLOCATE_CLEAR, 0));
	UInt32* vertices = new UInt32[numVertices];
	API_Coord3D* coords = new API_Coord3D[numVertices];

	coords[0].x = 0.0;
	coords[0].y = 0.0;
	coords[0].z = 0.0;
	coords[1].x = 2.0;
	coords[1].y = 0.0;
	coords[1].z = 0.0;
	coords[2].x = 2.0;
	coords[2].y = 2.0;
	coords[2].z = 0.0;
	coords[3].x = 0.0;
	coords[3].y = 2.0;
	coords[3].z = 0.0;

	coords[4].x = 1.0;
	coords[4].y = 0.0;
	coords[4].z = 2.0;
	coords[5].x = 3.0;
	coords[5].y = 0.0;
	coords[5].z = 2.0;
	coords[6].x = 3.0;
	coords[6].y = 2.0;
	coords[6].z = 2.0;
	coords[7].x = 1.0;
	coords[7].y = 2.0;
	coords[7].z = 2.0;

	coords[8].x = 0.0;
	coords[8].y = 0.0;
	coords[8].z = 4.0;
	coords[9].x = 2.0;
	coords[9].y = 0.0;
	coords[9].z = 4.0;
	coords[10].x = 2.0;
	coords[10].y = 2.0;
	coords[10].z = 4.0;
	coords[11].x = 0.0;
	coords[11].y = 2.0;
	coords[11].z = 4.0;


	for (int i = 0; i < numVertices; i++){
		ACAPI_Body_AddVertex(bodyData, coords[i], vertices[i]);
	}

	// connect the vertices to determine edges
	const int numEdges = 16;
	//const int numEdges = 12;
	//Int32* edges = (Int32*)BMAllocateHandle(numEdges * sizeof(Int32), ALLOCATE_CLEAR, 0);
	Int32* edges = new Int32[numEdges];

	//bottom
	ACAPI_Body_AddEdge(bodyData, vertices[0], vertices[1], edges[0]);
	ACAPI_Body_AddEdge(bodyData, vertices[1], vertices[2], edges[1]);
	ACAPI_Body_AddEdge(bodyData, vertices[2], vertices[3], edges[2]);
	ACAPI_Body_AddEdge(bodyData, vertices[3], vertices[0], edges[3]);

	//top
	ACAPI_Body_AddEdge(bodyData, vertices[8], vertices[9], edges[4]);
	ACAPI_Body_AddEdge(bodyData, vertices[9], vertices[10], edges[5]);
	ACAPI_Body_AddEdge(bodyData, vertices[10], vertices[11], edges[6]);
	ACAPI_Body_AddEdge(bodyData, vertices[11], vertices[8], edges[7]);

	//first stretch
	ACAPI_Body_AddEdge(bodyData, vertices[0], vertices[4], edges[8]);
	ACAPI_Body_AddEdge(bodyData, vertices[1], vertices[5], edges[9]);
	ACAPI_Body_AddEdge(bodyData, vertices[2], vertices[6], edges[10]);
	ACAPI_Body_AddEdge(bodyData, vertices[3], vertices[7], edges[11]);

	//second stretch
	ACAPI_Body_AddEdge(bodyData, vertices[4], vertices[8], edges[12]);
	ACAPI_Body_AddEdge(bodyData, vertices[5], vertices[9], edges[13]);
	ACAPI_Body_AddEdge(bodyData, vertices[6], vertices[10], edges[14]);
	ACAPI_Body_AddEdge(bodyData, vertices[7], vertices[11], edges[15]);

	/*
	ACAPI_Body_AddEdge(bodyData, vertices[4], vertices[5], edges[4]);
	ACAPI_Body_AddEdge(bodyData, vertices[5], vertices[6], edges[5]);
	ACAPI_Body_AddEdge(bodyData, vertices[6], vertices[7], edges[6]);
	ACAPI_Body_AddEdge(bodyData, vertices[7], vertices[4], edges[7]);

	ACAPI_Body_AddEdge(bodyData, vertices[0], vertices[4], edges[8]);
	ACAPI_Body_AddEdge(bodyData, vertices[1], vertices[5], edges[9]);
	ACAPI_Body_AddEdge(bodyData, vertices[2], vertices[6], edges[10]);
	ACAPI_Body_AddEdge(bodyData, vertices[3], vertices[7], edges[11]);
	*/


	// determine polygons from edges
	API_OverriddenAttribute material;
	material.overridden = false;
	material.attributeIndex = 1;

	//UInt32* polygons = (UInt32*)BMAllocateHandle(polygonsSizes.ilist_size() * sizeof(UInt32), ALLOCATE_CLEAR, 0);
	//Int32* polyEdges = (Int32*)BMAllocateHandle(max * sizeof(Int32), ALLOCATE_CLEAR, 0);

	UInt32* polygons = new UInt32[1];

	Int32 polyEdges[] = {
		-edges[0],
		-edges[3],
		-edges[2],
		-edges[1],

		edges[4],
		edges[5],
		edges[6],
		edges[7],

		edges[0],
		edges[9],
		edges[13],
		-edges[4],
		-edges[12],
		-edges[8],

		edges[1],
		edges[10],
		edges[14],
		-edges[5],
		-edges[13],
		-edges[9],

		edges[2],
		edges[11],
		edges[15],
		-edges[6],
		-edges[14],
		-edges[10],

		edges[3],
		edges[8],
		edges[12],
		-edges[7],
		-edges[15],
		-edges[11]
	};


	/*
	Int32 polyEdges[] = {
	-edges[0],
	-edges[3],
	-edges[2],
	-edges[1],

	edges[4],
	edges[5],
	edges[6],
	edges[7],

	edges[0],
	edges[9],
	-edges[4],
	-edges[8],

	edges[1],
	edges[10],
	-edges[5],
	-edges[9],

	edges[2],
	edges[11],
	-edges[6],
	-edges[10],

	edges[3],
	edges[8],
	-edges[7],
	-edges[11]
	};
	*/
	//ACAPI_Body_AddPolygon(bodyData, polyEdges, 32, 0, material, polygons[0]);
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 24, 0, material, polygons[0]);

	// close the body and copy it to the memo
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	ACAPI_Body_Finish(bodyData, &memo.morphBody, &memo.morphMaterialMapTable);
	ACAPI_Body_Dispose(&bodyData);

	// create the morph element
	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (morph)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	sendElementID(getClientSocket(), element);

	ACAPI_DisposeElemMemoHdls(&memo);
	delete(vertices);
	delete(edges);
	delete(polygons);
	delete(coords);
	return;

}

//NOT USED
void createBox(){
	API_Element		element;
	GSErrCode		err;
	boxmsg			boxMsg;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &boxMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.typeID = API_MorphID;
	err = ACAPI_Element_GetDefaults(&element, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (morph)", err);
		return;
	}

	double x1 = boxMsg.x1();
	double y1 = boxMsg.y1();
	double z1 = boxMsg.z1();

	double x2 = boxMsg.x2();
	double y2 = boxMsg.y2();
	double z2 = boxMsg.z2();

	double* tmx = element.morph.tranmat.tmx;
	tmx[0] = 1.0; tmx[4] = 0.0; tmx[8] = 0.0;
	tmx[1] = 0.0; tmx[5] = 1.0; tmx[9] = 0.0;
	tmx[2] = 0.0; tmx[6] = 0.0;	tmx[10] = 1.0;
	tmx[3] = 0.0; tmx[7] = 0.0; tmx[11] = 0.0;

	// build the body structure
	void* bodyData = NULL;
	ACAPI_Body_Create(NULL, NULL, &bodyData);
	if (bodyData == NULL) {
		ErrorBeep("bodyData == NULL", APIERR_MEMFULL);
		return;
	}

	element.header.floorInd = boxMsg.bottomlevel();

	element.morph.level = 0;

	element.morph.bodyType = APIMorphBodyType_SolidBody;

	// define the vertices
	// the dimensions of the morph element to be created
	UInt32 vertices[8];

	API_Coord3D coord[] = {
		{ x1, y1, z1 },
		{ x2, y1, z1 },
		{ x2, y2, z1 },
		{ x1, y2, z1 },
		{ x1, y1, z2 },
		{ x2, y1, z2 },
		{ x2, y2, z2 },
		{ x1, y2, z2 }
	};
	/*
	API_Coord3D coord[] = {
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 1, 1, 0 },
	{ 0, 1, 0 },
	{ 0, 0, 1 },
	{ 1, 0, 1 },
	{ 1, 1, 1 },
	{ 0, 1, 1 }
	};
	*/
	for (int i = 0; i < 8; i++){
		ACAPI_Body_AddVertex(bodyData, coord[i], vertices[i]);
	}

	// connect the vertices to determine edges
	Int32 edges[12];

	//base
	ACAPI_Body_AddEdge(bodyData, vertices[0], vertices[1], edges[0]); //0
	ACAPI_Body_AddEdge(bodyData, vertices[1], vertices[2], edges[1]); //1
	ACAPI_Body_AddEdge(bodyData, vertices[2], vertices[3], edges[2]); //2
	ACAPI_Body_AddEdge(bodyData, vertices[3], vertices[0], edges[3]); //3

	//top
	ACAPI_Body_AddEdge(bodyData, vertices[4], vertices[5], edges[4]); //4
	ACAPI_Body_AddEdge(bodyData, vertices[5], vertices[6], edges[5]); //5
	ACAPI_Body_AddEdge(bodyData, vertices[6], vertices[7], edges[6]); //6
	ACAPI_Body_AddEdge(bodyData, vertices[7], vertices[4], edges[7]); //7

	//sides
	ACAPI_Body_AddEdge(bodyData, vertices[0], vertices[4], edges[8]); //8
	ACAPI_Body_AddEdge(bodyData, vertices[1], vertices[5], edges[9]); //9
	ACAPI_Body_AddEdge(bodyData, vertices[2], vertices[6], edges[10]); //10
	ACAPI_Body_AddEdge(bodyData, vertices[3], vertices[7], edges[11]); //11

	// add polygon normal vector
	Int32	polyNormals[3];
	API_Vector3D normal;

	normal.x = 1.0;
	normal.y = 0.0;
	normal.z = 0.0;
	ACAPI_Body_AddPolyNormal(bodyData, normal, polyNormals[0]);

	normal.x = 0.0;
	normal.y = 1.0;
	normal.z = 0.0;
	ACAPI_Body_AddPolyNormal(bodyData, normal, polyNormals[1]);

	normal.x = 0.0;
	normal.y = 0.0;
	normal.z = 1.0;
	ACAPI_Body_AddPolyNormal(bodyData, normal, polyNormals[2]);

	// determine polygons from edges
	API_OverriddenAttribute material;
	material.overridden = true;

	UInt32 polygons[6];
	Int32 polyEdges[4];

	polyEdges[0] = edges[0];
	polyEdges[1] = edges[1];
	polyEdges[2] = edges[2];
	polyEdges[3] = edges[3];
	material.attributeIndex = 2;
	//ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, polyNormals[2], material, polygons[0]);
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[0]);

	polyEdges[0] = edges[4];
	polyEdges[1] = edges[5];
	polyEdges[2] = edges[6];
	polyEdges[3] = edges[7];
	material.attributeIndex = 1;
	//ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, polyNormals[2], material, polygons[1]);
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[1]);

	polyEdges[0] = edges[0];
	polyEdges[1] = edges[9];
	polyEdges[2] = -edges[4];
	polyEdges[3] = -edges[8];
	material.attributeIndex = 2;
	//ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, -polyNormals[1], material, polygons[2]);
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[2]);

	polyEdges[0] = edges[1];
	polyEdges[1] = edges[10];
	polyEdges[2] = -edges[5];
	polyEdges[3] = -edges[9];
	material.attributeIndex = 2;
	//ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, polyNormals[0], material, polygons[3]);
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[3]);

	polyEdges[0] = -edges[2];
	polyEdges[1] = edges[10];
	polyEdges[2] = edges[6];
	polyEdges[3] = -edges[11];
	material.attributeIndex = 2;
	//ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, polyNormals[1], material, polygons[4]);
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[4]);

	polyEdges[0] = edges[3];
	polyEdges[1] = edges[8];
	polyEdges[2] = -edges[7];
	polyEdges[3] = -edges[11];
	material.attributeIndex = 2;
	//ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, -polyNormals[0], material, polygons[5]);
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[5]);

	// close the body and copy it to the memo
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	ACAPI_Body_Finish(bodyData, &memo.morphBody, &memo.morphMaterialMapTable);
	ACAPI_Body_Dispose(&bodyData);

	// create the morph element
	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (morph)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}
	sendElementID(getClientSocket(), element);

	ACAPI_DisposeElemMemoHdls(&memo);
	return;
}

void changeMorphTrans(){
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
			t.tmx[i] = element.morph.tranmat.tmx[i];
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
			element.morph.tranmat.tmx[i] = t.tmx[i];
		}

		ACAPI_ELEMENT_MASK_CLEAR(mask);
		ACAPI_ELEMENT_MASK_SET(mask, API_MorphType, tranmat);

		err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
		if (err != NoError){
			ErrorBeep("ACAPI_Element_Create (slab)", err);
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}
		sendElementID(getClientSocket(), element);
	}
}

void applyMatrixMorph(){
	API_Element		element, mask;
	GSErrCode		err;
	applymatrix	msg;
	char buffer[256];
	double a[12];
	double r[12];
	readDelimitedFrom(getClientSocket(), &msg);

	BNZeroMemory(&element, sizeof(API_Element));

	element.header.guid = APIGuidFromString(msg.guid().c_str());

	if (ACAPI_Element_Get(&element) == NoError){

		for (int i = 0; i < 12; i++){
			//element.morph.tranmat.tmx[i] = msg.matrix(i);
			a[i] = msg.matrix(i);
		}

		multMatrix(a, element.morph.tranmat.tmx, r);

		for (int i = 0; i < 12; i++){
			element.morph.tranmat.tmx[i] = r[i];
		}

		ACAPI_ELEMENT_MASK_CLEAR(mask);
		ACAPI_ELEMENT_MASK_SET(mask, API_MorphType, tranmat);

		err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
		if (err != NoError){
			ErrorBeep("ACAPI_Element_Create (slab)", err);
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}
		sendElementID(getClientSocket(), element);
	}
}

//----------- Shells

void createSimpleShell(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	shellcomplexmessage	shellMsg;
	pointsmessage	pointsMsg;
	doublemessage	doubleMsg;

	readDelimitedFrom(getClientSocket(), &shellMsg);


	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&shellElement, sizeof(API_Element));

	shellElement.header.typeID = API_ShellID;
	shellElement.header.layer = 1;


	shellElement.shell.shellClass = API_RevolvedShellID;

	err = ACAPI_Element_GetDefaults(&shellElement, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (shell)", err);
		return;
	}

	shellElement.header.floorInd = getCurrentLevel();

	double* tmx = shellElement.shell.basePlane.tmx;

	/*
	for (int i = 0; i < 12; i++){
	readDelimitedFrom(getClientSocket(), &doubleMsg);
	tmx[i] = doubleMsg.d();
	}
	*/
	shellElement.shell.isFlipped = true;

	shellElement.shell.u.revolvedShell.slantAngle = 0;
	shellElement.shell.u.revolvedShell.revolutionAngle = 360 * DEGRAD;
	shellElement.shell.u.revolvedShell.distortionAngle = 90 * DEGRAD;
	shellElement.shell.u.revolvedShell.segmentedSurfaces = false;
	shellElement.shell.u.revolvedShell.segmentType = APIShellBase_SegmentsByCircle;
	shellElement.shell.u.revolvedShell.segmentsByCircle = 36;
	BNZeroMemory(&shellElement.shell.u.revolvedShell.axisBase, sizeof(API_Tranmat));

	shellElement.shell.u.revolvedShell.axisBase.tmx[0] = 0.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[6] = 1.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[9] = -1.0;

	shellElement.shell.u.revolvedShell.distortionVector.x = 0.0;
	shellElement.shell.u.revolvedShell.distortionVector.y = 0.0;
	shellElement.shell.u.revolvedShell.begAngle = 0.0;

	// constructing the revolving profile polyline
	shellElement.shell.u.revolvedShell.shellShape.nCoords = shellMsg.numpoints();
	shellElement.shell.u.revolvedShell.shellShape.nSubPolys = 1;
	shellElement.shell.u.revolvedShell.shellShape.nArcs = 1;


	memo.shellShapes[0].coords = (API_Coord**)BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(shellElement.shell.u.revolvedShell.shellShape.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof(GS::Bool8), ALLOCATE_CLEAR, 0);
	if (memo.shellShapes[0].coords == NULL || memo.shellShapes[0].pends == NULL || memo.shellShapes[0].parcs == NULL || memo.shellShapes[0].bodyFlags == NULL) {
		ErrorBeep("Not enough memory to create shell polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}

	readDelimitedFrom(getClientSocket(), &pointsMsg);

	for (int i = 1; i <= shellMsg.numpoints(); i++){
		(*memo.shellShapes[0].coords)[i].x = pointsMsg.px(i - 1);
		(*memo.shellShapes[0].coords)[i].y = pointsMsg.py(i - 1);
		(*memo.shellShapes[0].bodyFlags)[i] = true;
	}

	(*memo.shellShapes[0].pends)[1] = shellElement.shell.u.revolvedShell.shellShape.nCoords;

	(*memo.shellShapes[0].parcs)[0].begIndex = 1;
	(*memo.shellShapes[0].parcs)[0].endIndex = 2;
	//this will make a sphere
	(*memo.shellShapes[0].parcs)[0].arcAngle = 4 * 1.570785;

	// constructing the shell contour data
	shellElement.shell.hasContour = false;		// this shell will not be clipped

	//TODO use numHoles in msg
	shellElement.shell.numHoles = 0;

	// create the shell element
	err = ACAPI_Element_Create(&shellElement, &memo);

	if (err != NoError){
		//createCircle();
		ErrorBeep("ACAPI_Element_Create (revolved)", err);
	}

	sendElementID(getClientSocket(), shellElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createShell(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	shellmessage	shellMsg;
	doublemessage	doubleMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;

	readDelimitedFrom(getClientSocket(), &shellMsg);


	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&shellElement, sizeof(API_Element));

	shellElement.header.typeID = API_ShellID;
	shellElement.header.layer = 1;


	shellElement.shell.shellClass = API_RevolvedShellID;

	err = ACAPI_Element_GetDefaults(&shellElement, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (shell)", err);
		WriteReport_End(err);
		return;
	}

	shellElement.header.floorInd = getCurrentLevel();

	double* tmx = shellElement.shell.basePlane.tmx;
	tmx[0] = 1.0; tmx[1] = 0.0; tmx[2] = 0.0; tmx[3] = 0.0;
	tmx[4] = 0.0; tmx[5] = 1.0; tmx[6] = 0.0; tmx[7] = 0.0;
	tmx[8] = 0.0; tmx[9] = 0.0; tmx[10] = 1.0; tmx[11] = 0.0;

	shellElement.shell.isFlipped = true;

	shellElement.shell.u.revolvedShell.slantAngle = 0;
	shellElement.shell.u.revolvedShell.revolutionAngle = 360 * DEGRAD;
	shellElement.shell.u.revolvedShell.distortionAngle = 90 * DEGRAD;
	shellElement.shell.u.revolvedShell.segmentedSurfaces = false;
	shellElement.shell.u.revolvedShell.segmentType = APIShellBase_SegmentsByCircle;
	shellElement.shell.u.revolvedShell.segmentsByCircle = 36;
	BNZeroMemory(&shellElement.shell.u.revolvedShell.axisBase, sizeof(API_Tranmat));

	shellElement.shell.u.revolvedShell.axisBase.tmx[0] = 0.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[6] = 1.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[9] = -1.0;

	shellElement.shell.u.revolvedShell.distortionVector.x = 0.0;
	shellElement.shell.u.revolvedShell.distortionVector.y = 0.0;
	shellElement.shell.u.revolvedShell.begAngle = 0.0;

	// constructing the revolving profile polyline
	shellElement.shell.u.revolvedShell.shellShape.nCoords = shellMsg.numpoints();
	shellElement.shell.u.revolvedShell.shellShape.nSubPolys = 1;
	shellElement.shell.u.revolvedShell.shellShape.nArcs = shellMsg.numarcs();


	memo.shellShapes[0].coords = (API_Coord**)BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(shellElement.shell.u.revolvedShell.shellShape.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof(GS::Bool8), ALLOCATE_CLEAR, 0);
	if (memo.shellShapes[0].coords == NULL || memo.shellShapes[0].pends == NULL || memo.shellShapes[0].parcs == NULL || memo.shellShapes[0].bodyFlags == NULL) {
		ErrorBeep("Not enough memory to create shell polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls(&memo);
		WriteReport_End(APIERR_MEMFULL);
		return;
	}

	readDelimitedFrom(getClientSocket(), &pointsMsg);

	for (int i = 1; i <= shellMsg.numpoints(); i++){
		(*memo.shellShapes[0].coords)[i].x = pointsMsg.px(i - 1);
		(*memo.shellShapes[0].coords)[i].y = pointsMsg.py(i - 1);
		(*memo.shellShapes[0].bodyFlags)[i] = true;
	}
	/*
	for (int i = 1; i <= shellMsg.numpoints(); i++){
	readDelimitedFrom(getClientSocket(), &pointMsg);
	(*memo.shellShapes[0].coords)[i].x = pointMsg.p0x();
	(*memo.shellShapes[0].coords)[i].y = pointMsg.p0y();
	(*memo.shellShapes[0].bodyFlags)[i] = true;
	}
	*/
	(*memo.shellShapes[0].pends)[1] = shellElement.shell.u.revolvedShell.shellShape.nCoords;


	readDelimitedFrom(getClientSocket(), &polyarcsMsg);

	for (int i = 0; i < shellMsg.numarcs(); i++){
		(*memo.shellShapes[0].parcs)[i].begIndex = polyarcsMsg.begindex(i);
		(*memo.shellShapes[0].parcs)[i].endIndex = polyarcsMsg.endindex(i);
		(*memo.shellShapes[0].parcs)[i].arcAngle = polyarcsMsg.arcangle(i);
	}

	/*
	for (int i = 0; i < shellMsg.numarcs(); i++){
	readDelimitedFrom(getClientSocket(), &polyarcMsg);
	(*memo.shellShapes[0].parcs)[i].begIndex = polyarcMsg.begindex();
	(*memo.shellShapes[0].parcs)[i].endIndex = polyarcMsg.endindex();
	(*memo.shellShapes[0].parcs)[i].arcAngle = polyarcMsg.arcangle();
	}
	*/

	// constructing the shell contour data
	shellElement.shell.hasContour = false;		// this shell will not be clipped

	//TODO use numHoles in msg
	shellElement.shell.numHoles = 0;

	// create the shell element
	err = ACAPI_Element_Create(&shellElement, &memo);

	if (err != NoError){
		//createCircle();
		ErrorBeep("ACAPI_Element_Create (revolved)", err);
		WriteReport_End(err);
	}

	sendElementID(getClientSocket(), shellElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createComplexShell(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	shellcomplexmessage	shellMsg;
	pointmessage	pointMsg;
	polyarcmessage	polyarcMsg;
	doublemessage	doubleMsg;

	readDelimitedFrom(getClientSocket(), &shellMsg);


	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&shellElement, sizeof(API_Element));

	shellElement.header.typeID = API_ShellID;
	shellElement.header.layer = 1;


	shellElement.shell.shellClass = API_RevolvedShellID;

	err = ACAPI_Element_GetDefaults(&shellElement, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (shell)", err);
		return;
	}
	shellElement.header.floorInd = getCurrentLevel();

	double* tmx = shellElement.shell.basePlane.tmx;
	for (int i = 0; i < 12; i++){
		readDelimitedFrom(getClientSocket(), &doubleMsg);
		tmx[i] = doubleMsg.d();
	}

	shellElement.shell.isFlipped = true;

	shellElement.shell.u.revolvedShell.slantAngle = 0;
	shellElement.shell.u.revolvedShell.revolutionAngle = 360 * DEGRAD;
	shellElement.shell.u.revolvedShell.distortionAngle = 90 * DEGRAD;
	shellElement.shell.u.revolvedShell.segmentedSurfaces = false;
	shellElement.shell.u.revolvedShell.segmentType = APIShellBase_SegmentsByCircle;
	shellElement.shell.u.revolvedShell.segmentsByCircle = 36;
	BNZeroMemory(&shellElement.shell.u.revolvedShell.axisBase, sizeof(API_Tranmat));


	//TODO check this tmx
	/*
	shellElement.shell.u.revolvedShell.axisBase.tmx[0] = 1.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[6] = 1.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[9] = -1.0;
	*/
	/*
	shellElement.shell.u.revolvedShell.axisBase.tmx[0] = 0.0;
	//reflect y-axis, if 1.0 up, -1.0 down, 0.0 no y move
	shellElement.shell.u.revolvedShell.axisBase.tmx[6] = shellMsg.reflecty();
	shellElement.shell.u.revolvedShell.axisBase.tmx[9] = 0.0;

	//reflect x-axis, if 1.0 right, -1.0 left
	shellElement.shell.u.revolvedShell.axisBase.tmx[3] = shellMsg.reflectx();
	*/
	shellElement.shell.u.revolvedShell.axisBase.tmx[0] = 0.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[6] = 1.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[9] = -1.0;

	shellElement.shell.u.revolvedShell.distortionVector.x = 0.0;
	shellElement.shell.u.revolvedShell.distortionVector.y = 0.0;
	shellElement.shell.u.revolvedShell.begAngle = 0.0;

	// constructing the revolving profile polyline
	shellElement.shell.u.revolvedShell.shellShape.nCoords = shellMsg.numpoints();
	shellElement.shell.u.revolvedShell.shellShape.nSubPolys = 1;
	shellElement.shell.u.revolvedShell.shellShape.nArcs = shellMsg.numarcs();


	memo.shellShapes[0].coords = (API_Coord**)BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(shellElement.shell.u.revolvedShell.shellShape.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof(GS::Bool8), ALLOCATE_CLEAR, 0);
	if (memo.shellShapes[0].coords == NULL || memo.shellShapes[0].pends == NULL || memo.shellShapes[0].parcs == NULL || memo.shellShapes[0].bodyFlags == NULL) {
		ErrorBeep("Not enough memory to create shell polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}

	for (int i = 1; i <= shellMsg.numpoints(); i++){
		readDelimitedFrom(getClientSocket(), &pointMsg);
		(*memo.shellShapes[0].coords)[i].x = pointMsg.p0x();
		(*memo.shellShapes[0].coords)[i].y = pointMsg.p0y();
		(*memo.shellShapes[0].bodyFlags)[i] = true;
	}
	//(*memo.shellShapes[0].coords)[shellMsg.numpoints() + 1] = (*memo.shellShapes[0].coords)[1];
	//(*memo.shellShapes[0].bodyFlags)[shellMsg.numpoints() + 1] = (*memo.shellShapes[0].bodyFlags)[1];
	(*memo.shellShapes[0].pends)[1] = shellElement.shell.u.revolvedShell.shellShape.nCoords;

	for (int i = 0; i < shellMsg.numarcs(); i++){
		readDelimitedFrom(getClientSocket(), &polyarcMsg);
		(*memo.shellShapes[0].parcs)[i].begIndex = polyarcMsg.begindex();
		(*memo.shellShapes[0].parcs)[i].endIndex = polyarcMsg.endindex();
		(*memo.shellShapes[0].parcs)[i].arcAngle = polyarcMsg.arcangle();
	}

	// constructing the shell contour data
	shellElement.shell.hasContour = false;		// this shell will not be clipped



	//TODO use numHoles in msg
	shellElement.shell.numHoles = shellMsg.numholes();

	if (shellElement.shell.numHoles > 0){
		USize nContours = shellElement.shell.numHoles + (shellElement.shell.hasContour ? 1 : 0);
		memo.shellContours = (API_ShellContourData *)BMAllocatePtr(nContours * sizeof(API_ShellContourData), ALLOCATE_CLEAR, 0);
		if (memo.shellContours == NULL) {
			ErrorBeep("Not enough memory to create shell contour data", APIERR_MEMFULL);
			ACAPI_DisposeElemMemoHdls(&memo);
			return;
		}

		memo.shellContours[0].poly.nCoords = shellMsg.numhpoints();
		memo.shellContours[0].poly.nSubPolys = 1;
		memo.shellContours[0].poly.nArcs = shellMsg.numharcs();
		memo.shellContours[0].coords = (API_Coord**)BMAllocateHandle((memo.shellContours[0].poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
		memo.shellContours[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((memo.shellContours[0].poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
		memo.shellContours[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(memo.shellContours[0].poly.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
		if (memo.shellContours[0].coords == NULL || memo.shellContours[0].pends == NULL || memo.shellContours[0].parcs == NULL) {
			ErrorBeep("Not enough memory to create shell contour data", APIERR_MEMFULL);
			ACAPI_DisposeElemMemoHdls(&memo);
			return;
		}

		for (int i = 1; i <= shellMsg.numhpoints(); i++){
			readDelimitedFrom(getClientSocket(), &pointMsg);
			(*memo.shellContours[0].coords)[i].x = pointMsg.p0x();
			(*memo.shellContours[0].coords)[i].y = pointMsg.p0y();
		}

		(*memo.shellContours[0].pends)[1] = memo.shellContours[0].poly.nCoords;

		for (int i = 0; i < shellMsg.numharcs(); i++){
			readDelimitedFrom(getClientSocket(), &polyarcMsg);
			(*memo.shellContours[0].parcs)[i].begIndex = polyarcMsg.begindex();
			(*memo.shellContours[0].parcs)[i].endIndex = polyarcMsg.endindex();
			(*memo.shellContours[0].parcs)[i].arcAngle = polyarcMsg.arcangle();
		}

		memo.shellContours[0].height = shellMsg.holeheight();

		tmx = memo.shellContours[0].plane.tmx;

		for (int i = 0; i < 12; i++){
			readDelimitedFrom(getClientSocket(), &doubleMsg);
			tmx[i] = doubleMsg.d();
		}
		/*
		tmx[0] = 1.0;		tmx[4] = 0.0;		tmx[8] = 0.0;
		tmx[1] = 0.0;		tmx[5] = 1.0;		tmx[9] = 0.0;
		tmx[2] = 0.0;		tmx[6] = 0.0;		tmx[10] = 1.0;
		tmx[3] = 0.0;		tmx[7] = 0.0;		tmx[11] = 10.0;
		*/

	}




	// create the shell element
	err = ACAPI_Element_Create(&shellElement, &memo);



	if (err != NoError){
		//createCircle();
		ErrorBeep("ACAPI_Element_Create (revolved)", err);
	}

	sendElementID(getClientSocket(), shellElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createRevolvedShell(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	char buffer[256];
	revshellmsg	msg;

	readDelimitedFrom(getClientSocket(), &msg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&element, sizeof(API_Element));

	element.header.typeID = API_ShellID;
	element.header.layer = 1;

	element.shell.shellClass = API_RevolvedShellID;

	err = ACAPI_Element_GetDefaults(&element, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (shell)", err);
		WriteReport_End(err);
		sprintf(buffer, "Default Problem");
		ACAPI_WriteReport(buffer, true);
		return;
	}

	element.header.floorInd = msg.level();
	element.shell.shellBase.level = msg.height();

	short material = 0;
	if (msg.type() == "Basic"){
		material = searchBuildingMaterials(msg.material());
		element.shell.shellBase.buildingMaterial = material;
		element.shell.shellBase.modelElemStructureType = API_BasicStructure;
		if (msg.thickness() > 0){
			element.shell.shellBase.thickness = msg.thickness();
		}
	}
	else if (msg.type() == "Composite"){
		material = searchCompositeMaterials(msg.material());
		element.shell.shellBase.modelElemStructureType = API_CompositeStructure;
		element.shell.shellBase.composite = material;
	}

	double* tmx = element.shell.basePlane.tmx;
	tmx[0] = 1.0; tmx[1] = 0.0; tmx[2] = 0.0; tmx[3] = 0.0;
	tmx[4] = 0.0; tmx[5] = 1.0; tmx[6] = 0.0; tmx[7] = 0.0;
	tmx[8] = 0.0; tmx[9] = 0.0; tmx[10] = 1.0; tmx[11] = msg.height();

	element.shell.isFlipped = msg.flipped();

	element.shell.u.revolvedShell.slantAngle = msg.slantangle();
	element.shell.u.revolvedShell.revolutionAngle = msg.revangle();
	element.shell.u.revolvedShell.distortionAngle = msg.distortionangle();
	element.shell.u.revolvedShell.segmentedSurfaces = false;
	element.shell.u.revolvedShell.segmentType = APIShellBase_SegmentsByCircle;
	element.shell.u.revolvedShell.segmentsByCircle = 36;
	BNZeroMemory(&element.shell.u.revolvedShell.axisBase, sizeof(API_Tranmat));

	for (int i = 0; i < msg.axis_size(); i++){
		element.shell.u.revolvedShell.axisBase.tmx[i] = msg.axis(i);
	}

	/*
	element.shell.u.revolvedShell.axisBase.tmx[0] = 1.0;
	element.shell.u.revolvedShell.axisBase.tmx[6] = 1.0;
	element.shell.u.revolvedShell.axisBase.tmx[9] = 1.0;
	*/
	element.shell.u.revolvedShell.begAngle = msg.begangle();

	// constructing the revolving profile polyline
	element.shell.u.revolvedShell.shellShape.nCoords = msg.pts().px_size();
	element.shell.u.revolvedShell.shellShape.nSubPolys = 1;
	element.shell.u.revolvedShell.shellShape.nArcs = msg.arcs().arcangle_size();

	memo.shellShapes[0].coords = (API_Coord**)BMAllocateHandle((element.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(element.shell.u.revolvedShell.shellShape.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	memo.shellShapes[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.shell.u.revolvedShell.shellShape.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle((element.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof(GS::Bool8), ALLOCATE_CLEAR, 0);
	if (memo.shellShapes[0].coords == NULL || memo.shellShapes[0].parcs == NULL || memo.shellShapes[0].pends == NULL || memo.shellShapes[0].bodyFlags == NULL) {
		ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}
	for (int i = 0; i < msg.pts().px_size(); i++){
		(*memo.shellShapes[0].coords)[i + 1].x = msg.pts().px(i);
		(*memo.shellShapes[0].coords)[i + 1].y = msg.pts().py(i);
		(*memo.shellShapes[0].bodyFlags)[i] = true;
	}

	for (int i = 0; i < msg.arcs().arcangle_size(); i++){
		(*memo.shellShapes[0].parcs)[i].begIndex = msg.arcs().begindex(i);
		(*memo.shellShapes[0].parcs)[i].endIndex = msg.arcs().endindex(i);
		(*memo.shellShapes[0].parcs)[i].arcAngle = msg.arcs().arcangle(i);
	}
	
	(*memo.shellShapes[0].pends)[1] = element.shell.u.revolvedShell.shellShape.nCoords;

	// constructing the shell contour data
	element.shell.hasContour = false;		// this shell will not be clipped

	//TODO use numHoles in msg
	element.shell.numHoles = 0;

	// create the shell element
	err = ACAPI_Element_Create(&element, &memo);
	if (hasError(err)){
		quit();
		return;
	}

	sendElementID(getClientSocket(), element);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void createExtrudedShell(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	char buffer[256];
	extshellmsg	msg;

	readDelimitedFrom(getClientSocket(), &msg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&element, sizeof(API_Element));

	element.header.typeID = API_ShellID;
	element.header.layer = 1;

	element.shell.shellClass = API_ExtrudedShellID;

	err = ACAPI_Element_GetDefaults(&element, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (shell)", err);
		WriteReport_End(err);
		sprintf(buffer, "Default Problem");
		ACAPI_WriteReport(buffer, true);
		return;
	}

	element.header.floorInd = msg.level();
	element.shell.shellBase.level = 0;

	double* tmx = element.shell.basePlane.tmx;
	tmx[0] = 1.0; tmx[1] = 0.0; tmx[2] = 0.0; tmx[3] = 0.0;
	tmx[4] = 0.0; tmx[5] = 1.0; tmx[6] = 0.0; tmx[7] = 0.0;
	tmx[8] = 0.0; tmx[9] = 0.0; tmx[10] = 1.0; tmx[11] = 0.0;

	element.shell.isFlipped = msg.flipped();

	short material = 0;
	if (msg.type() == "Basic"){
		material = searchBuildingMaterials(msg.material());
		element.shell.shellBase.buildingMaterial = material;
		element.shell.shellBase.modelElemStructureType = API_BasicStructure;
		if (msg.thickness() > 0){
			element.shell.shellBase.thickness = msg.thickness();
		}
	}
	else if (msg.type() == "Composite"){
		material = searchCompositeMaterials(msg.material());
		element.shell.shellBase.modelElemStructureType = API_CompositeStructure;
		element.shell.shellBase.composite = material;
	}

	element.shell.u.extrudedShell.begC.x = msg.cextx();
	element.shell.u.extrudedShell.begC.y = msg.cexty();
	element.shell.u.extrudedShell.extrusionVector.x = msg.extx();
	element.shell.u.extrudedShell.extrusionVector.y = msg.exty();
	element.shell.u.extrudedShell.extrusionVector.z = msg.extz();

	element.shell.u.extrudedShell.shellShape.nCoords = msg.pts().px_size();
	element.shell.u.extrudedShell.shellShape.nSubPolys = 1;
	element.shell.u.extrudedShell.shellShape.nArcs = msg.arcs().arcangle_size();

	memo.shellShapes[0].coords = (API_Coord**)BMAllocateHandle((element.shell.u.extrudedShell.shellShape.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(element.shell.u.extrudedShell.shellShape.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	memo.shellShapes[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.shell.u.extrudedShell.shellShape.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle((element.shell.u.extrudedShell.shellShape.nCoords + 1) * sizeof(GS::Bool8), ALLOCATE_CLEAR, 0);
	if (memo.shellShapes[0].coords == NULL || memo.shellShapes[0].parcs == NULL || memo.shellShapes[0].pends == NULL || memo.shellShapes[0].bodyFlags == NULL) {
		ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}
	for (int i = 0; i < msg.pts().px_size(); i++){
		(*memo.shellShapes[0].coords)[i + 1].x = msg.pts().px(i);
		(*memo.shellShapes[0].coords)[i + 1].y = msg.pts().py(i);
		if (i < msg.visible_size()){
			(*memo.shellShapes[0].bodyFlags)[i] = msg.visible(i);
		}
		else{
			(*memo.shellShapes[0].bodyFlags)[i] = true;
		}
	}
	
	for (int i = 0; i < msg.arcs().arcangle_size(); i++){
		(*memo.shellShapes[0].parcs)[i].begIndex = msg.arcs().begindex(i);
		(*memo.shellShapes[0].parcs)[i].endIndex = msg.arcs().endindex(i);
		(*memo.shellShapes[0].parcs)[i].arcAngle = msg.arcs().arcangle(i);
	}

	(*memo.shellShapes[0].pends)[1] = element.shell.u.extrudedShell.shellShape.nCoords;

	// constructing the shell contour data
	element.shell.hasContour = false;		// this shell will not be clipped

	//TODO use numHoles in msg
	element.shell.numHoles = 0;

	// create the shell element
	err = ACAPI_Element_Create(&element, &memo);
	if (hasError(err)){
		quit();
		return;
	}

	sendElementID(getClientSocket(), element);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void rotateShell(){
	API_Element		shellElement, mask;
	GSErrCode		err;
	rotshellmessage	rotShellMsg;

	readDelimitedFrom(getClientSocket(), &rotShellMsg);

	BNZeroMemory(&shellElement, sizeof(API_Element));

	shellElement.header.guid = APIGuidFromString(rotShellMsg.guid().c_str());
	if (ACAPI_Element_Get(&shellElement) == NoError) {
		double tmx[12];
		double* tmxBase = shellElement.shell.basePlane.tmx;
		double theta = rotShellMsg.angle() * DEGRAD;

		if (rotShellMsg.axis() == "x" || rotShellMsg.axis() == "X"){

			tmx[0] = 1.0; tmx[1] = 0.0;		   tmx[2] = 0.0;		 tmx[3] = 0.0;
			tmx[4] = 0.0; tmx[5] = cos(theta); tmx[6] = -sin(theta); tmx[7] = 0.0;
			tmx[8] = 0.0; tmx[9] = sin(theta); tmx[10] = cos(theta); tmx[11] = 0.0;
		}
		if (rotShellMsg.axis() == "y" || rotShellMsg.axis() == "Y"){

			tmx[0] = cos(theta);  tmx[1] = 0.0; tmx[2] = sin(theta);	tmx[3] = 0.0;
			tmx[4] = 0.0;		  tmx[5] = 1.0; tmx[6] = 0.0;			tmx[7] = 0.0;
			tmx[8] = -sin(theta); tmx[9] = 0.0; tmx[10] = cos(theta);	tmx[11] = 0.0;
		}
		if (rotShellMsg.axis() == "z" || rotShellMsg.axis() == "Z"){

			tmx[0] = cos(theta); tmx[1] = -sin(theta);	tmx[2] = 0.0;	tmx[3] = 0.0;
			tmx[4] = sin(theta); tmx[5] = cos(theta);	tmx[6] = 0.0;	tmx[7] = 0.0;
			tmx[8] = 0.0;		 tmx[9] = 0.0;		    tmx[10] = 1.0;	tmx[11] = 0.0;
		}

		multTMX(tmxBase, tmx);

		ACAPI_ELEMENT_MASK_CLEAR(mask);
		ACAPI_ELEMENT_MASK_SET(mask, API_ShellType, basePlane);
	}

	err = ACAPI_Element_Change(&shellElement, &mask, NULL, 0, true);

	if (err != NoError){
		//createCircle();
		ErrorBeep("ACAPI_Element_Create (revolved)", err);
	}

	sendElementID(getClientSocket(), shellElement);
}

void translateShell(){
	API_Element		shellElement, mask;
	GSErrCode		err;
	tshellmessage	tShellMsg;

	readDelimitedFrom(getClientSocket(), &tShellMsg);

	BNZeroMemory(&shellElement, sizeof(API_Element));

	shellElement.header.guid = APIGuidFromString(tShellMsg.guid().c_str());
	if (ACAPI_Element_Get(&shellElement) == NoError) {
		double* tmxBase = shellElement.shell.basePlane.tmx;

		tmxBase[3] += tShellMsg.tx();
		tmxBase[7] += tShellMsg.ty();
		tmxBase[11] += tShellMsg.tz();

		ACAPI_ELEMENT_MASK_CLEAR(mask);
		ACAPI_ELEMENT_MASK_SET(mask, API_ShellType, basePlane);
	}

	err = ACAPI_Element_Change(&shellElement, &mask, NULL, 0, true);

	if (err != NoError){
		//createCircle();
		ErrorBeep("ACAPI_Element_Create (revolved)", err);
	}

	sendElementID(getClientSocket(), shellElement);
}

void createHoleInShell(){
	API_Element		shellElement, mask;
	API_ElementMemo memo;
	GSErrCode		err;
	oldholemessage		holeMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	char buffer[256];

	readDelimitedFrom(getClientSocket(), &holeMsg);

	readDelimitedFrom(getClientSocket(), &pointsMsg);

	readDelimitedFrom(getClientSocket(), &polyarcsMsg);

	BNZeroMemory(&shellElement, sizeof(API_Element));
	//BNZeroMemory(&memo, sizeof(API_ElementMemo));

	//Set the element GUID
	shellElement.header.guid = APIGuidFromString(holeMsg.guid().c_str());

	//Get both the element and memo
	if (ACAPI_Element_Get(&shellElement) == NoError
		&& ACAPI_Element_GetMemo(shellElement.header.guid, &memo, APIMemoMask_Polygon) == NoError) {

		API_ShellContourData* oldmemos = memo.shellContours;
		Int32	nold = BMGetPtrSize((GSPtr)(memo.shellContours)) / sizeof(API_ShellContourData);
		memo.shellContours = (API_ShellContourData*)BMAllocatePtr((nold + 1) * sizeof(API_ShellContourData), ALLOCATE_CLEAR, 0);
		if (memo.shellContours != NULL) {

			memo.shellContours[nold].poly.nCoords = pointsMsg.px_size();
			memo.shellContours[nold].poly.nSubPolys = 1;
			memo.shellContours[nold].poly.nArcs = polyarcsMsg.arcangle_size();
			memo.shellContours[nold].coords = (API_Coord**)BMAllocateHandle((memo.shellContours[nold].poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
			memo.shellContours[nold].pends = reinterpret_cast<Int32**> (BMAllocateHandle((memo.shellContours[nold].poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
			memo.shellContours[nold].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(memo.shellContours[nold].poly.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

			if (memo.shellContours[nold].coords == NULL || memo.shellContours[nold].pends == NULL || memo.shellContours[nold].parcs == NULL) {
				ErrorBeep("Not enough memory to create shell contour data", APIERR_MEMFULL);
			}

			Int32	i;
			for (i = 0; i < nold; i++) {
				memo.shellContours[i] = oldmemos[i];
			}

			//memo.shellContours[nold].poly.nCoords = poly.nCoords;
			for (int i = 1; i <= memo.shellContours[nold].poly.nCoords; i++){
				(*memo.shellContours[nold].coords)[i].x = pointsMsg.px(i - 1);
				(*memo.shellContours[nold].coords)[i].y = pointsMsg.py(i - 1);
			}

			memo.shellContours[nold].poly.nSubPolys = 1;
			(*memo.shellContours[nold].pends)[1] = memo.shellContours[nold].poly.nCoords;

			//memo.shellContours[nold].poly.nArcs = poly.nArcs;
			for (int i = 0; i < memo.shellContours[nold].poly.nArcs; i++){

				(*memo.shellContours[nold].parcs)[i].begIndex = polyarcsMsg.begindex(i);
				(*memo.shellContours[nold].parcs)[i].endIndex = polyarcsMsg.endindex(i);
				(*memo.shellContours[nold].parcs)[i].arcAngle = polyarcsMsg.arcangle(i);
			}

			memo.shellContours[nold].height = holeMsg.height();

			double* tmx = memo.shellContours[nold].plane.tmx;

			tmx[0] = 1.0f; tmx[1] = 0.0f; tmx[2] = 0.0f; tmx[3] = 0.0f;
			tmx[4] = 0.0f; tmx[5] = 1.0f; tmx[6] = 0.0f; tmx[7] = 0.0f;
			tmx[8] = 0.0f; tmx[9] = 0.0f; tmx[10] = 1.0f; tmx[11] = 10.0f;

			memo.shellContours[nold].edgeData = (API_ContourEdgeData*)BMAllocatePtr((memo.shellContours[nold].poly.nCoords + 1) * sizeof(API_ContourEdgeData), ALLOCATE_CLEAR, 0);

			shellElement.shell.numHoles = shellElement.shell.numHoles + 1;

			ACAPI_ELEMENT_MASK_CLEAR(mask);
			ACAPI_ELEMENT_MASK_SET(mask, API_ShellType, numHoles);

			err = ACAPI_Element_Change(&shellElement, &mask, &memo, APIMemoMask_Polygon | APIMemoMask_EdgeTrims, true);

			ACAPI_DisposeElemMemoHdls(&memo);
			if (err != NoError){
				ErrorBeep("Failed Hole", err);

				sprintf(buffer, "Error: %d", err);
				ACAPI_WriteReport(buffer, true);
			}
		}
	}
	sendElementID(getClientSocket(), shellElement);
}



