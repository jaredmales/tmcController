/** \file tmcController.hpp
 *  \brief Declare and define the tmcController class
 */

//***********************************************************************//
// Copyright 2023 Jared R. Males (jaredmales@pm.me)
//
// This file is part of tmcController.
//
// tmcController is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// tmcController is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with mxlib.  If not, see <http://www.gnu.org/licenses/>.
//***********************************************************************//

#ifndef tmcController_hpp
#define tmcController_hpp

#include <cstring>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

#include <ftdi.h>
 
/*
Links to FTDI docs defined in doxygen ALIASES
\libftdi1 https://www.intra2net.com/en/developer/libftdi/documentation/index.html
\ftdi_new https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8c.html#afd46c6784367fd95c5f852d3b38949a8
\ftdi_free https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8c.html#a3cc28e5b7f1f50246861fcb39e7dd291
\ftdi_context https://www.intra2net.com/en/developer/libftdi/documentation/structftdi__context.html
\ftdi_usb_open_desc_index https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8h.html#a3c5ccb0678b0e268b4081ea978e8eb78
\ftdi_usb_close https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8h.html#a9536eff48c7ae9313b2a97320743ee3b
\ftdi_read_chipid https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8c.html#ab978a76c2cc0dbfbc1fd6350db848300
\ftdi_set_baudrate https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8c.html#ae80f1dc38f5c80946a56ec34a329e7f5
\ftdi_set_line_property https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8c.html#a044fdde91eaa3f557b545172622ee460
\ftdi_tcioflush https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8c.html#aeca5f2979d57490be0f3646fb4f3116a
\ftdi_usb_reset https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8c.html#a266ccc351ea31289022063a8ae29edf4
\ftdi_setflowctrl https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8c.html#ac3b3d2cc69edf567e653340f434c91e0
\ftdi_setrts https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8c.html#a5713ba73288b3e1ac1d45096c917c1ab
\ftdi_write_data https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8h.html#a01199788c36ba93352f155a79ea295e8
\ftdi_read_data https://www.intra2net.com/en/developer/libftdi/documentation/ftdi_8h.html#a72d87e30015c98bd0be22e7c8c873345
\libusb_bulk_transfer https://libusb.sourceforge.io/api-1.0/group__libusb__syncio.html#ga2f90957ccc1285475ae96ad2ceb1f58c
*/


/// Class to manage the interface to a Thorlabs Motion Controller using \libftdi1
/** 
  * This is not thread safe.  You should mutex any access to an instance of this class.
  * 
  * Error handling: most functions return int to indicate errors. 0 is always success, < 0 indicates
  * an error.  This is usually the error code returned by the underlying libftdi1 function.
  */
class tmcController
{

/** \name Construction and Destruction
  *
  * @{ 
  */
public:
    /// Default c'tor
    /** Allocates \ref m_ftdi by calling \ftdi_new
      * 
      */
    tmcController();

    /// Destructor
    /** Allocates \ref m_ftdi by calling \ftdi_free
      * 
      */
    ~tmcController();

///@}

/** \name FTDI Context Data
  *
  * @{ 
  */
protected:
     
    /// The libftdi1 context structure
    /** See \ftdi_context
     * 
     */
    ftdi_context * m_ftdi {nullptr};
    
///@}

/** \name FTDI Context
  *
  * @{ 
  */
public:
     
    /// Get the libftdi1 context structure
    /** For use if you want to call a \libftdi1 function directly.
      * You should never call \ftdi_new or \ftdi_free directly.
      *   
      * \see m_ftdi
      * 
     */
    const ftdi_context * ftdi() const;
    
///@}

/** \name Internal Memory
  * @{ 
  */

protected:

    ///Memory used for sending data to the device
    unsigned char m_sndbuf[256];

    ///Memory used for reading data from the device
    unsigned char m_rdbuf[256];

///@}


/** \name Device Identification Data
  * @{ 
  */

protected:

    /// The USB vendor ID for this device
    /** Used to find the device when opening.
      * Default is 0x0403.
      * \see vendor(uint16_t)
      * \see vendor()
      */
    uint16_t m_vendor {0x0403};

    /// The USB product ID for this device
    /** Used to find the device when opening.
      * Default is 0x0faf0.
      * \see product(uint16_t)
      * \see product()
      */
    uint16_t m_product {0xfaf0};

    /// The USB device serial number
    /** Used to find the device when opening.
      * \see serial(const std::string &)
      * \see serial()
      */
    std::string m_serial;

///@}

/** \name Device Identification
  * @{ 
  */

public:
    /// Set the USB vendor ID 
    /** This is used to find the device when opening
      * \see m_vendor  
      */
    void vendor( uint16_t v /**< [in] the USB vendor ID */);

    /// Get the USB vendor ID
    /** \see m_vendor
      * \returns the USB vendor ID
      */
    uint16_t vendor();

    /// Set the USB product ID
    /** This is used to find the device when opening
      * \see m_product
      */
    void product( uint16_t p /**< [in] the USB product ID */ );
    
    /// Get the USB product ID
    /** \see m_product
      * \returns the USB product ID
      */
    uint16_t product();

    /// Set the USB serial number
    /** This is used the find the device
      * \see \ref m_serial   
      */
    void serial (const std::string & s /**< [in] the USB serial number */ );

    /// Get the USB serial number
    /** \see \ref m_serial
      * 
      * \returns the USB serial number
      */
    std::string serial();

///@}

/** \name Connection Management Data
  * @{ 
  */

protected:
    /// The baud rate
    /** Default is 115200.
      * 
      */
    uint32_t m_baud {115200};

    /// The time to sleep in milliseconds before calling \ftdi_tcioflush
    /** Used during \ref connect().
      * Default is 50 ms. 
      * 
      */
    uint32_t m_preFlushSleep {50};

    /// The time to sleep in milliseconds after calling \ftdi_tcioflush
    /** Used during \ref connect().
      * Default is 50 ms. 
      * 
      */
    uint32_t m_postFlushSleep {50};

    /// Flag indicating whether or not the USB device is open
    bool m_opened {false};

    /// Flag indicating whether or not the TMC device is connected
    bool m_connected {false};

    /// The chip ID of the FTDI on the TMC device.
    /** Read and set during \ref connect().
      * See \ftdi_read_chipid
      */
    unsigned int m_chipid {0};

///@}

/** \name Connection Management
  * @{ 
  */

public:

    /// Set the baud rate
    /** \see m_baud
      */
    void baud( uint32_t b /**< [in] the new baud rate*/ );

    /// Get the baud rate
    /** \see m_baud
      * 
      * \returns the current baud rate
      */
    uint32_t baud();

    /// Set the time to sleep in milliseconds before calling \ftdi_tcioflush
    /** \see m_preFlushSleep
      * 
      */
    void preFlushSleep( uint32_t s /**< [in] the pre-flush sleep time in ms */ );

