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
#include "RosettaArchiCAD.hpp"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message_lite.h>
*/

void msgArchiCAD(int msg){
	char buffer[256];
	sprintf(buffer, "Int: %d", msg);
	ACAPI_WriteReport(buffer, true);
}

void msgArchiCAD(double msg){
	char buffer[256];
	sprintf(buffer, "Double: %f", msg);
	ACAPI_WriteReport(buffer, true);
}

void msgArchiCAD(std::string msg){
	ACAPI_WriteReport(msg.c_str(), true);
}

bool visualFeedbackOn = false;

/*
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
*/
int	pointRelation(float x1, float x2){
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

void multTMX(double *resMat, double *aMatrix)
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

void multMatrix(double* a, double* b, double* r){
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			r[j * 4 + i] = 0.0f;
			for (int k = 0; k < 3; k++){
				r[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
			}
		}
	}
	r[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3];
	r[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7];
	r[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11];

}
/*
//Aux function for Create LibPart
static GSErrCode GetLocation(IO::Location*& loc, bool useEmbeddedLibrary)
{
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
*/
void handleParams(additionalparams msg, API_ParamOwnerType* paramOwner, API_GetParamsType* getParams){
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
		int numParams = BMhGetSize(reinterpret_cast<GSHandle> (getParams->params)) / Sizeof32(API_AddParType) - 1;
		bool isArray = msg.isarray(i);
		for (int j = 0; j < numParams; j++){
			if ((std::string)(*getParams->params)[j].name == parameterName){
				if (isArray){
					if (msg.paramtype(i) == "s"){
						//(*getParams.params)[j].typeMod = API_ParArray;
						//(*getParams.params)[j].typeID = APIParT_CString;

						int numberOfStrings = msg.stringarrays(stringArrayCounter).lst_size();
						int actualSize = numberOfStrings;
						for (int k = 0; k < numberOfStrings; k++){
							actualSize += msg.stringarrays(stringArrayCounter).lst(k).size();
						}

						(*getParams->params)[j].dim1 = numberOfStrings;
						(*getParams->params)[j].dim2 = 1;

						BMKillHandle(reinterpret_cast<GSHandle*>(&(*getParams->params)[j].value.array));
						(*getParams->params)[j].value.array = BMAllocateHandle(actualSize * (*getParams->params)[j].dim2 * sizeof(GS::uchar_t), ALLOCATE_CLEAR, 0);
						GS::uchar_t** arrHdl = reinterpret_cast<GS::uchar_t**>((*getParams->params)[j].value.array);

						int previousStringSize = 0;
						for (Int32 r = 0; r < numberOfStrings; r++){
							for (Int32 k = 0; k < msg.stringarrays(stringArrayCounter).lst(r).size(); k++){
								(*arrHdl)[previousStringSize + r + k] = msg.stringarrays(stringArrayCounter).lst(r).at(k);
							}
							(*arrHdl)[previousStringSize + r + msg.stringarrays(stringArrayCounter).lst(r).size()] = '\0';
							previousStringSize += msg.stringarrays(stringArrayCounter).lst(r).size();
						}
						stringArrayCounter++;
					}
					else if (msg.paramtype(i) == "d"){
						//(*getParams.params)[j].typeMod = API_ParArray;
						//(*getParams.params)[j].typeID = APIParT_RealNum;
						(*getParams->params)[j].dim1 = msg.doublearrays(doubleArrayCounter).lst_size();
						(*getParams->params)[j].dim2 = 1;

						BMKillHandle(reinterpret_cast<GSHandle*>(&(*getParams->params)[j].value.array));
						(*getParams->params)[j].value.array = BMAllocateHandle((*getParams->params)[j].dim1 * (*getParams->params)[j].dim2 * sizeof(double), ALLOCATE_CLEAR, 0);
						double** arrHdl = reinterpret_cast<double**>((*getParams->params)[j].value.array);

						for (Int32 k = 0; k < (*getParams->params)[j].dim1; k++){
							(*arrHdl)[k] = msg.doublearrays(doubleArrayCounter).lst(k);
						}
						doubleArrayCounter++;
					}
					else if (msg.paramtype(i) == "i"){
						//(*getParams.params)[j].typeMod = API_ParArray;
						//(*getParams.params)[j].typeID = APIParT_Integer;
						(*getParams->params)[j].dim1 = msg.intarrays(intArrayCounter).lst_size();
						(*getParams->params)[j].dim2 = 1;

						BMKillHandle(reinterpret_cast<GSHandle*>(&(*getParams->params)[j].value.array));
						(*getParams->params)[j].value.array = BMAllocateHandle((*getParams->params)[j].dim1 * (*getParams->params)[j].dim2 * sizeof(double), ALLOCATE_CLEAR, 0);
						double** arrHdl = reinterpret_cast<double**>((*getParams->params)[j].value.array);

						for (Int32 k = 0; k < (*getParams->params)[j].dim1; k++){
							(*arrHdl)[k] = msg.intarrays(intArrayCounter).lst(k);
						}
						intArrayCounter++;
					}
					else if (msg.paramtype(i) == "b"){
						//(*getParams.params)[j].typeMod = API_ParArray;
						//(*getParams.params)[j].typeID = APIParT_Boolean;
						(*getParams->params)[j].dim1 = msg.boolarrays(boolArrayCounter).lst_size();
						(*getParams->params)[j].dim2 = 1;

						BMKillHandle(reinterpret_cast<GSHandle*>(&(*getParams->params)[j].value.array));
						(*getParams->params)[j].value.array = BMAllocateHandle((*getParams->params)[j].dim1 * (*getParams->params)[j].dim2 * sizeof(double), ALLOCATE_CLEAR, 0);
						double** arrHdl = reinterpret_cast<double**>((*getParams->params)[j].value.array);

						for (Int32 k = 0; k < (*getParams->params)[j].dim1; k++){
							(*arrHdl)[k] = msg.boolarrays(boolArrayCounter).lst(k);
						}

						boolArrayCounter++;
					}

				}
				else{
					if (msg.paramtype(i) == "s"){
						API_AddParID attrType = (*getParams->params)[j].typeID;
						std::string stringValue = msg.strings(stringCounter);
						
						if (attrType == APIParT_LineTyp){
							(*getParams->params)[j].value.real = searchLineTypes(stringValue);
						}
						else if (attrType == APIParT_Mater){
							(*getParams->params)[j].value.real = searchOverrideMaterials(stringValue);
						}
						else{
							GS::UniString((*getParams->params)[j].value.uStr) = stringValue.c_str();
						}
						
						//GS::UniString((*getParams->params)[j].value.uStr) = stringValue.c_str();

						stringCounter++;
					}
					else if (msg.paramtype(i) == "d"){
						(*getParams->params)[j].value.real = msg.doubles(doubleCounter);
						doubleCounter++;
					}
					else if (msg.paramtype(i) == "i"){
						(*getParams->params)[j].value.real = msg.integers(intCounter);
						intCounter++;
					}
					else if (msg.paramtype(i) == "b"){
						if (msg.booleans(boolCounter)){
							(*getParams->params)[j].value.real = 1;
						}
						else{
							(*getParams->params)[j].value.real = 0;
						}
						boolCounter++;
					}
				}
			}
		}
	}
}

