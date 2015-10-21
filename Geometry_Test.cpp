// *****************************************************************************
// Source code for the Geometry Test Add-On
// API Development Kit 18; Mac/Win
//
// Namespaces:		Contact person:
//	-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "APIEnvir.h"

#define	_Geometry_Test_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<stdio.h>
#include	<string.h>

#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"APICommon.h"

#include	"basicgeometry.h"

#include	"Templates.h"

#include	"Messages.pb.h"

#include	"DGModule.hpp"

#include <list>

#include <iostream>
#include <fstream>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message_lite.h>
#include <boost/asio.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/register/point.hpp>

/*
typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian> Point;
//typedef boost::geometry::model::segment<Point> Segment;
struct legacy_point
{
	double x, y;
};
*/

BOOST_GEOMETRY_REGISTER_POINT_2D(API_Coord, double, cs::cartesian, x, y);
typedef boost::geometry::model::segment<API_Coord> Segment;

using boost::asio::ip::tcp;
using namespace std;
using namespace google::protobuf::io;


typedef void(*ScriptFunction)(void);
typedef std::map <std::string, ScriptFunction> scriptMap;
typedef std::map <std::string, short> buildingMaterialMap;
typedef std::map <std::string, int> objectsMap;

buildingMaterialMap buildingMaterials;
buildingMaterialMap compositeMaterials;
objectsMap objectsIds;

short wallDefaultMaterial;
short slabDefaultMaterial;

short wallDefaultComposite;
short slabDefaultComposite;

short searchMaterials(string name, buildingMaterialMap map){
	buildingMaterialMap::const_iterator iter = map.find(name);
	if (iter == map.end()){
		// not found
		return -1;
	}
	else{
		return iter->second;
	}
}

int searchObjects(string name, objectsMap map){
	objectsMap::const_iterator iter = map.find(name);
	if (iter == map.end()){
		// not found
		return -1;
	}
	else{
		return iter->second;
	}
}

string searchMaterialsValue(short n, buildingMaterialMap map){
	for (buildingMaterialMap::const_iterator it = map.begin(); it != map.end(); ++it){
		if (it->second == n){
			return it->first;
		}
	}
	return "Not Found";
}

string searchObjectsValue(int n, objectsMap map){
	for (objectsMap::const_iterator it = map.begin(); it != map.end(); ++it){
		if (it->second == n){
			return it->first;
		}
	}
	return "Not Found";
}
// ----------------------------------Communication Variables----------------------
google::protobuf::io::ZeroCopyInputStream * raw_in;
google::protobuf::io::ZeroCopyOutputStream *raw_out;
int currentLevel = 0;
// ----------------------------------FUNCTIONS------------------------------------

//Used to write a delimited message
//sends size followed by the message
bool writeDelimitedTo(const google::protobuf::MessageLite& message,
	google::protobuf::io::ZeroCopyOutputStream* rawOutput) {
	// We create a new coded stream for each message.  Don't worry, this is fast.
	google::protobuf::io::CodedOutputStream output(rawOutput);

	// Write the size.
	const int size = message.ByteSize();
	output.WriteVarint32(size);

	uint8_t* buffer = output.GetDirectBufferForNBytesAndAdvance(size);
	if (buffer != NULL) {
		// Optimization:  The message fits in one buffer, so use the faster
		// direct-to-array serialization path.
		message.SerializeWithCachedSizesToArray(buffer);
	}
	else {
		// Slightly-slower path when the message is multiple buffers.
		message.SerializeWithCachedSizes(&output);
		if (output.HadError()){
			return false;
		}
	}
	return true;
}

//Used to read a delimited message
//reads size followed by the message
bool readDelimitedFrom(google::protobuf::io::ZeroCopyInputStream* rawInput,
	google::protobuf::MessageLite* message) {
	// We create a new coded stream for each message.  Don't worry, this is fast,
	// and it makes sure the 64MB total size limit is imposed per-message rather
	// than on the whole stream.  (See the CodedInputStream interface for more
	// info on this limit.)
	google::protobuf::io::CodedInputStream input(rawInput);

	// Read the size.
	uint32_t size;
	if (!input.ReadVarint32(&size)) return false;

	// Tell the stream not to read beyond that size.
	google::protobuf::io::CodedInputStream::Limit limit =
		input.PushLimit(size);

	// Parse the message.
	if (!message->MergeFromCodedStream(&input)) return false;
	if (!input.ConsumedEntireMessage()) return false;

	// Release the limit.
	input.PopLimit(limit);

	return true;
}

// Search by UnID
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

//Send Element Id to Racket
void	sendElementID(API_Element element, bool crash){
	elementid		elementId;
	//Convert Element Id from GuId to char*
	GS::UniString guidString = APIGuidToString(element.header.guid);
	char s[64];
	APIGuid2GSGuid(element.header.guid).ConvertToString(s);
	elementId.set_guid(s);
	elementId.set_crashmaterial(crash);

	//Sending element Id
	writeDelimitedTo(elementId, raw_out);

	delete(raw_out);
}

void	sendElementID(API_Element element){
	sendElementID(element, false);
}

int		pointRelation(float x1, float x2){
	if (x1 < x2){
		return 1; //right
	}
	else if (x1 > x2){
		return 2; //left
	}
	else{
		//x1 == x2
		return 3;
	}
}

void	multTMX(double *resMat, double *aMatrix)
{

	double *a, *b, res[12];
	a = resMat;
	b = aMatrix;
	res[3] = a[3];
	res[7] = a[7];
	res[11] = a[11];

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			res[j * 4 + i] = 0.0f;
			for (int k = 0; k < 3; ++k) {
				res[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
			}
		}
	}
	memcpy(a, res, 12 * sizeof(double));
}


// ---------------------------------- Functions Objects No Com -------------------