    /// Get the time to sleep in milliseconds before calling \ftdi_tcioflush
    /** \see m_preFlushSleep
      * 
      */
    uint32_t preFlushSleep();

    /// Set the time to sleep in milliseconds after calling \ftdi_tcioflush
    /** \see m_postFlushSleep
      * 
      */
    void postFlushSleep( uint32_t s /**< [in] the post-flush sleep time in ms */ );

    /// Get the time to sleep in milliseconds after calling \ftdi_tcioflush
    /** \see m_postFlushSleep
      * 
      */
    uint32_t postFlushSleep();

    /// Open the device
    /** Finds the device described by \ref m_vendor, \ref m_product, and \ref m_serial
      * and opens it.  
      * Calls \ftdi_usb_open_desc_index.
      * 
      * \returns 0 on success
      * \returns < 0 on failure, see \ftdi_usb_open_desc_index
      */ 
    int open( bool errmsg = true /**< [in] [optional] flag controlling if an error message is printed on failure */);

    /// Set the USB serial number and open the device.
    /** \ref m_vendor and \ref m_product must already be set (see their defaults).
      * 
      * See \ref open.
      * 
      * \overload
      */
    int open( const std::string & s, ///< [in] the USB serial number (see \ref m_serial)
              bool errmsg = true     ///< [in] [optional] flag controlling if an error message is printed on failure
            );

    /// Set the USB device identifiers and open the device.
    /** See \ref open.
      * 
      * \overload
      */
    int open( uint16_t v,            ///< [in] the USB vendor ID (see \ref m_vendor)
              uint16_t p,            ///< [in] the USB product ID (see \ref m_product)
              const std::string & s, ///< [in] the USB serial number (see \ref m_serial)
              bool errmsg = true     ///< [in] [optional] flag controlling if an error message is printed on failure
            );

    /// Close the device
    /** Calls \ftdi_usb_close. 
      * 
      * \returns 0 on success (which includes if the device is not open)  
      * \returns <0 on error, see \ftdi_usb_close
      */
    int close( bool errmsg = true /**< [in] [optional] flag controlling if an error message is printed on failure */ );

    /// Connect to the device
    /** Takes the following steps:
      *  -# Calls \ref open if not already open
      *  -# Calls \ftdi_read_chipid
      *  -# Calls \ftdi_set_baudrate to set the baud rate to \ref m_baud 
      *  -# Calls \ftdi_set_line_property to set the serial line properties to 8-N-1
      *  -# Sleeps for \ref m_preFlushSleep milliseconds
      *  -# Call \ftdi_tcioflush
      *  -# Sleeps for \ref m_postFlushSleep milliseconds
      *  -# Calls \ftdi_usb_reset
      *  -# Calls \ftdi_setflowctrl to set SIO_RTS_CTS_HS
      *  -# Calls \ftdi_setrts to set RTS to 1
      * 
      * The error code returned by each function is offset in steps of -10 to allow you to decipher which \libftdi1 function failed.
      * 
      * \returns 0 on success
      * \returns < 0 if \ref open fails.  See \ftdi_usb_open_desc_index for codes.
      * \returns < -20 if \ftdi_read_chipid fails (-10 + return value)
      * \returns < -30 if \ftdi_set_baudrate fails (-20 + return value)
      * \returns < -40 if \ftdi_set_line_property fails (-30 + return value)
      * \returns -49 if an exception is thrown durign the pre-purge sleep
      * \returns < -50 if \ftdi_tcioflush fails
      * \returns -59 if an exception is thrown durign the post-purge sleep
      * \returns < -60 if \ftdi_usb_reset fails
      * \returns < -70 if \ftdi_setflowctrl fails
      * \returns < -80 if \ftdi_setrts fails
      */
    int connect( bool errmsg = true /**< [in] [optional] flag controlling if an error message is printed on failure*/);

    /// Set the USB serial number and connect to the device.
    /** \ref m_vendor and \ref m_product must already be set (see their defaults).
      * 
      * See \ref connect.
      * 
      * \overload
      */
    int connect( const std::string & s, ///< [in] the USB serial number (see \ref m_serial)
                 bool errmsg = true     ///< [in] [optional] flag controlling if an error message is printed on failure
               );

    /// Set the USB device identifiers and connect to the device.
    /** See \ref connect.
      * 
      * \overload
      */
    int connect( uint16_t v,            ///< [in] the USB vendor ID 
                 uint16_t p,            ///< [in] the USB product ID (see \ref m_product)
                 const std::string & s, ///< [in] the USB serial number (see \ref m_serial)
                 bool errmsg = true     ///< [in] [optional] flag controlling if an error message is printed on failure
               );

    /// Get the value of the flag indicating whether or not the USB device is open.
    /**
      * \returns the true if open, false if not
      */
    bool opened();

    /// Get the value of the flag indicating whether or not the TMC device is connected.
    /**
      * \returns the true if connected, false if not
      */
    bool connected();

    /// Get the chip ID of the FTDI on the TMC device
    /** This is read and set during \ref connect().
      */ 
    unsigned int chipid();

///@}

/** \name Command Management Data
  *
  * Member data to manage the sending of commands.
  *  
  * @{ 
  */

protected:

    /// The total number of bytes read
    /** Is set to 0 before a read attempt starts
      * 
      */
    int m_totrd{0};

    /// The time to sleep in milliseconds after changing the channel enable state
    /** Used during \ref mod_set_chanenablestate().
      * Default is 500 ms.  
      */
    uint32_t m_postChanEnableSleep {500};

///@}

/** \name Command Management
  *
  * Functions to manage the sending of commands.
  *  
  * @{ 
  */


public:

    /// Get the total number of bytes read.
    /** \see m_totrd
      * 
      */
    int totrd();

    /// Set the time to sleep in milliseconds after changing channel enable state. 
    /** \see m_postChanEnableSleep
      * 
      */
    void postChanEnableSleep( uint32_t s /**< [in] the post sleep time in ms */ );
    
    /// Get the time to sleep in milliseconds after changing channel enable state.
    /** \see m_postChanEnableSleep
      * 
      */
    uint32_t postChanEnableSleep();


/** \name Command Data Structures
  *
  * Data structures for sending commands to and receiving responses from the device.
  *  
  * @{ 
  */

public:

    /// The enable states of a driver chanel
    enum class EnableState : uint8_t{ invalid = 0x00, ///< For error detection only, not used by TMC
                                      enabled = 0x01, ///< The channel is or will be enabled
                                      disabled = 0x02 ///< The channel is or will be disabled
                                    };

    /// Hardware information filled in by \ref hw_req_info
    struct HWInfo
    {
        /// The device serial number
        /** Set during \ref hw_req_info
          */
        uint32_t serialNumber {0};
    
        /// The device model number
        /** Set during \ref hw_req_info
          */
        std::string modelNumber;
    
        /// The device type
        /** Set during \ref hw_req_info
          */
        uint16_t type {0};
    