void prepareParams(additionalparams* msg, API_ParamOwnerType* paramOwner, API_GetParamsType* getParams){
	int doubleCounter = 0;
	int stringCounter = 0;
	int intCounter = 0;
	int boolCounter = 0;

	int doubleArrayCounter = 0;
	int stringArrayCounter = 0;
	int intArrayCounter = 0;
	int boolArrayCounter = 0;

	int numParams = BMhGetSize(reinterpret_cast<GSHandle> (getParams->params)) / Sizeof32(API_AddParType) - 1;
	for (int i = 0; i < numParams; i++){
		API_AddParType currentParam = (*getParams->params)[i];
		std::string paramName = (std::string)currentParam.name;
		if (currentParam.flags == API_ParFlg_Disabled
			|| currentParam.flags == API_ParFlg_Hidden
			|| currentParam.flags == API_ParFlg_SHidden
			|| currentParam.flags == API_ParFlg_BoldName){
		}else{
			//msg->add_names(paramName);
			if (currentParam.typeID == APIParT_Integer){
				msg->add_names(paramName);
				msg->add_paramtype("i");
				//if (currentParam.dim1 > 1 || currentParam.dim2 > 1){
				if (currentParam.typeMod != API_ParSimple){
					intarray* iarray = msg->add_intarrays();
					int** arrHdl = reinterpret_cast<int**>(currentParam.value.array);
					for (int j = 0; j < (currentParam.dim1 + currentParam.dim2); j++){
						iarray->add_lst((*arrHdl)[j]);
					}
					msg->add_isarray(true);
				}
				else{
					msg->add_integers((int)currentParam.value.real);
					msg->add_isarray(false);
				}
			}
			else if(currentParam.typeID == APIParT_Length){ 
				
				if (currentParam.typeMod != API_ParSimple){
					//TODO
				}
				else{
					msg->add_names(paramName);
					msg->add_paramtype("d");
					msg->add_doubles(currentParam.value.real);
					msg->add_isarray(false);
				}
			}
			else if (currentParam.typeID == APIParT_Angle
					|| currentParam.typeID == APIParT_RealNum){
				msg->add_names(paramName);
				msg->add_paramtype("d");
				if (currentParam.typeMod != API_ParSimple){
					doublearray* darray = msg->add_doublearrays();
					double** arrHdl = reinterpret_cast<double**>(currentParam.value.array);
					for (int j = 0; j < (currentParam.dim1); j++){
						darray->add_lst((*arrHdl)[j]);
					}
					msg->add_isarray(true);
				}
				else{
					msg->add_doubles(currentParam.value.real);
					msg->add_isarray(false);
				}
			}
			else if (currentParam.typeID == APIParT_LineTyp){
				msg->add_names(paramName);
				msg->add_paramtype("s");
				msg->add_strings(searchLineTypes((int)currentParam.value.real));
				msg->add_isarray(false);
			}
			else if (currentParam.typeID == APIParT_Mater){
				msg->add_names(paramName);
				msg->add_paramtype("s");
				msg->add_strings(searchOverrideMaterialsValue((short)currentParam.value.real));
				msg->add_isarray(false);
			}
			else if (currentParam.typeID == APIParT_CString){
				msg->add_names(paramName);
				msg->add_paramtype("s");
				if (currentParam.typeMod != API_ParSimple){
					stringarray* sarray = msg->add_stringarrays();
					GS::uchar_t** arrHdl = reinterpret_cast<GS::uchar_t**>(currentParam.value.array);
					//int arrsize = BMhGetSize(reinterpret_cast<GSHandle> (arrHdl)) / Sizeof32(GS::uchar_t) - 1;
					int previousStringSize = 0;
					for (Int32 r = 0; r < currentParam.dim1; r++){
						std::string* currentString = sarray->add_lst();
						char* valueStr = *currentParam.value.array + r + previousStringSize;
						currentString->append(valueStr);
						//r += strlen(valueStr);	
						previousStringSize += strlen(valueStr);
					}
					msg->add_isarray(true);
				}
				else{
					msg->add_strings(GS::UniString(currentParam.value.uStr).ToCStr());
					msg->add_isarray(false);
				}
			}
			else if (currentParam.typeID == APIParT_Boolean){
				msg->add_names(paramName);
				msg->add_paramtype("b");
				if (currentParam.dim1 > 1 || currentParam.dim2 > 1){
					boolarray* barray = msg->add_boolarrays();
					bool** arrHdl = reinterpret_cast<bool**>(currentParam.value.array);
					for (int j = 0; j < (currentParam.dim1); j++){
						barray->add_lst((*arrHdl)[j]);
					}
					msg->add_isarray(true);
				}
				else{
					msg->add_booleans(currentParam.value.real);
					msg->add_isarray(false);
				}
			}
		}
	}
}

