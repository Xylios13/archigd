#include "RosettaArchiCAD.hpp"
/*
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
#include "RosettaArchiCAD.hpp"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message_lite.h>
*/

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
	circleElement.header.floorInd = 0;
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
		quadrangle[i].header.floorInd = 0;

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
	arcElement.header.floorInd = 0;

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
	shellElement.header.floorInd = 0;

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
	shellElement.header.floorInd = 0;

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
	shellElement.header.floorInd = 0;

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