        /// The device firmware minor version number
        /** Set during \ref hw_req_info
          */
        int fwMin {0};
        
        /// The device firmware interim version number
        /** Set during \ref hw_req_info
          */
        int fwInt {0};
        
        /// The device firmware major version number
        /** Set during \ref hw_req_info
          */
        int fwMaj {0};
        
        /// The device hardware version number
        /** Set during \ref hw_req_info
          */
        uint16_t hwVer {0};
        
        /// The device hardware modification state
        /** Set during \ref hw_req_info
          */
        uint16_t hwMod {0};
        
        /// The number of channels on the device
        /** Set during \ref hw_req_info
          */
        uint16_t nChannels {0};

        /// Dump details to a stream
        /**
          * \tparam streamT is an std::iostream like class 
          */
        template<class streamT>
        void dump(streamT & ios /**< [out] the stream to dump to*/);

    }; //struct HWInfo



    /// Status of the Piezos, filled in by \ref pz_req_pzstatusupdate
    /** \note this currently only works for the TPZ001 KPZ101
      * 
      */
    struct PZStatus
    {
        /// The output voltage applied to the piezo
        /** Set during \ref pz_req_pzstatusupdate
          *
          * Range -32768 to 32767 (-7FFF to 7FFF) which
          * corresponds to -100% to 100% of the maximum output
          * voltage  
          */
        int16_t voltage {0};

        /// The position of the piezo.
        /** Set during \ref pz_req_pzstatusupdate
          * 
          * range 0 to 32767 (0 to 7FFF) which corresponds to 0 to
          * 100% of the maximum position 
          */
        int16_t position {0};

        /// Whether or not the piezo actuator is connected
        /** Set during \ref pz_req_pzstatusupdate 
          */
        bool connected {false};

        /// Whether or not the piezo actuator has been zeroed
        /** Set during \ref pz_req_pzstatusupdate 
          */
        bool zeroed {false};

        /// Whether or not the piezo actuator is being zeroed
        /** Set during \ref pz_req_pzstatusupdate 
          */
        bool zeroing {false};

        /// Whether or not a strain gauge is connected
        /** Set during \ref pz_req_pzstatusupdate 
          */
        bool sgConnected {false};

        /// Position control mode
        /** Set during \ref pz_req_pzstatusupdate 
          *
          * 0 = open-loop
          * 1 = closed-loop
          *  
          */
        bool pcMode {false};

        ///
        /** Set during \ref pz_req_pzstatusupdate 
          */
        timespec statusTime {0,0};
    
        ///Get the status relative to current time
        /**
          * \returns the age in seconds 
          */
        double age();
    
        /// Dump details to a stream
        /**
          * \tparam streamT is an std::iostream like class 
          */
        template<class streamT>
        void dump(streamT & ios /**< [out] the stream to dump to*/);

    };

    /// Possible voltage limits for TPZ IO Settings
    /** See page 224 of the manual
      * 
      */
    enum class VoltLimit : uint16_t { INVALID = 0x00, ///< For error detection only, not used by TMC
                                      V75 = 0x01,     ///< Voltage limit of 75 V
                                      V100 = 0x02,    ///< Voltage limit of 100 V
                                      V150 = 0x03     ///< Voltage limit of 150 V
                                    };

    /// TPZ I/O Setting, filled in by \ref pz_req_tpz_iosettings 
    /** Used for the MGMSG_PZ_SET_TPZ_IOSETTINGS (0x07D4) and MGMSG_PZ_REQ_TPZ_IOSETTINGS (0x07D5) commands 
      * See page 224 of the manual.
      * 
      */
    struct TPZIOSettings
    { 
        VoltLimit VoltageLimit {VoltLimit::INVALID}; ///< The voltage limit
        uint16_t HubAnalogInput {0x00};              ///< The hub feedback setup

        /// Dump details to a stream
        /**
          * \tparam streamT is an std::iostream like class 
          */
        template<class streamT>
        void dump(streamT & ios /**< [out] the stream to dump to*/);
    };

    /// K-Cube MMI Params, filled in by \ref kpz_req_kcubemmiparams
    /** 
      * 
      */
    struct KMMIParams 
    {
        uint16_t JSMode {0x01};
        uint16_t JSVoltGearBox {0x03};
        int32_t JSVoltStep {1};
        int16_t DirSense {0};
        int32_t PresetVolt1 {0};
        int32_t PresetVolt2 {0};
        uint16_t DispBrightness {100};
        uint16_t DispTimeout {0};
        uint16_t DispDimLevel {10};

        /// Dump details to a stream
        /**
          * \tparam streamT is an std::iostream like class 
          */
        template<class streamT>
        void dump(streamT & ios /**< [out] the stream to dump to*/);
    };

///@}

/** \name APT Commands
  * The actual Thorlabs Motion Controllers Host-Controller Communications Protocol implementations.
  * Declared here in order in which they appear in the manual.  Page numbers refer to Issue 37 of the manual,
  * dated 22 May 2023.
  * 
  * In each command, error codes by \libftdi1 functions are offset to allow you to decipher which \libftdi1 function failed.
  * 
  * @{
  */

public:

    /// Instruct hardware unit to identify itself (by flashing its front panel LEDs).
    /** Sends the MGMSG_MOD_IDENTIFY command (0x0223).  
      * See page 46 of the APT Manual.
      * 
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns -666 if device not available in \ftdi_write_data 
      */
    int mod_identify( bool errmsg = true /**< [in] [optional] flag controlling if an error message is printed on failure*/);


    /// Enable or disable the specified drive channel 
    /** Sends the MGMSG_MOD_SET_CHANENABLESTATE command (0x0210)
      * See page 47 of the APT Manual
      * 
      * The KPZ101 sends a 10 byte reponse, after a delay, if this command changes the state.  This is undocumented in the manual.  
      * To account for this, after sending the command, this sleeps for \ref m_postChanEnableSleep milliseconds (default 500) and then flushes
      * the line with a read.  Nothing is done with the result, and it being 0 is not an error.
      * 
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns <-200 on error from \ftdi_read_data (see also \libusb_bulk_transfer)
      * \returns -666 if device not available in either \ftdi_write_data or \ftdi_read_data
      * \returns -700 if sleep throws an exception
      * \returns -1000 if \p ces is EnableState::invalid
      * 
      */
    int mod_set_chanenablestate( const uint8_t & chnum,   ///< [in] the channel number
                                 const EnableState & ces, ///< [in] the desired channel enable state, EnableState::enabled or EnableState::disabled. 
                                 bool errmsg = true       ///< [in] [optional] flag controlling if an error message is printed on failure
                               );
    
