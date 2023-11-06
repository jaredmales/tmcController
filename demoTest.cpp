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

#include <mx/sys/timeUtils.hpp>

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
    
    tmcc.hw_stop_updatemsgs();

    tmcController::KMMIParams par;
    tmcc.kpz_req_kcubemmiparams(par);

    par.dump(std::cout);
    
    par.DispBrightness = 0;

    tmcc.kpz_set_kcubemmiparams(par);

    tmcc.kpz_req_kcubemmiparams(par);

    par.dump(std::cout);


    tmcController::TPZIOSettings tios;
    tmcc.pz_req_tpz_iosettings(tios);
    tios.dump(std::cout);

    tios.VoltageLimit = tmcController::VoltLimit::V150;

    tmcc.pz_set_tpz_iosettings(tios);

    tmcc.pz_req_tpz_iosettings(tios);
    tios.dump(std::cout);


    tmcc.mod_set_chanenablestate(0x01, tmcController::EnableState::disabled);

    mx::sys::milliSleep(500);

    tmcController::EnableState ces;
    tmcc.mod_req_chanenablestate(ces, 0x01);

    std::cout << "enabled: " << (int) static_cast<uint8_t>(ces) << "\n";



    float ov;
    tmcc.pz_req_outputvolts(ov);

    std::cout << "Output Volts: " << ov*150.0 << "\n";

    ov = 75.0/150;

    std::cerr << ov*150. << "\n";
    tmcc.pz_set_outputvolts(ov);

    mx::sys::milliSleep(500);

    ov = 0;
    tmcc.pz_req_outputvolts(ov);
    std::cout << "Output Volts: " << ov*150 << "\n";

    return EXIT_SUCCESS;
}

