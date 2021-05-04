/********************************************************************/
/*              Read and Write to an Instrument Example             */
/*                                                                  */
/* This code demonstrates synchronous read and write commands to a  */
/* GPIB, serial or message-based VXI instrument using VISA.         */
/*                                                                  */
/* The general flow of the code is                                  */
/*      Open Resource Manager                                       */
/*      Open VISA Session to an Instrument                          */
/*      Write the Identification Query Using viWrite                */
/*      Try to Read a Response With viRead                          */
/*      Close the VISA Session                                      */
/********************************************************************/

#include <iostream>
#include <string>

#include <visa.h>

#include <udaq/helpers/visa.h>



const std::string IDN = "*IDN?";

int main(void)
{
    auto viInstr = udaq::visa::visa_comm_driver();

    viInstr.connect("TCPIP0::127.0.0.1::5020::SOCKET");
    viInstr.write(IDN);
    std::cout << viInstr.read() << std::endl;
    viInstr.disconnect();

    return 0;
}