    /// Request teh enable state of the specified drive channel
    /** Sends the MGMSG_MOD_REQ_CHANENABLESTATE command (0x0211)
      * See page 47 of the APT Manual
      * 
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns <-200 on error from \ftdi_read_data (see also \libusb_bulk_transfer)
      * \returns -300 if not enough data read
      * \returns -666 if device not available in either \ftdi_write_data or \ftdi_read_data 
      * \returns -1000 if \p ces is EnableState::invalid
      */
    int mod_req_chanenablestate( EnableState & ces,     ///< [out] the channel enable state. 0x01 enabled, 0x02 disabled
                                 const uint8_t & chnum, ///< [in] the channel number
                                 bool errmsg = true     ///< [in] [optional] flag controlling if an error message is printed on failure
                               );

    /// Stop automatic status updates from the controller 
    /** Sends the MGMSG_HW_STOP_UPDATEMSGS command (0x0012)
      * See page 51 of the APT Manual. 
      * 
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns -666 if device not available in either \ftdi_write_data 
      */
    int hw_stop_updatemsgs( bool errmsg = true /**< [in] [optional] flag controlling if an error message is printed on failure*/);

    /// Get hardware information from the device
    /** Sends the MGMSG_HW_REQ_INFO command (0x0005) and parses the result into a \ref HWInfo structure.
      * See page 52 of the APT manual.
      * 
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns <-200 on error from \ftdi_read_data (see also \libusb_bulk_transfer)
      * \returns -300 if not enough data read
      * \returns -666 if device not available in either \ftdi_write_data or \ftdi_read_data 
      */
    int hw_req_info( HWInfo & hwi,      ///< [in] the \ref HWInfo structure to populate
                     bool errmsg = true ///< [in] [optional] flag controlling if an error message is printed on failure
                   );

    /// Set the output voltage applied to the piezo actuator
    /** Sends the MGMSG_PZ_SET_OUTPUTVOLTS command (0x0643).
      * See page 198 of the manual.
      *   
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns -666 if device not available in either \ftdi_write_data 
      * \returns -980 if abs(ov) > 1
      */
    int pz_set_outputvolts( const float & ov, ///< [in] the output volts to set, converted from a percentage of max value
                            bool errmsg = true  ///< [in] [optional] flag controlling if an error message is printed on failure
                          );
    
    /// Get the output voltage applied to the piezo actuator
    /** Sends the MGMSG_PZ_REQ_OUTPUTVOLTS command (0x0644).
      * See page 198 of the manual.
      *   
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns <-200 on error from \ftdi_read_data (see also \libusb_bulk_transfer)
      * \returns -300 if not enough data read
      * \returns -666 if device not available in either \ftdi_write_data or \ftdi_read_data 
      */
    int pz_req_outputvolts( float & ov,      ///< [out] the output volts currently set, converted to a percentage of maximum value
                            bool errmsg = true ///< [in] [optional] flag controlling if an error message is printed on failure
                          );

    /// Get Piezo status
    /** Sends the MGMSG_PZ_REQ_PZSTATUSUPDATE command (0x0660) and parses the result into a \ref PZstatus structure.
      * See page 205 of the APT manual.
      *  
      * \note this currently only works for the TPZ001 KPZ101
      * 
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns <-200 on error from \ftdi_read_data (see also \libusb_bulk_transfer)
      * \returns -300 if not enough data read
      * \returns -666 if device not available in either \ftdi_write_data or \ftdi_read_data 
      */
    int pz_req_pzstatusupdate( PZStatus & pzs,    ///< [out] the \ref PZStatus structure to populate
                               bool errmsg = true ///< [in] [optional] flag controlling if an error message is printed on failure
                             );

    /// Set the intensity of the LED display on the front of the TPZ unit
    /** Sends the MGMSG_PZ_SET_TPZ_DISPSETTINGS command (0x07D1)
      * See page 223 of the APT manual.
      * 
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns -666 if device not available in either \ftdi_write_data 
      */
    int pz_set_tpz_dispsettings( const uint16_t & dispint, ///< [in] The value to set.  Manual says 0-255.  Devices tested have a more limited range of XX to XX.
                                 bool errmsg = true ///< [in] [optional] flag controlling if an error message is printed on failure
                               );

    /// Get the intensity of the LED display on the front of the TPZ unit
    /** Sends the MGMSG_PZ_REQ_TPZ_DISPSETTINGS command (0x07D2)
      * See page 223 of the APT manual.
      *
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns <-200 on error from \ftdi_read_data (see also \libusb_bulk_transfer)
      * \returns -300 if not enough data read
      * \returns -666 if device not available in either \ftdi_write_data or \ftdi_read_data 
      */  
    int pz_req_tpz_dispsettings( uint16_t & dispint, ///< [out] the intensity value returned by the device.
                                 bool errmsg = true ///< [in] [optional] flag controlling if an error message is printed on failure
                               );

    /// Set voltage limit and hub analog input
    /** Sends the MGMSG_PZ_SET_TPZ_IOSETTINGS command (0x07D4) 
      * See page 224 of the manual.
      * 
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns -666 if device not available in either \ftdi_write_data 
      */
    int pz_set_tpz_iosettings( const TPZIOSettings & tios, ///< [in] the \ref TPZIOSettings to set
                                bool errmsg = true         ///< [in] [optional] flag controlling if an error message is printed on failure
                             );


    /// Set voltage limit and hub analog input
    /** Sends the MGMSG_PZ_REQ_TPZ_IOSETTINGS command (0x07D5) 
      * See page 224 of the manual.
      * 
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns <-200 on error from \ftdi_read_data (see also \libusb_bulk_transfer)
      * \returns -300 if not enough data read
      * \returns -666 if device not available in either \ftdi_write_data or \ftdi_read_data 
      */ 
    int pz_req_tpz_iosettings( TPZIOSettings & tios, ///< [out] the \ref TPZIOSettings to populate
                               bool errmsg = true    ///< [in] [optional] flag controlling if an error message is printed on failure
                             );

    /// Set top panel wheel and display parameters
    /** Sends the MGMSG_KPZ_SET_KCUBEMMIPARAMS message (0x07F0).
      * See page 235 of the APT manual.
      * 
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns -666 if device not available in either \ftdi_write_data  
      */
    int kpz_set_kcubemmiparams( const KMMIParams & kmp, ///< [in] the \ref KMMIParams to set
                                bool errmsg = true      ///< [in] [optional] flag controlling if an error message is printed on failure
                              );

    /// Get top panel wheel and display parameters
    /** Sends the MGMSG_KPZ_REQ_KCUBEMMIPARAMS message (0x07F1) and parses the results.
      * See page 235 of the APT manual.
      * 
      * \returns 0 on succcess
      * \returns <0 on error from connect
      * \returns <-100 on error from \ftdi_write_data (see also \libusb_bulk_transfer)
      * \returns <-200 on error from \ftdi_read_data (see also \libusb_bulk_transfer)
      * \returns -300 if not enough data read
      * \returns -666 if device not available in either \ftdi_write_data or \ftdi_read_data 
      */
    int kpz_req_kcubemmiparams( KMMIParams & kmp,  ///< [out] the \ref KMMIParams structure to populate
                                bool errmsg = true ///< [in] [optional] flag controlling if an error message is printed on failure
                              );


///@}

/** \name Error Handling
  * @{ 
  */

public:

