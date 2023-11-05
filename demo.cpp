/** \file demo.cpp
  *  \brief A simple demonstration program
  * 
  * This program demonstrates using tmcController to talk to a k-cube
  * 
  * Compile with 
  * \verbatim
    g++ -o demo demo.cpp -I/usr/include/libftdi1/ -lftdi1
    \endverbatim
  * (change the include path as needed.  you may also need to add the -L library path)
  * 
  * Run with
  * \verbatim
    ./demo 29252712
   \endverbatim
  * where you change the serial number to match the USB device serial number of your k-cube. 
  * You can find this with `dmesg`.
  * 
  */

#include "tmcController.hpp"

/** The demo main program.
  */ 
int main( int argc,    ///< [in] the number of command line arguments, must be 2
          char **argv  ///< [in] the command line arguments. argv[1] is the serial number of the device.
        )
{
    if(argc != 2)
    {
        std::cerr << "Please provide the USB device serial number as the only argument.\n";
        return EXIT_FAILURE;
    }

    tmcController tmcc;

    tmcc.connect( 0x0403, 0xfaf0, argv[1]);

    tmcController::HWInfo hwi;
    tmcc.hw_req_info(hwi);
    hwi.dump(std::cout);
    
    std::cout << "\n";

    tmcController::PZStatus pzs;
    tmcc.pz_req_pzstatusupdate(pzs);
    pzs.dump(std::cout);

    std::cerr << "\nIdentifying device, look for blinking display\n";
    tmcc.mod_identify();
    

    return EXIT_SUCCESS;
}

