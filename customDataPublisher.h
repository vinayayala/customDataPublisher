#ifndef CUSTOM_DATA_PUBLISHER_H_
#define CUSTOM_DATA_PUBLISHER_H_

// Debugging Statement
// #define CUSTOM_DATA_PUBLISHER_H_DEBUG

#ifdef CUSTOM_DATA_PUBLISHER_H_DEBUG
#define MS_DEBUGGING_STD "CustomPublisher"
#endif

// Included Dependencies
#include <ModSensorDebugger.h>
#undef MS_DEBUGGING_STD

#include <dataPublisherBase.h>


class CustomPublisher : public dataPublisher {
 public:
    // Constructors
    /**
     * @brief Construct a new EnviroDIY Custom Publisher object with no members set.
     */
    CustomPublisher();
    /**
     * @brief Construct a new EnviroDIY Custom Publisher object
     *
     * @note If a client is never specified, the publisher will attempt to
     * create and use a client on a LoggerModem instance tied to the attached
     * logger.
     *
     * @param baseLogger The logger supplying the data to be published
     * @param sendEveryX Currently unimplemented, intended for future use to
     * enable caching and bulk publishing
     * @param sendOffset Currently unimplemented, intended for future use to
     * enable publishing data at a time slightly delayed from when it is
     * collected
     *
     * @note It is possible (though very unlikey) that using this constructor
     * could cause errors if the compiler attempts to initialize the publisher
     * instance before the logger instance.  If you suspect you are seeing that
     * issue, use the null constructor and a populated begin(...) within your
     * set-up function.
     */
    explicit CustomPublisher(Logger& baseLogger, uint8_t sendEveryX = 1,
                                uint8_t sendOffset = 0);
    /**
     * @brief Construct a new EnviroDIY Custom Publisher object
     *
     * @param baseLogger The logger supplying the data to be published
     * @param inClient An Arduino client instance to use to print data to.
     * Allows the use of any type of client and multiple clients tied to a
     * single TinyGSM modem instance
     * @param sendEveryX Currently unimplemented, intended for future use to
     * enable caching and bulk publishing
     * @param sendOffset Currently unimplemented, intended for future use to
     * enable publishing data at a time slightly delayed from when it is
     * collected
     *
     * @note It is possible (though very unlikey) that using this constructor
     * could cause errors if the compiler attempts to initialize the publisher
     * instance before the logger instance.  If you suspect you are seeing that
     * issue, use the null constructor and a populated begin(...) within your
     * set-up function.
     */
    CustomPublisher(Logger& baseLogger, Client* inClient,
                       uint8_t sendEveryX = 1, uint8_t sendOffset = 0);
    /**
     * @brief Construct a new EnviroDIY Custom Publisher object
     *
     * @param baseLogger The logger supplying the data to be published
     * @param registrationToken The registration token for the site on the
     * Monitor My Watershed data portal.
     * @param samplingFeatureUUID The sampling feature UUID for the site on the
     * Monitor My Watershed data portal.
     * @param sendEveryX Currently unimplemented, intended for future use to
     * enable caching and bulk publishing
     * @param sendOffset Currently unimplemented, intended for future use to
     * enable publishing data at a time slightly delayed from when it is
     * collected
     */
    CustomPublisher(Logger& baseLogger, const char* registrationToken,
                       const char* samplingFeatureUUID, uint8_t sendEveryX = 1,
                       uint8_t sendOffset = 0);
    /**
     * @brief Construct a new EnviroDIY Custom Publisher object
     *
     * @param baseLogger The logger supplying the data to be published
     * @param inClient An Arduino client instance to use to print data to.
     * Allows the use of any type of client and multiple clients tied to a
     * single TinyGSM modem instance
     * @param registrationToken The registration token for the site on the
     * Monitor My Watershed data portal.
     * @param samplingFeatureUUID The sampling feature UUID for the site on the
     * Monitor My Watershed data portal.
     * @param sendEveryX Currently unimplemented, intended for future use to
     * enable caching and bulk publishing
     * @param sendOffset Currently unimplemented, intended for future use to
     * enable publishing data at a time slightly delayed from when it is
     * collected
     */
    CustomPublisher(Logger& baseLogger, Client* inClient,
                       const char* registrationToken,
                       const char* samplingFeatureUUID, uint8_t sendEveryX = 1,
                       uint8_t sendOffset = 0);
    /**
     * @brief Destroy the EnviroDIY Custom Publisher object
     */
    virtual ~CustomPublisher();