    /// Print a message to std::cerr describing an error from an \libftdi1 function
    /** Intended to be overriden in a derived class to provide custom error messaging.
      */
    virtual void ftdiErrmsg( const std::string & src,  ///< [in] The source of the error (the tmcController function)
                             const std::string & msg,  ///< [in] The message describing the error
                             int rv,                   ///< [in] The return value of the \libftdi1 function
                             const std::string & file, ///< [in] The file name of this file
                             int line                  ///< [in] The line number at which the error was recorded
                           );

    /// Print a message to std::cerr describing an error 
    /** Intended to be overriden in a derived class to provide custom error messaging.
      */
    virtual void otherErrmsg( const std::string & src,  ///< [in] The source of the error (the tmcController function)
                              const std::string & msg,  ///< [in] The message describing the error
                              const std::string & file, ///< [in] The file name of this file
                              int line                  ///< [in] The line number at which the error was recorded
                            );

 ///@}                   


};

inline
tmcController::tmcController()
{
    m_ftdi = ftdi_new();

    ///\todo throw on failure
}

inline
tmcController::~tmcController()
{
    close();
    ftdi_free(m_ftdi);
}

inline
const ftdi_context * tmcController::ftdi() const
{
    return m_ftdi;
}

inline
void tmcController::vendor( uint16_t v )
{
    m_vendor = v;
}

inline   
void tmcController::product( uint16_t p )
{
    m_product = p;
}

inline    
void tmcController::serial (const std::string & s )
{
    m_serial = s;
}

inline
void tmcController::baud( uint32_t b /* [in] the new baud rate*/ )
{
    m_baud = b;
}

inline
uint32_t tmcController::baud()
{
    return m_baud;
}

inline
void tmcController::preFlushSleep( uint32_t s /* [in] the pre-flush sleep time in ms */ )
{
    m_preFlushSleep = s;
}

inline
uint32_t tmcController::preFlushSleep()
{
    return m_preFlushSleep;
}

inline
void tmcController::postFlushSleep( uint32_t s /* [in] the post-flush sleep time in ms */ )
{
    m_postFlushSleep = s;
}

inline
uint32_t tmcController::postFlushSleep()
{
    return m_postFlushSleep;
}

inline
int tmcController::open(bool errmsg /*default=true*/)
{
    int rv;

    if((rv = ftdi_usb_open_desc_index(m_ftdi, m_vendor, m_product, NULL, m_serial.c_str(), 0)) < 0)
    {
        if(errmsg)
        {
            ftdiErrmsg("tmcController::open", "unable to open ftdi device", rv, __FILE__, __LINE__-4);
        }

        m_opened = false;

        return rv;
    }

    m_opened = true;

    return 0;
}

inline
int tmcController::open( const std::string & s,
                         bool errmsg /*default=true*/
                       )
{
    serial(s);
    return open(errmsg);
}

inline
int tmcController::open( uint16_t v,
                         uint16_t p,
                         const std::string & s,
                         bool errmsg /*default=true*/
                       )
{
    vendor(v);
    product(p);
    serial(s);
    return open(errmsg);
}

inline
int tmcController::close( bool errmsg /*default=true*/)
{
    if(!m_opened) 
    {
        return 0;
    }

    int rv;
    if((rv = ftdi_usb_close(m_ftdi)) < 0)
    {
        if(errmsg)
        {
            ftdiErrmsg("tmcController::close", "unable to close device", rv, __FILE__, __LINE__-4);
        }
        return rv;
    }

    m_opened = false;
    m_connected = false;

    return 0;
}

inline
int tmcController::connect(bool errmsg /*default=true*/)
{
    if(!m_opened)
    {
        int rv = open();
        if( (rv < 0) || !m_opened)
        {
            if(errmsg)
            {
                ftdiErrmsg("tmcController::connect", "open failed", rv, __FILE__, __LINE__-5);
            }
            return rv;
        }
    }

    int rv;
    if((rv = ftdi_read_chipid(m_ftdi, &m_chipid)) < 0)
    {
        if(errmsg)
        {
            ftdiErrmsg("tmcController::connect", "unable to read chip id", rv, __FILE__, __LINE__-4);
        }
        return -20 + rv;
    }

    if((rv = ftdi_set_baudrate(m_ftdi, m_baud)) < 0)
    {
        if(errmsg)
        {
            ftdiErrmsg("tmcController::connect", "unable to set baud rate", rv, __FILE__, __LINE__-4);
        }
        return -30 + rv;
    }

    if((rv = ftdi_set_line_property(m_ftdi, BITS_8, STOP_BIT_1, NONE)) < 0)
    {
        if(errmsg)
        {
            ftdiErrmsg("tmcController::connect", "unable to set line property", rv, __FILE__, __LINE__-4);
        }
        return -40 + rv;
    }

    try
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_preFlushSleep));
    }
    catch(const std::exception& e)
    {
        if(errmsg)
        {
            otherErrmsg("tmcController::connect", std::string("exception from sleep_for: ") + e.what(), __FILE__, __LINE__-6);
        }
        return -49;
    }

    if((rv = ftdi_tcioflush(m_ftdi)) < 0)
    {
        if(errmsg)
        {
            ftdiErrmsg("tmcController::connect", "unable to tcio flush", rv, __FILE__, __LINE__-4);
        }
        return -50 + rv;
    }

    try
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_postFlushSleep));
    }
    catch(const std::exception& e)
    {
        if(errmsg)
        {
            otherErrmsg("tmcController::connect", std::string("exception from sleep_for: ") + e.what(), __FILE__, __LINE__-6);
        }
        return -59;
    }

    if((rv = ftdi_usb_reset(m_ftdi)) < 0)
    {
        if(errmsg)
        {
            ftdiErrmsg("tmcController::connect", "unable to reset device", rv, __FILE__, __LINE__-4);
        }
        return -60 + rv;
    }

    if((rv = ftdi_setflowctrl(m_ftdi, SIO_RTS_CTS_HS)) < 0)
    {
        if(errmsg)
        {
            ftdiErrmsg("tmcController::connect", "unable to set flow control", rv, __FILE__, __LINE__-4);
        }
        return -70 + rv;
    }


    if((rv = ftdi_setrts(m_ftdi, 1)) < 0 )
    {
        if(errmsg)
        {
            ftdiErrmsg("tmcController::connect", "unable to set RTS", rv, __FILE__, __LINE__-4);
        }
        return -80 + rv;
    }

    m_connected = true;

    return 0;
}

inline
int tmcController::connect( const std::string & s,
                            bool errmsg /*default=true*/
                          )
{
    serial(s);
    return connect(errmsg);
}