void populateMemo(API_ElementMemo *memo, pointsmessage pts, polyarcsmessage arcs){
	char buffer[256];

	memo->coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((pts.px_size() + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	memo->parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((arcs.arcangle_size()) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	if (memo->coords == NULL || memo->parcs == NULL) {
		ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
	}

	for (int i = 0; i < pts.px_size(); i++){
		(*memo->coords)[i + 1].x = pts.px(i);
		(*memo->coords)[i + 1].y = pts.py(i);
	}

	for (int i = 0; i < arcs.arcangle_size(); i++){
		(*memo->parcs)[i].begIndex = arcs.begindex(i);
		(*memo->parcs)[i].endIndex = arcs.endindex(i);
		(*memo->parcs)[i].arcAngle = arcs.arcangle(i);
	}
}

void writeMaterialsWall(){
	GSErrCode err;
	std::ofstream file;
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

void turnRefreshOn(){
	visualFeedbackOn = true;
}

void turnRefreshOff(){
	visualFeedbackOn = false;
}

bool getVisualFeedback(){
	return visualFeedbackOn;
}

void render(){
	API_PhotoRenderPars     rendPars;
	rendermsg msg;
	IO::Location            fileLoc;
	GSErrCode               err;

	readDelimitedFrom(getClientSocket(), &msg);
	fileLoc.Set(msg.file().c_str());
	
	if (hasError(fileLoc.GetStatus())){
		quit();
		return;
	}

	BNZeroMemory(&rendPars, sizeof(API_PhotoRenderPars));
	rendPars.fileTypeID = APIFType_PNGFile;
	rendPars.file = &fileLoc;
	rendPars.colorDepth = APIColorDepth_TC32;
	
	err = ACAPI_Automate(APIDo_PhotoRenderID, &rendPars, NULL);

	if (hasError(err)){
		quit();
		return;
	}
}

int getLayerNumber(std::string name){
	if (name == "ArchiCAD Layer"){
		return 1;
	}
	else{
		return searchLayers(name);
	}
}

void holeMemo(int oldSize, int oldPends, int oldArcSize, pointsmessage pointsMsg, polyarcsmessage polyarcsMsg, API_ElementMemo oldMemo, API_ElementMemo* memo, int* newSize, int* newPends, int* newArcSize){
	char buffer[256];
	*newSize = oldSize + pointsMsg.px_size();
	*newPends = oldPends + 1;
	*newArcSize += oldArcSize + polyarcsMsg.arcangle_size();

	memo->coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((*newSize + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	memo->pends = reinterpret_cast<Int32**> (BMAllocateHandle((*newPends + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo->parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle((*newArcSize) * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

	if (memo->coords == NULL || memo->pends == NULL || memo->parcs == NULL) {
		ErrorBeep("Not enough memory to create slab polygon data", APIERR_MEMFULL);
		sprintf(buffer, "Memory Problem");
		ACAPI_WriteReport(buffer, true);
		ACAPI_DisposeElemMemoHdls(memo);
		quit();
		return;
	}

	memo->edgeTrims = reinterpret_cast<API_EdgeTrim**> (BMAllocateHandle((*newSize + 1) * sizeof(API_EdgeTrim), ALLOCATE_CLEAR, 0));
	memo->sideMaterials = reinterpret_cast<API_MaterialOverrideType*> (BMAllocatePtr((*newSize + 1) * sizeof(API_MaterialOverrideType), ALLOCATE_CLEAR, 0));

	//Populate memo with old information
	for (int i = 1; i <= oldSize; i++){
		(*memo->coords)[i].x = (*oldMemo.coords)[i].x;
		(*memo->coords)[i].y = (*oldMemo.coords)[i].y;
	}

	(*memo->pends)[0] = 0;
	for (int i = 1; i <= oldPends; i++){
		(*memo->pends)[i] = (*oldMemo.pends)[i];
	}

	for (int i = 0; i < oldArcSize; i++){
		(*memo->parcs)[i].begIndex = (*oldMemo.parcs)[i].begIndex;
		(*memo->parcs)[i].endIndex = (*oldMemo.parcs)[i].endIndex;
		(*memo->parcs)[i].arcAngle = (*oldMemo.parcs)[i].arcAngle;
	}
	//////////////////////////////////////////////////

	//Populate memo with new information, hole
	int auxIndex = 0;
	for (int i = oldSize + 1; i <= *newSize; i++){
		(*memo->coords)[i].x = pointsMsg.px(auxIndex);
		(*memo->coords)[i].y = pointsMsg.py(auxIndex);
		auxIndex++;
	}

	(*memo->pends)[*newPends] = *newSize;
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
	for (int i = oldArcSize; i < *newArcSize; i++){
		(*memo->parcs)[i].begIndex = oldSize + polyarcsMsg.begindex(auxIndex);
		(*memo->parcs)[i].endIndex = oldSize + polyarcsMsg.endindex(auxIndex);
		(*memo->parcs)[i].arcAngle = polyarcsMsg.arcangle(auxIndex);
		auxIndex++;
	}

}