    // Returns the data destination
    String getEndpoint(void) override {
        return String(enviroDIYHost);
    }

    // Adds the site registration token
    /**
     * @brief Set the site registration token
     *
     * @param registrationToken The registration token for the site on the
     * Monitor My Watershed data portal.
     */
    void setToken(const char* registrationToken);

    /**
     * @brief Calculates how long the outgoing JSON will be
     *
     * @return uint16_t The number of characters in the JSON object.
     */
    uint16_t calculateJsonSize();
    // /**
    //  * @brief Calculates how long the full post request will be, including
    //  * headers
    //  *
    //  * @return uint16_t The length of the full request including HTTP
    //  headers.
    //  */
    // uint16_t calculatePostSize();

    /**
     * @brief This generates a properly formatted JSON for EnviroDIY and prints
     * it to the input Arduino stream object.
     *
     * @param stream The Arduino stream to write out the JSON to.
     */
    void printSensorDataJSON(Stream* stream);

    /**
     * @brief This prints a fully structured post request for Monitor My
     * Watershed/EnviroDIY to the specified stream.
     *
     * @param stream The Arduino stream to write out the request to.
     */
    void printEnviroDIYRequest(Stream* stream);

    // A way to begin with everything already set
    /**
     * @copydoc dataPublisher::begin(Logger& baseLogger, Client* inClient)
     * @param registrationToken The registration token for the site on the
     * Monitor My Watershed data portal.
     * @param samplingFeatureUUID The sampling feature UUID for the site on the
     * Monitor My Watershed data portal.
     */
    void begin(Logger& baseLogger, Client* inClient,
               const char* registrationToken, const char* samplingFeatureUUID);
    /**
     * @copydoc dataPublisher::begin(Logger& baseLogger)
     * @param registrationToken The registration token for the site on the
     * Monitor My Watershed data portal.
     * @param samplingFeatureUUID The sampling feature UUID for the site on the
     * Monitor My Watershed data portal.
     */
    void begin(Logger& baseLogger, const char* registrationToken,
               const char* samplingFeatureUUID);

    // int16_t postDataEnviroDIY(void);
    /**
     * @brief Utilize an attached modem to open a a TCP connection to the
     * EnviroDIY/ODM2DataSharingPortal and then stream out a post request over
     * that connection.
     *
     * This depends on an internet connection already having been made and a
     * client being available.
     *
     * @param outClient An Arduino client instance to use to print data to.
     * Allows the use of any type of client and multiple clients tied to a
     * single TinyGSM modem instance
     * @return **int16_t** The http status code of the response.
     */
    int16_t publishData(Client* outClient) override;

 protected:
    static const char* postEndpoint;   ///< The endpoint
    static const char* enviroDIYHost;  ///< The host name
    static const int   enviroDIYPort;  ///< The host port
    static const char* tokenHeader;    ///< The token header text
    // static const char *cacheHeader;  ///< The cache header text
    // static const char *connectionHeader;  ///< The keep alive header text
    static const char* contentLengthHeader;  ///< The content length header text
    static const char* contentTypeHeader;    ///< The content type header text

    static const char* samplingFeatureTag;  ///< The JSON feature UUID tag
    static const char* timestampTag;        ///< The JSON feature timestamp tag

    static const char* iFloodFolderName;

 private:
    // Tokens and UUID's for EnviroDIY
    const char* _registrationToken;

   int dataCount = 0;
   String rawData = "";
   int maxNumOfRows = 1;
   String formatTimeString();
   void storeRawData();
   void updateDataUploadInterval();
};

#endif
