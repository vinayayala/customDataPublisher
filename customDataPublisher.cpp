#include "customDataPublisher.h"


// ============================================================================
//  Functions for the EnviroDIY data portal receivers.
// ============================================================================

// Constant values for post requests
// I want to refer to these more than once while ensuring there is only one copy
// in memory
const char* CustomPublisher::postEndpoint  = "/prod/mayflyupload";
const char* CustomPublisher::enviroDIYHost = "d3uyjvsgi4dmwc.cloudfront.net";
const int   CustomPublisher::enviroDIYPort = 80;
const char* CustomPublisher::tokenHeader   = "\r\nTOKEN: ";
// const unsigned char *CustomPublisher::cacheHeader = "\r\nCache-Control:
// no-cache"; const unsigned char *CustomPublisher::connectionHeader =
// "\r\nConnection: close";
const char* CustomPublisher::contentLengthHeader = "\r\nContent-Length: ";
const char* CustomPublisher::contentTypeHeader =
    "\r\nContent-Type: application/json\r\n\r\n";

const char* CustomPublisher::samplingFeatureTag = "{\"sampling_feature\":\"";
const char* CustomPublisher::timestampTag       = "\",\"timestamp\":\"";

const char* CustomPublisher::iFloodFolderName = "DigiBeeData2022\n";

// CSV header
// Date,Battery_Voltage,Temperature,Signal_Percent,Distance
// static char *csvHeader = "Date,Battery_Voltage,Temperature,Signal_Percent,Distance\n";

// Constructors
CustomPublisher::CustomPublisher() : dataPublisher() {
    // MS_DBG(F("dataPublisher object created"));
    _registrationToken = NULL;
    updateDataUploadInterval();
}
CustomPublisher::CustomPublisher(Logger& baseLogger, uint8_t sendEveryX,
                                       uint8_t sendOffset)
    : dataPublisher(baseLogger, sendEveryX, sendOffset) {
    _registrationToken = NULL;
    updateDataUploadInterval();
}
CustomPublisher::CustomPublisher(Logger& baseLogger, Client* inClient,
                                       uint8_t sendEveryX, uint8_t sendOffset)
    : dataPublisher(baseLogger, inClient, sendEveryX, sendOffset) {
    updateDataUploadInterval();
}
CustomPublisher::CustomPublisher(Logger&     baseLogger,
                                       const char* registrationToken,
                                       const char* samplingFeatureUUID,
                                       uint8_t sendEveryX, uint8_t sendOffset)
    : dataPublisher(baseLogger, sendEveryX, sendOffset) {
    setToken(registrationToken);
    _baseLogger->setSamplingFeatureUUID(samplingFeatureUUID);
    updateDataUploadInterval();
}
CustomPublisher::CustomPublisher(Logger& baseLogger, Client* inClient,
                                       const char* registrationToken,
                                       const char* samplingFeatureUUID,
                                       uint8_t sendEveryX, uint8_t sendOffset)
    : dataPublisher(baseLogger, inClient, sendEveryX, sendOffset) {
    setToken(registrationToken);
    _baseLogger->setSamplingFeatureUUID(samplingFeatureUUID);
    updateDataUploadInterval();
}
// Destructor
CustomPublisher::~CustomPublisher() {}

void CustomPublisher::setToken(const char* registrationToken) {
    _registrationToken = registrationToken;
}

// Calculates how long the JSON will be
uint16_t CustomPublisher::calculateJsonSize() {
    uint16_t jsonLength = 0;
    jsonLength += strlen(iFloodFolderName);
    jsonLength += rawData.length();
    
    // Checking for a new data length which is about tobe stored
    jsonLength += 19;          // 2021-10-26 01:06:00
    jsonLength += 1;           // ,
    for (uint8_t i = 0; i < _baseLogger->getArrayVarCount(); i++) {
        jsonLength += _baseLogger->getValueStringAtI(i).length();
        if (i + 1 != _baseLogger->getArrayVarCount()) {
            jsonLength += 1;  // ,
        }
    }
    jsonLength += 1; // "\n"

    return jsonLength;
}