inline
int tmcController::connect( uint16_t v,
                            uint16_t p,
                            const std::string & s,
                            bool errmsg /*default=true*/
                          )
{
    vendor(v);
    product(p);
    serial(s);
    return connect(errmsg);
}

inline
bool tmcController::opened()
{
    return m_opened;
}

inline
bool tmcController::connected()
{
    return m_connected;
}

inline
unsigned int tmcController::chipid()
{
    return m_chipid;
}

inline
int tmcController::totrd()
{
    return m_totrd;
}

inline
void tmcController::postChanEnableSleep( uint32_t s )
{
    m_postChanEnableSleep = s;
}

inline
uint32_t tmcController::postChanEnableSleep()
{
    return m_postChanEnableSleep;
}

#define TMCC_CHECK_CONNECTED(fxn)                                                                \
    if(!m_connected)                                                                             \
    {                                                                                            \
        int rv = connect(errmsg);                                                                \
        if( (rv < 0) || !m_connected)                                                            \
        {                                                                                        \
            if(errmsg)                                                                           \
            {                                                                                    \
                otherErrmsg("tmcController::" fxn, "connect failed", __FILE__, __LINE__);        \
            }                                                                                    \
            return rv;                                                                           \
        }                                                                                        \
    }             

template<class streamT>
void tmcController::HWInfo::dump(streamT & ios)
{
    ios << "Connected to: \n";
    ios << "      Model: " << modelNumber << "\n";
    ios << "       Type: " << type << "\n";
    ios << "    Ser Num: " << serialNumber << "\n";
    ios << "     HW Ver: " << hwVer << "\n";
    ios << "     HW Mod: " << hwMod << "\n";
    ios << "   Num. Ch.: " << nChannels << "\n";
    ios << "   F/W Ver.: " << fwMaj << "." << fwMin << "." << fwInt << "\n";
}

double tmcController::PZStatus::age()
{
  timespec tsp;
  clock_gettime(CLOCK_REALTIME, &tsp);
  return (tsp.tv_sec + ((1.0*tsp.tv_nsec)/1e9)) - (statusTime.tv_sec + ((1.0*statusTime.tv_nsec)/1e9));
}

template<class streamT>
void tmcController::PZStatus::dump(streamT & ios)
{
    ios << "PZ Status: \n";
    ios << "    Voltage: " << voltage << "\n";
    ios << "   Position: " << voltage << "\n";
    ios << "  Connected: " << connected << "\n";
    ios << "     Zeroed: " << zeroed << "\n";
    ios << "    Zeroing: " << zeroing << "\n";
    ios << "   SG Conn.: " << sgConnected << "\n";
    ios << "  P.C. Mode: " << pcMode << "\n";
    ios << "        Age: " << age() << " sec\n";
}

template<class streamT>
void tmcController::TPZIOSettings::dump(streamT & ios)
{
    ios << "TPZ IO Settings: \n";
    int vl;
    if(VoltageLimit == VoltLimit::V75) vl = 75;
    if(VoltageLimit == VoltLimit::V100) vl = 100;
    if(VoltageLimit == VoltLimit::V150) vl = 150;

    ios << "     VoltageLimit: " << vl << "\n";
    ios << "   HubAnalogInput: " << HubAnalogInput << "\n";
}

template<class streamT>
void tmcController::KMMIParams::dump(streamT & ios)
{
    ios << "K-Cube MMI Params: \n";
    ios << "             JSMode: " << JSMode << "\n";
    ios << "      JSVoltGearBox: " << JSVoltGearBox << "\n";
    ios << "         JSVoltStep: " << JSVoltStep << "\n";
    ios << "           DirSense: " << DirSense << "\n";
    ios << "        PresetVolt1: " << PresetVolt1 << "\n";
    ios << "        PresetVolt2: " << PresetVolt2 << "\n";
    ios << "     DispBrightness: " << DispBrightness << "\n";
    ios << "        DispTimeout: " << DispTimeout << "\n";
    ios << "       DispDimLevel: " << DispDimLevel << "\n";
}

#define TMCC_SNDBUF_HEAD(b0,b1,b2,b3,b4,b5) \
    m_sndbuf[0] = b0;                       \
    m_sndbuf[1] = b1;                       \
    m_sndbuf[2] = b2;                       \
    m_sndbuf[3] = b3;                       \
    m_sndbuf[4] = b4;                       \
    m_sndbuf[5] = b5;

#define TMCC_WRITE_REQUEST(fxn)                                                                 \
    int rv;                                                                                     \
    if((rv = ftdi_write_data(m_ftdi, m_sndbuf, 6))  < 0)                                        \
    {                                                                                           \
        if(errmsg)                                                                              \
        {                                                                                       \
            ftdiErrmsg("tmcController::" fxn, "unable to write data", rv, __FILE__, __LINE__);  \
        }                                                                                       \
        if(rv == -666) return rv;                                                               \
        else return -100 + rv;                                                                  \
    } 

#define TMCC_WRITE_COMMAND(fxn, esz)                                                            \
    int rv;                                                                                     \
    rv = ftdi_tcioflush(m_ftdi); \
    std::this_thread::sleep_for(std::chrono::milliseconds(m_postFlushSleep));\
    if((rv = ftdi_write_data(m_ftdi, m_sndbuf, esz))  < 0)                                      \
    {                                                                                           \
        if(errmsg)                                                                              \
        {                                                                                       \
            ftdiErrmsg("tmcController::" fxn, "unable to write data", rv, __FILE__, __LINE__);  \
        }                                                                                       \
        if(rv == -666) return rv;                                                               \
        else return -100 + rv;                                                                  \
    }

#define TMCC_READ_RESPONSE(fxn, esz)                                                                           \
    {                                                                                                          \
        /** \todo this needs a timeout, and/or look for ftdi function to manage this*/                         \
        m_totrd = 0;                                                                                           \
        do                                                                                                     \
        {                                                                                                      \
            int rd = ftdi_read_data(m_ftdi, m_rdbuf + m_totrd, sizeof(m_rdbuf)-m_totrd);                       \
            if(rd < 0)                                                                                         \
            {                                                                                                  \
                if(errmsg)                                                                                     \
                {                                                                                              \
                    ftdiErrmsg("tmcController::" fxn, "unable to read data", rd, __FILE__, __LINE__);          \
                }                                                                                              \
                if(rd == -666) return rd;                                                                      \
                return -200+rd;                                                                                \
            }                                                                                                  \
            m_totrd += rd;                                                                                     \
        }                                                                                                      \
        while(m_totrd < esz);                                                                                  \
                                                                                                               \
        if(m_totrd != esz && esz > 0)                                                                          \
        {                                                                                                      \
            if(errmsg)                                                                                         \
            {                                                                                                  \
                otherErrmsg("tmcController::" fxn, "did not read correct amount of data, got " +               \
                                                                  std::to_string(m_totrd), __FILE__, __LINE__);\
            }                                                                                                  \
            return -300;                                                                                       \
        }                                                                                                      \
    } /*TMCC_READ_RESPONSE(fxn, esz)*/

