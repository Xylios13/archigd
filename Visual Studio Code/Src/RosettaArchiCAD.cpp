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

#undef UNICODE

#ifndef WIN32_LEAN_AND_MEAN

#define WIN32_LEAN_AND_MEAN

#endif


#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <list>
#include <iostream>
#include <fstream>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message_lite.h>

#include "UniString.hpp"
#include "VectorImage.hpp"
#include "ProfileAdditionalInfo.hpp"
#include "ACAPinc.h"					// also includes APIdefs.h
#include "APICommon.h"
#include "basicgeometry.h"
#include "Messages.pb.h"
#include "DGModule.hpp"
#include "FileSystem.hpp"
#include "Folder.hpp"

#include "RosettaArchiCAD.hpp"

//#include "CommunicationPalette.h"


//using namespace std;
//using namespace google::protobuf::io;

//API_Element elementCreated;
//bool createdElement = false;



// ----------------------------------Winsock----------------------


/*
google::protobuf::uint32 readHdr(char *buf, const int bufSize, bool* success)
{
	google::protobuf::uint32 size;
	google::protobuf::io::ArrayInputStream ais(buf, bufSize);
	google::protobuf::io::CodedInputStream coded_input(&ais);
	(*success) = coded_input.ReadVarint32(&size);
	return size;
}

//Used to write a delimited message
//sends size followed by the message
bool writeDelimitedTo(SOCKET ClientSocket, const google::protobuf::MessageLite& message) {
	char bufferErr[256];
	//int siz = message.ByteSize() + sizeof(message.ByteSize());
	const int size = message.ByteSize();
	const int headerSize = google::protobuf::io::CodedOutputStream::VarintSize32(size);
	const int sizeWithHeader = size + headerSize;
	char *pkt = new char[sizeWithHeader];

	google::protobuf::io::ArrayOutputStream aos(pkt, sizeWithHeader);
	google::protobuf::io::CodedOutputStream output(&aos);

	// Write the size.
	output.WriteVarint32(size);

	message.SerializeWithCachedSizes(&output);

	int iSendResult;

	iSendResult = send(ClientSocket, pkt, sizeWithHeader, 0);
	if (iSendResult == SOCKET_ERROR) {
		sprintf(bufferErr, "send failed with error: %d", WSAGetLastError());
		ACAPI_WriteReport(bufferErr, true);
		closesocket(ClientSocket);
		WSACleanup();
		return false;
	}

	//ACAPI_WriteReport(pkt, true);

	delete(pkt);
	//delete(raw_out);
	return true;
}

//Used to read a delimited message
//reads size followed by the message
bool readDelimitedFrom(SOCKET ClientSocket, google::protobuf::MessageLite* message) {
	// We create a new coded stream for each message.  Don't worry, this is fast,
	// and it makes sure the 64MB total size limit is imposed per-message rather
	// than on the whole stream.  (See the CodedInputStream interface for more
	// info on this limit.)
	int bytecount;
	int actualBytesPeeked = 0;

	char bufferErr[256];
	char buffer[10];

	bool success = false;
	int tryBytes = 1;
	google::protobuf::uint32 siz;
	
	while (!success){
		actualBytesPeeked = recv(ClientSocket, buffer, 10, MSG_PEEK);
		if (actualBytesPeeked == SOCKET_ERROR){
			ACAPI_WriteReport("Error receiving data", true);
			return false;
		}
		siz = readHdr(buffer, actualBytesPeeked, &success);
	}

	int sizeOfsiz = google::protobuf::io::CodedOutputStream::VarintSize32(siz);

	char* bufferBody = new char[siz + sizeOfsiz];

	//Read the entire buffer including the hdr
	bytecount = recv(ClientSocket, bufferBody, siz + sizeOfsiz, MSG_WAITALL);
	if (bytecount == SOCKET_ERROR){
		ACAPI_WriteReport("Error receiving data", true);
		return false;
	}

	google::protobuf::io::ZeroCopyInputStream* raw_in = new google::protobuf::io::ArrayInputStream(bufferBody, siz + sizeOfsiz);
	google::protobuf::io::CodedInputStream input(raw_in);

	// Read the size.
	google::protobuf::uint32 size;
	if (!input.ReadVarint32(&size)) {
		return false;
	}

	// Tell the stream not to read beyond that size.
	google::protobuf::io::CodedInputStream::Limit limit = input.PushLimit(size);

	// Parse the message.
	if (!message->MergeFromCodedStream(&input)) {
		ACAPI_WriteReport("MergeFromCodedStream Error", true);
		return false;
	}

	if (!input.ConsumedEntireMessage()) {
		ACAPI_WriteReport("ConsumedEntireMessage Error", true);
		return false;
	}
	// Release the limit.
	input.PopLimit(limit);
	//delete(buffer);	
	delete(bufferBody);
	delete(raw_in);

	return true;
}

//Send Element Id to Racket
void	sendElementID(SOCKET ClientSocket, API_Element element, bool crash){
	elementid		elementId;
	//Convert Element Id from GuId to char*
	GS::UniString guidString = APIGuidToString(element.header.guid);
	char s[64];
	APIGuid2GSGuid(element.header.guid).ConvertToString(s);
	elementId.set_guid(s);
	elementId.set_crashmaterial(crash);

	//Sending element Id
	writeDelimitedTo(ClientSocket, elementId);
}

void	sendElementID(SOCKET clientSocket, API_Element element){
	sendElementID(ClientSocket, element, false);
}
*/