/*
// Calculates how long the full post request will be, including headers
uint16_t CustomPublisher::calculatePostSize()
{
    uint16_t postLength = 32;  // "POST /prod/mayflyupload HTTP/1.1"
    postLength += 37;  // "\r\nHost: d3uyjvsgi4dmwc.cloudfront.net"
    postLength += 11;  // "\r\nTOKEN: "
    postLength += 36;  // registrationToken
    // postLength += 27;  // "\r\nCache-Control: no-cache"
    // postLength += 21;  // "\r\nConnection: close"
    postLength += 20;  // "\r\nContent-Length: "
    postLength += String(calculateJsonSize()).length();
    postLength += 42;  // "\r\nContent-Type: application/json\r\n\r\n"
    postLength += calculateJsonSize();
    return postLength;
}
*/

// This prints a properly formatted JSON for EnviroDIY to an Arduino stream
void CustomPublisher::printSensorDataJSON(Stream* stream) {
    stream->print(samplingFeatureTag);
    stream->print(_baseLogger->getSamplingFeatureUUID());
    stream->print(timestampTag);
    stream->print(_baseLogger->formatDateTime_ISO8601(Logger::markedEpochTime));
    stream->print(F("\","));

    for (uint8_t i = 0; i < _baseLogger->getArrayVarCount(); i++) {
        stream->print('"');
        stream->print(_baseLogger->getVarUUIDAtI(i));
        stream->print(F("\":"));
        stream->print(_baseLogger->getValueStringAtI(i));
        if (i + 1 != _baseLogger->getArrayVarCount()) { stream->print(','); }
    }

    stream->print('}');
}

// This prints a fully structured post request for EnviroDIY to the
// specified stream.
void CustomPublisher::printEnviroDIYRequest(Stream* stream) {
    // Stream the HTTP headers for the post request
    stream->print(postHeader);
    stream->print(postEndpoint);
    stream->print(HTTPtag);
    stream->print(hostHeader);
    stream->print(enviroDIYHost);
    stream->print(tokenHeader);
    stream->print(_registrationToken);
    // stream->print(cacheHeader);
    // stream->print(connectionHeader);
    stream->print(contentLengthHeader);
    stream->print(calculateJsonSize());
    stream->print(contentTypeHeader);

    // Stream the JSON itself
    printSensorDataJSON(stream);
}

// A way to begin with everything already set
void CustomPublisher::begin(Logger& baseLogger, Client* inClient,
                               const char* registrationToken,
                               const char* samplingFeatureUUID) {
    setToken(registrationToken);
    dataPublisher::begin(baseLogger, inClient);
    _baseLogger->setSamplingFeatureUUID(samplingFeatureUUID);
}
void CustomPublisher::begin(Logger&     baseLogger,
                               const char* registrationToken,
                               const char* samplingFeatureUUID) {
    setToken(registrationToken);
    dataPublisher::begin(baseLogger);
    _baseLogger->setSamplingFeatureUUID(samplingFeatureUUID);
}