void	createCircleNoCom(){
	API_ElementMemo memo;
	API_Element		circleElement;
	GSErrCode		err;

	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	BNZeroMemory(&circleElement, sizeof(API_Element));
	circleElement.circle.r = 1.0;
	circleElement.circle.angle = 45.0;
	circleElement.circle.ratio = 1.0;
	circleElement.header.typeID = API_CircleID;
	circleElement.header.layer = 1;
	circleElement.header.floorInd = currentLevel;
	err = ACAPI_Element_Create(&circleElement, &memo);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createSquareNoCom(){
	API_ElementMemo memo;
	API_Element		quadrangle[4];
	GSErrCode		err;

	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	BNZeroMemory(&quadrangle[0], sizeof(API_Element));
	quadrangle[0].line.begC.x = -1.0; quadrangle[0].line.begC.y = 1.0;
	BNZeroMemory(&quadrangle[1], sizeof(API_Element));
	quadrangle[1].line.begC.x = -1.0; quadrangle[1].line.begC.y = -1.0;
	BNZeroMemory(&quadrangle[2], sizeof(API_Element));
	quadrangle[2].line.begC.x = 1.0; quadrangle[2].line.begC.y = -1.0;
	BNZeroMemory(&quadrangle[3], sizeof(API_Element));
	quadrangle[3].line.begC.x = 1.0; quadrangle[3].line.begC.y = 1.0;

	quadrangle[0].line.endC = quadrangle[1].line.begC;
	quadrangle[1].line.endC = quadrangle[2].line.begC;
	quadrangle[2].line.endC = quadrangle[3].line.begC;
	quadrangle[3].line.endC = quadrangle[0].line.begC;

	//create lines
	for (int i = 0; i < 4; i++) {

		quadrangle[i].header.typeID = API_LineID;
		quadrangle[i].header.layer = 1;
		quadrangle[i].header.floorInd = currentLevel;

		err = ACAPI_Element_Create(&quadrangle[i], &memo);
		if (err != NoError)
			break;
	}

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createArcNoCom(){
	API_ElementMemo memo;
	API_Element		arcElement;
	GSErrCode		err;

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&arcElement, sizeof(API_Element));

	arcElement.arc.origC.x = 0.0;
	arcElement.arc.origC.y = 0.0;
	arcElement.arc.r = 1.0;
	arcElement.arc.angle = 0.0;
	arcElement.arc.begAng = 0;
	arcElement.arc.endAng = 1.570785; //quarter-circle

	arcElement.arc.ratio = 1.0;
	arcElement.header.typeID = API_ArcID;
	arcElement.header.layer = 1;
	arcElement.header.floorInd = currentLevel;

	err = ACAPI_Element_Create(&arcElement, &memo);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createChapelNoCom(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	API_Coord		centerPoint;

	centerPoint.x = 0.0; 
	centerPoint.y = 0.0;

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&shellElement, sizeof(API_Element));

	shellElement.header.typeID = API_ShellID;
	shellElement.header.layer = 1;
	shellElement.header.floorInd = currentLevel;

	shellElement.shell.shellClass = API_RevolvedShellID;

	err = ACAPI_Element_GetDefaults(&shellElement, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (shell)", err);
		return;
	}

	double* tmx = shellElement.shell.basePlane.tmx;
	
	tmx[0] = 1.0;				tmx[4] = 0.0;				tmx[8] = 0.0;
	tmx[1] = 0.0;				tmx[5] = 0.0;				tmx[9] = 1.0;
	tmx[2] = 0.0;				tmx[6] = -1.0;				tmx[10] = 0.0;
	tmx[3] = centerPoint.x;	tmx[7] = centerPoint.y;	tmx[11] = 0.0;
	
	shellElement.shell.isFlipped = true;

	shellElement.shell.u.revolvedShell.slantAngle = 0;
	shellElement.shell.u.revolvedShell.revolutionAngle = 360 * DEGRAD;
	shellElement.shell.u.revolvedShell.distortionAngle = 90 * DEGRAD;
	shellElement.shell.u.revolvedShell.segmentedSurfaces = false;
	shellElement.shell.u.revolvedShell.segmentType = APIShellBase_SegmentsByCircle;
	shellElement.shell.u.revolvedShell.segmentsByCircle = 36;
	BNZeroMemory(&shellElement.shell.u.revolvedShell.axisBase, sizeof(API_Tranmat));
	shellElement.shell.u.revolvedShell.axisBase.tmx[0] = 1.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[6] = 1.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[9] = -1.0;
	shellElement.shell.u.revolvedShell.distortionVector.x = 0.0;
	shellElement.shell.u.revolvedShell.distortionVector.y = 0.0;
	shellElement.shell.u.revolvedShell.begAngle = 0.0;

	// constructing the revolving profile polyline
	shellElement.shell.u.revolvedShell.shellShape.nCoords = 13;
	shellElement.shell.u.revolvedShell.shellShape.nSubPolys = 1;
	shellElement.shell.u.revolvedShell.shellShape.nArcs = 3;

	memo.shellShapes[0].coords = (API_Coord**)BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellShapes[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(shellElement.shell.u.revolvedShell.shellShape.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	memo.shellShapes[0].bodyFlags = (GS::Bool8**) BMAllocateHandle((shellElement.shell.u.revolvedShell.shellShape.nCoords + 1) * sizeof(GS::Bool8), ALLOCATE_CLEAR, 0);
	if (memo.shellShapes[0].coords == NULL || memo.shellShapes[0].pends == NULL || memo.shellShapes[0].parcs == NULL || memo.shellShapes[0].bodyFlags == NULL) {
		ErrorBeep("Not enough memory to create shell polygon data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}

	(*memo.shellShapes[0].coords)[1].x = 0.0;
	(*memo.shellShapes[0].coords)[1].y = 0.0;
	(*memo.shellShapes[0].coords)[2].x = 0.4;
	(*memo.shellShapes[0].coords)[2].y = 5.0;
	(*memo.shellShapes[0].coords)[3].x = 1.0;
	(*memo.shellShapes[0].coords)[3].y = 5.0;
	(*memo.shellShapes[0].coords)[4].x = 1.0;
	(*memo.shellShapes[0].coords)[4].y = 6.0;
	(*memo.shellShapes[0].coords)[5].x = 1.7;
	(*memo.shellShapes[0].coords)[5].y = 6.0;
	(*memo.shellShapes[0].coords)[6].x = 1.7;
	(*memo.shellShapes[0].coords)[6].y = 7.0;
	(*memo.shellShapes[0].coords)[7].x = 2.4;
	(*memo.shellShapes[0].coords)[7].y = 7.0;
	(*memo.shellShapes[0].coords)[8].x = 2.4;
	(*memo.shellShapes[0].coords)[8].y = 7.4;
	(*memo.shellShapes[0].coords)[9].x = 0.0;
	(*memo.shellShapes[0].coords)[9].y = 7.7;
	(*memo.shellShapes[0].coords)[10].x = 0.0;
	(*memo.shellShapes[0].coords)[10].y = 8.0;
	(*memo.shellShapes[0].coords)[11].x = 8.0;
	(*memo.shellShapes[0].coords)[11].y = 10.0;
	(*memo.shellShapes[0].coords)[12].x = 12.0;
	(*memo.shellShapes[0].coords)[12].y = 0.0;
	(*memo.shellShapes[0].coords)[13] = (*memo.shellShapes[0].coords)[1];
	(*memo.shellShapes[0].pends)[1] = shellElement.shell.u.revolvedShell.shellShape.nCoords;
	(*memo.shellShapes[0].parcs)[0].begIndex = 1;
	(*memo.shellShapes[0].parcs)[0].endIndex = 2;
	(*memo.shellShapes[0].parcs)[0].arcAngle = -0.143099565651258;
	(*memo.shellShapes[0].parcs)[1].begIndex = 10;
	(*memo.shellShapes[0].parcs)[1].endIndex = 11;
	(*memo.shellShapes[0].parcs)[1].arcAngle = 0.566476134070805;
	(*memo.shellShapes[0].parcs)[2].begIndex = 11;
	(*memo.shellShapes[0].parcs)[2].endIndex = 12;
	(*memo.shellShapes[0].parcs)[2].arcAngle = 0.385936923743763;
	(*memo.shellShapes[0].bodyFlags)[1] = true;
	(*memo.shellShapes[0].bodyFlags)[2] = true;
	(*memo.shellShapes[0].bodyFlags)[3] = true;
	(*memo.shellShapes[0].bodyFlags)[4] = true;
	(*memo.shellShapes[0].bodyFlags)[5] = true;
	(*memo.shellShapes[0].bodyFlags)[6] = true;
	(*memo.shellShapes[0].bodyFlags)[7] = true;
	(*memo.shellShapes[0].bodyFlags)[8] = true;
	(*memo.shellShapes[0].bodyFlags)[9] = true;
	(*memo.shellShapes[0].bodyFlags)[10] = true;
	(*memo.shellShapes[0].bodyFlags)[11] = true;
	(*memo.shellShapes[0].bodyFlags)[12] = false;
	(*memo.shellShapes[0].bodyFlags)[13] = (*memo.shellShapes[0].bodyFlags)[1];
	

	// constructing the shell contour data
	shellElement.shell.hasContour = false;		// this shell will not be clipped
	shellElement.shell.numHoles = 1;				// but will have a hole

	USize nContours = shellElement.shell.numHoles + (shellElement.shell.hasContour ? 1 : 0);
	memo.shellContours = (API_ShellContourData *)BMAllocatePtr(nContours * sizeof(API_ShellContourData), ALLOCATE_CLEAR, 0);
	if (memo.shellContours == NULL) {
		ErrorBeep("Not enough memory to create shell contour data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}

	memo.shellContours[0].poly.nCoords = 5;
	memo.shellContours[0].poly.nSubPolys = 1;
	memo.shellContours[0].poly.nArcs = 1;
	memo.shellContours[0].coords = (API_Coord**)BMAllocateHandle((memo.shellContours[0].poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.shellContours[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((memo.shellContours[0].poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.shellContours[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(memo.shellContours[0].poly.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	if (memo.shellContours[0].coords == NULL || memo.shellContours[0].pends == NULL || memo.shellContours[0].parcs == NULL) {
		ErrorBeep("Not enough memory to create shell contour data", APIERR_MEMFULL);
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}

	(*memo.shellContours[0].coords)[1].x = -1.5;
	(*memo.shellContours[0].coords)[1].y = -0.3;

	(*memo.shellContours[0].coords)[2].x = -1.5;
	(*memo.shellContours[0].coords)[2].y = 3.1;
	
	(*memo.shellContours[0].coords)[3].x = 1.5;
	(*memo.shellContours[0].coords)[3].y = 3.1;
	
	(*memo.shellContours[0].coords)[4].x = 1.5;
	(*memo.shellContours[0].coords)[4].y = -0.3;
	(*memo.shellContours[0].coords)[5] = (*memo.shellContours[0].coords)[1];
	(*memo.shellContours[0].pends)[1] = 5;
	(*memo.shellContours[0].parcs)[0].begIndex = 2;
	(*memo.shellContours[0].parcs)[0].endIndex = 3;
	(*memo.shellContours[0].parcs)[0].arcAngle = -240.0 * DEGRAD;

	memo.shellContours[0].height = -5.2;
	tmx = memo.shellContours[0].plane.tmx;
	tmx[0] = 1.0;		tmx[4] = 0.0;		tmx[8] = 0.0;
	tmx[1] = 0.0;		tmx[5] = 1.0;		tmx[9] = 0.0;
	tmx[2] = 0.0;		tmx[6] = 0.0;		tmx[10] = 1.0;
	tmx[3] = 0.0;		tmx[7] = 0.0;		tmx[11] = 10.0;

	// create the shell element
	err = ACAPI_Element_Create(&shellElement, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (revolved)", err);
	}

	ACAPI_DisposeElemMemoHdls(&memo);

}

void	createLemonNoCom(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	API_Coord		centerPoint;

	centerPoint.x = 0.0;
	centerPoint.y = 0.0;

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&shellElement, sizeof(API_Element));

	shellElement.header.typeID = API_ShellID;
	shellElement.header.layer = 1;
	shellElement.header.floorInd = currentLevel;

	shellElement.shell.shellClass = API_RevolvedShellID;
	
	err = ACAPI_Element_GetDefaults(&shellElement, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (shell)", err);
		return;
	}
	
	double* tmx = shellElement.shell.basePlane.tmx;
	
	tmx[0] = 1.0;				tmx[4] = 0.0;				tmx[8] = 0.0;
	tmx[1] = 0.0;				tmx[5] = 1.0;				tmx[9] = 0.0;
	tmx[2] = 0.0;				tmx[6] = 0.0;				tmx[10] = 1.0;
	tmx[3] = 0.0;		tmx[7] = 0.0;		tmx[11] = 1.0;


	shellElement.shell.isFlipped = true;

	shellElement.shell.u.revolvedShell.revolutionAngle = 360 * DEGRAD;
	shellElement.shell.u.revolvedShell.distortionAngle = 90 * DEGRAD;
	shellElement.shell.u.revolvedShell.segmentedSurfaces = false;
	shellElement.shell.u.revolvedShell.segmentType = APIShellBase_SegmentsByCircle;
	shellElement.shell.u.revolvedShell.segmentsByCircle = 36;
	BNZeroMemory(&shellElement.shell.u.revolvedShell.axisBase, sizeof(API_Tranmat));
	
	shellElement.shell.u.revolvedShell.axisBase.tmx[0] = 1.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[6] = 1.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[9] = 1.0;
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
	
	(*memo.shellShapes[0].coords)[1].x = 0.0;
	(*memo.shellShapes[0].coords)[1].y = 0.0;
	(*memo.shellShapes[0].coords)[2].x = 0.4;
	(*memo.shellShapes[0].coords)[2].y = 5.0;
	(*memo.shellShapes[0].coords)[3] = (*memo.shellShapes[0].coords)[1];
	

	(*memo.shellShapes[0].pends)[1] = shellElement.shell.u.revolvedShell.shellShape.nCoords;

	(*memo.shellShapes[0].parcs)[0].begIndex = 1;
	(*memo.shellShapes[0].parcs)[0].endIndex = 2;
	(*memo.shellShapes[0].parcs)[0].arcAngle = 4 * 1.570785;
	
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

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createSphereNoCom(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	API_Coord		centerPoint;

	centerPoint.x = 0.0;
	centerPoint.y = 0.0;

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&shellElement, sizeof(API_Element));

	shellElement.header.typeID = API_ShellID;
	shellElement.header.layer = 1;
	shellElement.header.floorInd = currentLevel;

	shellElement.shell.shellClass = API_RevolvedShellID;

	err = ACAPI_Element_GetDefaults(&shellElement, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (shell)", err);
		return;
	}

	double* tmx = shellElement.shell.basePlane.tmx;
	/*
	tmx[0] = 1.0;				tmx[4] = 0.0;				tmx[8] = 0.0;
	tmx[1] = 0.0;				tmx[5] = 0.0;				tmx[9] = 1.0;
	tmx[2] = 0.0;				tmx[6] = -1.0;				tmx[10] = 0.0;
	tmx[3] = centerPoint.x;	tmx[7] = centerPoint.y;	tmx[11] = 0.0;
	*/

	tmx[0] = 1.0;				tmx[4] = 0.0;				tmx[8] = 0.0;
	tmx[1] = 0.0;				tmx[5] = 1.0;				tmx[9] = 0.0;
	tmx[2] = 0.0;				tmx[6] = 0.0;				tmx[10] = 1.0;
	//center Coords (x, y, z)
	tmx[3] = 0.0;				tmx[7] = 0.0;				tmx[11] = 0.0;


	shellElement.shell.isFlipped = true;

	shellElement.shell.u.revolvedShell.slantAngle = 0;
	shellElement.shell.u.revolvedShell.revolutionAngle = 360 * DEGRAD;
	shellElement.shell.u.revolvedShell.distortionAngle = 90 * DEGRAD;
	shellElement.shell.u.revolvedShell.segmentedSurfaces = false;
	shellElement.shell.u.revolvedShell.segmentType = APIShellBase_SegmentsByCircle;
	shellElement.shell.u.revolvedShell.segmentsByCircle = 36;
	BNZeroMemory(&shellElement.shell.u.revolvedShell.axisBase, sizeof(API_Tranmat));

	shellElement.shell.u.revolvedShell.axisBase.tmx[0] = 0.0;
	//reflect y-axis, if 1.0 up, -1.0 down
	shellElement.shell.u.revolvedShell.axisBase.tmx[6] = 0.0;
	shellElement.shell.u.revolvedShell.axisBase.tmx[9] = 0.0;

	//reflect x-axis, if 1.0 right, -1.0 left
	shellElement.shell.u.revolvedShell.axisBase.tmx[3] = 0.0;


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

	(*memo.shellShapes[0].coords)[1].x = 0.0;
	(*memo.shellShapes[0].coords)[1].y = 0.0;
	(*memo.shellShapes[0].coords)[2].x = 0.0;
	(*memo.shellShapes[0].coords)[2].y = 0.0001;
	(*memo.shellShapes[0].coords)[3] = (*memo.shellShapes[0].coords)[1];
	

	(*memo.shellShapes[0].pends)[1] = shellElement.shell.u.revolvedShell.shellShape.nCoords;

	(*memo.shellShapes[0].parcs)[0].begIndex = 1;
	(*memo.shellShapes[0].parcs)[0].endIndex = 2;
	(*memo.shellShapes[0].parcs)[0].arcAngle = 4 * 1.570785;
	
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

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createCurtainWallNoCom(){
	API_Element			element;
	API_ElementMemo		memo;
	GSErrCode			err;

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	element.header.typeID = API_CurtainWallID;
	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}


	

	memo.coords = (API_Coord**)BMAllocateHandle((6) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(1 * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	(*memo.coords)[1].x = 1.0;
	(*memo.coords)[1].y = 1.0;

	(*memo.coords)[2].x = 2.0;
	(*memo.coords)[2].y = 1.0;

	(*memo.coords)[3].x = 2.0;
	(*memo.coords)[3].y = 2.0;

	(*memo.coords)[4].x = 2.0;
	(*memo.coords)[4].y = 1.0;

	(*memo.coords)[5] = (*memo.coords)[1];

	(*memo.parcs)[0].begIndex = 1;
	(*memo.parcs)[0].endIndex = 2;
	(*memo.parcs)[0].arcAngle = 360 * DEGRAD;
	


	element.curtainWall.nSegments = BMhGetSize(reinterpret_cast<GSHandle> (memo.coords)) / Sizeof32(API_Coord) - 2;
	
	// Modify segment data
	element.curtainWall.segmentData.primaryPatternNum += 2;
	element.curtainWall.segmentData.secondaryPatternNum++;
	element.curtainWall.segmentData.panelPatternNum = element.curtainWall.segmentData.primaryPatternNum * element.curtainWall.segmentData.secondaryPatternNum;
	memo.cWSegPrimaryPattern = (double*)BMReallocPtr((GSPtr)memo.cWSegPrimaryPattern, element.curtainWall.segmentData.primaryPatternNum*sizeof(double), REALLOC_MOVEABLE, 0);
	if (memo.cWSegPrimaryPattern != NULL) {
		memo.cWSegPrimaryPattern[0] = 0.5;
		memo.cWSegPrimaryPattern[element.curtainWall.segmentData.primaryPatternNum - 2] = memo.cWSegPrimaryPattern[element.curtainWall.segmentData.primaryPatternNum - 1] = 0.75;
	}
	memo.cWSegSecondaryPattern = (double*)BMReallocPtr((GSPtr)memo.cWSegSecondaryPattern, element.curtainWall.segmentData.secondaryPatternNum*sizeof(double), REALLOC_MOVEABLE, 0);
	if (memo.cWSegSecondaryPattern != NULL) {
		memo.cWSegSecondaryPattern[0] = 0.35;
		memo.cWSegSecondaryPattern[element.curtainWall.segmentData.secondaryPatternNum - 1] = 1;
	}
	memo.cWSegPanelPattern = (GS::Bool8*) BMReallocPtr((GSPtr)memo.cWSegPanelPattern, element.curtainWall.segmentData.panelPatternNum*sizeof(GS::Bool8), REALLOC_MOVEABLE, 0);
	if (memo.cWSegPanelPattern != NULL) {
		for (UInt32 ii = 0; ii < element.curtainWall.segmentData.panelPatternNum; ++ii)
			memo.cWSegPanelPattern[ii] = GS::Bool8(ii % 3);
	}
	
	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_Create", err);
		return;
	}

	ACAPI_DisposeElemMemoHdls(&memo);
}


// ---------------------------------- Test Functions -----------------------------


// ---------------------------------- Functions Objects --------------------------

void	createCircle(){
	API_ElementMemo memo;
	API_Element		circleElement;
	GSErrCode		err;
	circlemessage	circleMsg;

	readDelimitedFrom(raw_in, &circleMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&circleElement, sizeof(API_Element));

	circleElement.circle.origC.x = circleMsg.p0x();
	circleElement.circle.origC.y = circleMsg.p0y();
	circleElement.circle.r = circleMsg.radius();
	
	circleElement.circle.ratio = 1.0;
	circleElement.header.typeID = API_CircleID;
	circleElement.header.layer = 1;
	circleElement.header.floorInd = currentLevel;

	err = ACAPI_Element_Create(&circleElement, &memo);
	if (err != NoError){
		//createCircleNoCom();
		ErrorBeep("ACAPI_Element_Create (circle)", err);
	}
	sendElementID(circleElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createArc(){
	API_ElementMemo memo;
	API_Element		arcElement;
	GSErrCode		err;
	arcmessage		arcMsg;

	readDelimitedFrom(raw_in, &arcMsg);

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
	arcElement.header.floorInd = currentLevel;

	err = ACAPI_Element_Create(&arcElement, &memo);

	sendElementID(arcElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createWall(){
	API_Element		wallElement;
	API_ElementMemo memo;
	API_StoryInfo	storyInfo;
	GSErrCode		err;
	wallmessage		wallMsg;
	elementid		wallId;
	short			material = 0;
	bool			crash = false;
	char buffer[256];

	readDelimitedFrom(raw_in, &wallMsg);
	
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
		material = searchMaterials(wallMsg.material(), buildingMaterials);
		wallElement.wall.buildingMaterial = material;
		wallElement.wall.modelElemStructureType = API_BasicStructure;
		if (wallMsg.thickness() > 0){
			wallElement.wall.thickness = wallMsg.thickness();
		}
	}
	else if (wallMsg.type() == "CompositeStructure"){
		material = searchMaterials(wallMsg.material(), compositeMaterials);
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
		wallElement.header.floorInd = currentLevel;
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
	sendElementID( wallElement, crash);
	
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);
	
}

void	createNewWall(){
	API_Element		wallElement;
	API_ElementMemo memo;
	API_StoryInfo	storyInfo;
	GSErrCode		err;
	wallmsg			wallMsg;
	elementid		wallId;
	elementidlist	elementIDList;
	pointsmessage	pointsMsg;
	short			material = 0;
	bool			crash = false;
	char buffer[256];

	readDelimitedFrom(raw_in, &wallMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	BNZeroMemory(&wallElement, sizeof(API_Element));

	wallElement.header.typeID = API_WallID;
	wallElement.header.layer = 1;

	err = ACAPI_Element_GetDefaults(&wallElement, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	wallElement.header.floorInd = wallMsg.bottomindex();

	if (wallMsg.type() == "Basic"){
		material = searchMaterials(wallMsg.material(), buildingMaterials);
		wallElement.wall.buildingMaterial = material;
		wallElement.wall.modelElemStructureType = API_BasicStructure;
		if (wallMsg.thickness() > 0){
			wallElement.wall.thickness = wallMsg.thickness();
		}
	}
	else if (wallMsg.type() == "Composite"){
		material = searchMaterials(wallMsg.material(), compositeMaterials);
		wallElement.wall.modelElemStructureType = API_CompositeStructure;
		wallElement.wall.composite = material;
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
	wallElement.wall.height = (*storyInfo.data)[wallMsg.upperindex()].level - (*storyInfo.data)[wallMsg.bottomindex()].level;


	if (wallMsg.referenceline() == "Center"){
		wallElement.wall.referenceLineLocation = APIWallRefLine_Center;
	}
	else if (wallMsg.referenceline() == "Outside"){
		wallElement.wall.referenceLineLocation = APIWallRefLine_Outside;
	}
	else if (wallMsg.referenceline() == "Inside"){
		wallElement.wall.referenceLineLocation = APIWallRefLine_Inside;
	}


	readDelimitedFrom(raw_in, &pointsMsg);
	for (int i = 0; i < pointsMsg.px_size() - 1; i++){

		//Beginning Point
		wallElement.wall.begC.x = pointsMsg.px(i);
		wallElement.wall.begC.y = pointsMsg.py(i);
		//Ending Point
		wallElement.wall.endC.x = pointsMsg.px(i + 1);
		wallElement.wall.endC.y = pointsMsg.py(i + 1);

		if (wallMsg.typeprofile() == "Normal"){
			wallElement.wall.profileType = APISect_Normal;
		}
		else if(wallMsg.typeprofile() == "Slanted"){
			wallElement.wall.profileType = APISect_Slanted;
		}
		else if (wallMsg.typeprofile() == "DoubleSlanted"){
			wallElement.wall.profileType = APISect_Trapez;
		}
		
		//////

		wallElement.wall.slantAlpha = wallMsg.alphaangle();
		wallElement.wall.slantBeta = wallMsg.betaangle();
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
	writeDelimitedTo(elementIDList, raw_out);

	delete(raw_out);

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

void	createPolyWall(){
	API_Element		wallElement;
	API_ElementMemo memo;
	GSErrCode		err;
	wallmessage		wallMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage	polyarcsMsg;
	elementid		wallId;
	API_StoryCmdType s;
	char buffer[256];


	readDelimitedFrom(raw_in, &wallMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&wallElement, sizeof(API_Element));

	readDelimitedFrom(raw_in, &pointsMsg);
	readDelimitedFrom(raw_in, &polyarcsMsg);


	wallElement.header.typeID = API_WallID;
	wallElement.header.layer = 1;
	wallElement.header.floorInd = currentLevel;

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
	wallElement.wall.poly.nCoords = (pointsMsg.px_size()*2) + 1;
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

void	createMultiWall(){
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

	readDelimitedFrom(raw_in, &wallMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	BNZeroMemory(&wallElement, sizeof(API_Element));
	wallElement.header.typeID = API_WallID;
	wallElement.header.layer = 1;
	

	err = ACAPI_Element_GetDefaults(&wallElement, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	wallElement.header.floorInd = currentLevel;

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
		material = searchMaterials(wallMsg.material(), buildingMaterials);
		wallElement.wall.buildingMaterial = material;
		wallElement.wall.modelElemStructureType = API_BasicStructure;
		if (wallMsg.thickness() > 0){
			wallElement.wall.thickness = wallMsg.thickness();
		}
	}
	else if (wallMsg.type() == "CompositeStructure"){
		material = searchMaterials(wallMsg.material(), compositeMaterials);
		wallElement.wall.modelElemStructureType = API_CompositeStructure;
		wallElement.wall.composite = material;
	}

	if (material == -1){
		crash = true;
	}

	readDelimitedFrom(raw_in, &pointsMsg);
	readDelimitedFrom(raw_in, &polyarcsMsg);


	for (int i = 0; i < pointsMsg.px_size()-1; i++){

		//Beginning Point
		wallElement.wall.begC.x = pointsMsg.px(i);
		wallElement.wall.begC.y = pointsMsg.py(i);
		//Ending Point
		wallElement.wall.endC.x = pointsMsg.px(i+1);
		wallElement.wall.endC.y = pointsMsg.py(i+1);

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
	writeDelimitedTo(elementIDList, raw_out);

	delete(raw_out);

	ACAPI_DisposeElemMemoHdls(&memo);

}

void	createDoor(){
	API_Element			doorElement, wallElement;
	API_ElementMemo		memo;
	GSErrCode			err;
	doormessage			doorMsg;
	API_SubElemMemoMask	marker;
	API_AddParType		**markAddPars;
	API_Coord			beginWall, endWall;
	double				wallLength;
	double				doorStartingPoint;
	readDelimitedFrom(raw_in, &doorMsg);

	BNZeroMemory(&doorElement, sizeof(API_Element));
	BNZeroMemory(&wallElement, sizeof(API_Element));
	BNZeroMemory(&marker, sizeof(API_SubElemMemoMask));
	doorElement.header.typeID = API_DoorID;
	doorElement.header.layer = 1;
	

	marker.subType = APISubElemMemoMask_MainMarker;

	err = ACAPI_Element_GetDefaultsExt(&doorElement, &memo, 1UL, &marker);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memo);
		ACAPI_DisposeElemMemoHdls(&marker.memo);
		return;
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

	if (doorMsg.height() != -10000){
		doorElement.door.openingBase.height = doorMsg.height();
	}

	if (doorMsg.width() != -10000){
		doorElement.door.openingBase.width = doorMsg.width();
	}

	if (doorMsg.hole()){
		doorElement.door.openingBase.libInd = 6;
	}

	doorElement.door.owner = GSGuid2APIGuid(wallGuid);

	wallElement.header.typeID = API_WallID;
	wallElement.header.guid = GSGuid2APIGuid(wallGuid);

	if (ACAPI_Element_Get(&wallElement) == NoError){
		doorElement.header.floorInd = wallElement.header.floorInd;
	}

	/*
	API_Coord c2, begC;
	c2.x = 1.0;
	c2.y = 1.0;
	begC.x = 0.0;
	begC.y = 0.0;
	doorElement.door.objLoc = DistCPtr(&c2, &begC);
	*/
	err = ACAPI_Element_CreateExt(&doorElement, &memo, 1UL, &marker);

	//if there is an error a circle will be created
	if (err != NoError){
		//createCircleNoCom();
	}
	sendElementID(doorElement);

	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&marker.memo);
}

void	createWindow(){
	API_Element			windowElement, wallElement;
	API_ElementMemo		memo;
	GSErrCode			err;
	windowmessage		windowMsg;
	API_SubElemMemoMask	marker;
	API_AddParType		**markAddPars;
	API_Coord			beginWall, endWall;
	double				wallLength;
	double				windowStartingPoint;
	readDelimitedFrom(raw_in, &windowMsg);

	BNZeroMemory(&windowElement, sizeof(API_Element));
	BNZeroMemory(&wallElement, sizeof(API_Element));
	BNZeroMemory(&marker, sizeof(API_SubElemMemoMask));
	windowElement.header.typeID = API_WindowID;
	windowElement.header.layer = 1;
	windowElement.header.floorInd = currentLevel;

	marker.subType = APISubElemMemoMask_MainMarker;

	err = ACAPI_Element_GetDefaultsExt(&windowElement, &memo, 1UL, &marker);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memo);
		ACAPI_DisposeElemMemoHdls(&marker.memo);
		return;
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

	/*
	API_Coord c2, begC;
	c2.x = 1.0;
	c2.y = 1.0;
	begC.x = 0.0;
	begC.y = 0.0;
	windowElement.window.objLoc = DistCPtr(&c2, &begC);
	*/
	err = ACAPI_Element_CreateExt(&windowElement, &memo, 1UL, &marker);

	//if there is an error a circle will be created
	if (err != NoError){
		//createCircleNoCom();
	}
	sendElementID(windowElement);

	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&marker.memo);
}

void	createSphere(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	spheremessage	sphereMsg;
	API_StoryInfo		storyInfo;

	readDelimitedFrom(raw_in, &sphereMsg);

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
	double levelHeight = (*storyInfo.data)[currentLevel - storyInfo.firstStory].level;
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));

	shellElement.header.floorInd = sphereMsg.level();

	double* tmx = shellElement.shell.basePlane.tmx;

	tmx[0] = 1.0;				tmx[4] = 0.0;				tmx[8] = 0.0;
	tmx[1] = 0.0;				tmx[5] = 1.0;				tmx[9] = 0.0;
	tmx[2] = 0.0;				tmx[6] = 0.0;				tmx[10] = 1.0;
	//center Coords (x, y, z)
	tmx[3] = sphereMsg.c0x();		tmx[7] = sphereMsg.c0y();		tmx[11] = levelHeight + sphereMsg.c0z();

	shellElement.shell.isFlipped = true;

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
	
	(*memo.shellShapes[0].coords)[2].x = - sphereMsg.radius();
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

	sendElementID(shellElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createCylinder(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	cylindermsg	cylinderMsg;
	API_StoryInfo		storyInfo;
	char buffer[256];

	readDelimitedFrom(raw_in, &cylinderMsg);

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
	double levelHeight = (*storyInfo.data)[currentLevel - storyInfo.firstStory].level;
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

	shellElement.shell.shellBase.buildingMaterial = searchMaterials("GENERIC - EXTERNAL CLADDING", buildingMaterials);
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
	(*memo.shellShapes[0].coords)[2].x = - cylinderMsg.radius();
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

	sendElementID(shellElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createCurtainWall(){
	API_Element			curtainWallElement;
	API_ElementMemo		memo;
	GSErrCode			err;
	curtainwallmsg		cWallMsg;
	pointsmessage		pointsMsg;
	polyarcsmessage		polyarcsMsg;
	char buffer[256];

	BNZeroMemory(&curtainWallElement, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	curtainWallElement.header.typeID = API_CurtainWallID;
	
	
	err = ACAPI_Element_GetDefaults(&curtainWallElement, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	readDelimitedFrom(raw_in, &cWallMsg);

	curtainWallElement.curtainWall.polygon.nCoords = cWallMsg.numpoints();
	curtainWallElement.curtainWall.polygon.nSubPolys = 1;
	curtainWallElement.curtainWall.polygon.nArcs = 2;

	memo.coords = (API_Coord**)BMAllocateHandle((cWallMsg.numpoints() + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);

	//TODO --- VER MEMO.PENDS AQUI!!!!!!
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((curtainWallElement.curtainWall.polygon.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));

	//memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((cWallMsg.numarcs() + 1) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((cWallMsg.numarcs()) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	//memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((2) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	/*
	memo.bezierDirs = reinterpret_cast<API_SplineDir**> (BMAllocateHandle(2 * sizeof(API_SplineDir), ALLOCATE_CLEAR, 0));
	(*memo.bezierDirs)[1].lenPrev = 1.0;
	(*memo.bezierDirs)[1].lenNext = 1.0;
	(*memo.bezierDirs)[1].dirAng = 45.0 * DEGRAD;
	*/

	readDelimitedFrom(raw_in, &pointsMsg);

	for (int i = 1; i <= cWallMsg.numpoints(); i++){
		(*memo.coords)[i].x = pointsMsg.px(i - 1);
		(*memo.coords)[i].y = pointsMsg.py(i - 1);
	}

	(*memo.pends)[1] = curtainWallElement.curtainWall.polygon.nCoords;

	readDelimitedFrom(raw_in, &polyarcsMsg);

	for (int i = 0; i < cWallMsg.numarcs(); i++){
		(*memo.parcs)[i].begIndex = polyarcsMsg.begindex(i);
		(*memo.parcs)[i].endIndex = polyarcsMsg.endindex(i);
		(*memo.parcs)[i].arcAngle = polyarcsMsg.arcangle(i);
	}
	
	/*
	(*memo.parcs)[0].begIndex = 1;
	(*memo.parcs)[0].endIndex = 2;
	(*memo.parcs)[0].arcAngle = 180.0 * DEGRAD;

	(*memo.parcs)[1].begIndex = 2;
	(*memo.parcs)[1].endIndex = 3;
	(*memo.parcs)[1].arcAngle = 180.0 * DEGRAD;
	*/
	

	curtainWallElement.header.layer = 1;
	curtainWallElement.header.floorInd = currentLevel;
	curtainWallElement.curtainWall.nSegments = BMhGetSize(reinterpret_cast<GSHandle> (memo.coords)) / Sizeof32(API_Coord) - 2;
	//TODO
	//curtainWallElement.curtainWall.height = cWallMsg.height();
	
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
	
	////////////////////////////////////////////////////////////////////

	
	// Modify segment data
	curtainWallElement.curtainWall.segmentData.primaryPatternNum = 1;
	
	//sprintf(buffer, "int: %d", curtainWallElement.curtainWall.segmentData.primaryPatternNum);
	//ACAPI_WriteReport(buffer, true);

	//curtainWallElement.curtainWall.segmentData.secondaryPatternNum++;
	
	//curtainWallElement.curtainWall.segmentData.panelPatternNum = curtainWallElement.curtainWall.segmentData.primaryPatternNum * curtainWallElement.curtainWall.segmentData.secondaryPatternNum;
	//curtainWallElement.curtainWall.segmentData.panelPatternNum = curtainWallElement.curtainWall.segmentData.primaryPatternNum;

	memo.cWSegPrimaryPattern = (double*)BMReallocPtr((GSPtr)memo.cWSegPrimaryPattern, curtainWallElement.curtainWall.segmentData.primaryPatternNum*sizeof(double), REALLOC_MOVEABLE, 0);
	if (memo.cWSegPrimaryPattern != NULL) {
		memo.cWSegPrimaryPattern[0] = 1.0;
		//memo.cWSegPrimaryPattern[1] = 2.0;
		//memo.cWSegPrimaryPattern[2] = 0.7;
		//memo.cWSegPrimaryPattern[3] = 1.0;
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
	}

	sendElementID(curtainWallElement);

	ACAPI_DisposeElemMemoHdls(&memo);
	
}

void	createNewCurtainWall(){
	API_Element			curtainWallElement;
	API_ElementMemo		memo;
	API_StoryInfo	storyInfo;
	GSErrCode			err;
	curtainwallmsg		cWallMsg;
	pointsmessage		pointsMsg;
	polyarcsmessage		polyarcsMsg;
	char buffer[256];

	BNZeroMemory(&curtainWallElement, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	curtainWallElement.header.typeID = API_CurtainWallID;
	
	
	err = ACAPI_Element_GetDefaults(&curtainWallElement, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	readDelimitedFrom(raw_in, &cWallMsg);

	curtainWallElement.curtainWall.polygon.nCoords = cWallMsg.numpoints();
	curtainWallElement.curtainWall.polygon.nSubPolys = 1;
	curtainWallElement.curtainWall.polygon.nArcs = 2;

	memo.coords = (API_Coord**)BMAllocateHandle((cWallMsg.numpoints() + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);

	//TODO --- VER MEMO.PENDS AQUI!!!!!!
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((curtainWallElement.curtainWall.polygon.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));

	//memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((cWallMsg.numarcs() + 1) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((cWallMsg.numarcs()) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	//memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((2) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	/*
	memo.bezierDirs = reinterpret_cast<API_SplineDir**> (BMAllocateHandle(2 * sizeof(API_SplineDir), ALLOCATE_CLEAR, 0));
	(*memo.bezierDirs)[1].lenPrev = 1.0;
	(*memo.bezierDirs)[1].lenNext = 1.0;
	(*memo.bezierDirs)[1].dirAng = 45.0 * DEGRAD;
	*/

	readDelimitedFrom(raw_in, &pointsMsg);

	for (int i = 1; i <= cWallMsg.numpoints(); i++){
		(*memo.coords)[i].x = pointsMsg.px(i - 1);
		(*memo.coords)[i].y = pointsMsg.py(i - 1);
	}

	(*memo.pends)[1] = curtainWallElement.curtainWall.polygon.nCoords;

	readDelimitedFrom(raw_in, &polyarcsMsg);

	for (int i = 0; i < cWallMsg.numarcs(); i++){
		(*memo.parcs)[i].begIndex = polyarcsMsg.begindex(i);
		(*memo.parcs)[i].endIndex = polyarcsMsg.endindex(i);
		(*memo.parcs)[i].arcAngle = polyarcsMsg.arcangle(i);
	}
	
	/*
	(*memo.parcs)[0].begIndex = 1;
	(*memo.parcs)[0].endIndex = 2;
	(*memo.parcs)[0].arcAngle = 180.0 * DEGRAD;

	(*memo.parcs)[1].begIndex = 2;
	(*memo.parcs)[1].endIndex = 3;
	(*memo.parcs)[1].arcAngle = 180.0 * DEGRAD;
	*/
	

	curtainWallElement.header.layer = 1;
	curtainWallElement.header.floorInd = cWallMsg.bottomindex();
	curtainWallElement.curtainWall.nSegments = BMhGetSize(reinterpret_cast<GSHandle> (memo.coords)) / Sizeof32(API_Coord) - 2;
	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);

	curtainWallElement.curtainWall.height = (*storyInfo.data)[cWallMsg.upperindex()].level - (*storyInfo.data)[cWallMsg.bottomindex()].level;
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
	
	////////////////////////////////////////////////////////////////////

	
	// Modify segment data
	curtainWallElement.curtainWall.segmentData.primaryPatternNum = 1;
	
	//sprintf(buffer, "int: %d", curtainWallElement.curtainWall.segmentData.primaryPatternNum);
	//ACAPI_WriteReport(buffer, true);

	//curtainWallElement.curtainWall.segmentData.secondaryPatternNum++;
	
	//curtainWallElement.curtainWall.segmentData.panelPatternNum = curtainWallElement.curtainWall.segmentData.primaryPatternNum * curtainWallElement.curtainWall.segmentData.secondaryPatternNum;
	//curtainWallElement.curtainWall.segmentData.panelPatternNum = curtainWallElement.curtainWall.segmentData.primaryPatternNum;

	memo.cWSegPrimaryPattern = (double*)BMReallocPtr((GSPtr)memo.cWSegPrimaryPattern, curtainWallElement.curtainWall.segmentData.primaryPatternNum*sizeof(double), REALLOC_MOVEABLE, 0);
	if (memo.cWSegPrimaryPattern != NULL) {
		memo.cWSegPrimaryPattern[0] = 1.0;
		//memo.cWSegPrimaryPattern[1] = 2.0;
		//memo.cWSegPrimaryPattern[2] = 0.7;
		//memo.cWSegPrimaryPattern[3] = 1.0;
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
	}

	sendElementID(curtainWallElement);

	ACAPI_DisposeElemMemoHdls(&memo);
	
}

void	addArcsToElement(){
	API_Element			element, mask, aux;
	API_ElementMemo		memo, memo2;
	GSErrCode			err;
	elementid			eleGuid;
	polyarcsmessage		polyarcsMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &eleGuid);

	readDelimitedFrom(raw_in, &polyarcsMsg);

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

	sendElementID(element);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createColumn(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode			err;
	columnmsg		columnMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &columnMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.typeID = API_ColumnID;
	element.header.layer = 1;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	element.header.floorInd = currentLevel;

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
	sendElementID(element);

	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);

}

void	createNewColumn(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	columnmsg		columnMsg;
	API_StoryInfo	storyInfo;
	char buffer[256];

	readDelimitedFrom(raw_in, &columnMsg);

	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.typeID = API_ColumnID;
	element.header.layer = 1;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}

	element.header.floorInd = columnMsg.bottomindex();

	element.column.bottomOffset = 0;
	element.column.topOffset = 0;
	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);

	element.column.height = (*storyInfo.data)[columnMsg.upperindex()].level - (*storyInfo.data)[columnMsg.bottomindex()].level;

	element.column.circleBased = columnMsg.circlebased();
	element.column.origoPos.x = columnMsg.posx();
	element.column.origoPos.y = columnMsg.posy();

	element.column.angle = columnMsg.angle();
	element.column.coreDepth = columnMsg.depth();
	element.column.coreWidth = columnMsg.width();

	element.column.isSlanted = true;
	element.column.slantAngle = columnMsg.slantangle();
	element.column.slantDirectionAngle = columnMsg.slantdirection();

	err = ACAPI_Element_Create(&element, &memo);
	sendElementID(element);

	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);

}

void	createSlab(){
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

	readDelimitedFrom(raw_in, &slabMsg);
	readDelimitedFrom(raw_in, &pointsMsg);
	readDelimitedFrom(raw_in, &polyarcsMsg);

	element.header.typeID = API_SlabID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		sprintf(buffer, "Default Problem");
		ACAPI_WriteReport(buffer, true);
	}
	
	element.header.floorInd = currentLevel;
	
	element.slab.level = slabMsg.level();

	if (slabMsg.type() == "BasicStructure"){
		material = searchMaterials(slabMsg.material(), buildingMaterials);
		element.slab.modelElemStructureType = API_BasicStructure;
		element.slab.buildingMaterial = material;
		if (slabMsg.thickness() > 0){
			element.slab.thickness = slabMsg.thickness();
		}
	}
	else if (slabMsg.type() == "CompositeStructure"){
		material = searchMaterials(slabMsg.material(), compositeMaterials);
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

	sendElementID(element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);

}

void	createNewSlab(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	slabmessage		slabMsg;
	pointsmessage	pointsMsg;
	short			material;
	bool crash = false;
	char buffer[256];

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	readDelimitedFrom(raw_in, &slabMsg);
	readDelimitedFrom(raw_in, &pointsMsg);

	element.header.typeID = API_SlabID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		sprintf(buffer, "Default Problem");
		ACAPI_WriteReport(buffer, true);
	}

	element.header.floorInd = slabMsg.bottomlevel();

	element.slab.level = slabMsg.level();

	if (slabMsg.type() == "Basic"){
		material = searchMaterials(slabMsg.material(), buildingMaterials);
		element.slab.modelElemStructureType = API_BasicStructure;
		element.slab.buildingMaterial = material;
		if (slabMsg.thickness() > 0){
			element.slab.thickness = slabMsg.thickness();
		}
	}
	else if (slabMsg.type() == "Composite"){
		material = searchMaterials(slabMsg.material(), compositeMaterials);
		element.slab.composite = material;
		element.slab.modelElemStructureType = API_CompositeStructure;
	}

	if (material == -1){
		crash = true;
	}

	element.slab.poly.nCoords = pointsMsg.px_size();
	element.slab.poly.nSubPolys = slabMsg.subpolygons_size();
	element.slab.poly.nArcs = 0;

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

	for (int i = 1; i <= slabMsg.subpolygons_size(); i++){
		(*memo.pends)[i] = slabMsg.subpolygons(i - 1);
	}

	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (slab)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		//sprintf(buffer, "No creation");
		//ACAPI_WriteReport(buffer, true);
	}

	sendElementID(element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);

}

void	createWallsFromSlab(){
	API_Element		slabElement, element, mask;
	API_ElementMemo slabMemo, memo;
	GSErrCode		err;
	wallsfromslab	msg;
	elementidlist	elementIDList;
	short			material;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(raw_in, &msg);

	BNZeroMemory(&slabElement, sizeof(API_Element));
	BNZeroMemory(&slabMemo, sizeof(API_ElementMemo));

	slabElement.header.guid = APIGuidFromString(msg.guid().c_str());


	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_WallID;
	element.header.layer = 1;

	
	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}
	
	if (msg.type() == "BasicStructure"){
		material = searchMaterials(msg.material(), buildingMaterials);
		element.wall.modelElemStructureType = API_BasicStructure;
		element.wall.buildingMaterial = material;
	}
	else if (msg.type() == "CompositeStructure"){
		material = searchMaterials(msg.material(), compositeMaterials);
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

	element.wall.rLinInd = material;
	element.wall.rLinEndInd = material;
	element.wall.refInd = material;
	element.wall.refEndInd = material;
	element.wall.oppInd = material;
	element.wall.oppEndInd = material;

	if (ACAPI_Element_Get(&slabElement) == NoError
		&& ACAPI_Element_GetMemo(slabElement.header.guid, &slabMemo, APIMemoMask_Polygon) == NoError) {
		element.header.floorInd = slabElement.header.floorInd;
		int pIndex = 1;
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
			/*
			if (i < numberOfAngles){
				element.wall.angle = (*slabMemo.parcs)[i].arcAngle;
			}
			else{
				element.wall.angle = 0;
			}
			*/
			element.wall.bottomOffset = slabElement.slab.level;

			/*
			sprintf(buffer, "BegIndex: %d EndIndex: %d ArcAngle: %f", (*slabMemo.parcs)[i].begIndex, (*slabMemo.parcs)[i].endIndex, (*slabMemo.parcs)[i].arcAngle);
			ACAPI_WriteReport(buffer, true);
			*/

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
		writeDelimitedTo(elementIDList, raw_out);

		delete(raw_out);
	}
	else{
		sprintf(buffer, "Fail");
		ACAPI_WriteReport(buffer, true);
	}

	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&slabMemo);
}

void	createCWallsFromSlab(){
	API_Element		slabElement, element, mask;
	API_ElementMemo slabMemo, memo;
	GSErrCode		err;
	elementid		eleIdMsg;
	doublemessage	heightMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &eleIdMsg);
	readDelimitedFrom(raw_in, &heightMsg);

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
		element.header.floorInd = currentLevel;
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

		sendElementID(element);
	}
}

void	createObject(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	objectmsg		objectMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &objectMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_ObjectID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		//sprintf(buffer, "Default Problem");
		//ACAPI_WriteReport(buffer, true);
	}

	element.header.floorInd = currentLevel;

	//sprintf(buffer, "libInd: %d", element.object.libInd);
	//ACAPI_WriteReport(buffer, true);

	element.object.libInd = objectMsg.index();
	element.object.pos.x = objectMsg.posx();
	element.object.pos.y = objectMsg.posy();

	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (slab)", err);
		//sprintf(buffer, ErrID_To_Name(err));
		//ACAPI_WriteReport(buffer, true);
	}

	sendElementID(element);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createRoof(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	roofmsg			roofMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	short			material;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(raw_in, &roofMsg);
	readDelimitedFrom(raw_in, &pointsMsg);
	readDelimitedFrom(raw_in, &polyarcsMsg);

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
		material = searchMaterials(roofMsg.material(), buildingMaterials);
		element.roof.shellBase.modelElemStructureType = API_BasicStructure;
		element.roof.shellBase.buildingMaterial = material;
		if (roofMsg.thickness() > 0){
			element.slab.thickness = roofMsg.thickness();
		}
	}
	else if (roofMsg.type() == "CompositeStructure"){
		material = searchMaterials(roofMsg.material(), compositeMaterials);
		element.roof.shellBase.composite = material;
		element.roof.shellBase.modelElemStructureType = API_CompositeStructure;
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

	element.header.floorInd = currentLevel;

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

	sendElementID(element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createNewRoof(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	roofmsg			roofMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	short			material;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(raw_in, &roofMsg);
	readDelimitedFrom(raw_in, &pointsMsg);

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

	if (roofMsg.type() == "Basic"){
		material = searchMaterials(roofMsg.material(), buildingMaterials);
		element.roof.shellBase.modelElemStructureType = API_BasicStructure;
		element.roof.shellBase.buildingMaterial = material;
		if (roofMsg.thickness() > 0){
			element.slab.thickness = roofMsg.thickness();
		}
	}
	else if (roofMsg.type() == "Composite"){
		material = searchMaterials(roofMsg.material(), compositeMaterials);
		element.roof.shellBase.composite = material;
		element.roof.shellBase.modelElemStructureType = API_CompositeStructure;
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

	sendElementID(element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createPolyRoof(){
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

	readDelimitedFrom(raw_in, &roofMsg);
	readDelimitedFrom(raw_in, &pointsMsg);
	readDelimitedFrom(raw_in, &polyarcsMsg);
	readDelimitedFrom(raw_in, &subPolyMsg);
	readDelimitedFrom(raw_in, &roofLevelsMsg);

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

	if (roofMsg.type() == "BasicStructure"){
		material = searchMaterials(roofMsg.material(), buildingMaterials);
		element.roof.shellBase.modelElemStructureType = API_BasicStructure;
		element.roof.shellBase.buildingMaterial = material;
		if (roofMsg.thickness() > 0){
			element.slab.thickness = roofMsg.thickness();
		}
	}
	else if (roofMsg.type() == "CompositeStructure"){
		material = searchMaterials(roofMsg.material(), compositeMaterials);
		element.roof.shellBase.composite = material;
		element.roof.shellBase.modelElemStructureType = API_CompositeStructure;
	}

	if (material == -1){
		crash = true;
	}

	element.roof.shellBase.level = roofMsg.height();

	element.header.floorInd = currentLevel;

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

	sendElementID(element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createHole(){
	API_Element		element, mask;
	API_ElementMemo memo, oldMemo;
	GSErrCode		err;
	holemsg			holeMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &holeMsg);
	readDelimitedFrom(raw_in, &pointsMsg);
	readDelimitedFrom(raw_in, &polyarcsMsg);

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

		ACAPI_ELEMENT_MASK_CLEAR(mask);
		ACAPI_ELEMENT_MASK_SET(mask, API_SlabType, poly);

		int oldSize = element.slab.poly.nCoords;
		int oldPends = element.slab.poly.nSubPolys;
		int oldArcSize = element.slab.poly.nArcs;
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
		/*
		for (int i = 1; i <= element.slab.poly.nCoords; i++){
			sprintf(buffer, "x %f y %f", (*memo.coords)[i].x, (*memo.coords)[i].y);
			ACAPI_WriteReport(buffer, true);
		}

		for (int i = 0; i <= element.slab.poly.nSubPolys; i++){
			sprintf(buffer, "pends %d", (*memo.pends)[i]);
			ACAPI_WriteReport(buffer, true);
		}
		*/
		auxIndex = 0;
		for (int i = oldArcSize; i < polyarcsMsg.arcangle_size(); i++){
			(*memo.parcs)[i].begIndex = polyarcsMsg.begindex(auxIndex);
			(*memo.parcs)[i].endIndex = polyarcsMsg.endindex(auxIndex);
			(*memo.parcs)[i].arcAngle = polyarcsMsg.arcangle(auxIndex);
			auxIndex++;
		}
		//////////////////////////////////////////////////

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

		if (err != NoError){
			ErrorBeep("ACAPI_Element_Create (slab)", err);
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}

		sendElementID(element);

		element.header.guid = APIGuidFromString(holeMsg.guid().c_str());
		ACAPI_Element_Get(&element);
		API_Elem_Head* toDelete;
		toDelete = &element.header;
		ACAPI_Element_Delete(&toDelete, 1);
	}
	else{
		sprintf(buffer, "Error on retreiving slab");
		ACAPI_WriteReport(buffer, true);
		sendElementID(element);

	}
	
	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&oldMemo);
	
}

void	createHoleTest(){
	API_Element			element;
	API_ElementMemo		memo, subMemo;
	GSErrCode			err;
	holemsg			holeMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &holeMsg);
	readDelimitedFrom(raw_in, &pointsMsg);
	readDelimitedFrom(raw_in, &polyarcsMsg);

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

		sendElementID(element);
	}
}

void	createStairs(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	stairsmsg		stairsMsg;
	char buffer[256];
	bool crash = false;

	readDelimitedFrom(raw_in, &stairsMsg);

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

	element.header.floorInd = stairsMsg.bottomindex();

	element.object.pos.x = stairsMsg.posx();
	element.object.pos.y = stairsMsg.posy();
	element.object.useXYFixSize = stairsMsg.usexyfixsize();
	element.object.useObjSectAttrs = true;
	element.object.xRatio = stairsMsg.xratio();
	element.object.yRatio = stairsMsg.yratio();
	element.object.level = stairsMsg.bottom();
	element.object.angle = stairsMsg.angle();
	element.object.libInd = searchObjects(stairsMsg.name(), objectsIds);

	if (element.object.libInd == -1){
		crash = true;
	}

	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (slab)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	sendElementID(element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createMesh(){
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

	readDelimitedFrom(raw_in, &meshMsg);
	readDelimitedFrom(raw_in, &pointsMsg);
	readDelimitedFrom(raw_in, &linesMsg);

	element.header.typeID = API_MeshID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		sprintf(buffer, "Default Problem");
		ACAPI_WriteReport(buffer, true);
	}

	element.header.floorInd = meshMsg.bottomlevel();

	element.mesh.level = meshMsg.level();

	material = searchMaterials(meshMsg.material(), buildingMaterials);

	if (material == -1){
		crash = true;
	}
	else{
		element.mesh.buildingMaterial = material;
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
	sendElementID(element, crash);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createMorph(){
	API_Element		element;
	GSErrCode		err;
	morphmsg		morphMsg;
	pointsmessage	pointsMsg;
	pointsmessage	edgesMsg;
	pointsmessage	polygonsMsg;
	intlistmsg		polygonsSizes;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(raw_in, &morphMsg);
	readDelimitedFrom(raw_in, &pointsMsg);
	readDelimitedFrom(raw_in, &edgesMsg);
	readDelimitedFrom(raw_in, &polygonsMsg);
	readDelimitedFrom(raw_in, &polygonsSizes);

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.typeID = API_MorphID;
	err = ACAPI_Element_GetDefaults(&element, NULL);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (morph)", err);
		return;
	}

	double* tmx = element.morph.tranmat.tmx;
	tmx[0] = 1.0; tmx[4] = 0.0; tmx[8] = 0.0;
	tmx[1] = 0.0; tmx[5] = 1.0; tmx[9] = 0.0;
	tmx[2] = 0.0; tmx[6] = 0.0;	tmx[10] = 1.0;
	tmx[3] = morphMsg.refx(); tmx[7] = morphMsg.refy(); tmx[11] = morphMsg.refz();

	// build the body structure
	void* bodyData = NULL;
	ACAPI_Body_Create(NULL, NULL, &bodyData);
	if (bodyData == NULL) {
		ErrorBeep("bodyData == NULL", APIERR_MEMFULL);
		return;
	}

	// define the vertices
	// the dimensions of the morph element to be created
	const int numVertices = pointsMsg.px_size();
	UInt32* vertices = new UInt32[numVertices];

	for (int i = 0; i < numVertices; i++){
		API_Coord3D coord = {pointsMsg.px(i), pointsMsg.py(i), pointsMsg.pz(i)};
		ACAPI_Body_AddVertex(bodyData, coord, vertices[i]);
	}

	// connect the vertices to determine edges
	const int numEdges = edgesMsg.px_size();
	Int32* edges = new Int32[numEdges];
	for (int i = 0; i < numEdges; i++){
		ACAPI_Body_AddEdge(bodyData, vertices[(int)edgesMsg.px(i)], vertices[(int)edgesMsg.py(i)], edges[i]);
	}
	
	// determine polygons from edges
	API_MaterialOverrideType material;
	material.overrideMaterial = false;
	material.material = 1;
	UInt32* polygons = new UInt32[polygonsSizes.ilist_size()];
	Int32* polyEdges = new Int32[numEdges];

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
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (morph)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	sendElementID(element);

	ACAPI_DisposeElemMemoHdls(&memo);
	delete(vertices);
	delete(edges);
	delete(polygons);
	delete(polyEdges);
	return;
}

void	createBox(){
	API_Element		element;
	GSErrCode		err;
	boxmsg			boxMsg;
	bool crash = false;
	char buffer[256];

	readDelimitedFrom(raw_in, &boxMsg);

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
	tmx[3] = 0.0; tmx[7] = 0.0; tmx[11] = 1.0;

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
	API_MaterialOverrideType material;
	material.overrideMaterial = true;

	UInt32 polygons[6];
	Int32 polyEdges[4];

	polyEdges[0] = edges[0];
	polyEdges[1] = edges[1];
	polyEdges[2] = edges[2];
	polyEdges[3] = edges[3];
	material.material = 2;
	//ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, polyNormals[2], material, polygons[0]);
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[0]);
	
	polyEdges[0] = edges[4];
	polyEdges[1] = edges[5];
	polyEdges[2] = edges[6];
	polyEdges[3] = edges[7];
	material.material = 1;
	//ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, polyNormals[2], material, polygons[1]);
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[1]);
	
	polyEdges[0] = edges[0];
	polyEdges[1] = edges[9];
	polyEdges[2] = -edges[4];
	polyEdges[3] = -edges[8];
	material.material = 2;
	//ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, -polyNormals[1], material, polygons[2]);
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[2]);

	polyEdges[0] = edges[1];
	polyEdges[1] = edges[10];
	polyEdges[2] = -edges[5];
	polyEdges[3] = -edges[9];
	material.material = 2;
	//ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, polyNormals[0], material, polygons[3]);
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[3]);

	polyEdges[0] = -edges[2];
	polyEdges[1] = edges[10];
	polyEdges[2] = edges[6];
	polyEdges[3] = -edges[11];
	material.material = 2;
	//ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, polyNormals[1], material, polygons[4]);
	ACAPI_Body_AddPolygon(bodyData, polyEdges, 4, 0, material, polygons[4]);

	polyEdges[0] = edges[3];
	polyEdges[1] = edges[8];
	polyEdges[2] = -edges[7];
	polyEdges[3] = -edges[11];
	material.material = 2;
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
	sendElementID(element);

	ACAPI_DisposeElemMemoHdls(&memo);
	return;
}

void	translateElement(){
	API_Element		element;
	API_EditPars	editPars;
	GSErrCode		err = NoError;
	translatemsg	translateMsg;
	API_Neig		**items;
	API_Neig		*auxItems;
	API_ElemTypeID  elemTypeID;
	char buffer[256];

	readDelimitedFrom(raw_in, &translateMsg);

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

	(*items)[0].neigID = APINeig_CurtainWall;			(*items)[0].inIndex = 1;

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

void	rotateElementZ(){
	API_Element		element, mask;
	API_ElementMemo memo;
	GSErrCode		err;
	rotatemsg		rotMsg;
	elementidlist	elementIDList;
	char buffer[256];

	readDelimitedFrom(raw_in, &rotMsg);
	
	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	
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
	ACAPI_DisposeElemMemoHdls(&memo);
	elementIDList.set_crashmaterial(false);
	writeDelimitedTo(elementIDList, raw_out);
	delete(raw_out);
}

//ONLY WORKS WITH SHELLS AND ROOFS - NO SLABS 
void	trimElement(){
	
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	trimmsg			trimMsg;
	GS::Array<API_Guid> guidsToTrim;
	char buffer[256];

	readDelimitedFrom(raw_in, &trimMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	guidsToTrim.Push(APIGuidFromString(trimMsg.guid1().c_str()));
	guidsToTrim.Push(APIGuidFromString(trimMsg.guid2().c_str()));

	//err = ACAPI_Element_Trim_Elements(guidsToTrim);
	err = ACAPI_Element_Merge_Elements(guidsToTrim);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (slab)", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}

	element.header.guid = APIGuidFromString(trimMsg.guid1().c_str());
	ACAPI_Element_Get(&element);

	ACAPI_DisposeElemMemoHdls(&memo);
	
	sendElementID(element);
	
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

void	intersections(API_Coord* polygon, int nPoints, API_Coord beg, API_Coord end, std::list<API_Coord>* intersections){
	int numPoints = nPoints;
	int pIndex = 1;
	//Segment BQ(Point(beg.x, beg.y), Point(end.x, end.y));
	Segment BE(beg, end);
	API_Coord intrsct;
	bool repeatedIntersection = false;
	char buffer[256];

	for (int i = 0; i < numPoints - 1; i++){
		//Segment II(Point(polygon[pIndex].x, polygon[pIndex].y), Point(polygon[pIndex + 1].x, polygon[pIndex + 1].y));
		API_Coord p1 = polygon[pIndex];
		API_Coord p2 = polygon[pIndex + 1];
		Segment II(p1, p2);
		std::vector<API_Coord> output;
		//std::deque<Point> output;
		boost::geometry::intersection(BE, II, output);
		
		for (int i = 0; i < output.size(); i++){
			intrsct.x = output[i].x;
			intrsct.y = output[i].y;
			//sprintf(buffer, "ix: %f iy: %f", intrsct.x, intrsct.y);
			//ACAPI_WriteReport(buffer, true);

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
		pIndex++;
	}
}

void	intersectWall(){
	API_Element		element, mask;
	API_Element		elementToIntersect;
	//API_ElementMemo memo;
	API_ElementMemo memoToIntersect;
	GSErrCode		err;
	intersectmsg	intersectMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &intersectMsg);

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

			sendElementID(elementToIntersect);
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
			sendElementID(element);
		}

		ACAPI_DisposeElemMemoHdls(&memoToIntersect);
	}
}

void	destructiveIntersectWall(){
	API_Element		element, mask;
	API_Element		elementToIntersect;
	//API_ElementMemo memo;
	API_ElementMemo memoToIntersect;
	GSErrCode		err;
	intersectmsg	intersectMsg;
	API_Elem_Head*  test;
	char buffer[256];

	readDelimitedFrom(raw_in, &intersectMsg);

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

			sendElementID(elementToIntersect);
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
			sendElementID(element);
		}

		test = &elementToIntersect.header;
		ACAPI_Element_Delete(&test, 1);

		ACAPI_DisposeElemMemoHdls(&memoToIntersect);

	}
}

void	mirrorElement(){
	API_Element		element, mask;
	API_ElementMemo memo;
	GSErrCode		err;
	mirrormsg		mirrorMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &mirrorMsg);

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
	sendElementID(element);
}

//----------- Shells

void	createSimpleShell(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	shellcomplexmessage	shellMsg;
	pointsmessage	pointsMsg;
	doublemessage	doubleMsg;

	readDelimitedFrom(raw_in, &shellMsg);


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

	shellElement.header.floorInd = currentLevel;

	double* tmx = shellElement.shell.basePlane.tmx;

	/*
	for (int i = 0; i < 12; i++){
	readDelimitedFrom(raw_in, &doubleMsg);
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

	readDelimitedFrom(raw_in, &pointsMsg);

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

	sendElementID(shellElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createShell(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	shellmessage	shellMsg;
	doublemessage	doubleMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;

	readDelimitedFrom(raw_in, &shellMsg);


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

	shellElement.header.floorInd = currentLevel;

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

	readDelimitedFrom(raw_in, &pointsMsg);

	for (int i = 1; i <= shellMsg.numpoints(); i++){
		(*memo.shellShapes[0].coords)[i].x = pointsMsg.px(i - 1);
		(*memo.shellShapes[0].coords)[i].y = pointsMsg.py(i - 1);
		(*memo.shellShapes[0].bodyFlags)[i] = true;
	}
	/*
	for (int i = 1; i <= shellMsg.numpoints(); i++){
	readDelimitedFrom(raw_in, &pointMsg);
	(*memo.shellShapes[0].coords)[i].x = pointMsg.p0x();
	(*memo.shellShapes[0].coords)[i].y = pointMsg.p0y();
	(*memo.shellShapes[0].bodyFlags)[i] = true;
	}
	*/
	(*memo.shellShapes[0].pends)[1] = shellElement.shell.u.revolvedShell.shellShape.nCoords;


	readDelimitedFrom(raw_in, &polyarcsMsg);

	for (int i = 0; i < shellMsg.numarcs(); i++){
		(*memo.shellShapes[0].parcs)[i].begIndex = polyarcsMsg.begindex(i);
		(*memo.shellShapes[0].parcs)[i].endIndex = polyarcsMsg.endindex(i);
		(*memo.shellShapes[0].parcs)[i].arcAngle = polyarcsMsg.arcangle(i);
	}

	/*
	for (int i = 0; i < shellMsg.numarcs(); i++){
	readDelimitedFrom(raw_in, &polyarcMsg);
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

	sendElementID(shellElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	createComplexShell(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	shellcomplexmessage	shellMsg;
	pointmessage	pointMsg;
	polyarcmessage	polyarcMsg;
	doublemessage	doubleMsg;

	readDelimitedFrom(raw_in, &shellMsg);


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
	shellElement.header.floorInd = currentLevel;

	double* tmx = shellElement.shell.basePlane.tmx;
	for (int i = 0; i < 12; i++){
		readDelimitedFrom(raw_in, &doubleMsg);
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
		readDelimitedFrom(raw_in, &pointMsg);
		(*memo.shellShapes[0].coords)[i].x = pointMsg.p0x();
		(*memo.shellShapes[0].coords)[i].y = pointMsg.p0y();
		(*memo.shellShapes[0].bodyFlags)[i] = true;
	}
	//(*memo.shellShapes[0].coords)[shellMsg.numpoints() + 1] = (*memo.shellShapes[0].coords)[1];
	//(*memo.shellShapes[0].bodyFlags)[shellMsg.numpoints() + 1] = (*memo.shellShapes[0].bodyFlags)[1];
	(*memo.shellShapes[0].pends)[1] = shellElement.shell.u.revolvedShell.shellShape.nCoords;

	for (int i = 0; i < shellMsg.numarcs(); i++){
		readDelimitedFrom(raw_in, &polyarcMsg);
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
			readDelimitedFrom(raw_in, &pointMsg);
			(*memo.shellContours[0].coords)[i].x = pointMsg.p0x();
			(*memo.shellContours[0].coords)[i].y = pointMsg.p0y();
		}

		(*memo.shellContours[0].pends)[1] = memo.shellContours[0].poly.nCoords;

		for (int i = 0; i < shellMsg.numharcs(); i++){
			readDelimitedFrom(raw_in, &polyarcMsg);
			(*memo.shellContours[0].parcs)[i].begIndex = polyarcMsg.begindex();
			(*memo.shellContours[0].parcs)[i].endIndex = polyarcMsg.endindex();
			(*memo.shellContours[0].parcs)[i].arcAngle = polyarcMsg.arcangle();
		}

		memo.shellContours[0].height = shellMsg.holeheight();

		tmx = memo.shellContours[0].plane.tmx;

		for (int i = 0; i < 12; i++){
			readDelimitedFrom(raw_in, &doubleMsg);
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

	sendElementID(shellElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	rotateShell(){
	API_Element		shellElement, mask;
	GSErrCode		err;
	rotshellmessage	rotShellMsg;

	readDelimitedFrom(raw_in, &rotShellMsg);

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

	sendElementID(shellElement);
}

void	translateShell(){
	API_Element		shellElement, mask;
	GSErrCode		err;
	tshellmessage	tShellMsg;

	readDelimitedFrom(raw_in, &tShellMsg);

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

	sendElementID(shellElement);
}

void	createHoleInShell(){
	API_Element		shellElement, mask;
	API_ElementMemo memo;
	GSErrCode		err;
	oldholemessage		holeMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &holeMsg);

	readDelimitedFrom(raw_in, &pointsMsg);

	readDelimitedFrom(raw_in, &polyarcsMsg);

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
	sendElementID(shellElement);
}

//----------- Story Functions

void	checkStory(){
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

	writeDelimitedTo(storyInfoMsg, raw_out);
	delete(raw_out);

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

void	checkStoryAbove(){
	API_StoryInfo		storyInfo;
	GSErrCode			err;
	storymsg			storyMsg;
	storyinfo			storyInfoMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &storyMsg);

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
	
	writeDelimitedTo(storyInfoMsg, raw_out);
	delete(raw_out);
	
	BMKillHandle(reinterpret_cast<GSHandle*> (&storyInfo.data));
	return;
}

void	checkStoryBelow(){
	API_StoryInfo		storyInfo;
	GSErrCode			err;
	storymsg			storyMsg;
	storyinfo			storyInfoMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &storyMsg);

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

	writeDelimitedTo(storyInfoMsg, raw_out);
	delete(raw_out);

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

void	createStory(){
	API_StoryInfo		storyInfo;
	GSErrCode			err;
	storymsg			storyMsg;
	API_StoryCmdType	storyCmd;
	storyinfo			storyInfoMsg;
	bool				newHeight = true;
	char buffer[256];
	double				diff = 10000000;
	double				currentDiff = 0;
	readDelimitedFrom(raw_in, &storyMsg);

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
		if (doublediff < 0.1 && doublediff > -0.1){
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
		storyCmd.index = currentLevel;
		storyCmd.height = abs(storyMsg.height() - (*storyInfo.data)[currentLevel - storyInfo.firstStory].level);
		for (int i = 0; i < storyMsg.name().size(); i++){
			storyCmd.name[i] = storyMsg.name().c_str()[i];
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
		if (err != NoError){
			ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
		}
	}
	
	storyInfoMsg.set_exists(true);
	storyInfoMsg.set_index(currentLevel);
	storyInfoMsg.set_level((*storyInfo.data)[currentLevel - storyInfo.firstStory].level);
	storyInfoMsg.set_name("Story");

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));

	writeDelimitedTo(storyInfoMsg, raw_out);
	delete(raw_out);

	storyCmd.action = APIStory_GoTo;
	storyCmd.index = currentLevel;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
}

void	upperLevel(){
	API_StoryInfo		storyInfo;
	GSErrCode			err;
	upperlevelmsg		upperLevelMsg;
	API_StoryCmdType	storyCmd;
	storyinfo			storyInfoMsg;
	bool				newHeight = true;
	double				diff = 10000000;
	double				currentDiff = 0;
	char buffer[256];

	readDelimitedFrom(raw_in, &upperLevelMsg);

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
	string name = "Story";
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
		strcpy(storyCmd.name, "Story");
		storyCmd.action = APIStory_InsAbove;
		currentLevel++;
		
		err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
		if (err != NoError){
			ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
		}
	}
	//int upperlevelindex = currentLevel + 1;
	storyInfoMsg.set_exists(true);
	storyInfoMsg.set_index(currentLevel);
	storyInfoMsg.set_level((*storyInfo.data)[currentLevel - storyInfo.firstStory].level);
	storyInfoMsg.set_name("Story");

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));

	writeDelimitedTo(storyInfoMsg, raw_out);
	delete(raw_out);
	
	storyCmd.action = APIStory_GoTo;
	storyCmd.index = currentLevel - 1;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
	
}

void	createStoryAbove(){
	API_StoryCmdType	storyCmd;
	GSErrCode			err;
	storymsg			storyMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &storyMsg);
	
	BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
	storyCmd.action = APIStory_InsAbove;
	storyCmd.index = currentLevel;
	storyCmd.height = storyMsg.height();
	
	for (int i = 0; i < storyMsg.name().size(); i++){
		storyCmd.name[i] = storyMsg.name().c_str()[i];
	}

	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
	currentLevel++;

	storyCmd.action = APIStory_GoTo;
	storyCmd.index = currentLevel;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
}

void	createStoryBelow(){
	API_StoryCmdType	storyCmd;
	GSErrCode			err;
	storymsg			storyMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &storyMsg);

	BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
	storyCmd.action = APIStory_InsBelow;
	//storyCmd.index = (short)(storyInfo.firstStory - storyInfo.lastStory);
	storyCmd.index = currentLevel;
	storyCmd.height = storyMsg.height();

	for (int i = 0; i < storyMsg.name().size(); i++){
		storyCmd.name[i] = storyMsg.name().c_str()[i];
	}

	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);

		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
	}
	currentLevel--;

	storyCmd.action = APIStory_GoTo;
	storyCmd.index = currentLevel;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
}

void	chooseStory(){
	doublemessage		doubleMsg;
	API_StoryCmdType	storyCmd;
	GSErrCode			err;

	readDelimitedFrom(raw_in, &doubleMsg);

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

void	deleteStories(){
	API_StoryInfo		storyInfo;
	API_StoryCmdType	storyCmd;
	GSErrCode			err;
	char buffer[256];

	BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
	

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	int numberOfStories = BMhGetSize(reinterpret_cast<GSHandle> (storyInfo.data)) / Sizeof32(API_StoryType) - 1;

	storyCmd.action = APIStory_InsAbove;
	storyCmd.index = storyInfo.lastStory;
	storyCmd.height = 10;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}

	storyCmd.action = APIStory_Delete;
	for (int i = 0; i < numberOfStories; i++){
		storyCmd.index = (*storyInfo.data)[0].index;
		err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
		if (err != NoError){
			ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
		}
	}

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	//sprintf(buffer, "index %d", (*storyInfo.data)[0].index);
	//ACAPI_WriteReport(buffer, true);

	currentLevel = 0;

	storyCmd.action = APIStory_GoTo;
	storyCmd.index = currentLevel;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

void	getLevels(){
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

	writeDelimitedTo(levelsMsg, raw_out);
	delete(raw_out);

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

//----------- Delete Elements

void	deleteElements(){
	API_Element		element;
	elementidlist	elementsToDelete;
	
	char buffer[256];

	readDelimitedFrom(raw_in, &elementsToDelete);

	BNZeroMemory(&element, sizeof(API_Element));

	API_Elem_Head* test;
	for (int i = 0; i < elementsToDelete.guid_size(); i++){
		element.header.guid = APIGuidFromString(elementsToDelete.guid(i).c_str());
		if (ACAPI_Element_Get(&element) == NoError){
			test = &element.header;
			ACAPI_Element_Delete(&test, 1);
		}
	}
	
}

//----------- Element Management

void	getWalls(){
	API_Element element;
	GSErrCode err;
	char buffer[256];
	wallinfo wallInfo;
	wallrepeated wallMsg;
	API_StoryInfo		storyInfo;
	storyinfo* storyInfoMsg;

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}

	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_WallID, &elemList);
	int counter = 0;
	for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate(); it != NULL; ++it) {

		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			double x1 = element.wall.begC.x;
			double y1 = element.wall.begC.y;
			double x2 = element.wall.endC.x;
			double y2 = element.wall.endC.y;
			double length = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
			double volume = element.wall.thickness * length * element.wall.height;
			/*
			sprintf(buffer, "Thickness: %f", element.wall.thickness);
			ACAPI_WriteReport(buffer, true);
			sprintf(buffer, "Length: %f", length);
			ACAPI_WriteReport(buffer, true);
			sprintf(buffer, "Height: %f", element.wall.height);
			ACAPI_WriteReport(buffer, true);
			sprintf(buffer, "Volume: %f", volume);
			ACAPI_WriteReport(buffer, true);
			*/
			wallInfo.add_height(element.wall.height);
			wallInfo.add_thickness(element.wall.thickness);
			wallInfo.add_length(length);
			wallInfo.add_volume(volume);

			wallMsg.add_p0x(x1);
			wallMsg.add_p0y(y1);
			wallMsg.add_p1x(x2);
			wallMsg.add_p1y(y2);
			wallMsg.add_thickness(element.wall.thickness);
			wallMsg.add_angle(element.wall.angle);

			string materialName;
			
			if (element.wall.modelElemStructureType == API_BasicStructure){
				materialName = searchMaterialsValue(element.wall.buildingMaterial, buildingMaterials);
				wallMsg.add_type("Basic");
			}
			else{
				materialName = searchMaterialsValue(element.wall.composite, compositeMaterials);
				wallMsg.add_type("Composite");
			}
			if (materialName == "Not Found"){
				sprintf(buffer, "Found no material %d", element.wall.buildingMaterial);
				ACAPI_WriteReport(buffer, true);
			}
			else{
				wallMsg.add_material(materialName);
			}
			
			if (element.wall.referenceLineLocation == APIWallRefLine_Center){
				wallMsg.add_referenceline("Center");
			}
			else if (element.wall.referenceLineLocation == APIWallRefLine_Outside){
				wallMsg.add_referenceline("Outside");
			}
			else if (element.wall.referenceLineLocation == APIWallRefLine_Inside){
				wallMsg.add_referenceline("Inside");
			}

			wallMsg.add_alphaangle(element.wall.slantAlpha);
			wallMsg.add_betaangle(element.wall.slantBeta);

			if (element.wall.profileType == APISect_Normal){
				wallMsg.add_typeprofile("Normal");
			}
			else if (element.wall.profileType == APISect_Slanted){
				wallMsg.add_typeprofile("Slanted");
			}
			else if (element.wall.profileType == APISect_Trapez){
				wallMsg.add_typeprofile("DoubleSlanted");
			}
			char s[64];
			APIGuid2GSGuid(element.header.guid).ConvertToString(s);
			wallMsg.add_guid(s);

			storyInfoMsg = wallMsg.add_bottomlevel();
			storyInfoMsg->set_exists(true);
			storyInfoMsg->set_index(element.header.floorInd);
			storyInfoMsg->set_level((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].level);
			storyInfoMsg->set_name((*storyInfo.data)[element.header.floorInd - storyInfo.firstStory].name);

			storyInfoMsg = wallMsg.add_toplevel();
			storyInfoMsg->set_exists(true);
			storyInfoMsg->set_index(element.header.floorInd + 1);
			storyInfoMsg->set_level((*storyInfo.data)[element.header.floorInd + 1 - storyInfo.firstStory].level);
			storyInfoMsg->set_name((*storyInfo.data)[element.header.floorInd + 1 - storyInfo.firstStory].name);
			
			counter++;

		}
		else{
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}
	}
	//writeDelimitedTo(wallInfo, raw_out);
	writeDelimitedTo(wallMsg, raw_out);
	delete(raw_out);
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

void	getSlabs(){
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
				for (int i = 1; i <= element.slab.poly.nCoords; i++){
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
				
				string materialName;

				if (element.slab.modelElemStructureType == API_BasicStructure){
					materialName = searchMaterialsValue(element.slab.buildingMaterial, buildingMaterials);
					slabMsg.add_type("Basic");
				}
				else{
					materialName = searchMaterialsValue(element.slab.composite, compositeMaterials);
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
	writeDelimitedTo(slabMsg, raw_out);
	delete(raw_out);
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

//void	getRoofs(){}

void	getColumns(){
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
	writeDelimitedTo(columnMsg, raw_out);
	delete(raw_out);
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

void	getObjects(){
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
			
			string objectName = searchObjectsValue(element.object.libInd, objectsIds);
			
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
	writeDelimitedTo(objectMsg, raw_out);
	delete(raw_out);
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

void	getRoofs(){
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

				string materialName;

				if (element.roof.shellBase.modelElemStructureType == API_BasicStructure){
					materialName = searchMaterialsValue(element.roof.shellBase.buildingMaterial, buildingMaterials);
					roofMsg.add_type("Basic");
				}
				else{
					materialName = searchMaterialsValue(element.roof.shellBase.composite, compositeMaterials);
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
	writeDelimitedTo(roofMsg, raw_out);
	delete(raw_out);
	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
}

//TODO
void	getMeshes(){


}
//TODO more gets!


void	selectElement(){
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

	sendElementID(element);

}

void	highlightElementByID(){
	GSErrCode err;
	API_NeigID neigID;
	API_Element element;
	elementid eleMsg;
	API_StoryCmdType	storyCmd;
	char buffer[256];

	readDelimitedFrom(raw_in, &eleMsg);

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

//----------- Test Function

void	testFunction(){
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
	
	GSErrCode err;
	API_NeigID neigID;
	API_Element element;
	elementid eleMsg;
	API_StoryCmdType	storyCmd;
	char buffer[256];

	readDelimitedFrom(raw_in, &eleMsg);

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
}

// ---------------------------------- Old Functions ----------------------------

void	oldWorkcreateHoleInShell(){
	API_Element		shellElement, mask;
	API_ElementMemo memo;
	GSErrCode		err;
	oldholemessage		holeMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &holeMsg);

	readDelimitedFrom(raw_in, &pointsMsg);

	readDelimitedFrom(raw_in, &polyarcsMsg);

	BNZeroMemory(&shellElement, sizeof(API_Element));
	//BNZeroMemory(&memo, sizeof(API_ElementMemo));

	//Set the element GUID
	shellElement.header.guid = APIGuidFromString(holeMsg.guid().c_str());

	//Get both the element and memo
	if (ACAPI_Element_Get(&shellElement) == NoError
		&& ACAPI_Element_GetMemo(shellElement.header.guid, &memo, APIMemoMask_Polygon) == NoError) {

		shellElement.shell.numHoles = shellElement.shell.numHoles + 1;
		USize nContours = shellElement.shell.numHoles + (shellElement.shell.hasContour ? 1 : 0);

		memo.shellContours = (API_ShellContourData *)BMAllocatePtr(nContours * sizeof(API_ShellContourData), ALLOCATE_CLEAR, 0);

		if (memo.shellContours == NULL) {
			ErrorBeep("Not enough memory to create shell contour data", APIERR_MEMFULL);
			WriteReport_End(APIERR_MEMFULL);
		}

		memo.shellContours[0].poly.nCoords = pointsMsg.px_size();
		memo.shellContours[0].poly.nSubPolys = 1;
		memo.shellContours[0].poly.nArcs = polyarcsMsg.arcangle_size();
		memo.shellContours[0].coords = (API_Coord**)BMAllocateHandle((memo.shellContours[0].poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
		memo.shellContours[0].pends = reinterpret_cast<Int32**> (BMAllocateHandle((memo.shellContours[0].poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
		memo.shellContours[0].parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(memo.shellContours[0].poly.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

		if (memo.shellContours[0].coords == NULL || memo.shellContours[0].pends == NULL || memo.shellContours[0].parcs == NULL) {
			ErrorBeep("Not enough memory to create shell contour data", APIERR_MEMFULL);
			WriteReport_End(APIERR_MEMFULL);
		}

		for (int i = 1; i <= memo.shellContours[0].poly.nCoords; i++){
			(*memo.shellContours[0].coords)[i].x = pointsMsg.px(i - 1);
			(*memo.shellContours[0].coords)[i].y = pointsMsg.py(i - 1);
		}

		(*memo.shellContours[0].pends)[1] = memo.shellContours[0].poly.nCoords;

		for (int i = 0; i < memo.shellContours[0].poly.nArcs; i++){

			(*memo.shellContours[0].parcs)[i].begIndex = polyarcsMsg.begindex(i);
			(*memo.shellContours[0].parcs)[i].endIndex = polyarcsMsg.endindex(i);
			(*memo.shellContours[0].parcs)[i].arcAngle = polyarcsMsg.arcangle(i);
		}

		memo.shellContours[0].height = holeMsg.height();

		double* tmx = memo.shellContours[0].plane.tmx;

		tmx[0] = 1.0f; tmx[1] = 0.0f; tmx[2] = 0.0f; tmx[3] = 0.0f;
		tmx[4] = 0.0f; tmx[5] = 1.0f; tmx[6] = 0.0f; tmx[7] = 0.0f;
		tmx[8] = 0.0f; tmx[9] = 0.0f; tmx[10] = 1.0f; tmx[11] = 10.0f;

		memo.shellContours[0].edgeData = (API_ContourEdgeData*)BMAllocatePtr((memo.shellContours[0].poly.nCoords + 1) * sizeof(API_ContourEdgeData), ALLOCATE_CLEAR, 0);

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

void	OLDaddArcsToElement(){
	API_Element			element, mask, aux;
	API_ElementMemo		memo;
	GSErrCode			err;
	elementid			eleGuid;
	polyarcsmessage		polyarcsMsg;

	readDelimitedFrom(raw_in, &eleGuid);

	readDelimitedFrom(raw_in, &polyarcsMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	//BNZeroMemory(&memo, sizeof(API_ElementMemo));	

	element.header.guid = APIGuidFromString(eleGuid.guid().c_str());

	err = ACAPI_Element_Get(&element);

	if (err == NoError
		&& ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_Polygon) == NoError) {

		//memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((polyarcsMsg.arcangle_size() + 1) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
		if (memo.parcs != NULL){

			for (int i = 0; i < polyarcsMsg.arcangle_size(); i++){
				//(*memo.parcs)[i].begIndex = polyarcsMsg.begindex(i);
				//(*memo.parcs)[i].endIndex = polyarcsMsg.endindex(i);
				//(*memo.parcs)[i].arcAngle = polyarcsMsg.arcangle(i);
				(*memo.parcs)[i].arcAngle = 0.0;
			}

			err = ACAPI_Element_Change(&element, &mask, &memo, APIMemoMask_Polygon | APIMemoMask_EdgeTrims, true);

			if (err != NoError){
				//createCircle();
				ErrorBeep("Cannot add arcs", err);
			}
		}
		else {
			ErrorBeep("Cannot add arcs", err);
		}


		/*

		//err = ACAPI_Element_ChangeMemo(element.header.guid, APIMemoMask_Polygon, &memo);

		if ((*memo.parcs)[0].begIndex == 1){
		ErrorBeep("Cannot add arcs", err);
		}

		if ((*memo.coords)[1].x == 0.0){
		//ErrorBeep("Cannot add arcs", err);
		}

		//element.curtainWall.height = 20.0;

		//ACAPI_ELEMENT_MASK_CLEAR(mask);
		//ACAPI_ELEMENT_MASK_SET(mask, API_CurtainWallType, height);

		//ACAPI_ELEMENT_MASK_CLEAR(mask);

		//aux.header.guid = APIGuidFromString(eleGuid.guid().c_str());
		//ACAPI_Element_Get(&aux);

		//element.curtainWall.height = 10.0;

		err = ACAPI_Element_Create(&element, &memo);

		//API_Elem_Head* test[1] = { &aux.header };
		//ACAPI_Element_Delete(test, 1);

		//err = ACAPI_Element_Change(&element, &mask, &memo, 1, true);

		//err = ACAPI_Element_ChangeMemo(element.header.guid, APIMemoMask_All, &memo);

		if (err != NoError){
		//createCircle();
		ErrorBeep("Cannot add arcs", err);
		}
		*/

	}

	sendElementID(element);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	OLDTWOaddArcsToElement(){
	API_Element			element, mask, aux;
	API_ElementMemo		memo, memo2;
	GSErrCode			err;
	elementid			eleGuid;
	polyarcsmessage		polyarcsMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &eleGuid);

	readDelimitedFrom(raw_in, &polyarcsMsg);

	BNZeroMemory(&element, sizeof(API_Element));

	element.header.guid = APIGuidFromString(eleGuid.guid().c_str());

	if (ACAPI_Element_Get(&element) == NoError
		&& ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_Polygon) == NoError) {

		for (int i = 1; i <= 3; i++){
			sprintf(buffer, "x: %f y: %f", (*memo.coords)[i].x, (*memo.coords)[i].y);
			ACAPI_WriteReport(buffer, true);
		}

		for (int i = 0; i < polyarcsMsg.arcangle_size(); i++){
			sprintf(buffer, "bindex: %d eindex: %d angle: %f", (*memo.parcs)[i].begIndex, (*memo.parcs)[i].endIndex, (*memo.parcs)[i].arcAngle);
			ACAPI_WriteReport(buffer, true);
		}

		BMKillHandle((GSHandle *)&memo.parcs);
		memo.parcs = (API_PolyArc**)BMhAllClear((polyarcsMsg.arcangle_size()) * sizeof(API_PolyArc));
		//memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((polyarcsMsg.arcangle_size() + 1) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

		if (memo.parcs != NULL){

			//sprintf(buffer, "x: %f y: %f", (*memo.coords)[2].x, (*memo.coords)[2].y);
			//ACAPI_WriteReport(buffer, true);
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

			//element.curtainWall.polygon.nArcs = polyarcsMsg.arcangle_size();
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

	sendElementID(element);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	ToSendaddArcsToElement(){
	API_Element			element, mask, aux;
	API_ElementMemo		memo, memo2;
	GSErrCode			err;
	elementid			eleGuid;
	char buffer[256];

	/*
	* This function readDelimitedFrom is used to communicate with
	* a Racket Application, using sockets and protocol buffers.
	* Basically this gives me the information needed about the
	* the curtain wall that was created
	*
	*/
	readDelimitedFrom(raw_in, &eleGuid);
	BNZeroMemory(&element, sizeof(API_Element));

	//Here I'm getting the guid from the protocol buffers message.
	element.header.guid = APIGuidFromString(eleGuid.guid().c_str());


	if (ACAPI_Element_Get(&element) == NoError
		&& ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_Polygon) == NoError) {

		BMKillHandle((GSHandle *)&memo.parcs);
		memo.parcs = (API_PolyArc**)BMhAllClear(2 * sizeof(API_PolyArc));

		if (memo.parcs != NULL){

			(*memo.parcs)[0].begIndex = 1;
			(*memo.parcs)[0].endIndex = 2;
			(*memo.parcs)[0].arcAngle = 90 * DEGRAD;

			(*memo.parcs)[1].begIndex = 2;
			(*memo.parcs)[1].endIndex = 3;
			(*memo.parcs)[1].arcAngle = 90 * DEGRAD;

			element.curtainWall.polygon.nArcs = 2;
			ACAPI_ELEMENT_MASK_CLEAR(mask);
			ACAPI_ELEMENT_MASK_SET(mask, API_CurtainWallType, polygon);

			err = ACAPI_Element_Change(&element, &mask, &memo, APIMemoMask_Polygon, true);

			if (err != NoError){
				ErrorBeep("Cannot add arcs", err);
				sprintf(buffer, "Error: %d", err);
				ACAPI_WriteReport(buffer, true);
			}

		}
		else {
			ErrorBeep("Cannot add arcs", APIERR_MEMFULL);
		}

	}

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	NOTBOOSTintersections(API_Coord* polygon, int nPoints, API_Coord beg, API_Coord end, std::list<API_Coord>* intersections){
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

void	intersectWallV01(){
	API_Element		element, mask;
	API_Element		elementToIntersect;
	API_ElementMemo memo;
	API_ElementMemo memoToIntersect;
	GSErrCode		err;
	intersectmsg	intersectMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &intersectMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&elementToIntersect, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&memoToIntersect, sizeof(API_ElementMemo));

	element.header.guid = APIGuidFromString(intersectMsg.guid1().c_str());
	elementToIntersect.header.guid = APIGuidFromString(intersectMsg.guid2().c_str());

	if (ACAPI_Element_Get(&element) == NoError
		&& ACAPI_Element_Get(&elementToIntersect) == NoError
		&& ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_Polygon) == NoError
		&& ACAPI_Element_GetMemo(elementToIntersect.header.guid, &memoToIntersect, APIMemoMask_Polygon) == NoError){
		//TODO
		//1' Check if point inside elementToIntersect
		//2a' if inside, the intersection point is the point itself
		//2b1' if outside, get the two nearest points to the point
		//2b2' form a line with the two points
		//2b3' intersect wall line with the line of 2b2

		int numPoints = BMhGetSize(reinterpret_cast<GSHandle> (memoToIntersect.coords)) / Sizeof32(API_Coord) - 1;
		int pIndex = 1;

		bool bRight, bLeft, bTop, bBottom;
		bool eRight, eLeft, eTop, eBottom;
		int aux;
		double bMaxX = -100000;
		double bMinX = 100000;
		double bMaxY = -100000;
		double bMinY = 100000;

		for (int i = 0; i < numPoints; i++){
			if (bMaxX < (*memoToIntersect.coords)[pIndex].x){
				bMaxX = (*memoToIntersect.coords)[pIndex].x;
			}
			if (bMinX >(*memoToIntersect.coords)[pIndex].x){
				bMinX = (*memoToIntersect.coords)[pIndex].x;
			}
			if (bMaxY < (*memoToIntersect.coords)[pIndex].y){
				bMaxY = (*memoToIntersect.coords)[pIndex].y;
			}
			if (bMinY >(*memoToIntersect.coords)[pIndex].y){
				bMinY = (*memoToIntersect.coords)[pIndex].y;
			}
			pIndex++;
		}
		double xx = element.wall.begC.x;
		double yy = element.wall.begC.y;
		double dist = 100000;
		double distAux;
		API_Coord coord;
		pIndex = 1;
		if (!(xx > bMinX && xx < bMaxX && yy > bMinY && yy < bMaxY)){
			sprintf(buffer, "NumPoints %d", numPoints);
			ACAPI_WriteReport(buffer, true);
			for (int i = 0; i < numPoints; i++){
				distAux = pow(pow((*memoToIntersect.coords)[pIndex].x - xx, 2) + pow((*memoToIntersect.coords)[pIndex].y - yy, 2), 0.5);
				if (distAux < dist){
					dist = distAux;
					coord = (*memoToIntersect.coords)[pIndex];
				}
				pIndex++;
			}
			double theta = atan2(coord.y - yy, coord.x - xx);
			double sumValue = cos(theta) * dist;
			element.wall.begC.x = xx + (sumValue * cos(theta));
			element.wall.begC.y = yy + (sumValue * sin(theta));
			sprintf(buffer, "begC.x: %f coord.x: %f coord.y: %f dist: %f sumValue: %f theta: %f cosTheta: %f", element.wall.begC.x, coord.x, coord.y, dist, sumValue, theta, cos(theta));
			ACAPI_WriteReport(buffer, true);

		}

		xx = element.wall.endC.x;
		yy = element.wall.endC.y;
		dist = 100000;
		pIndex = 1;
		if (!(xx > bMinX && xx < bMaxX && yy > bMinY && yy < bMaxY)){
			for (int i = 0; i < numPoints; i++){
				distAux = pow(pow((*memoToIntersect.coords)[pIndex].x - xx, 2) + pow((*memoToIntersect.coords)[pIndex].y - yy, 2), 0.5);
				if (distAux < dist){
					dist = distAux;
					coord = (*memoToIntersect.coords)[pIndex];
				}
				pIndex++;
			}
			double theta = atan2(coord.y - yy, coord.x - xx);
			double sumValue = cos(theta) * dist;
			element.wall.endC.x = xx + (sumValue * cos(theta));
			element.wall.endC.y = yy + (sumValue * sin(theta));

		}
		ACAPI_ELEMENT_MASK_CLEAR(mask);
		ACAPI_ELEMENT_MASK_SET(mask, API_WallType, begC);
		//ACAPI_ELEMENT_MASK_SET(mask, API_WallType, endC);

		//err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
		err = ACAPI_Element_Create(&element, &memo);
		if (err != NoError){
			ErrorBeep("ACAPI_Element_Create (slab)", err);
			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}

		sendElementID(element);
	}
}

void	intersectWallWithBB(){
	API_Element		element, mask;
	API_Element		elementToIntersect;
	API_ElementMemo memo;
	API_ElementMemo memoToIntersect;
	GSErrCode		err;
	intersectmsg	intersectMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &intersectMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&elementToIntersect, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&memoToIntersect, sizeof(API_ElementMemo));

	element.header.guid = APIGuidFromString(intersectMsg.guid1().c_str());
	elementToIntersect.header.guid = APIGuidFromString(intersectMsg.guid2().c_str());

	if (ACAPI_Element_Get(&element) == NoError
		&& ACAPI_Element_Get(&elementToIntersect) == NoError
		&& ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_Polygon) == NoError
		&& ACAPI_Element_GetMemo(elementToIntersect.header.guid, &memoToIntersect, APIMemoMask_Polygon) == NoError){
		//TODO
		//1' Check if point inside elementToIntersect
		//2a' if inside, the intersection point is the point itself
		//2b1' if outside, get the two nearest points to the point
		//2b2' form a line with the two points
		//2b3' intersect wall line with the line of 2b2

		int numPoints = BMhGetSize(reinterpret_cast<GSHandle> (memoToIntersect.coords)) / Sizeof32(API_Coord) - 1;
		int pIndex = 1;

		bool bRight, bLeft, bTop, bBottom;
		bool eRight, eLeft, eTop, eBottom;
		int aux;
		double bMaxX = -100000;
		double bMinX = 100000;
		double bMaxY = -100000;
		double bMinY = 100000;

		for (int i = 0; i < numPoints; i++){
			if (bMaxX < (*memoToIntersect.coords)[pIndex].x){
				bMaxX = (*memoToIntersect.coords)[pIndex].x;
			}
			if (bMinX >(*memoToIntersect.coords)[pIndex].x){
				bMinX = (*memoToIntersect.coords)[pIndex].x;
			}
			if (bMaxY < (*memoToIntersect.coords)[pIndex].y){
				bMaxY = (*memoToIntersect.coords)[pIndex].y;
			}
			if (bMinY >(*memoToIntersect.coords)[pIndex].y){
				bMinY = (*memoToIntersect.coords)[pIndex].y;
			}
			pIndex++;
		}
		double xx = element.wall.begC.x;
		double yy = element.wall.begC.y;
		double upDist = 1000000;
		double downDist = 1000000;
		double distAux;
		double wallDist = pow(pow(element.wall.endC.x - element.wall.begC.x, 2) + pow(element.wall.endC.y - element.wall.begC.y, 2), 0.5);
		double theta;
		API_Coord upCoord = element.wall.begC;
		API_Coord downCoord = element.wall.begC;
		double determinant;
		double x1, y1, x2, y2, x3, y3, x4, y4;
		double lowValue;
		pIndex = 1;
		//begC
		if (!(xx > bMinX && xx < bMaxX && yy > bMinY && yy < bMaxY)){
			for (int i = 0; i < numPoints; i++){

				distAux = pow(pow((*memoToIntersect.coords)[pIndex].x - xx, 2) + pow((*memoToIntersect.coords)[pIndex].y - yy, 2), 0.5);

				//theta = acos( ((*memoToIntersect.coords)[pIndex].x * xx + (*memoToIntersect.coords)[pIndex].y * yy) / (wallDist * distAux)  );

				determinant = (element.wall.endC.x - element.wall.begC.x) * ((*memoToIntersect.coords)[pIndex].y - element.wall.begC.y) - (element.wall.endC.y - element.wall.begC.y)*((*memoToIntersect.coords)[pIndex].x - element.wall.begC.x);

				//Consider the point to be up
				if (determinant > 0){
					if (distAux < upDist){
						upDist = distAux;
						upCoord = (*memoToIntersect.coords)[pIndex];
					}
				}
				else{
					if (distAux < downDist){
						downDist = distAux;
						downCoord = (*memoToIntersect.coords)[pIndex];
					}
				}
				pIndex++;
			}

			x1 = element.wall.begC.x;	y1 = element.wall.begC.y;
			x2 = element.wall.endC.x;	y2 = element.wall.endC.y;
			x3 = upCoord.x;				y3 = upCoord.y;
			x4 = downCoord.x;			y4 = downCoord.y;

			sprintf(buffer, "upCoord.x: %f upCoord.y: %f downCoord.x: %f downCoord.y: %f", upCoord.x, upCoord.y, downCoord.x, downCoord.y);
			ACAPI_WriteReport(buffer, true);

			lowValue = (x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4);
			element.wall.begC.x = ((x1*y2 - y1*x2) * (x3 - x4) - (x1 - x2) * (x3*y4 - y3*x4)) / lowValue;
			element.wall.begC.y = ((x1*y2 - y1*x2) * (y3 - y4) - (y1 - y2) * (x3*y4 - y3*x4)) / lowValue;
		}

		//endC
		xx = element.wall.endC.x;
		yy = element.wall.endC.y;
		upDist = 1000000;
		downDist = 1000000;
		pIndex = 1;
		if (!(xx > bMinX && xx < bMaxX && yy > bMinY && yy < bMaxY)){
			for (int i = 0; i < numPoints; i++){

				distAux = pow(pow((*memoToIntersect.coords)[pIndex].x - xx, 2) + pow((*memoToIntersect.coords)[pIndex].y - yy, 2), 0.5);

				//theta = acos( ((*memoToIntersect.coords)[pIndex].x * xx + (*memoToIntersect.coords)[pIndex].y * yy) / (wallDist * distAux)  );

				determinant = (element.wall.endC.x - element.wall.begC.x) * ((*memoToIntersect.coords)[pIndex].y - element.wall.begC.y) - (element.wall.endC.y - element.wall.begC.y)*((*memoToIntersect.coords)[pIndex].x - element.wall.begC.x);

				//Consider the point to be up
				if (determinant > 0){
					if (distAux < upDist){
						upDist = distAux;
						upCoord = (*memoToIntersect.coords)[pIndex];
					}
				}
				else{
					if (distAux < downDist){
						downDist = distAux;
						downCoord = (*memoToIntersect.coords)[pIndex];
					}
				}
				pIndex++;
			}
			x1 = element.wall.begC.x;	y1 = element.wall.begC.y;
			x2 = element.wall.endC.x;	y2 = element.wall.endC.y;
			x3 = upCoord.x;				y3 = upCoord.y;
			x4 = downCoord.x;			y4 = downCoord.y;

			sprintf(buffer, "upCoord.x: %f upCoord.y: %f downCoord.x: %f downCoord.y: %f", upCoord.x, upCoord.y, downCoord.x, downCoord.y);
			ACAPI_WriteReport(buffer, true);
			lowValue = (x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4);

			element.wall.endC.x = ((x1*y2 - y1*x2) * (x3 - x4) - (x1 - x2) * (x3*y4 - y3*x4)) / lowValue;
			element.wall.endC.y = ((x1*y2 - y1*x2) * (y3 - y4) - (y1 - y2) * (x3*y4 - y3*x4)) / lowValue;
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

		sendElementID(element);
	}
}

void	intersectWallV02(){
	API_Element		element, mask;
	API_Element		elementToIntersect;
	//API_ElementMemo memo;
	API_ElementMemo memoToIntersect;
	GSErrCode		err;
	intersectmsg	intersectMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &intersectMsg);

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
		for (std::list<API_Coord>::iterator point = iPoints.begin(); point != iPoints.end(); point++){
			//sprintf(buffer, "ix %f iy %f", point->x, point->y);
			//ACAPI_WriteReport(buffer, true);
		}
		iPoints.clear();

		intersections((*memoToIntersect.coords), numPoints, element.wall.begC, element.wall.endC, &iPoints);

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

		sendElementID(element);
	}
}

void	intersectWallV03(){
	API_Element		element, mask;
	API_Element		elementToIntersect;
	//API_ElementMemo memo;
	API_ElementMemo memoToIntersect;
	GSErrCode		err;
	intersectmsg	intersectMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &intersectMsg);

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
		for (std::list<API_Coord>::iterator point = iPoints.begin(); point != iPoints.end(); point++){
			//sprintf(buffer, "ix %f iy %f", point->x, point->y);
			//ACAPI_WriteReport(buffer, true);
		}
		iPoints.clear();

		intersections((*memoToIntersect.coords), numPoints, element.wall.begC, element.wall.endC, &iPoints);

		double minDist = 1000000;
		double currentDist = 1000000;
		double newX = element.wall.begC.x;
		double newY = element.wall.begC.y;

		if (!begInside){
			for (std::list<API_Coord>::iterator point = iPoints.begin(); point != iPoints.end(); point++){
				currentDist = pow(pow(point->x - element.wall.begC.x, 2) + pow(point->y - element.wall.begC.y, 2), 0.5);
				if (currentDist < minDist){
					minDist = currentDist;
					newX = point->x;
					newY = point->y;
				}
			}
			element.wall.begC.x = newX;
			element.wall.begC.y = newY;
		}

		minDist = 1000000;
		currentDist = 1000000;
		newX = element.wall.endC.x;
		newY = element.wall.endC.y;

		if (!begInside){
			for (std::list<API_Coord>::iterator point = iPoints.begin(); point != iPoints.end(); point++){
				currentDist = pow(pow(point->x - element.wall.endC.x, 2) + pow(point->y - element.wall.endC.y, 2), 0.5);
				if (currentDist < minDist){
					minDist = currentDist;
					newX = point->x;
					newY = point->y;
				}
			}
			element.wall.endC.x = newX;
			element.wall.endC.y = newY;
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

		sendElementID(element);
	}
}

void	oldCreateStoryAbove(){
	API_StoryInfo		storyInfo;
	API_StoryCmdType	storyCmd;
	GSErrCode			err;
	storymsg			storyMsg;
	bool				newHeight = true;
	char buffer[256];

	readDelimitedFrom(raw_in, &storyMsg);

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		return;
	}
	int numberOfStories = BMhGetSize(reinterpret_cast<GSHandle> (storyInfo.data)) / Sizeof32(API_StoryType) - 1;

	for (int i = 0; i < numberOfStories; i++){
		//sprintf(buffer, "level: %f height: %f", (*storyInfo.data)[i].level, storyMsg.height());
		//ACAPI_WriteReport(buffer, true);
		if ((*storyInfo.data)[i].level == (storyMsg.height() + (*storyInfo.data)[currentLevel].level)){
			newHeight = false;
			currentLevel = (*storyInfo.data)[i].index;
			break;
		}
	}

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));

	if (newHeight){
		BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
		storyCmd.action = APIStory_InsAbove;
		//storyCmd.index = (short)(storyInfo.lastStory - storyInfo.firstStory);
		storyCmd.index = currentLevel;
		storyCmd.height = storyMsg.height();

		err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
		if (err != NoError){
			ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
		}
		currentLevel++;
	}

	storyCmd.action = APIStory_GoTo;
	storyCmd.index = currentLevel;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}



}

void	oldCreateStoryBelow(){
	API_StoryInfo		storyInfo;
	API_StoryCmdType	storyCmd;
	GSErrCode			err;
	storymsg			storyMsg;
	bool				newHeight = true;
	char buffer[256];

	readDelimitedFrom(raw_in, &storyMsg);

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, NULL);
	if (err != NoError) {
		ErrorBeep("APIEnv_GetStorySettingsID", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		return;
	}

	int numberOfStories = BMhGetSize(reinterpret_cast<GSHandle> (storyInfo.data)) / Sizeof32(API_StoryType) - 1;

	for (int i = 0; i < numberOfStories; i++){
		//sprintf(buffer, "i: %d level: %f actStory: %d", i, (*storyInfo.data)[i].level, currentLevel - storyInfo.firstStory);
		//ACAPI_WriteReport(buffer, true);

		if ((*storyInfo.data)[i].level == ((*storyInfo.data)[currentLevel - storyInfo.firstStory].level - storyMsg.height())){
			newHeight = false;
			currentLevel = (*storyInfo.data)[i].index;
			break;
		}
	}

	BMKillHandle(reinterpret_cast<GSHandle *> (&storyInfo.data));
	if (newHeight){
		BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
		storyCmd.action = APIStory_InsBelow;
		//storyCmd.index = (short)(storyInfo.firstStory - storyInfo.lastStory);
		storyCmd.index = currentLevel;
		storyCmd.height = storyMsg.height();

		err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
		if (err != NoError){
			ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);

			sprintf(buffer, ErrID_To_Name(err));
			ACAPI_WriteReport(buffer, true);
		}
		currentLevel--;
	}

	storyCmd.action = APIStory_GoTo;
	storyCmd.index = currentLevel;
	err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd, NULL);
	if (err != NoError){
		ErrorBeep("APIEnv_ChangeStorySettingsID (new story)", err);
	}
}

void	oldcreateSphere(){
	API_Element		shellElement;
	API_ElementMemo memo;
	GSErrCode		err;
	spheremessage	sphereMsg;

	readDelimitedFrom(raw_in, &sphereMsg);


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

	shellElement.header.floorInd = currentLevel;

	double* tmx = shellElement.shell.basePlane.tmx;

	tmx[0] = 1.0;				tmx[4] = 0.0;				tmx[8] = 0.0;
	tmx[1] = 0.0;				tmx[5] = 1.0;				tmx[9] = 0.0;
	tmx[2] = 0.0;				tmx[6] = 0.0;				tmx[10] = 1.0;
	//center Coords (x, y, z)
	tmx[3] = sphereMsg.c0x();		tmx[7] = sphereMsg.c0y();		tmx[11] = 0.0;


	shellElement.shell.isFlipped = true;

	shellElement.shell.u.revolvedShell.slantAngle = 0;
	shellElement.shell.u.revolvedShell.revolutionAngle = 360 * DEGRAD;
	shellElement.shell.u.revolvedShell.distortionAngle = 90 * DEGRAD;
	shellElement.shell.u.revolvedShell.segmentedSurfaces = false;
	shellElement.shell.u.revolvedShell.segmentType = APIShellBase_SegmentsByCircle;
	shellElement.shell.u.revolvedShell.segmentsByCircle = 36;
	BNZeroMemory(&shellElement.shell.u.revolvedShell.axisBase, sizeof(API_Tranmat));

	shellElement.shell.u.revolvedShell.axisBase.tmx[0] = 0.0;
	//reflect y-axis, if 1.0 up, -1.0 down, 0.0 no y move
	//shellElement.shell.u.revolvedShell.axisBase.tmx[6] = sphereMsg.reflecty();
	shellElement.shell.u.revolvedShell.axisBase.tmx[9] = 0.0;

	//reflect x-axis, if 1.0 right, -1.0 left
	//shellElement.shell.u.revolvedShell.axisBase.tmx[3] = sphereMsg.reflectx();


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

	(*memo.shellShapes[0].coords)[1].x = 0.0;
	(*memo.shellShapes[0].coords)[1].y = 0.0;
	(*memo.shellShapes[0].coords)[2].x = 0.0;
	//radius, values are strange
	(*memo.shellShapes[0].coords)[2].y = sphereMsg.radius();
	(*memo.shellShapes[0].coords)[3] = (*memo.shellShapes[0].coords)[1];

	(*memo.shellShapes[0].pends)[1] = shellElement.shell.u.revolvedShell.shellShape.nCoords;

	(*memo.shellShapes[0].parcs)[0].begIndex = 1;
	(*memo.shellShapes[0].parcs)[0].endIndex = 2;
	//this will make a sphere
	(*memo.shellShapes[0].parcs)[0].arcAngle = 4 * 1.570785;

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

	sendElementID(shellElement);

	ACAPI_DisposeElemMemoHdls(&memo);
}

// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------


// ---------------------------------- Prototypes -------------------------------

void createDefaultObject(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	objectmsg		objectMsg;
	char buffer[256];

	readDelimitedFrom(raw_in, &objectMsg);

	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));

	element.header.typeID = API_ObjectID;

	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		//sprintf(buffer, "Default Problem");
		//ACAPI_WriteReport(buffer, true);
	}

	element.header.floorInd = currentLevel;

	//sprintf(buffer, "libInd: %d", element.object.libInd);
	//ACAPI_WriteReport(buffer, true);

	element.object.pos.x = objectMsg.posx();
	element.object.pos.y = objectMsg.posy();
	element.object.useXYFixSize = false;
	element.object.useObjSectAttrs = false;
	element.object.xRatio = 10;
	element.object.yRatio = 10;

	err = ACAPI_Element_Create(&element, &memo);
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create (slab)", err);
		//sprintf(buffer, ErrID_To_Name(err));
		//ACAPI_WriteReport(buffer, true);
	}

	sprintf(buffer, "Index: %d", element.object.libInd);
	ACAPI_WriteReport(buffer, true);

	sendElementID(element);

	ACAPI_DisposeElemMemoHdls(&memo);
}

// =============================================================================
//
// Main functions
//
// =============================================================================

int	userQuit = 0;
void quit(){
	userQuit = 1;
}


void writeMaterialsWall(){
	GSErrCode err;
	ofstream file;
	API_Attribute  attrib;
	API_LibPart  libPart;
	Int32        count;
	short n;

	BNZeroMemory(&attrib, sizeof(API_Attribute));

	attrib.header.typeID = API_BuildingMaterialID;
	err = ACAPI_Attribute_GetNum(API_BuildingMaterialID, &n);
	file.open("ListOfMaterials.txt");
	for (int i = 1; i <= n && err == NoError; i++) {
		attrib.header.index = i;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			file << attrib.buildingMaterial.head.name << "\n";
		}
		if (err == APIERR_DELETED){
			err = NoError;
		}
	}
	file.close();

	attrib.header.typeID = API_CompWallID;
	err = ACAPI_Attribute_GetNum(API_CompWallID, &n);
	file.open("ListOfCompositeMaterials.txt");
	for (int i = 1; i <= n && err == NoError; i++) {
		attrib.header.index = i;
		err = ACAPI_Attribute_Get(&attrib);
		if (err == NoError) {
			file << attrib.compWall.head.name << "\n";
		}
		if (err == APIERR_DELETED){
			err = NoError;
		}
	}
	file.close();

	
	err = ACAPI_LibPart_GetNum(&count);
	if (!err) {
		file.open("ListOfLibParts.txt");
		for (int i = 1; i <= count; i++) {
			BNZeroMemory(&libPart, sizeof(API_LibPart));
			libPart.index = i;
			err = ACAPI_LibPart_Get(&libPart);
			if (!err) {
				if (libPart.typeID == APILib_ObjectID){
					file << libPart.index << " = " << (const char *)GS::UniString(libPart.docu_UName).ToCStr() << "\n";
				}
			}
			if (libPart.location != NULL)
				delete libPart.location;
		}
		file.close();
	}
	
}

scriptMap m;

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



static void		Do_Test (void)
{
	GSErrCode		err;
	namemessage		namemsg;
	API_LibPart  libPart;
	Int32        count;
	//For receiving messages
	//google::protobuf::io::ZeroCopyInputStream *raw_in;

	API_Attribute  attrib;
	short          n;

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

	err = ACAPI_LibPart_GetNum(&count);
	if (!err) {
		for (int i = 1; i <= count; i++) {
			BNZeroMemory(&libPart, sizeof(API_LibPart));
			libPart.index = i;
			err = ACAPI_LibPart_Get(&libPart);
			if (!err) {
				if (libPart.typeID == APILib_ObjectID){
					objectsIds.insert(std::make_pair((const char *)GS::UniString(libPart.docu_UName).ToCStr(), libPart.index));
				}
			}
			if (libPart.location != NULL)
				delete libPart.location;
		}
	}

	wallDefaultMaterial = searchMaterials("GENERIC - STRUCTURAL", buildingMaterials);
	slabDefaultMaterial = searchMaterials("GENERIC - INTERNAL CLADDING", buildingMaterials);

	wallDefaultComposite = searchMaterials("Generic Wall/Shell", compositeMaterials);
	slabDefaultComposite = searchMaterials("Generic Slab/Roof", compositeMaterials);

	m.insert(std::make_pair("Wall",						&createWall));
	m.insert(std::make_pair("NewWall",					&createNewWall));
	m.insert(std::make_pair("MultiWall",				&createMultiWall));
	m.insert(std::make_pair("Door",						&createDoor));
	m.insert(std::make_pair("Window",					&createWindow));
	m.insert(std::make_pair("Circle",					&createCircle));
	m.insert(std::make_pair("Arc",						&createArc));
	m.insert(std::make_pair("Sphere",					&createSphere));
	m.insert(std::make_pair("Cylinder",					&createCylinder));
	m.insert(std::make_pair("ChapelNoCom",				&createChapelNoCom));
	m.insert(std::make_pair("ComplexShell",				&createComplexShell));
	m.insert(std::make_pair("SimpleShell",				&createSimpleShell));
	m.insert(std::make_pair("Shell",					&createShell));
	m.insert(std::make_pair("Lemon",					&createLemonNoCom));
	m.insert(std::make_pair("RotateShell",				&rotateShell));
	m.insert(std::make_pair("TranslateShell",			&translateShell));
	m.insert(std::make_pair("Hole",						&createHoleInShell));
	m.insert(std::make_pair("CurtainWall",				&createNewCurtainWall));
	m.insert(std::make_pair("Slab",						&createSlab));
	m.insert(std::make_pair("NewSlab",					&createNewSlab));
	m.insert(std::make_pair("HoleSlab",					&createHole));
	m.insert(std::make_pair("WallsSlab",				&createWallsFromSlab));
	m.insert(std::make_pair("CWallsSlab",				&createCWallsFromSlab));
	m.insert(std::make_pair("AddArcs",					&addArcsToElement));
	m.insert(std::make_pair("Translate",				&translateElement));
	m.insert(std::make_pair("RotateZ",					&rotateElementZ));
	m.insert(std::make_pair("Mirror",					&mirrorElement));
	m.insert(std::make_pair("Trim",						&trimElement));
	m.insert(std::make_pair("IntersectWall",			&intersectWall));
	m.insert(std::make_pair("DestructiveIntersectWall", &destructiveIntersectWall));
	m.insert(std::make_pair("Column",					&createColumn));
	m.insert(std::make_pair("NewColumn",				&createNewColumn));
	m.insert(std::make_pair("Object",					&createObject));
	m.insert(std::make_pair("Stairs",					&createStairs));
	m.insert(std::make_pair("Roof",						&createRoof));
	m.insert(std::make_pair("NewRoof",					&createNewRoof));
	m.insert(std::make_pair("PolyRoof",					&createPolyRoof));
	m.insert(std::make_pair("Mesh",						&createMesh));
	m.insert(std::make_pair("Morph", &createMorph));
	m.insert(std::make_pair("Box",						&createBox));
	m.insert(std::make_pair("Story",					&createStory));
	m.insert(std::make_pair("StoryAbove",				&createStoryAbove));
	m.insert(std::make_pair("StoryBelow",				&createStoryBelow));
	m.insert(std::make_pair("CheckStory",				&checkStory));
	m.insert(std::make_pair("CheckStoryAbove",			&checkStoryAbove));
	m.insert(std::make_pair("CheckStoryBelow",			&checkStoryBelow));
	m.insert(std::make_pair("ChooseStory",				&chooseStory));
	m.insert(std::make_pair("UpperLevel",				&upperLevel));
	m.insert(std::make_pair("DeleteStories",			&deleteStories));
	m.insert(std::make_pair("Delete",					&deleteElements));
	m.insert(std::make_pair("Test",						&testFunction));
	m.insert(std::make_pair("WriteMaterialsFile",		&writeMaterialsWall));
	m.insert(std::make_pair("quit",						&quit));
	m.insert(std::make_pair("Quit",						&quit));

	m.insert(std::make_pair("SelectElement", &selectElement));
	m.insert(std::make_pair("Highlight", &highlightElementByID));

	m.insert(std::make_pair("GetLevels", &getLevels));
	m.insert(std::make_pair("GetWalls", &getWalls));
	m.insert(std::make_pair("GetSlabs", &getSlabs));
	m.insert(std::make_pair("GetColumns", &getColumns));
	m.insert(std::make_pair("GetObjects", &getObjects));
	m.insert(std::make_pair("GetRoofs", &getRoofs));

	m.insert(std::make_pair("HoleTest", &createHoleTest));

	m.insert(std::make_pair("DefaultObject", &createDefaultObject));

	boost::asio::io_service io_service;
	tcp::endpoint endpoint(tcp::v4(), 53800);
	tcp::acceptor acceptor(io_service, endpoint);
	tcp::socket socket(io_service);
	boost::system::error_code ec;
	acceptor.accept(socket, ec);
	
	//Check communication error
	if (!ec)
	{
		AsioOutputStream<boost::asio::ip::tcp::socket> ais(socket);
		AsioInputStream<boost::asio::ip::tcp::socket> aisIn(socket);
		raw_in = new google::protobuf::io::CopyingInputStreamAdaptor(&aisIn);
		while (userQuit == 0){
			err = ACAPI_OpenUndoableSession("Geometry Test -- Create elements");
			if (err != NoError) {
				ErrorBeep("ACAPI_OpenUndoableSession", err);
				return;
			}

			//Reading First Message
			readDelimitedFrom(raw_in, &namemsg);

			raw_out = new google::protobuf::io::CopyingOutputStreamAdaptor(&ais);

			callScript(namemsg.name());
			
			ACAPI_CloseUndoableSession();		
		}
	
	}
	else{
		ErrorBeep("Communication Failed", (GS::GSErrCode)10);
	}

	// Optional:  Delete all global objects allocated by libprotobuf.
	google::protobuf::ShutdownProtobufLibrary();

	return;
}		/* Do_Test */

/*
//Cicle to try and capture mouse position so the user could
//continue to work
//ATM it justs captures the position
//the user can do stuff but nothing is updated
DGMousePosData mousePosition;
short mDialogId = 0;

myfile.open("errors.txt");

API_Coord c;
API_GetPointType	pointInfo;

while (socket.available() == 0){
myfile << "Entrou \n";
//ClickAPoint("", &c);
BNZeroMemory(&pointInfo, sizeof(API_GetPointType));
CHTruncate("", pointInfo.prompt, sizeof(pointInfo.prompt));
pointInfo.changeFilter = false;
pointInfo.changePlane = false;
ACAPI_Interface(APIIo_GetPointID, &pointInfo, NULL);
}
myfile.close();
*/


// -----------------------------------------------------------------------------
// Entry points to handle ArchiCAD events
//
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	MenuCommandHandler (const API_MenuParams *params)
{
	switch (params->menuItemRef.itemIndex) {
		case 1:		Do_Test ();				break;
	}

	return NoError;
}		// DoCommand


// =============================================================================
//
// Required functions
//
// =============================================================================


//------------------------------------------------------
// Dependency definitions
//------------------------------------------------------
API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32000, 2);

	return APIAddon_Normal;
}		/* CheckEnvironment */


//------------------------------------------------------
// Interface definitions
//------------------------------------------------------
GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	ACAPI_Register_Menu (32500, 0, MenuCode_UserDef, MenuFlag_Default);

	return NoError;
}		/* RegisterInterface */


//------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
//------------------------------------------------------
GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32500, MenuCommandHandler);
	if (err != NoError)
		DBPrintf ("Geometry_Test:: Initialize() ACAPI_Install_MenuHandler failed\n");

	return err;
}		/* Initialize */


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	return NoError;
}		// FreeData