// ---------------------------------- Test Functions -----------------------------

void	createSimpleWall(){
	API_Element		wallElement;
	API_ElementMemo memo;
	GSErrCode		err;

	err = ACAPI_OpenUndoableSession("Geometry Test -- Create elements");
	if (err != NoError) {
		ErrorBeep("ACAPI_OpenUndoableSession", err);
		return;
	}

	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&wallElement, sizeof(API_Element));

	wallElement.header.typeID = API_WallID;
	wallElement.header.layer = 1;

	err = ACAPI_Element_GetDefaults(&wallElement, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetMemo", err);
		return;
	}
	
	wallElement.header.floorInd = 0;

	wallElement.wall.begC.x = 0.0;
	wallElement.wall.endC.y = 0.0;
	wallElement.wall.endC.x = 5.0;
	wallElement.wall.endC.y = 0.0;
	wallElement.wall.modelElemStructureType = API_BasicStructure;
	wallElement.wall.buildingMaterial = searchBuildingMaterials("Glass");
	wallElement.wall.thickness = 0.3;
	wallElement.wall.height = 3.0;
	wallElement.wall.profileType = APISect_Normal;
	wallElement.wall.referenceLineLocation = APIWallRefLine_Center;

	err = ACAPI_Element_Create(&wallElement, &memo);

	ACAPI_DisposeElemMemoHdls(&memo);

	ACAPI_CloseUndoableSession();
}