inline
int tmcController::mod_identify(bool errmsg /*default=true*/)
{
    TMCC_CHECK_CONNECTED("mod_identify")

    TMCC_SNDBUF_HEAD(0x23,0x02,0x00,0x00,0x50,0x01)
    
    TMCC_WRITE_REQUEST("mod_identify")

    return 0;
}

inline
int tmcController::mod_set_chanenablestate( const uint8_t & chnum,
                                            const EnableState & ces,
                                            bool errmsg
                                          )
{
    if(ces == EnableState::invalid)
    {
        if(errmsg)
        {
            otherErrmsg("tmcController::mod_set_chanenablestate", "EnableState is invalid", __FILE__, __LINE__-4);
        }
        return -1000;
    }

    TMCC_CHECK_CONNECTED("mod_set_chanenablestate")

    TMCC_SNDBUF_HEAD(0x10,0x02,chnum,static_cast<uint8_t>(ces),0x50,0x01)
    
    TMCC_WRITE_REQUEST("mod_set_chanenablestate")

    //Sleep to let the device send the undocumented 10 character response on a state change
    try
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_postChanEnableSleep));
    }
    catch(const std::exception& e)
    {
        if(errmsg)
        {
            otherErrmsg("tmcController::mod_set_chanenablestate", std::string("exception from sleep_for: ") + e.what(), __FILE__, __LINE__-6);
        }
        return -700;
    }

    //Now do a 0 read to flush the line
    TMCC_READ_RESPONSE("pz_set_outputvolts", 0) 

    return 0;
}

inline   
int tmcController::mod_req_chanenablestate( EnableState & ces,
                                            const uint8_t & chnum,     
                                            bool errmsg
                                          )
{
    TMCC_CHECK_CONNECTED("mod_req_chanenablestate")

    TMCC_SNDBUF_HEAD(0x11,0x02,chnum,0x00,0x50,0x01)

    TMCC_WRITE_REQUEST("mod_req_chanenablestate")

    TMCC_READ_RESPONSE("mod_req_chanenablestate", 6);

    if(m_rdbuf[3] == 0x01)
    {
        ces = EnableState::enabled;
    }
    else if(m_rdbuf[3] == 0x02)
    {
        ces = EnableState::disabled;
    }
    else
    {
        ces = EnableState::invalid;
        if(errmsg)
        {
            otherErrmsg("tmcController::mod_req_chanenablestate", "EnableState is invalid", __FILE__, __LINE__-5);
        }
        return -1000;
    }

    return 0;
}

inline
int tmcController::hw_stop_updatemsgs( bool errmsg )
{
    TMCC_CHECK_CONNECTED("hw_stop_updatemsgs")

    TMCC_SNDBUF_HEAD(0x12,0x00,0x00,0x00,0x50,0x01)
    
    TMCC_WRITE_REQUEST("hw_stop_updatemsgs")

    return 0;
}

inline
int tmcController::hw_req_info( HWInfo & hwi,
                                bool errmsg /*default=true*/
                              )
{
    TMCC_CHECK_CONNECTED("hw_req_info")

    TMCC_SNDBUF_HEAD(0x05,0x00,0x00,0x00,0x50,0x01)

    TMCC_WRITE_REQUEST("hw_req_info")

    TMCC_READ_RESPONSE("hw_req_info", 90);

    hwi.serialNumber = *((uint32_t*)&m_rdbuf[6]);
    char modnum[9];
    memcpy(modnum, &m_rdbuf[10], 8);
    modnum[8] = '\0';
    hwi.modelNumber = modnum;

    hwi.type = *((uint16_t*)&m_rdbuf[18]);
    hwi.fwMin  = m_rdbuf[20];
    hwi.fwInt = m_rdbuf[21];
    hwi.fwMaj = m_rdbuf[22];
    hwi.hwVer = *((uint16_t*)&m_rdbuf[84]);
    hwi.hwMod = *((uint16_t*)&m_rdbuf[86]);
    hwi.nChannels = *((uint16_t*)&m_rdbuf[88]);

    return 0;

}

inline
int tmcController::pz_set_outputvolts( const float & ov, 
                                       bool errmsg
                                     )
{
    int16_t iov = 0x00;

    if(fabs(ov) > 1.0)
    {
        if(errmsg)
        {
            otherErrmsg("tmcController::pz_set_outputvolts", "output volts > 1 (>100% of max): " + std::to_string(ov), __FILE__, __LINE__-2);
        }
        return -980;
    }

    if(ov > 0)
    {
        iov = ov*32767;
    }
    else
    {
        iov = ov*32768;
    }

    TMCC_CHECK_CONNECTED("pz_set_outputvolts")

    TMCC_SNDBUF_HEAD(0x43,0x06,0x04,0x00, 0x50 | 0x80,0x01)

    m_sndbuf[6] = 0x01;
    m_sndbuf[7] = 0x00;
    *((int16_t*) &m_sndbuf[8]) = iov;

    TMCC_WRITE_COMMAND("pz_set_outputvolts",10)

    return 0;
}

inline  
int tmcController::pz_req_outputvolts( float & ov, 
                                       bool errmsg
                                     )
{
    

    TMCC_CHECK_CONNECTED("pz_req_outputvolts")

    TMCC_SNDBUF_HEAD(0x44,0x06,0x01,0x00,0x50,0x01)

    TMCC_WRITE_REQUEST("pz_req_outputvolts")

    TMCC_READ_RESPONSE("pz_req_outputvolts", 10)

    int16_t iov = *((int16_t *) &m_rdbuf[8]);

    if(iov > 0)
    {
        ov = iov/32767.0;
    }
    else
    {
        ov = iov/32768.0;
    }

    return 0;

}

inline
int tmcController::pz_req_pzstatusupdate( PZStatus & pzs,
                                          bool errmsg /*default=true*/
                                        )
{
    TMCC_CHECK_CONNECTED("pz_req_pzstatusupdate")

    TMCC_SNDBUF_HEAD(0x60,0x06,0x01,0x00,0x50,0x01)

    TMCC_WRITE_REQUEST("pz_req_pzstatusupdate")

    TMCC_READ_RESPONSE("pz_req_pzstatusupdate", 16)

    clock_gettime(CLOCK_REALTIME, &pzs.statusTime);

    pzs.voltage = *((int16_t *) &m_rdbuf[8]);
    pzs.position = *((int16_t *) &m_rdbuf[10]);

    uint32_t bits = *((uint32_t *) &m_rdbuf[12]);

    pzs.connected = bits & 0x00000001;
    pzs.zeroed = bits & 0x00000010;
    pzs.zeroing = bits & 0x00000020;
    pzs.sgConnected = bits & 0x00000100;
    pzs.pcMode = bits & 0x00000400;

    return 0;

}

