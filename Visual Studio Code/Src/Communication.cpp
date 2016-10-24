#include "RosettaArchiCAD.hpp"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "53800"

SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;


int userQuit = 0;

SOCKET getClientSocket(){
	return ClientSocket;
}

SOCKET getListenSocket(){
	return ListenSocket;
}

void quit(){
	userQuit = 1;
}

int getUserQuit(){
	return userQuit;
}

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
	/*
	while (!success){
	actualBytesPeeked = recv(ClientSocket, buffer, tryBytes, MSG_PEEK);
	//actualBytesPeeked = recv(ClientSocket, buffer, 4, MSG_WAITALL);
	if (actualBytesPeeked == SOCKET_ERROR){
	ACAPI_WriteReport("Error receiving data", true);
	return false;
	}
	siz = readHdr(buffer, tryBytes, &success);
	if (actualBytesPeeked == tryBytes){
	tryBytes++;
	}
	if (tryBytes > 100){
	ACAPI_WriteReport("Overflow header buffer error", true);
	return false;
	}
	}
	*/

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
void sendElementID(SOCKET ClientSocket, API_Element element, bool crash){
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

void sendElementID(SOCKET ClientSocket, API_Element element){
	sendElementID(ClientSocket, element, false);
}

bool hasError(GSErrCode err){
	char buffer[256];
	if (err != NoError){
		ErrorBeep("ACAPI_Element_Create", err);
		sprintf(buffer, ErrID_To_Name(err));
		ACAPI_WriteReport(buffer, true);
		return true;
	}
	return false;
}

int winsockcom(bool infiniteCycle){
	WSADATA wsaData;
	int iResult;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	ListenSocket = INVALID_SOCKET;
	ClientSocket = INVALID_SOCKET;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	//bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	//bind(ListenSocket, result->ai_addr, std::placeholders::_1);

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ListenSocket);

	// No longer need server socket
	//closesocket(ListenSocket);

	GSErrCode		err;
	namemessage		namemsg;
	//For receiving messages
	//google::protobuf::io::ZeroCopyInputStream *raw_in;

	char bufferErr[256];

	int count = 0;

	char buffer[4];
	int bytecount = 0;
	std::string output, pl;

	memset(buffer, '\0', 4);

	while (true){

		while (getUserQuit() == 0){

			if (!readDelimitedFrom(ClientSocket, &namemsg)){
				ACAPI_WriteReport("Communication Failed", true);
				break;
			}

			//raw_out = new google::protobuf::io::CopyingOutputStreamAdaptor(&ais);

			err = ACAPI_OpenUndoableSession("Geometry Test -- Create elements");

			if (err != NoError) {
				sprintf(bufferErr, ErrID_To_Name(err));
				ACAPI_WriteReport(bufferErr, true);
				return 0;
			}

			callScript(namemsg.name());

			if (getVisualFeedback() &&
				namemsg.name() != "Profile" &&
				namemsg.name() != "AddToProfile"){
				refresh3DView();
			}

			ACAPI_CloseUndoableSession();
		}
		if (!infiniteCycle){
			break;
		}
	}

	// Receive until the peer shuts down the connection
	//while (iResult > 0) {
	//	iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	//	if (iResult > 0) {
	//		printf("Bytes received: %d\n", iResult);
	//		// Echo the buffer back to the sender
	//		iSendResult = send(ClientSocket, recvbuf, iResult, 0);
	//		if (iSendResult == SOCKET_ERROR) {
	//			printf("send failed with error: %d\n", WSAGetLastError());
	//			closesocket(ClientSocket);
	//			WSACleanup();
	//			return 1;
	//		}
	//		printf("Bytes sent: %d\n", iSendResult);
	//	}
	//	else if (iResult == 0)
	//		printf("Connection closing...\n");
	//	else  {
	//		printf("recv failed with error: %d\n", WSAGetLastError());
	//		closesocket(ClientSocket);
	//		WSACleanup();
	//		return 1;
	//	}
	//}

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}

DWORD WINAPI MyThreadFunction(LPVOID lpParam){
	//ACAPI_WriteReport("Thread alive", true);
	try{
		winsockcom(true);
	}
	catch (std::exception& e){
		ACAPI_WriteReport(e.what(), true);
	}
	return 0;
}