void	createSimpleSlab(){
	API_Element		element;
	API_ElementMemo memo;
	GSErrCode		err;
	err = ACAPI_OpenUndoableSession("Geometry Test -- Create elements");
	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	element.header.typeID = API_SlabID;
	err = ACAPI_Element_GetDefaults(&element, &memo);
	element.slab.modelElemStructureType = API_CompositeStructure;
	element.slab.buildingMaterial = 2;
	element.slab.poly.nCoords = 4;
	element.slab.poly.nSubPolys = 1;
	element.slab.poly.nArcs = 0;
	memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((element.slab.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.slab.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((element.slab.poly.nArcs) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	if (memo.coords == NULL || memo.pends == NULL || memo.parcs == NULL) {
		ACAPI_DisposeElemMemoHdls(&memo);
	}
	(*memo.coords)[1].x = 0.0; (*memo.coords)[1].y = 0.0;
	(*memo.coords)[2].x = 5.0; (*memo.coords)[2].y = 0.0;
	(*memo.coords)[3].x = 5.0; (*memo.coords)[3].y = 5.0;
	(*memo.coords)[4].x = 0.0; (*memo.coords)[4].y = 5.0;
	(*memo.coords)[5] = (*memo.coords)[1];
	(*memo.pends)[1] = element.slab.poly.nCoords;
	err = ACAPI_Element_Create(&element, &memo);
	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_CloseUndoableSession();
}

// ---------------------------------- Old Functions ----------------------------

/*
void	oldWorkcreateHoleInShell(){
	API_Element		shellElement, mask;
	API_ElementMemo memo;
	GSErrCode		err;
	oldholemessage		holeMsg;
	pointsmessage	pointsMsg;
	polyarcsmessage polyarcsMsg;
	char buffer[256];

	readDelimitedFrom(ClientSocket, &holeMsg);

	readDelimitedFrom(ClientSocket, &pointsMsg);

	readDelimitedFrom(ClientSocket, &polyarcsMsg);

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

	readDelimitedFrom(ClientSocket, &eleGuid);

	readDelimitedFrom(ClientSocket, &polyarcsMsg);

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

	}

	sendElementID(ClientSocket, element);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	OLDTWOaddArcsToElement(){
	API_Element			element, mask, aux;
	API_ElementMemo		memo, memo2;
	GSErrCode			err;
	elementid			eleGuid;
	polyarcsmessage		polyarcsMsg;
	char buffer[256];

	readDelimitedFrom(ClientSocket, &eleGuid);

	readDelimitedFrom(ClientSocket, &polyarcsMsg);

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

	sendElementID(ClientSocket, element);

	ACAPI_DisposeElemMemoHdls(&memo);
}

void	ToSendaddArcsToElement(){
	API_Element			element, mask, aux;
	API_ElementMemo		memo, memo2;
	GSErrCode			err;
	elementid			eleGuid;
	char buffer[256];

	
	readDelimitedFrom(ClientSocket, &eleGuid);
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

void	intersectWallV01(){
	API_Element		element, mask;
	API_Element		elementToIntersect;
	API_ElementMemo memo;
	API_ElementMemo memoToIntersect;
	GSErrCode		err;
	intersectmsg	intersectMsg;
	char buffer[256];

	readDelimitedFrom(ClientSocket, &intersectMsg);

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

		sendElementID(ClientSocket, element);
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

	readDelimitedFrom(ClientSocket, &intersectMsg);

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

		sendElementID(ClientSocket, element);
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

	readDelimitedFrom(ClientSocket, &intersectMsg);

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

		sendElementID(ClientSocket, element);
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

	readDelimitedFrom(ClientSocket, &intersectMsg);

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

		sendElementID(ClientSocket, element);
	}
}
*/


/*
static GSErrCode __ACENV_CALL PetPaletteCallback(short actValue){
	GSErrCode err = NoError;
	char buffer[254];

	//sprintf(buffer, "PaletteCallback");
	//ACAPI_WriteReport(buffer, true);

	return err;
}
*/


//TODO!!!


static void		Do_Test (void)
{
	GSErrCode		err;
	namemessage		namemsg;
	//For receiving messages
	//google::protobuf::io::ZeroCopyInputStream *raw_in;

	char buffer[256];
	
	/*
	API_PetPaletteType petPaletteInfo;
	short**             petItemIdsHdl;
	short               petItemIds[5] = { 32201, 32202, 32203, 32204, 32205 };

	BNZeroMemory(&petPaletteInfo, sizeof(API_PetPaletteType));

	// Constructing the handle, which contains the pet item icons' resource IDs
	short nIcons = sizeof(petItemIds) / sizeof(short);
	petItemIdsHdl = (short**)BMhAll(nIcons * sizeof(short));
	for (short i = 0; i < nIcons; i++){
		(*petItemIdsHdl)[i] = petItemIds[i];
	}

	petPaletteInfo.petPaletteID = 32200;
	petPaletteInfo.nCols = 5;
	petPaletteInfo.nRows = 1;
	petPaletteInfo.value = 0;
	petPaletteInfo.grayBits = 0;
	petPaletteInfo.petIconIDsHdl = petItemIdsHdl;
	petPaletteInfo.dhlpResourceID = 32200;

	err = ACAPI_Interface(APIIo_PetPaletteID, &petPaletteInfo, PetPaletteCallback);

	BMhKill((GSHandle *)&petItemIdsHdl);
	*/
	
	//if (!CommunicationPaletteManager::IsCommunicationPaletteOpened()){
	//	CommunicationPaletteManager::OpenCommunicationPalette();
	//}
	
	
	setupHashTables();

	bool usingThreads = false;

	if (usingThreads){
		HANDLE threadHandle;
		DWORD   dwThreadId;


		threadHandle = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			MyThreadFunction,       // thread function name
			0,						// argument to thread function 
			0,                      // use default creation flags 
			&dwThreadId);			// returns the thread identifier

		if (threadHandle == NULL)
		{
			ACAPI_WriteReport("Error creating thread", true);
			return;
		}

		CloseHandle(threadHandle);
	}
	else{
		//communicationCycle(false);
		winsockcom(false);
	}

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
	ACAPI_Register_Menu (32502, 0, MenuCode_UserDef, MenuFlag_Default);

	return NoError;
}		/* RegisterInterface */


//------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
//------------------------------------------------------
GSErrCode	__ACENV_CALL Initialize	(void)
{
	GSErrCode err = ACAPI_Install_MenuHandler (32502, MenuCommandHandler);
	if (err != NoError){
		DBPrintf("Geometry_Test:: Initialize() ACAPI_Install_MenuHandler failed\n");
	}
	/*
	err = ACAPI_RegisterModelessWindow(CommunicationPalette::paletteRefId, CommunicationPalette::PaletteAPIControlCallBack,
		API_PalEnabled_FloorPlan + API_PalEnabled_3D + API_PalEnabled_Layout,
		GSGuid2APIGuid(CommunicationPalette::paletteGuid));
	if (err != NoError){
		DBPrintf("DG_Test:: Initialize() ACAPI_RegisterModelessWindow failed\n");
	}
	*/
	return err;
}		/* Initialize */


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData (void)
{
	//ACAPI_UnregisterModelessWindow(CommunicationPalette::paletteRefId);
	return NoError;
}		// FreeData