inline
int tmcController::pz_set_tpz_dispsettings( const uint16_t & dispint,
                                            bool errmsg
                                          )
{
    TMCC_CHECK_CONNECTED("pz_set_tpz_dispsettings")

    TMCC_SNDBUF_HEAD(0xD1,0x07,0x02,0x00, 0x50 | 0x80,0x01)

    *((uint16_t*) &m_sndbuf[6]) = dispint;

    TMCC_WRITE_COMMAND("pz_set_tpz_dispsettings",8)

    return 0;
}

inline
int tmcController::pz_req_tpz_dispsettings( uint16_t & dispint,
                                            bool errmsg
                                          )
{
    TMCC_CHECK_CONNECTED("pz_req_tpz_dispsettings")

    TMCC_SNDBUF_HEAD(0xD2,0x07,0x01,0x00,0x50,0x01)

    TMCC_WRITE_REQUEST("pz_req_tpz_dispsettings")

    TMCC_READ_RESPONSE("pz_req_tpz_dispsettings", 8)

    dispint = *((uint16_t *) &m_rdbuf[6]);

    return 0;
}

int tmcController::pz_set_tpz_iosettings( const TPZIOSettings & tios,
                                          bool errmsg       
                                        )
{
    if(tios.VoltageLimit == VoltLimit::INVALID)
    {
        return -1000;
    }

    TMCC_CHECK_CONNECTED("pz_set_tpz_iosettings")

    TMCC_SNDBUF_HEAD(0xD4,0x07,0x0A,0x00, 0x50 | 0x80,0x01)

    m_sndbuf[6] = 0x01;
    m_sndbuf[7] = 0x00;
    *((uint16_t*) &m_sndbuf[8]) = static_cast<uint16_t>(tios.VoltageLimit);
    *((uint16_t*) &m_sndbuf[10]) = tios.HubAnalogInput;
    m_sndbuf[12] = 0x00;
    m_sndbuf[13] = 0x00;
    m_sndbuf[14] = 0x00;
    m_sndbuf[15] = 0x00;

    TMCC_WRITE_COMMAND("pz_set_tpz_iosettings",16)

    return 0;
}

int tmcController::pz_req_tpz_iosettings( TPZIOSettings & tios, 
                                          bool errmsg   
                                        )
{
    TMCC_CHECK_CONNECTED("pz_req_tpz_iosettings")

    TMCC_SNDBUF_HEAD(0xD5,0x07,0x01,0x00,0x50,0x01)

    TMCC_WRITE_REQUEST("pz_req_tpz_iosettings")

    TMCC_READ_RESPONSE("pz_req_tpz_iosettings", 16)

    uint16_t vl = *((uint16_t*) &m_rdbuf[8]);
    if(vl == 0x01)
    {
        tios.VoltageLimit = VoltLimit::V75;
    }
    else if (vl == 0x02)
    {
        tios.VoltageLimit = VoltLimit::V100;
    }
    else if(vl == 0x03)
    {
        tios.VoltageLimit = VoltLimit::V150;
    }
    else
    {
        tios.VoltageLimit = VoltLimit::INVALID;
    }

    tios.HubAnalogInput = *((uint16_t*) &m_rdbuf[10]);

    return 0;
    
}

int tmcController::kpz_set_kcubemmiparams( const KMMIParams & kmp,
                                           bool errmsg /*default = true*/ 
                                         )
{
    TMCC_CHECK_CONNECTED("kpz_set_kcubemmiparams")

    TMCC_SNDBUF_HEAD(0xF0,0x07,0x22,0x00, 0x50 | 0x80,0x01)

    m_sndbuf[6] = 0x01;
    m_sndbuf[7] = 0x00;
    *((uint16_t*) &m_sndbuf[8]) = kmp.JSMode;
    *((uint16_t*) &m_sndbuf[10]) = kmp.JSVoltGearBox;
    *((int32_t*) &m_sndbuf[12]) = kmp.JSVoltStep;
    *((uint16_t*) &m_sndbuf[16]) = kmp.DirSense;
    *((int32_t*) &m_sndbuf[18]) = kmp.PresetVolt1;
    *((int32_t*) &m_sndbuf[22]) = kmp.PresetVolt2;
    *((uint16_t*) &m_sndbuf[26]) = kmp.DispBrightness;
    *((uint16_t*) &m_sndbuf[28]) = kmp.DispTimeout;
    *((uint16_t*) &m_sndbuf[30]) = kmp.DispDimLevel;
    m_sndbuf[34] = 0;
    m_sndbuf[35] = 0;
    m_sndbuf[36] = 0;
    m_sndbuf[37] = 0;
    m_sndbuf[38] = 0;
    m_sndbuf[39] = 0;

    TMCC_WRITE_COMMAND("kpz_set_kcubemmiparams",40)

    return 0;

}

int tmcController::kpz_req_kcubemmiparams( KMMIParams & kmp,
                                           bool errmsg /*default = true*/ 
                                         )
{
    TMCC_CHECK_CONNECTED("kpz_req_kcubemmiparams")

    TMCC_SNDBUF_HEAD(0xF1,0x07,0x01,0x00,0x50,0x01)

    TMCC_WRITE_REQUEST("kpz_req_kcubemmiparams")

    TMCC_READ_RESPONSE("kpz_req_kcubemmiparams", 40)

    kmp.JSMode = *((uint16_t*) &m_rdbuf[8]);
    kmp.JSVoltGearBox = *((uint16_t*) &m_rdbuf[10]);
    kmp.JSVoltStep = *((int32_t*) &m_rdbuf[12]);
    kmp.DirSense = *((uint16_t*) &m_rdbuf[16]);
    kmp.PresetVolt1 = *((int32_t*) &m_rdbuf[18]);
    kmp.PresetVolt2 = *((int32_t*) &m_rdbuf[22]);
    kmp.DispBrightness = *((uint16_t*) &m_rdbuf[26]);
    kmp.DispTimeout = *((uint16_t*) &m_rdbuf[28]);
    kmp.DispDimLevel = *((uint16_t*) &m_rdbuf[30]);

    return 0;

}


inline
void tmcController::ftdiErrmsg( const std::string & src,
                                const std::string & msg,
                                int rv,
                                const std::string & file,
                                int line
                              )
{
    std::cerr << src << ": " << msg << " [" << rv << ":" << ftdi_get_error_string(m_ftdi) << "]\n";
    std::cerr << "in " << file << " at line " << line << "\n";
}

inline
void tmcController::otherErrmsg( const std::string & src,
                                 const std::string & msg,
                                 const std::string & file,
                                 int line
                               )
{
    std::cerr << src << ": " << msg << "\n";
    std::cerr << "in " << file << " at line " << line << "\n";
}

#endif //tmcController_hpp