// This utilizes an attached modem to make a TCP connection to the
// EnviroDIY/ODM2DataSharingPortal and then streams out a post request
// over that connection.
// The return is the http status code of the response.
// int16_t CustomPublisher::postDataEnviroDIY(void)
int16_t CustomPublisher::publishData(Client* outClient) {
    // Create a buffer for the portions of the request and response
    char     tempBuffer[37] = "";
    uint16_t did_respond    = 0;
    int16_t responseCode = 0;
    
    if ((dataCount + 1) >= maxNumOfRows) {
        // Send the data to internet

        MS_DBG(F("Outgoing JSON size:"), calculateJsonSize());
        PRINTOUT(F("\n -- Test --"), calculateJsonSize());

        // Open a TCP/IP connection to the Enviro DIY Data Portal (WebSDL)
        MS_DBG(F("Connecting client"));
        MS_START_DEBUG_TIMER;
        if (outClient->connect(enviroDIYHost, enviroDIYPort)) {
            MS_DBG(F("Client connected after"), MS_PRINT_DEBUG_TIMER, F("ms\n"));

            // copy the initial post header into the tx buffer
            strcpy(txBuffer, postHeader);
            strcat(txBuffer, postEndpoint);
            strcat(txBuffer, HTTPtag);

            PRINTOUT(F("\n -- Test --"), txBuffer);
            if (bufferFree() < 33) printTxBuffer(outClient);

            // add the rest of the HTTP POST headers to the outgoing buffer
            // before adding each line/chunk to the outgoing buffer, we make sure
            // there is space for that line, sending out buffer if not
            strcat(txBuffer, hostHeader);
            strcat(txBuffer, enviroDIYHost);

            PRINTOUT(F("\n -- Test --"), txBuffer);
            if (bufferFree() < 36) printTxBuffer(outClient);

            strcat(txBuffer, contentLengthHeader);
            itoa(calculateJsonSize(), tempBuffer, 10);  // BASE 10
            strcat(txBuffer, tempBuffer);

            PRINTOUT(F("\n -- Test --"), txBuffer);
            if (bufferFree() < 22) printTxBuffer(outClient);

            strcat(txBuffer, contentTypeHeader);

            PRINTOUT(F("\n -- Test --"), txBuffer);
            if (bufferFree() < 35) printTxBuffer(outClient);

            strcat(txBuffer, iFloodFolderName);

            PRINTOUT(F("\n -- Test --"), txBuffer);
            if (bufferFree() < 22) printTxBuffer(outClient);

            storeRawData();

            strcat(txBuffer, rawData.c_str());

            PRINTOUT(F("\n -- Test --"), txBuffer);
            printTxBuffer(outClient, true);

            // Wait 10 seconds for a response from the server
            uint32_t start = millis();
            while ((millis() - start) < 10000L && outClient->available() < 12) {
                delay(10);
            }

            // Read only the first 12 characters of the response
            // We're only reading as far as the http code, anything beyond that
            // we don't care about.
            did_respond = outClient->readBytes(tempBuffer, 12);

            // Close the TCP/IP connection
            MS_DBG(F("Stopping client"));
            MS_RESET_DEBUG_TIMER;
            outClient->stop();
            MS_DBG(F("Client stopped after"), MS_PRINT_DEBUG_TIMER, F("ms"));
        } else {
            PRINTOUT(F("\n -- Unable to Establish Connection to EnviroDIY Data "
                    "Portal --"));
        }

        // Process the HTTP response
        if (did_respond > 0) {
            char responseCode_char[4];
            for (uint8_t i = 0; i < 3; i++) {
                responseCode_char[i] = tempBuffer[i + 9];
            }
            responseCode = atoi(responseCode_char);
        } else {
            responseCode = 504;
        }

        PRINTOUT(F("-- Response Code --"));
        PRINTOUT(responseCode);

        if (responseCode >= 200 && responseCode <= 299) {
            // Reset the values
            dataCount = 0;
            rawData = "";
        }
    } else {
        // Store the data until the dataCount is equal to maxRowCount
        storeRawData();
    }

    return responseCode;
}

String CustomPublisher::formatTimeString() {
    String dtStr;
    DateTime dt = Logger::dtFromEpoch(Logger::markedEpochTime);
    dt.addToString(dtStr);
    return dtStr;
}

void CustomPublisher::storeRawData() {
    dataCount++;

    PRINTOUT(F("\n -- Storing raw data -- "), dataCount);

    // Insert DateTime
    rawData += formatTimeString();
    rawData += ",";

    // Insert sensor values
    for (uint8_t i = 0; i < _baseLogger->getArrayVarCount(); i++) {
        rawData += _baseLogger->getValueStringAtI(i);
        if (i + 1 != _baseLogger->getArrayVarCount()) {
            rawData += ",";
        }
    }
    rawData += "\n";
}

void CustomPublisher::updateDataUploadInterval() {
    uint16_t loggingInterval = _baseLogger->getLoggingInterval();

    // If interval cannot divide 60 minutes make it so
    while (60 % loggingInterval) {
        loggingInterval--;
    }

    _baseLogger->setLoggingInterval(loggingInterval);

    if (_sendEveryX <= 0) {
        _sendEveryX = 1; // 1 hour interval for upload
    }

    int minutes = _sendEveryX * 60;
    if (minutes % loggingInterval) {
        PRINTOUT(F("\n -- Some fraction of data could not be uploaded --"));
        PRINTOUT(F("\n -- Change the logging and data upload interval --"));
    }
    maxNumOfRows = minutes / loggingInterval;

    // TODO
    maxNumOfRows = 12;
}
